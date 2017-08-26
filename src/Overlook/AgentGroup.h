#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define GROUP_COUNT 2
#define SYM_COUNT 21
#define SENSOR_SIZE SYM_COUNT*4*4*GROUP_COUNT
#define SIGNAL_SIZE SYM_COUNT*GROUP_COUNT

class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	float year_time, week_time;
	float sensor[SENSOR_SIZE];
	float signal[SIGNAL_SIZE];
	float open[SYM_COUNT];
};

struct SingleFixedSimBroker {
	double equity;
	int order_count;
	
	
	SingleFixedSimBroker();
	void Cycle(Snapshot& snap) PARALLEL;
	void RefreshOrders(Snapshot& snap) PARALLEL;
	void Reset() PARALLEL;
	double AccountEquity() const PARALLEL {return equity;}
};

struct Agent : Moveable<Agent> {
	DQNAgent<1,1,1,1> dqn;
	SingleFixedSimBroker broker;
	float prev_equity;
	int sym_id, sym, proxy_sym, input_count, group_id;
	int cursor;
	
	Agent();
	void Create();
	void Init();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void CloseAll(Snapshot& snap) PARALLEL;
	void Forward(Snapshot& snap) PARALLEL;
	void Backward(Snapshot& snap, double reward) PARALLEL;
	
};

struct Joiner : Moveable<Joiner> {
	DQNAgent<1,1,1,1> dqn;
	
	
};

struct Final : Moveable<Final> {
	DQNAgent<1,1,1,1> dqn;
	
	
};

class AgentGroup {
	
	
public:
	
	// Persistent
	Vector<Agent> agents;
	Vector<Joiner> joiners;
	Final fin;
	Vector<Snapshot> snaps;
	Vector<FactoryDeclaration> indi_ids;
	Vector<float> agent_equities;
	Index<int> sym_ids;
	Time created;
	String name;
	double limit_factor;
	int phase;
	int group_count;
	bool enable_training;
	
	
	// Temp
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Core*> databridge_cores;
	Index<String> allowed_symbols;
	TimeStop last_store, last_datagather;
	System* sys;
	int main_tf;
	int data_begin;
	int buf_count;
	int sym_count;
	int data_size, signal_size;
	int max_memory_size, agents_total_size, memory_for_snapshots, snaps_per_phase, snap_phase_count;
	int snap_phase_id;
	bool running, stopped;
	Mutex work_lock;
	
	enum {PHASE_SEEKSNAPS, PHASE_TRAINING, PHASE_WEIGHTS, PHASE_FINAL, PHASE_UPDATE, PHASE_REAL, PHASE_WAIT};
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup(System* sys);
	~AgentGroup();
	
	
	void Init();
	void InitThread();
	void Start();
	void Stop();
	void StoreThis();
	void LoadThis();
	void SetEpsilon(double d);
	void RefreshSnapshots();
	void Progress(int actual, int total, String desc);
	void SubProgress(int actual, int total);
	void Serialize(Stream& s);
	void ResetValueBuffers();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessDataBridgeQueue();
	bool Seek(Snapshot& snap, int shift);
	void CreateAgents();
	
	void Main();
	
	
	
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
