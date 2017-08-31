#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_


struct TraineeBase {
	
	// Persistent
	float result[AGENT_RESULT_COUNT];
	double best_result = 0.0;
	double last_drawdown = 0.0;
	long iter = 0;
	int result_cursor = 0;
	int result_count = 0;
	int id = -1;
	int cursor = 0;
	
	
	// Temporary
	double reward_sum = 0;
	double average_reward = 0;
	float prev_equity = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int type = -1;
	bool is_training = false;
	
	
	TraineeBase() {
		
	}
	
	void Create() {
		for(int i = 0; i < AGENT_RESULT_COUNT; i++)
			result[i] = 0.0f;
		result_cursor = 0;
	}
	
	inline void ResetEpoch() PARALLEL {
		
		signal = 0;
		timestep_actual = 0;
		timestep_total = 1;
		cursor = 1;
		
	}
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		else if (s.IsStoring()) {
			s.Put(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		s % best_result % last_drawdown % iter % result_cursor % result_count % id % cursor;
	}
	
};

struct Agent : Moveable<Agent>, public TraineeBase {
	
	// Persistent
	DQNAgent<AGENT_ACTIONCOUNT, AGENT_STATES> dqn;
	double begin_equity = 0.0;
	float spread_points = 0;
	int sym_id = -1;
	int sym__ = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	int group_id = -1;
	
	
	// Temporary
	SingleFixedSimBroker broker;
	
	
	
	Agent() {
		type = 0;
	}
	
	void Create() {
		
		dqn.Reset();
		
		TraineeBase::Create();
		
	}
	
	void Init() {
		
		broker.sym_id = sym_id;
		broker.begin_equity = begin_equity;
		broker.spread_points = spread_points;
		broker.proxy_id = proxy_id;
		broker.proxy_base_mul = proxy_base_mul;
		
		ResetEpoch();
		
	}
	
	inline void ResetEpoch() PARALLEL {
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
	
	inline void Main(Snapshot& cur_snap, Snapshot& prev_snap, tinymt& rand) PARALLEL {
		if (timestep_actual <= 0) {
			
			broker.RefreshOrders(cur_snap);
			double equity = broker.AccountEquity();
			double reward = equity - prev_equity;
			
			
			Backward(reward, rand);
			
			
			if (broker.equity < 0.25 * broker.begin_equity) broker.Reset();
			prev_equity = broker.equity;
			
			
			Forward(cur_snap, prev_snap, rand);
			
		}
		
		// LOG("Agent " << id << ": " << cursor << ", " << signal << ", " << timestep_actual << "/" << timestep_total);
		
		WriteSignal(cur_snap);
	}
	
	inline void Forward(Snapshot& cur_snap, Snapshot& prev_snap, tinymt& rand) PARALLEL {
		
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
		
		AMPASSERT(cursor == AGENT_STATES);
		
		
		int action = dqn.Act(input_array, rand);
		AMPASSERT(action >= 0 && action < AGENT_ACTIONCOUNT);
	    
	    
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
	
	
	inline void WriteSignal(Snapshot& cur_snap) PARALLEL {
		
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
		AMPASSERT(sigpos >= 0 && sigpos+2 < SIGNAL_SIZE);
		cur_snap.signal[sigpos + 0] = pos;
		cur_snap.signal[sigpos + 1] = neg;
		cur_snap.signal[sigpos + 2] = idl;
		
	}
	
	inline void Backward(double reward, tinymt& rand) PARALLEL {
		
		// pass to brain for learning
		if (is_training && cursor > 1)
			dqn.Learn(reward, rand);
		
		reward_sum += reward;
		
		if (iter % 50 == 0) {
			average_reward = reward_sum / 50;
			reward_sum = 0;
		}
		
		iter++;
	}
	
	void Serialize(Stream& s) {
		TraineeBase::Serialize(s);
		s % dqn % begin_equity % spread_points % sym_id
		  % sym__ % proxy_id % proxy_base_mul % group_id;
	}
};


#endif
