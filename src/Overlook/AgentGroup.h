#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define GROUP_COUNT			2
#define SYM_COUNT			21
#define INPUT_SENSORS		(4 * 4 * 2)
#define SIGNAL_SENSORS		3
#define GROUP_SENSOR_SIZE	(SYM_COUNT * INPUT_SENSORS)
#define GROUP_SIGNAL_SIZE	(SYM_COUNT * SIGNAL_SENSORS)
#define SENSOR_SIZE			GROUP_SENSOR_SIZE
#define SIGNAL_SIZE			GROUP_SIGNAL_SIZE * GROUP_COUNT
#define AGENT_STATES		(2 + GROUP_SENSOR_SIZE + GROUP_SIGNAL_SIZE)
#define AGENT_ACTIONCOUNT	30

class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	float year_timesensor, week_timesensor;
	float sensor[SENSOR_SIZE];
	float signal[SIGNAL_SIZE];
	float open[SYM_COUNT];
};

struct FixedOrder {
	float open = 0.0;
	float volume = 0.0;
	float profit = 0.0;
	char type = -1;
	char is_open = 0;
};

struct SingleFixedSimBroker {
	FixedOrder order;
	double balance = 0.0;
	double equity = 0.0;
	double begin_equity = 0.0;
	double spread_points = 0.001;
	double profit_sum = 0.0;
	double loss_sum = 0.0;
	int order_count = 0;
	int sym_id = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	
	
	SingleFixedSimBroker();
	void Cycle(int signal, const Snapshot& snap) PARALLEL;
	void RefreshOrders(const Snapshot& snap) PARALLEL;
	void Reset() PARALLEL;
	float RealtimeBid(const Snapshot& snap, int sym_id) const PARALLEL;
	float RealtimeAsk(const Snapshot& snap, int sym_id) const PARALLEL;
	void OrderSend(int type, float volume, float price) PARALLEL;
	void OrderClose(const Snapshot& snap) PARALLEL;
	double GetCloseProfit(const Snapshot& snap) const PARALLEL;
	double AccountEquity() const PARALLEL {return equity;}
	double GetDrawdown() const PARALLEL {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
};

#define AGENT_RESULT_COUNT 1000

struct TraineeBase {
	float result[AGENT_RESULT_COUNT];
	int result_cursor = 0;
	int result_count = 0;
	int id = -1;
	int cursor = 0;
	
	
	TraineeBase();
	void Create();
};

struct Agent : Moveable<Agent>, public TraineeBase {
	DQNAgent<AGENT_ACTIONCOUNT, AGENT_STATES> dqn;
	SingleFixedSimBroker broker;
	double average_reward = 0;
	double reward_sum = 0;
	double begin_equity = 0.0;
	double best_result = 0.0;
	double last_drawdown = 0.0;
	float spread_points = 0;
	float prev_equity = 0;
	int64 iter = 0;
	int sym_id = -1;
	int sym__ = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	int group_id = -1;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	bool is_training = false;
	
	
	Agent();
	void Create();
	void ResetEpoch();
	void Init();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Backward(Snapshot& snap, double reward) PARALLEL;
	void WriteSignal(Snapshot& cur_snap) PARALLEL;
	
};

struct Joiner : Moveable<Joiner> {
	DQNAgent<1, 1> dqn;
	
	
};

struct Final : Moveable<Final> {
	DQNAgent<1, 1> dqn;
	
	
};

class AgentGroup {
	
	
public:
	
	// Persistent
	Vector<Agent> agents;
	Vector<Joiner> joiners;
	Final fin;
	Vector<Snapshot> snaps;
	Vector<FactoryDeclaration> indi_ids;
	Index<int> sym_ids;
	Time created;
	double limit_factor;
	int phase;
	int group_count;
	bool enable_training;
	
	
	// Temp
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Core*> databridge_cores;
	Vector<double> agent_equities;
	Index<String> allowed_symbols;
	TimeStop last_store, last_datagather;
	System* sys;
	int main_tf;
	int data_begin;
	int buf_count;
	int sym_count;
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
	void SetAgentsTraining(bool b);
	
	
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
