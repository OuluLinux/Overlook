#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

class AgentGroup;
class Agent;

enum {
	PHASE_SIGNAL_TRAINING,
	PHASE_AMP_TRAINING,
	PHASE_REAL
};


struct AgentSignal {
	
	
	// Persistent
	DQNAgent<SIGNAL_ACTIONCOUNT, SIGNAL_STATES> dqn;
	Vector<double> result_equity, result_drawdown, rewards;
	Vector<int64> action_counts;
	int64 iter = 0, deep_iter = 0;
	
	
	// Temporary
	Vector<double> equity;
	double all_reward_sum = 0, pos_reward_sum = 0, neg_reward_sum = 0;
	double reward_sum = 0;
	double change_sum = 0, change_sum_limit = 0;
	int change_count = 0;
	int reward_count = 0;
	int signal = 0;
	int cursor = 0;
	int fwd_cursor = 0;
	int lower_output_signal = 0;
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
	Vector<int64> action_counts;
	int64 iter = 0, deep_iter = 0;
	
	
	// Temporary
	FixedSimBroker broker;
	Vector<double> equity;
	double prev_equity = 0;
	double reward_sum = 0;
	double change_sum = 0, change_sum_limit = 0;
	int change_count = 0;
	int reward_count = 0;
	int signal = 0;
	int cursor = 0;
	int lower_output_signal = 0;
	int prev_lower_output_signal = 0;
	int prev_reset_iter = 0;
	bool skip_learn = true;
	bool clean_epoch = true;
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



class Agent : Moveable<Agent> {
	
public:
	
	// Persistent
	AgentSignal sig;
	AgentAmp amp;
	bool group_active_lower[3];
	int sym_id = -1;
	int sym = -1;
	int group_id = -1;
	
	
	// Temporary
	bool is_training = false;
	double spread_points = 0.0;
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
	void SetFreeMarginLevel(double d) {amp.broker.free_margin_level = d;}
	int GetCursor(int phase) const;
	double GetLastDrawdown(int phase) const;
	double GetLastResult(int phase) const;
	int64 GetIter(int phase) const;
	void RefreshGroupSettings();
	bool IsTrained(int phase) const;
};


#endif
