#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

class AgentGroup {
	
public:
	
	// Persistent
	Index<int> tf_ids, sym_ids, indi_ids;
	Time created;
	String name;
	double limit_factor;
	bool enable_training;
	
	
	
	
	// Temp
	TimeStop last_store, last_datagather;
	int main_tf;
	
	/*double prev_equity;
	double prev_reward;
	int buf_count;
	int data_size, signal_size;
	int act_iter;
	int main_tf, main_tf_pos;
	int timeslot_tf, timeslot_tf_pos;
	int current_submode;
	int symid_count;
	int timeslot_minutes, timeslots;
	int prev_least_results;
	int random_loops;
	int realtime_count;
	bool reset_optimizer;
	bool is_realtime;
	bool is_looping;*/
	System* sys;
	Mutex work_lock;
	//TimeCallback watchdog;
	
	enum {MODE_AGENT, MODE_GROUP, MODE_REAL};
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup();
	~AgentGroup();
	
	
	void PutLatest(Brokerage& b);
	
	void Init();
	void Start();
	bool StartGroup();
	int  StartAgents(int submode);
	int  StartAgentsFast(int submode);
	void FreezeAgents(int submode);
	void Stop();
	void StopGroup();
	void StopAgents();
	void Main();
	void Data();
	virtual void Create(int width, int height);
	virtual void Forward(Snapshot& snap, SimBroker& broker) {Forward(snap, (Brokerage&)broker);}
	virtual void Backward(double reward);
	void Forward(Snapshot& snap, Brokerage& broker);
	void StoreThis();
	void LoadThis();
	void Serialize(Stream& s);
	void RefreshSnapshots();
	void ResetSnapshot(Snapshot& snap);
	bool Seek(Snapshot& snap, int shift);
	void ResetValueBuffers();
	void CreateAgents();
	void Progress(int actual, int total, String desc);
	void SubProgress(int actual, int total);
	void SetEpsilon(double d);
	void SetMode(int i);
	virtual void SetAskBid(SimBroker& sb, int pos);
	void LoopAgentsToEnd(int submode, bool tail_only);
	void LoopAgentsToEndTf(int tf_id, bool tail_only);
	void LoopAgentsForRandomness(int submode);
	void SetTfLimit(int tf_id, double limit);
	void CheckAgentSubMode();
	
	int GetSignalBegin() const;
	int GetSignalEnd() const;
	int GetSignalPos(int group_id) const;
	double GetTfDrawdown(int tf_id);
	int GetAgentSubMode();
	
	Callback3<int, int, String> WhenProgress;
	Callback2<int, int> WhenSubProgress;
	int a0, t0, a1, t1;
	String prog_desc;
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
};

}

#endif
