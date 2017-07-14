#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

struct AgentGroup {
	
	// Persistent
	Array<Agent> agents;
	Vector<int> train_pos;
	Index<int> tf_ids, sym_ids;
	Time created;
	String name;
	String param_str;
	double global_free_margin_level;
	int reward_period;
	int input_width, input_height;
	bool single_data, single_signal;
	bool sig_freeze;
	bool enable_training;
	
	
	// Temp
	Vector<Vector<Vector<ConstBuffer*> > > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Core*> databridge_cores;
	Array<Snapshot> snaps;
	Vector<int> data_begins;
	Vector<int> tf_periods;
	Index<int> indi_ids;
	SimBroker broker;
	int buf_count;
	int data_size, signal_size, total_size;
	System* sys;
	
	// Maybe
	/*Vector<int> pos, tfs, periods, period_in_slower;
	Time begin;
	int begin_ts;
	int bars;*/
	
	
	AgentGroup();
	~AgentGroup();
	void Init();
	void Start();
	void Stop();
	void StoreThis();
	void LoadThis();
	void Serialize(Stream& s);
	void GenerateSnapshots();
	void RefreshSnapshots();
	void ResetSnapshot(Snapshot& snap);
	bool Seek(Snapshot& snap, int shift);
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	void ResetValueBuffers();
	void InitThreads();
	void CreateAgents();
	void Progress(int actual, int total, String desc);
	void SubProgress(int actual, int total);
	
	
	int GetSignalBegin() const;
	int GetSignalEnd() const;
	int GetSignalPos(int group_id) const;
	
	Callback3<int, int, String> WhenProgress;
	Callback2<int, int> WhenSubProgress;
	int a0, t0, a1, t1;
	String prog_desc;
};

}

#endif
