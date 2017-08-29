#include "Overlook.h"

namespace Overlook {

TraineeBase::TraineeBase() {
	
}

void TraineeBase::Create() {
	for(int i = 0; i < AGENT_RESULT_COUNT; i++)
		result[i] = 0.0f;
	result_cursor = 0;
}

void TraineeBase::ResetEpoch() {
	
	signal = 0;
	timestep_actual = 0;
	timestep_total = 1;
	cursor = 1;
	
}







Agent::Agent() {
	
}

void Agent::Create() {
	
	dqn.Reset();
	
	TraineeBase::Create();
	
}

void Agent::Init() {
	
	broker.sym_id = sym_id;
	broker.begin_equity = begin_equity;
	broker.spread_points = spread_points;
	broker.proxy_id = proxy_id;
	broker.proxy_base_mul = proxy_base_mul;
	
	ResetEpoch();
	
}

void Agent::ResetEpoch() {
	if (broker.order_count > 0) {
		last_drawdown = broker.GetDrawdown();
		if (broker.equity > best_result)
			best_result = broker.equity;
		result[result_cursor] = broker.equity;
		result_cursor = (result_cursor + 1) % AGENT_RESULT_COUNT;
		result_count++;
	}
	
	broker.Reset();
	
	TraineeBase::ResetEpoch();
	prev_equity = broker.AccountEquity();
}

void Agent::Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL {
	if (timestep_actual <= 0) {
		
		broker.RefreshOrders(cur_snap);
		double equity = broker.AccountEquity();
		double reward = equity - prev_equity;
		
		
		Backward(reward);
		
		
		if (broker.equity < 0.25 * broker.begin_equity) broker.Reset();
		prev_equity = broker.equity;
		
		
		Forward(cur_snap, prev_snap);
		
	}
	
	// LOG("Agent " << id << ": " << cursor << ", " << signal << ", " << timestep_actual << "/" << timestep_total);
	
	WriteSignal(cur_snap);
}

void Agent::Forward(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL {
	
	// Input values
	// - time_values
	// - input sensors
	// - previous signals
	float input_array[AGENT_STATES];
	int cursor = 0;
	
	
	// time_values
	input_array[cursor++] = cur_snap.year_timesensor;
	input_array[cursor++] = cur_snap.week_timesensor;
	input_array[cursor++] = cur_snap.day_timesensor;
	
	
	// sensor data for current tf
	for(int i = 0; i < SENSOR_SIZE; i++)
		input_array[cursor++] = cur_snap.sensor[i];
	
	
	// all previous signals from same tf agents
	for(int i = 0; i < SIGNAL_SIZE; i++)
		input_array[cursor++] = prev_snap.signal[i];
	
	ASSERT(cursor == AGENT_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < AGENT_ACTIONCOUNT);
    
    
    // Convert action to simple signal
    
	// Long/Short
	if (action < 14) {
		int exp = 3 + action / 2;
		bool neg = exp % 2; // 0,+1,-1,-2,+2,+4,-4 ...
		bool dir = (action % 2) != neg;
		signal = dir ? -1 : +1;
		timestep_total = 1 << exp;
	}
	
	// Idle
	else {
		action -= 14;
		signal = 0;
		timestep_total = 1 << action;
	}
	
	timestep_actual = timestep_total;
	
	
	// Set signal to broker
	broker.Cycle(signal, cur_snap);
	
	
}


void Agent::WriteSignal(Snapshot& cur_snap) PARALLEL {
	
	if (timestep_actual < 0) timestep_actual = 0;
	float timestep_sensor = 0.75 - 0.75 * timestep_actual / timestep_total;
	
	
	// Write latest average to the group values
	float pos, neg, idl;
	if (signal == 0) {
		pos = 1.0;
		neg = 1.0;
		idl = timestep_sensor;
	}
	else if (signal > 0) {
		pos = timestep_sensor;
		neg = 1.0;
		idl = 1.0;
	}
	else {
		pos = 1.0;
		neg = timestep_sensor;
		idl = 1.0;
	}
	
	
	int group_begin = group_id * GROUP_SIGNAL_SIZE;
	int sigpos = group_begin + sym_id * SIGNAL_SENSORS;
	ASSERT(sigpos >= 0 && sigpos+2 < SIGNAL_SIZE);
	cur_snap.signal[sigpos + 0] = pos;
	cur_snap.signal[sigpos + 1] = neg;
	cur_snap.signal[sigpos + 2] = idl;
	
}

void Agent::Backward(double reward) PARALLEL {
	
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

}
