#include "Overlook.h"


namespace Overlook {

TraineeBase::TraineeBase() {
	group = NULL;
	group_id = -1;
	iter = 0;
	epoch_actual = 0;
	epoch_total = 0;
	peak_value = 0.0;
	best_result = 0.0;
	training_time = 0.0;
}

void TraineeBase::Create(int width, int height) {
	dqn.Init(width, height, ACTIONCOUNT);
	dqn.Reset();
	
	ASSERT(!group->param_str.IsEmpty());
	dqn.LoadInitJSON(group->param_str);
}

void TraineeBase::Init() {
	ASSERT(group != NULL);
	ASSERT(group->agent_input_width != 0);
	ASSERT(group->group_input_width != 0);
	ASSERT(group->snaps.GetCount() != 0);
	
	thrd_equity.SetCount(group->snaps.GetCount(), 0);
	
	broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	broker.InitLightweight();
}

void TraineeBase::Action() {
	
	if (!epoch_actual) {
		broker.Clear();
		broker.SetSignal(0,0);
		begin_equity = broker.AccountEquity();
		ts.Reset();
	}
	else {
		// Just reset if fail is too much to take
		if (broker.AccountEquity() < 0.4 * begin_equity) {
			broker.Clear();
			broker.SetSignal(0,0);
		}
	}
	
	
	// Set broker signals based on DQN-agent action
	int time_pos;
	int snap_pos, snap_next_pos;
	if (group_id == -1) {
		snap_pos = epoch_actual;
		snap_next_pos = snap_pos + 1;
		time_pos = group->train_pos_all[snap_pos];
		if (snap_next_pos >= epoch_total)
			snap_next_pos = -1;
	} else {
		snap_pos = group->train_pos[group_id][epoch_actual];
		snap_next_pos = snap_pos + 1;
		time_pos = group->train_pos_all[snap_pos];
		int next_time_pos = time_pos + 1;
		// Skip the end and discontinuation
		int j = epoch_actual + 1;
		if (j < group->train_pos[group_id].GetCount()) {
			int time_pos_of_actual_next_snap = group->train_pos_all[group->train_pos[group_id][j]];
			if (time_pos_of_actual_next_snap != next_time_pos)
				snap_next_pos = -1;
		}
		else
			snap_next_pos = -1;
	}
	
	
	// Refresh values
	SetAskBid(broker, time_pos);
	broker.RefreshOrders();
	broker.CycleChanges();
	
	
	Snapshot& snap = group->snaps[snap_pos];
	Snapshot* next_snap = snap_next_pos != -1 ? &group->snaps[snap_next_pos] : NULL;
	Forward(snap, broker, next_snap);
	
	
	// Refresh odrers
	if (next_snap)
		broker.Cycle();
	else
		broker.CloseAll();
	double reward = broker.PopCloseSum();
	Backward(reward);
	double equity = broker.AccountEquity();
	double diff = equity - broker.GetInitialBalance();
	if (diff > peak_value) peak_value = diff;
	
	
	// Write some stats for plotter
	thrd_equity[epoch_actual] = equity;
	
	
	epoch_actual++;
	if (epoch_actual >= epoch_total) {
		seq_results.Add(equity);
		epoch_actual = 0;
		if (diff > best_result) best_result = diff;
		training_time += ts.Elapsed() / (1000.0 * 60.0 * 60.0);
		ts.Reset();
	}
}

void TraineeBase::Serialize(Stream& s) {
	s % dqn % seq_results % reward_average % loss_average % peak_value % best_result % training_time
	  % group_id % iter;
}

}
