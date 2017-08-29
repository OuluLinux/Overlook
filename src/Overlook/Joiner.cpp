#include "Overlook.h"

namespace Overlook {

Joiner::Joiner() {
	
}

void Joiner::Create() {
	
	dqn.Reset();
	
	TraineeBase::Create();
	
}

void Joiner::Init() {
	
	for(int i = 0; i < SYM_COUNT; i++) {
		broker.spread_points[i] = spread_points[i];
		broker.proxy_id[i] = proxy_id[i];
		broker.proxy_base_mul[i] = proxy_base_mul[i];
	}
	broker.begin_equity = begin_equity;
	broker.leverage = leverage;
	
	ResetEpoch();
	
}

void Joiner::ResetEpoch() {
	if (broker.order_count > 0) {
		last_drawdown = broker.GetDrawdown();
		if (broker.equity > best_result)
			best_result = broker.equity;
		result[result_cursor] = broker.equity;
		result_cursor = (result_cursor + 1) % JOINER_RESULT_COUNT;
		result_count++;
	}
	
	broker.Reset();
	
	TraineeBase::ResetEpoch();
	prev_equity = broker.AccountEquity();
}

void Joiner::Main(const array_view<Snapshot, 1>& snap_view) PARALLEL {
	Snapshot& cur_snap = snap_view[cursor];
	
	if (timestep_actual <= 0) {
		
		broker.RefreshOrders(cur_snap);
		double equity = broker.AccountEquity();
		double reward = equity - prev_equity;
		
		
		Backward(reward);
		
		
		if (broker.equity < 0.25 * broker.begin_equity) broker.Reset();
		prev_equity = broker.equity;
		
		
		Forward(snap_view);
		
	}
	
	LOG("Joiner " << id << ": " << cursor << ", " << signal << ", " << timestep_actual << "/" << timestep_total);
	
	WriteSignal(cur_snap);
}

void Joiner::Forward(const array_view<Snapshot, 1>& snap_view) PARALLEL {
	/*
		What was NOT implemented:
			- checking individual orders and beginning of orders
			- requiring many agents with same sym_id to have same type (e.g. all long/short)
			- not using sum of signals
	*/
	
	Snapshot& cur_snap  = snap_view[cursor];
	Snapshot& prev_snap = snap_view[cursor - 1];
	
	
	// Input values
	// - time_values
	// - input sensors
	// - previous signals
	float input_array[JOINER_STATES];
	int cursor = 0;
	
	
	// time_values
	input_array[cursor++] = cur_snap.year_timesensor;
	input_array[cursor++] = cur_snap.week_timesensor;
	input_array[cursor++] = cur_snap.day_timesensor;
	
	
	// sensor data for current tf
	for(int i = 0; i < SENSOR_SIZE; i++)
		input_array[cursor++] = cur_snap.sensor[i];
	
	
	// all current signals from same tf agents
	for(int i = 0; i < SIGNAL_SIZE; i++)
		input_array[cursor++] = cur_snap.signal[i];
	
	
	// all previous signals from same tf joiners
	for(int i = 0; i < JOINERSIGNAL_SIZE; i++)
		input_array[cursor++] = prev_snap.joiner_signal[i];
	
	ASSERT(cursor == JOINER_STATES);
	
	
	int action = dqn.Act(input_array);
	
	ASSERT(action >= 0 && action < JOINER_ACTIONCOUNT);
    
    if (action < JOINER_NORMALACTS) {
		
		const int maxscale_steps = 3;
		const int fmlevel_steps = 3;
		const int timebwd_steps = 3;
		const int timefwd_steps = 3;
		ASSERT(JOINER_NORMALACTS == (maxscale_steps * fmlevel_steps * timebwd_steps * timefwd_steps));
		
		int maxscale_step	= action % maxscale_steps;			action /= maxscale_steps;
		int fmlevel_step	= action % fmlevel_steps;			action /= fmlevel_steps;
		int timebwd_step	= action % timebwd_steps;			action /= timebwd_steps;
		int timefwd_step	= action % timefwd_steps;			action /= timefwd_steps;
		
		prev_signals[0]		= 1.0 * maxscale_step / maxscale_steps;
		prev_signals[1]		= 1.0 * fmlevel_step  / fmlevel_steps;
		prev_signals[2]		= 1.0 * timebwd_step  / timebwd_steps;
		prev_signals[3]		= 1.0 * timefwd_step  / timefwd_steps;
		
		int maxscale		= 2 + maxscale_step * 4;		// 2, 6, 10
		double fmlevel		= 0.75 + 0.1  * fmlevel_step;	// 0.75, 0.85, 0.95
		int timebwd			= 1 << (timebwd_step * 3 + 1);	// 2, 16, 128
		int timefwd			= 1 << (timefwd_step * 3 + 1);	// 2, 16, 128
		
		
		broker.free_margin_level = fmlevel;
		timestep_total = timefwd;
		
		
		int begin_snap_cursor = Upp::max(0, cursor - timebwd);
		Snapshot& begin_snap = snap_view[begin_snap_cursor];
		
		
		int symsignals[SYM_COUNT];
		double symchanges[SYM_COUNT];
		double changes_total[AGENT_COUNT];
		double signals_total[AGENT_COUNT];
		
		double min_change = 0.0;
		double max_change = 0.0;
		
		for(int i = 0; i < SYM_COUNT; i++)
			symchanges[i] = cur_snap.open[i] / begin_snap.open[i] - 1.0;
		
		for(int i = 0; i < AGENT_COUNT; i++) {
			int sym = i % SYM_COUNT;
			double change = symchanges[sym];
			int signal;
			
			int sensor_begin = i * SIGNAL_SENSORS;
			float pos = cur_snap.signal[sensor_begin + 0];
			float neg = cur_snap.signal[sensor_begin + 1];
			float idl = cur_snap.signal[sensor_begin + 2];
			
			if (idl < 1.0f) {
				change = 0;
				signal = 0;
			}
			else if (pos < neg) {
				signal = +1;
			}
			else {
				change *= -1.0;
				signal = -1;
			}
			
			changes_total[i] = change;
			signals_total[i] = signal;
			
			if (change < min_change) min_change = change;
			if (change > max_change) max_change = change;
		}
		
		double range = max_change - min_change;
		for(int i = 0; i < AGENT_COUNT; i++) {
			double& change = changes_total[i];
			if (change == 0.0)
				continue;
			change = ((change - min_change) / range) * (maxscale - 1) + 1.0;
			ASSERT(change >= 1 && change <= maxscale);
		}
		
		
		for(int i = 0; i < SYM_COUNT; i++)
			symsignals[i] = 0;
			
			
		for(int i = 0; i < AGENT_COUNT; i++)
			symsignals[i % SYM_COUNT] += signals_total[i] * changes_total[i];
		
		
		for(int i = 0; i < SYM_COUNT; i++)
			broker.SetSignal(i, symsignals[i]);
    }
    
    else {
        action -= JOINER_NORMALACTS;
        
        timestep_total = 1 << action;
        
		prev_signals[0]		= 1.0;
		prev_signals[1]		= 1.0;
		prev_signals[2]		= 1.0;
		prev_signals[3]		= 1.0;
		
		for(int i = 0; i < SYM_COUNT; i++)
			broker.SetSignal(i, 0);
    }
	
	
	broker.Cycle(cur_snap);
	
	
	timestep_actual = timestep_total;
	
}

void Joiner::WriteSignal(Snapshot& cur_snap) PARALLEL {
	
	if (timestep_actual < 0) timestep_actual = 0;
	float timestep_sensor = 1.00 - timestep_actual / timestep_total;
	
	
	int group_begin = id * JOINERSIGNAL_SENSORS;
	ASSERT(group_begin >= 0 && group_begin < JOINERSIGNAL_SIZE);
	
	
	int cursor = group_begin;
	for(int i = 0; i < 4; i++)
		cur_snap.joiner_signal[cursor++] = prev_signals[i];
	cur_snap.joiner_signal[cursor++] = timestep_sensor;
	ASSERT(cursor <= JOINERSIGNAL_SIZE);
	
}

void Joiner::Backward(double reward) PARALLEL {
	
	// pass to brain for learning
	if (is_training && cursor > 1)
		dqn.Learn(reward);
	
	reward_sum += reward;
	
	if (iter % 50 == 0) {
		average_reward = reward_sum / 50;
		reward_sum = 0;
	}
	
	iter++;
}

/*
bool Joiner::PutLatest(Brokerage& broker) {
	
	Time time = GetMetaTrader().GetTime();
	int shift = sys->GetShiftFromTimeTf(time, main_tf);
	if (shift != train_pos_all.Top()) {
		WhenError(Format("Current shift doesn't match the lastest snapshot shift (%d != %d)", shift, train_pos_all.Top()));
		return false;
	}
	
	
	group->WhenInfo("Looping agents until latest snapshot");
	LoopAgentsToEnd(tf_ids.GetCount(), true);
	Snapshot& shift_snap = snaps[train_pos_all.GetCount()-1];
	
	
	// Reset signals
	if (is_realtime) {
		if (realtime_count == 0) {
			for(int i = 0; i < broker.GetSymbolCount(); i++)
				broker.SetSignal(i, 0);
		}
		realtime_count++;
	}
	
	
	// Set probability for random actions to 0
	Forward(shift_snap, broker);
	
	
	String sigstr = "Signals ";
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		if (i) sigstr << ",";
		sigstr << broker.GetSignal(sym_ids[i]);
	}
	group->WhenInfo(sigstr);
	
	
	group->WhenInfo("Refreshing broker data");
	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);
	if (mt) {
		mt->Data();
	} else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}
	broker.SetLimitFactor(limit_factor);
	
	
	group->WhenInfo("Updating orders");
	broker.SignalOrders(true);
	broker.RefreshLimits();
	
	return true;
}
*/

}
