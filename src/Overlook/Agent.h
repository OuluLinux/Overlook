#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

class AgentGroup;
class Agent;

enum {
	PHASE_PREFUSE1_TRAINING,
	PHASE_PREFUSE2_TRAINING,
	PHASE_PREFUSE3_TRAINING,
	PHASE_SIGNAL_TRAINING,
	PHASE_AMP_TRAINING,
	PHASE_FUSE_TRAINING,
	PHASE_REAL
};


struct AgentFilter {
	
	
	// Persistent
	DQNAgent<FILTER_ACTIONCOUNT, FILTER_STATES> dqn;
	Vector<double> result_equity, result_drawdown, rewards;
	int64 iter = 0;
	
	
	// Temporary
	OnlineAverage1 change_av;
	Vector<double> equity;
	double all_reward_sum = 0, pos_reward_sum = 0, neg_reward_sum = 0;
	double reward_sum = 0;
	int reward_count = 0;
	int signal = 0, sig_mul = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	int fwd_cursor = 0;
	int lower_output_signal = 0;
	int prev_reset_iter = 0;
	int level = -1;
	bool skip_learn = true;
	Agent* agent = NULL;
	
	
	AgentFilter();
	void Create();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s);
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


struct AgentSignal {
	
	
	// Persistent
	DQNAgent<SIGNAL_ACTIONCOUNT, SIGNAL_STATES> dqn;
	Vector<double> result_equity, result_drawdown, rewards;
	int64 iter = 0, deep_iter = 0;
	int64 extra_timesteps = 0;
	
	
	// Temporary
	#ifdef flagHAVE_SIGSENS
	OnlineAverage1 epoch_av[SIGSENS_COUNT];
	#endif
	SingleFixedSimBroker broker;
	Vector<double> equity;
	double prev_equity = 0;
	double reward_sum = 0;
	int reward_count = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	int prev_equity_cursor = 0;
	int lower_output_signal = 0;
	int cursor_sigbegin = 0;
	int prev_reset_iter = 0;
	bool skip_learn = true;
	Agent* agent = NULL;
	
	
	AgentSignal();
	void Create();
	void DeepCreate();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps);
	void Backward(double reward);
	void Serialize(Stream& s);
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


struct AgentAmp {
	
	
	// Persistent
	DQNAgent<AMP_ACTIONCOUNT, AMP_STATES> dqn;
	Vector<double> result_equity, result_drawdown, rewards;
	int64 iter = 0, deep_iter = 0;
	int64 extra_timesteps = 0;
	
	
	// Temporary
	#ifdef flagHAVE_SIGSENS
	OnlineAverage1 epoch_av[SIGSENS_COUNT];
	#endif
	FixedSimBroker broker;
	Vector<double> equity;
	double prev_signals[AMP_SENSORS];
	double prev_equity = 0;
	double reward_sum = 0;
	int reward_count = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	int cursor_sigbegin = 0;
	int prev_equity_cursor = 0;
	int lower_output_signal = 0;
	int prev_lower_output_signal = 0;
	int prev_reset_iter = 0;
	bool skip_learn = true;
	Agent* agent = NULL;
	
	
	
	AgentAmp();
	void Create();
	void DeepCreate();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps);
	void Backward(double reward);
	void Serialize(Stream& s);
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};

struct AgentFuse {
	
	
	// Persistent
	DQNAgent<FUSE_ACTIONCOUNT, FUSE_STATES> dqn;
	Vector<double> result_equity, result_drawdown, rewards;
	int64 iter = 0;
	
	
	// Temporary
	#ifdef flagHAVE_SIGSENS
	OnlineAverage1 epoch_av[SIGSENS_COUNT];
	#endif
	FixedSimBroker test_broker, broker;
	Vector<double> equity;
	double prev_equity = 0;
	double reward_sum = 0;
	double pos_dd_sum = 0;
	double neg_dd_sum = 0;
	double begin_equity = 0;
	double dd, begin_dd;
	int dd_count;
	int reward_count = 0;
	int signal = 0;
	int cursor = 0;
	int lower_output_signal = 0;
	int prev_lower_output_signal = 0;
	int prev_reset_iter = 0;
	bool skip_learn = true;
	Agent* agent = NULL;
	
	
	
	AgentFuse();
	void Create();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps);
	void Backward(double reward);
	void Serialize(Stream& s) {s % dqn % result_equity % result_drawdown % rewards % iter;}
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


class Agent : Moveable<Agent> {
	
public:
	
	// Persistent
	AgentFilter filter[FILTER_COUNT];
	AgentSignal sig;
	AgentAmp amp;
	AgentFuse fuse;
	bool group_active_lower[3];
	int sym_id = -1;
	int sym = -1;
	int group_id = -1;
	int group_step = 0;
	
	
	// Temporary
	bool is_training = false;
	AgentGroup* group = NULL;
	
	
	
public:
	
	Agent();
	void CreateAll();
	void Init();
	void RefreshSnapEquities();
	void ResetEpochAll();
	void ResetEpoch(int phase);
	void Main(int phase, Vector<Snapshot>& snaps);
	void Serialize(Stream& s);
	void SetFreeMarginLevel(double d) {amp.broker.free_margin_level = d; fuse.test_broker.free_margin_level = d; fuse.broker.free_margin_level = d;}
	int GetCursor(int phase) const;
	double GetLastDrawdown(int phase) const;
	double GetLastResult(int phase) const;
	int64 GetIter(int phase) const;
	void RefreshGroupSettings();
};


#endif
