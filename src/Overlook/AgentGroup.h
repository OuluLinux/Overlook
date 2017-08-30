#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define GROUP_COUNT				4 //TODO
#define SYM_COUNT				10
#define AGENT_COUNT				(GROUP_COUNT * SYM_COUNT)
#define TIME_SENSORS			3
#define INPUT_SENSORS			(4 * 4 * 2)
#define SIGNAL_SENSORS			3
#define SENSOR_SIZE				(SYM_COUNT * INPUT_SENSORS)
#define GROUP_SIGNAL_SIZE		(SYM_COUNT * SIGNAL_SENSORS)
#define SIGNAL_SIZE				(GROUP_SIGNAL_SIZE * GROUP_COUNT)
#define AGENT_STATES			(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE)
#define AGENT_ACTIONCOUNT		24
#define AGENT_RESULT_COUNT		1000
#define JOINER_COUNT			4 //TODO
#define JOINERSIGNAL_SENSORS	5
#define JOINER_RESULT_COUNT		1000
#define JOINERSIGNAL_SIZE		(JOINER_COUNT * JOINERSIGNAL_SENSORS)
#define JOINER_STATES			(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE + JOINERSIGNAL_SIZE)
#define JOINER_NORMALACTS		(3*3*3*3)
#define JOINER_IDLEACTS			8
#define JOINER_ACTIONCOUNT		(JOINER_NORMALACTS + JOINER_IDLEACTS)

class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	float year_timesensor;
	float week_timesensor;
	float day_timesensor;
	float sensor			[SENSOR_SIZE];
	float signal			[SIGNAL_SIZE];
	float open				[SYM_COUNT];
	float joiner_signal		[JOINERSIGNAL_SIZE];
};

struct FixedOrder {
	float open = 0.0;
	float close = 0.0;
	float volume = 0.0;
	float profit = 0.0;
	int type = -1;
	int is_open = 0;
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
	void OrderSend(int type, float volume, float price) PARALLEL;
	void OrderClose(const Snapshot& snap) PARALLEL;
	double GetCloseProfit(const Snapshot& snap) const PARALLEL;
	float RealtimeBid(const Snapshot& snap, int sym_id) const PARALLEL;
	float RealtimeAsk(const Snapshot& snap, int sym_id) const PARALLEL;
	double AccountEquity() const PARALLEL {return equity;}
	double GetDrawdown() const PARALLEL {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
};


struct FixedSimBroker {
	#define ORDERS_PER_SYMBOL	3
	#define MAX_ORDERS			(ORDERS_PER_SYMBOL * SYM_COUNT)
	
	FixedOrder order[MAX_ORDERS];
	int signal[SYM_COUNT];
	int proxy_id[SYM_COUNT];
	int proxy_base_mul[SYM_COUNT];
	double balance = 0.0;
	double equity = 0.0;
	double begin_equity = 0.0;
	double spread_points[SYM_COUNT];
	double profit_sum = 0.0;
	double loss_sum = 0.0;
	double free_margin_level = 0.80;
	double leverage = 1000.0;
	int order_count = 0;
	
	
	FixedSimBroker();
	bool Cycle(const Snapshot& snap) PARALLEL;
	void RefreshOrders(const Snapshot& snap) PARALLEL;
	void Reset() PARALLEL;
	void OrderSend(int sym_id, int type, float volume, float price) PARALLEL;
	//void OrderClose(int sym_id, const Snapshot& snap) PARALLEL;
	void OrderClose(int sym_id, double lots, FixedOrder& order, const Snapshot& snap) PARALLEL;
	void CloseAll(const Snapshot& snap) PARALLEL;
	double GetMargin(const Snapshot& snap, int sym_id, double volume) PARALLEL;
	double GetCloseProfit(int sym_id, const FixedOrder& order, const Snapshot& snap) const PARALLEL;
	float RealtimeBid(const Snapshot& snap, int sym_id) const PARALLEL;
	float RealtimeAsk(const Snapshot& snap, int sym_id) const PARALLEL;
	double AccountEquity() const PARALLEL {return equity;}
	double GetDrawdown() const PARALLEL {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
	void SetSignal(int sym_id, int sig) {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); signal[sym_id] = sig;}
};


struct TraineeBase {
	
