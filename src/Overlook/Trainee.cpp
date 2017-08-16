#include "Overlook.h"

namespace Overlook {

TraineeBase::TraineeBase() {
	group = NULL;
	group_id = -1;
	tf_id = -1;
	tf = -1;
	iter = 0;
	epoch_actual = 0;
	epoch_total = 0;
	data_begin = 0;
	peak_value = 0.0;
	best_result = 0.0;
	training_time = 0.0;
	last_drawdown = 1.0;
	main_id = -1;
	at_main = false;
	save_epoch = true;
	end_of_epoch = false;
	is_training = true;
}

void TraineeBase::Init() {
	ASSERT(group != NULL);
	ASSERT(group->group_input_width != 0);
	ASSERT(group->snaps.GetCount() != 0);
	
	thrd_equity.SetCount(group->snaps.GetCount(), 0);
	
	broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	broker.InitLightweight();
}

void TraineeBase::Action() {
	thrd_equity.SetCount(group->snaps.GetCount(), 0);
	
	if (epoch_actual == 0) {
		broker.Clear();
		broker.SetSignal(0,0);
		begin_equity = broker.AccountEquity();
		prev_equity = begin_equity;
		ts.Reset();
		reward_average.Clear();
		loss_average.Clear();
	}
	else {
		// Just reset if fail is too much to take
		if (save_epoch && broker.AccountEquity() < 0.3 * begin_equity) {
			broker.Clear();
			broker.SetSignal(0,0);
		}
	}
	
	SeekActive();
	
	
	// Set broker signals based on DQN-agent action
	int time_pos = group->train_pos_all[epoch_actual];
	
	
	// Refresh values
	SetAskBid(broker, time_pos);
	broker.RefreshOrders();
	broker.CycleChanges();
	
	double equity = broker.AccountEquity();
	double reward = equity - prev_equity;
	prev_equity = equity;
	if (reward > 0)			reward_average.Add(reward);
	else if (reward < 0)	loss_average.Add(-reward);
	double diff = equity - broker.GetInitialBalance();
	if (diff > peak_value) peak_value = diff;
	
	Snapshot& snap = group->snaps[epoch_actual];
	ASSERT(snap.id == epoch_actual);
	Forward(snap, broker);
	
	
	// Refresh odrers
	if (epoch_actual < group->snaps.GetCount()-1)	broker.Cycle();
	else											broker.CloseAll();
	
	Backward(reward);
	
	
	// Write some stats for plotter
	thrd_equity[epoch_actual] = equity;
	
	
	epoch_actual++;
	SeekActive();
	
	if (epoch_actual >= epoch_total && !group->is_realtime && !group->is_looping) {
		seq_results.Add(equity);
		epoch_actual = 0;
		if (diff > best_result || best_result == 0.0)
			best_result = diff;
		training_time += ts.Elapsed() / (1000.0 * 60.0 * 60.0);
		ts.Reset();
		last_drawdown = broker.GetDrawdown();
		end_of_epoch = true;
	} else {
		end_of_epoch = false;
	}
}

void TraineeBase::SeekActive() {
	// Skip useless snapshots for this agent
	for (; epoch_actual < group->snaps.GetCount(); epoch_actual++) {
		Snapshot& snap = group->snaps[epoch_actual];
		if (snap.shift < data_begin)
			continue;
		if (snap.IsActive(tf_id))
			break;
	}
}

void TraineeBase::Serialize(Stream& s) {
	s % seq_results % reward_average % loss_average % peak_value % best_result % training_time
	  % last_drawdown % tf_id % tf % group_id % iter % is_training;
}

}
