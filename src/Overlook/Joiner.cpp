#include "Overlook.h"

namespace Overlook {

Joiner::Joiner() {
	type = 1;
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

void Joiner::Main(Vector<Snapshot>& snaps) {
	
	if (timestep_actual <= 0) {
		
		broker.RefreshOrders(snaps[cursor]);
		double equity = broker.AccountEquity();
		double reward = equity - prev_equity;
		
		
		Backward(reward);
		
		
		if (broker.equity < 0.50 * broker.begin_equity)
			broker.Reset();
		prev_equity = broker.equity;
		
		
		Forward(snaps);
		
	}
	
	//LOG("Joiner " << id << ": " << cursor << ", " << signal << ", " << timestep_actual << "/" << timestep_total);
	
	WriteSignal(snaps[cursor]);
}

void Joiner::Forward(Vector<Snapshot>& snaps) {
	/*
		What was NOT implemented:
			- checking individual orders and beginning of orders
			- requiring many agents with same sym_id to have same type (e.g. all long/short)
			- not using sum of signals
	*/
	
	Snapshot& cur_snap  = snaps[cursor];
	Snapshot& prev_snap = snaps[cursor - 1];
	
	
	// Input values
	// - time_values
	// - input sensors
	// - previous signals
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
		
		int maxscale		= 1 + maxscale_step * 2;		// 1, 3, 5
		double fmlevel		= 0.75 + 0.1  * fmlevel_step;	// 0.75, 0.85, 0.95
		int timebwd			= 1 << (timebwd_step * 3 + 1);	// 2, 16, 128
		int timefwd			= 1 << (timefwd_step * 3 + 1);	// 2, 16, 128
		
		
		free_margin_level = fmlevel;
		broker.free_margin_level = fmlevel;
		timestep_total = timefwd;
		
		
		int begin_snap_cursor = cursor - timebwd;
		if (begin_snap_cursor < 0) begin_snap_cursor = 0;
		Snapshot& begin_snap = snaps[begin_snap_cursor];
		
		
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
			
			if (idl < 1.0f || (idl == pos && pos == neg)) {
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
		if (range > 0.0) {
			for(int i = 0; i < AGENT_COUNT; i++) {
				if (signals_total[i] == 0)
					continue;
				double& change = changes_total[i];
				change = ((change - min_change) / range) * (maxscale - 1) + 1.0;
				ASSERT(change >= 1 && change <= maxscale);
			}
		} else {
			for(int i = 0; i < AGENT_COUNT; i++)
				changes_total[i] = 0.0;
		}
		
		
		for(int i = 0; i < SYM_COUNT; i++)
			symsignals[i] = 0;
			
			
		for(int i = 0; i < AGENT_COUNT; i++)
			symsignals[i % SYM_COUNT] += signals_total[i] * changes_total[i];
		
	}
	
	else {
		action -= JOINER_NORMALACTS;
		
		free_margin_level = 0.95;
		timestep_total = 1 << action;
		
		prev_signals[0]		= 1.0;
		prev_signals[1]		= 1.0;
		prev_signals[2]		= 1.0;
		prev_signals[3]		= 1.0;
		
		for(int i = 0; i < SYM_COUNT; i++)
			symsignals[i] = 0;
	}
	
	for(int i = 0; i < SYM_COUNT; i++)
		broker.SetSignal(i, symsignals[i]);
	
	bool succ = broker.Cycle(cur_snap);
	
	if (!succ)
		broker.Reset();
	
	timestep_actual = timestep_total;
	
}

void Joiner::WriteSignal(Snapshot& cur_snap) {
	
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

void Joiner::Backward(double reward) {
	
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

bool Joiner::PutLatest(AgentGroup& ag, Brokerage& broker, Vector<Snapshot>& snaps) {
	System& sys = GetSystem();
	sys.WhenPushTask("Putting latest signals");
		
	
	// Set probability for random actions to 0
	cursor = snaps.GetCount() - 1;
	Forward(snaps);
	
	
	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);
	if (mt) {
		mt->Data();
		broker.RefreshLimits();
	} else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}
	
	
	for(int i = 0; i < ag.sym_ids.GetCount(); i++) {
		int sym = ag.sym_ids[i];
		int sig = symsignals[i];
		if (sig == broker.GetSignal(sym) && sig != 0)
			broker.SetSignalFreeze(sym, true);
		else {
			broker.SetSignal(sym, sig);
			broker.SetSignalFreeze(sym, false);
		}
	}
	
	broker.SetFreeMarginLevel(free_margin_level);
	broker.SignalOrders(true);
	
	sys.WhenPopTask();
	
	return true;
}

void Joiner::Data() {
	MetaTrader& mt = GetMetaTrader();
	Vector<Order> orders;
	Vector<int> signals;

	orders <<= mt.GetOpenOrders();
	signals <<= mt.GetSignals();

	mt.Data();

	int file_version = 1;
	double balance = mt.AccountBalance();
	double equity = mt.AccountEquity();
	Time time = mt.GetTime();

	FileAppend fout(ConfigFile("agentgroup.log"));
	int64 begin_pos = fout.GetSize();
	int size = 0;
	fout.Put(&size, sizeof(int));
	fout % file_version % balance % equity % time % signals % orders;
	int64 end_pos = fout.GetSize();
	size = end_pos - begin_pos - sizeof(int);
	fout.Seek(begin_pos);
	fout.Put(&size, sizeof(int));
}

void Joiner::Serialize(Stream& s) {
	TraineeBase::Serialize(s);
	s % dqn % begin_equity % leverage;
	if (s.IsLoading()) {
		s.Get(spread_points, sizeof(double) * SYM_COUNT);
		s.Get(proxy_id, sizeof(int) * SYM_COUNT);
		s.Get(proxy_base_mul, sizeof(int) * SYM_COUNT);
	}
	if (s.IsStoring()) {
		s.Put(spread_points, sizeof(double) * SYM_COUNT);
		s.Put(proxy_id, sizeof(int) * SYM_COUNT);
		s.Put(proxy_base_mul, sizeof(int) * SYM_COUNT);
	}
}

}