	// Persistent
	float result[AGENT_RESULT_COUNT];
	double best_result = 0.0;
	double last_drawdown = 0.0;
	long iter = 0;
	int result_cursor = 0;
	int result_count = 0;
	int id = -1;
	int cursor = 0;
	
	
	// Temporary
	double reward_sum = 0;
	double average_reward = 0;
	float prev_equity = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	int type = -1;
	bool is_training = false;
	
	
	
	TraineeBase();
	void Create();
	void ResetEpoch();
	
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		else if (s.IsStoring()) {
			s.Put(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		s % best_result % last_drawdown % iter % result_cursor % result_count % id % cursor;
	}
};

struct Agent : Moveable<Agent>, public TraineeBase {
	
	// Persistent
	DQNAgent<AGENT_ACTIONCOUNT, AGENT_STATES> dqn;
	double begin_equity = 0.0;
	float spread_points = 0;
	int sym_id = -1;
	int sym__ = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	int group_id = -1;
	
	
	// Temporary
	SingleFixedSimBroker broker;
	
	
	Agent();
	void Create();
	void Init();
	void ResetEpoch();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Backward(double reward) PARALLEL;
	void WriteSignal(Snapshot& cur_snap) PARALLEL;
	
	void Serialize(Stream& s) {
		TraineeBase::Serialize(s);
		s % dqn % begin_equity % spread_points % sym_id
		  % sym__ % proxy_id % proxy_base_mul % group_id;
	}
};

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
	double prev_signals[4];
	
	
	Joiner();
	void Create();
	void Init();
	void ResetEpoch();
	void Main(const array_view<Snapshot, 1>& snap_view) PARALLEL;
	void Forward(const array_view<Snapshot, 1>& snap_view) PARALLEL;
	void Backward(double reward) PARALLEL;
	void WriteSignal(Snapshot& cur_snap) PARALLEL;
	
	void Serialize(Stream& s) {
		TraineeBase::Serialize(s);
		s % dqn % begin_equity % leverage;
		if (s.IsLoading()) {
			s.Get(spread_points, sizeof(double) * SYM_COUNT);
			s.Get(proxy_id, sizeof(int) * SYM_COUNT);
			s.Get(proxy_base_mul, sizeof(int) * SYM_COUNT);
		}
		if (s.IsStoring()) {
			s.Put(spread_points, sizeof(double) * SYM_COUNT);
			s.Put(proxy_id, sizeof(int) * SYM_COUNT);
			s.Put(proxy_base_mul, sizeof(int) * SYM_COUNT);
		}
	}
};

class AgentGroup {
	
	
public:
	
	// Persistent
	Vector<Agent> agents;
	Vector<Joiner> joiners;
	Vector<FactoryDeclaration> indi_ids;
	Time created;
	int phase = 0;
	int group_count = 0;
	
	
	// Temp
	Vector<Snapshot> amp_snaps;
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Snapshot> snaps;
	Vector<Core*> databridge_cores;
	Vector<double> agent_equities, joiner_equities;
	Index<String> allowed_symbols;
	Index<int> sym_ids;
	TimeStop last_store, last_datagather;
	System* sys;
	double epsilon;
	int main_tf = -1;
	int data_begin = 0;
	int buf_count = 0;
	int sym_count = 0;
	int max_memory_size = 0, agents_total_size = 0, memory_for_snapshots = 0, snaps_per_phase = 0, snap_phase_count = 0;
	int snap_phase_id = 0;
	int counted_bars = 0;
	int snap_begin = 0;
	bool running = false, stopped = true;
	Mutex work_lock;
	
	enum {PHASE_SEEKSNAPS, PHASE_TRAINING, PHASE_JOINER};
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup(System* sys);
	~AgentGroup();
	
	
	void Init();
	void InitThread();
	void InitAgent(Agent& a);
	void InitJoiner(Joiner& j);
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
	void CreateJoiners();
	double GetAverageDrawdown();
	double GetAverageIterations();
	double GetEpsilon() {return epsilon;}
	void RefreshEpsilon();
	
	void Main();
	void SetAgentsTraining(bool b);
	
	
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
