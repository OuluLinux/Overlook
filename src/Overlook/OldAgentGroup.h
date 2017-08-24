#if 0

#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

class AgentGroup {
	
public:
	
	// Persistent
	Array<SymGroup> symgroups;
	Array<JoinerAgent> joiners;
	Index<int> tf_ids, sym_ids, indi_ids;
	Time created;
	String name;
	double limit_factor;
	int mode, submode;
	bool enable_training;
	
	
	
	
	// Temp
	TimeStop last_store, last_datagather;
	int main_tf;
	bool running, stopped;
	
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
	//TimeCallback watchdog;
	
	enum {PHASE_TRAINING, PHASE_WEIGHTS, PHASE_FINAL, PHASE_REAL};
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup();
	~AgentGroup();
	
	
	void PutLatest(Brokerage& b);
	
	bool StartGroup();
	int  StartAgents(int submode);
	int  StartAgentsFast(int submode);
	void FreezeAgents(int submode);
	void StopGroup();
	void StopAgents();
	void Main();
	void Data();
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
	
};

}

#endif
#endif
