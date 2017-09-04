#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_


struct TraineeBase {
	
	// Persistent
	double result[TRAINEE_RESULT_COUNT];
	double best_result = 0.0;
	double last_drawdown = 0.0;
	long iter = 0;
	int result_cursor = 0;
	int result_count = 0;
	int id = -1;
	int cursor = 0;
	int sym_id = -1;
	int sym = -1;
	int group_id = -1;
	
	
	// Temporary
	double reward_sum = 0;
	double average_reward = 0;
	double prev_equity = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int type = -1;
	bool is_training = false;
	
	
	TraineeBase();
	void Create();
	void ResetEpoch();
	void Serialize(Stream& s);
};

struct Agent : Moveable<Agent>, public TraineeBase {
	
	// Persistent
	DQNAgent<AGENT_ACTIONCOUNT, AGENT_STATES> dqn;
	double begin_equity = 0.0;
	double spread_points = 0;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	
	
	// Temporary
	SingleFixedSimBroker broker;
	double input_array[AGENT_STATES];
	
	
	Agent();
	void Create();
	void Init();
	void ResetEpoch();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void WriteSignal(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s);
};


#endif
