#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define GROUP_COUNT			2
#define SYM_COUNT			21
#define TIME_SENSORS		3
#define INPUT_SENSORS		(4 * 4 * 2)
#define SIGNAL_SENSORS		3
#define SENSOR_SIZE			(SYM_COUNT * INPUT_SENSORS)
#define GROUP_SIGNAL_SIZE	(SYM_COUNT * SIGNAL_SENSORS)
#define SIGNAL_SIZE			GROUP_SIGNAL_SIZE * GROUP_COUNT
#define AGENT_STATES		(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE)
#define AGENT_ACTIONCOUNT	24
#define AGENT_RESULT_COUNT	1000
#define JOINER_STATES		AGENT_STATES
#define JOINER_ACTIONCOUNT	1

class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	float year_timesensor, week_timesensor, day_timesensor;
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
	double buy_lots[SYM_COUNT];
	double sell_lots[SYM_COUNT];
	int buy_signals[SYM_COUNT];
	int sell_signals[SYM_COUNT];
	int signal[SYM_COUNT];
	int proxy_id[SYM_COUNT];
	int proxy_base_mul[SYM_COUNT];
	int proxy_factor[SYM_COUNT];
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
	void Cycle(const Snapshot& snap) PARALLEL;
	void RefreshOrders(const Snapshot& snap) PARALLEL;
	void Reset() PARALLEL;
	void OrderSend(int sym_id, int type, float volume, float price) PARALLEL;
	void OrderClose(int sym_id, const Snapshot& snap) PARALLEL;
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
	int result_cursor = 0;
	int result_count = 0;
	int id = -1;
	int cursor = 0;
	
	
	// Temporary
	float prev_equity = 0;
	int signal = 0;
	int timestep_actual = 0;
	int timestep_total = 1;
	
	
	
	TraineeBase();
	void Create();
	
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		else if (s.IsStoring()) {
			s.Put(result, sizeof(float) * AGENT_RESULT_COUNT);
		}
		s % best_result % last_drawdown % result_cursor % result_count % id % cursor;
	}
};

struct Agent : Moveable<Agent>, public TraineeBase {
	
	// Persistent
	DQNAgent<AGENT_ACTIONCOUNT, AGENT_STATES> dqn;
	double begin_equity = 0.0;
	float spread_points = 0;
	int64 iter = 0;
	int sym_id = -1;
	int sym__ = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	int group_id = -1;
	
	
	
	// Temporary
	SingleFixedSimBroker broker;
	double average_reward = 0;
	double reward_sum = 0;
	bool is_training = false;
	
	
	Agent();
	void Create();
	void Init();
	void ResetEpoch();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Forward(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL;
	void Backward(Snapshot& snap, double reward) PARALLEL;
	void WriteSignal(Snapshot& cur_snap) PARALLEL;
	
	void Serialize(Stream& s) {
		TraineeBase::Serialize(s);
		s % dqn % begin_equity % spread_points % iter % sym_id
		  % sym__ % proxy_id % proxy_base_mul % group_id;
	}
};

struct Joiner : Moveable<Joiner>, public TraineeBase {
	
	// Persistent
	DQNAgent<JOINER_STATES, JOINER_STATES> dqn;
	
	
	// Temporary
	FixedSimBroker broker;
	
	
	Joiner();
	void ResetEpoch();
	void Main(Snapshot& cur_snap, Snapshot& prev_snap);
	
	void Serialize(Stream& s) {
		TraineeBase::Serialize(s);
		s % dqn;
	}
};

class AgentGroup {
	
	
public:
	
	// Persistent
	Vector<Agent> agents;
	Joiner joiner;
	Vector<FactoryDeclaration> indi_ids;
	Time created;
	int phase = 0;
	int group_count = 0;
	
	
	// Temp
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
	bool running = false, stopped = true;
	Mutex work_lock;
	
	enum {PHASE_SEEKSNAPS, PHASE_TRAINING, PHASE_JOINER, PHASE_UPDATE, PHASE_REAL, PHASE_WAIT};
	
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
