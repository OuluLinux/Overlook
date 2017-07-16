#include "Overlook.h"


namespace Overlook {

TraineeBase::TraineeBase() {
	group = NULL;
	iter = 0;
	prev_reward = 0;
	prev_equity = 0;
	epoch_actual = 0;
	epoch_total = 0;
	peak_value = 0.0;
	best_result = 0.0;
	training_time = 0.0;
}

void TraineeBase::Create() {
	dqn.Init(group->input_width, group->input_height, ACTIONCOUNT);
	dqn.Reset();
	
	ASSERT(!group->param_str.IsEmpty());
	dqn.LoadInitJSON(group->param_str);
}

void TraineeBase::Init() {
	ASSERT(group != NULL);
	ASSERT(group->input_width != 0);
	ASSERT(group->snaps.GetCount() != 0);
	
	thrd_equity.SetCount(group->snaps.GetCount(), 0);
	
	broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	broker.InitLightweight();
}

void TraineeBase::Action() {
	
	if (!epoch_actual) {
		broker.Clear();
		broker.SetSignal(0,0);
		prev_reward = 0;
		prev_equity = broker.AccountEquity();
		begin_equity = prev_equity;
		ts.Reset();
	}
	else {
		// Just reset if fail is too much to take
		if (prev_equity < 0.4 * begin_equity) {
			broker.Clear();
			broker.SetSignal(0,0);
		}
	}
	
	
	// Set broker signals based on DQN-agent action
	Snapshot& snap = group->snaps[epoch_actual];
	Snapshot* next_snap = epoch_actual + 1 < epoch_total ? &group->snaps[epoch_actual + 1] : NULL;
	Forward(snap, broker, next_snap);
	
	
	// Refresh values
	SetAskBid(broker, group->train_pos[epoch_actual]);
	//broker.RefreshOrders();
	//broker.CycleChanges();
	
	
	// Refresh odrers
	if (epoch_actual < group->snaps.GetCount()-1)
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
	s % dqn % seq_results % peak_value % best_result % training_time % iter;
}

}
