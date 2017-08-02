#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

class AgentGroup : public TraineeBase {
	
public:
	
	// Persistent
	CoreUtils::GeneticOptimizer go;
	Array<Agent> agents;
	Vector<double> tf_limit;
	Index<int> tf_ids, sym_ids;
	Time created;
	String name;
	String param_str;
	double fmlevel;
	double limit_factor;
	int group_input_width, group_input_height;
	int mode;
	bool sig_freeze;
	bool enable_training;
	bool accum_signal;
	
	// Temp
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Vector<Core*> > databridge_cores;
	Array<Snapshot> snaps;
	Vector<int> train_pos_all;
	Vector<int> symsignals;
	Vector<int> data_begins;
	Vector<int> tf_minperiods, tf_periods, tf_types;
	Index<int> indi_ids;
	TimeStop last_store, last_datagather;
	double prev_equity;
	double prev_reward;
	int buf_count;
	int data_size, signal_size, total_size;
	int act_iter;
	int main_tf, main_tf_pos;
	int current_submode;
	int symid_count;
	int fastest_period_mins, timeslots;
	int prev_least_results;
	int random_loops;
	bool reset_optimizer;
	bool is_realtime;
	bool is_looping;
	System* sys;
	Mutex work_lock;
	TimeCallback watchdog;
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup();
	~AgentGroup();
	void Init();
	void Start();
	void StartGroup();
	void StartAgents(int submode);
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
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	void ResetValueBuffers();
	void CreateAgents();
	void Progress(int actual, int total, String desc);
	void SubProgress(int actual, int total);
	void SetEpsilon(double d);
	void SetMode(int i);
	bool PutLatest(Brokerage& broker);
	virtual void SetAskBid(SimBroker& sb, int pos);
	void LoopAgentsToEnd();
	void LoopAgentToEnd(int i);
	void LoopAgentsForRandomness();
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
