#ifndef _Overlook_Agent_h_
#define _Overlook_Agent_h_

class AgentGroup;
class Agent;

struct AgentSignal {
	
	
	// Persistent
	DQNAgent<SIGNAL_ACTIONCOUNT, SIGNAL_STATES> dqn;
	Vector<double> result_equity, result_drawdown;
	long iter = 0;
	
	
	// Temporary
	SingleFixedSimBroker broker;
	Vector<double> equity;
	double prev_equity = 0;
	double reward_sum = 0;
	double average_reward = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	Agent* agent = NULL;
	
	
	AgentSignal();
	void Create();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s) {s % dqn % result_equity % result_drawdown % iter;}
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


struct AgentAmp {
	
	
	// Persistent
	DQNAgent<AMP_ACTIONCOUNT, AMP_STATES> dqn;
	Vector<double> result_equity, result_drawdown;
	long iter = 0;
	
	
	// Temporary
	FixedSimBroker broker;
	Vector<double> equity;
	double prev_signals[AMP_SENSORS];
	double prev_equity = 0;
	double reward_sum = 0;
	double average_reward = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	Agent* agent = NULL;
	
	
	AgentAmp();
	void Create();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s) {s % dqn % result_equity % result_drawdown % iter;}
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


struct AgentFuse {
	
	
	// Persistent
	DQNAgent<FUSE_ACTIONCOUNT, FUSE_STATES> dqn;
	Vector<double> result_equity, result_drawdown;
	long iter = 0;
	
	
	// Temporary
	FixedSimBroker broker;
	Vector<double> equity;
	double prev_equity = 0;
	double reward_sum = 0;
	double average_reward = 0;
	int signal = 0, sig_mul = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int cursor = 0;
	Agent* agent = NULL;
	
	
	AgentFuse();
	void Create();
	void ResetEpoch();
	void Main(Vector<Snapshot>& snaps);
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap);
	void Write(Snapshot& cur_snap);
	void Backward(double reward);
	void Serialize(Stream& s) {s % dqn % result_equity % result_drawdown % iter;}
	inline double GetLastDrawdown() const {return result_drawdown.IsEmpty() ? 100.0 : result_drawdown.Top();}
	inline double GetLastResult() const {return result_equity.IsEmpty() ? 0.0 : result_equity.Top();}
};


class Agent : Moveable<Agent> {
	
public:
	
	// Persistent
	AgentSignal sig;
	AgentAmp amp;
	AgentFuse fuse;
	int sym_id = -1;
	int sym = -1;
	int group_id = -1;
	
	
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
	void SetFreeMarginLevel(double d) {amp.broker.free_margin_level = d; fuse.broker.free_margin_level = d;}
	int GetCursor(int phase) const;
	double GetLastDrawdown(int phase) const;
	double GetLastResult(int phase) const;
};


#endif
