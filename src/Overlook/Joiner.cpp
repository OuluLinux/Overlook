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
	broker.begin_equity = Upp::max(10000.0, begin_equity);
	broker.leverage = leverage;
	broker.free_margin_level = free_margin_level;
	broker.part_sym_id = sym_id;
	
	ResetEpoch();
	
}

void Joiner::ResetEpoch() {
	if (broker.order_count > 0) {
		last_drawdown = broker.GetDrawdown();
		if (broker.equity > best_result)
			best_result = broker.equity;
		result[result_cursor] = broker.equity;
		result_cursor = (result_cursor + 1) % TRAINEE_RESULT_COUNT;
		result_count++;
	}
	
	broker.Reset();
	
	TraineeBase::ResetEpoch();
	prev_equity = broker.PartialEquity();
}

void Joiner::Main(Vector<Snapshot>& snaps) {
	
	if (timestep_actual <= 0) {
		
		broker.RefreshOrders(snaps[cursor]);
		
		if (signal) {
			double equity = broker.PartialEquity();
			double reward = equity - prev_equity;
			
			reward /= broker.equity / broker.begin_equity;
			
			Backward(reward);
		}
		
		if (broker.equity < 0.25 * broker.begin_equity)
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
	for(int i = 0; i < AGENT_SIGNAL_SIZE; i++)
		input_array[cursor++] = cur_snap.agent_signal[i];
	
	
	// all previous signals from same tf joiners
	for(int i = 0; i < JOINER_SIGNAL_SIZE; i++)
		input_array[cursor++] = prev_snap.joiner_signal[i];
	
	ASSERT(cursor == JOINER_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < JOINER_ACTIONCOUNT);
	
	const int maxscale_steps = 3;
	const int timefwd_steps = 6;
	ASSERT(JOINER_ACTIONCOUNT == (maxscale_steps * timefwd_steps));
	
	int maxscale_step	= action % maxscale_steps;			action /= maxscale_steps;
	int timefwd_step	= action % timefwd_steps;			action /= timefwd_steps;
	
	prev_signals[0]		= 1.0 * maxscale_step / maxscale_steps;
	prev_signals[1]		= 1.0 * timefwd_step  / timefwd_steps;
	
	int maxscale		= 1 + maxscale_step * 2;
	timestep_total		= 1 << (timefwd_step + 2);
	signal				= cur_snap.agent_broker_signal[id];
	
	if (signal == 0) {
		timestep_total = 1;
	}
	else {
		signal *= maxscale;
	}
	
	cur_snap.joiner_broker_signal[id] = signal;
	
	int joiner_begin = group_id * SYM_COUNT;
	for(int i = 0; i < SYM_COUNT; i++)
		symsignals[i] = cur_snap.joiner_broker_signal[joiner_begin + i];
	
	for(int i = 0; i < SYM_COUNT; i++)
		broker.SetSignal(i, symsignals[i]);
	
	bool succ = broker.Cycle(cur_snap);
	
	if (!succ)
		broker.Reset();
	
	timestep_actual = timestep_total;
	
}

void Joiner::WriteSignal(Snapshot& cur_snap) {

	cur_snap.joiner_broker_signal[id] = signal;
	
	if (timestep_actual < 0) timestep_actual = 0;
	prev_signals[JOINER_SIGNAL_SENSORS-1] = 1.00 - 1.0 * timestep_actual / timestep_total;
	
	
	int cursor = id * JOINER_SIGNAL_SENSORS;
	ASSERT(cursor >= 0 && cursor < JOINER_SIGNAL_SIZE);
	
	
	for(int i = 0; i < JOINER_SIGNAL_SENSORS; i++)
		cur_snap.joiner_signal[cursor++] = prev_signals[i];
	ASSERT(cursor <= JOINER_SIGNAL_SIZE);
	
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
