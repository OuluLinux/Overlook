#ifndef _Overlook_Joiner_h_
#define _Overlook_Joiner_h_

struct Joiner : Moveable<Joiner>, public TraineeBase {
	
	// Persistent
	DQNAgent<JOINER_ACTIONCOUNT, JOINER_STATES> dqn;
	double begin_equity = 0.0;
	double leverage = 1000.0;
	double spread_points[SYM_COUNT];
	int proxy_id[SYM_COUNT];
	int proxy_base_mul[SYM_COUNT];
	
	
	// Temporary
	FixedSimBroker broker;
	double prev_signals[JOINER_SIGNAL_SENSORS];
	double free_margin_level = 0.95;
	double input_array[JOINER_STATES];
	int symsignals[SYM_COUNT];
	int group_id = 0;
	int signal = 0;
	
	
	Joiner();
	void Create();
	void Init();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Vector<Snapshot>& snaps);
	void WriteSignal(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s);
	void SetFreeMarginLevel(double d) {free_margin_level = d; broker.free_margin_level = d;}
};

#endif
