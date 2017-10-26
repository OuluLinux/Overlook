#if 0

#ifndef _Overlook_AgentSystem_h_
#define _Overlook_AgentSystem_h_

namespace Overlook {

#ifndef flagHAVE_ALLSYM
#define SYM_COUNT					6
#else
#define SYM_COUNT					19
#endif


#if defined flagVERY_FAST
#define MEASURE_PERIOD(j)			(1 << (4 + j))
#define RESULT_EXTRACENTERS			8
#define GROUP_COUNT					22
#define HAVE_POLES					0
#define FUSE_TRIGGERPERIOD			(+2)
#elif defined flagFAST
#define MEASURE_PERIOD(j)			(1 << (6 + j))
#define RESULT_EXTRACENTERS			20
#define GROUP_COUNT					55
#define HAVE_POLES					1
#define FUSE_TRIGGERPERIOD			(-1)
#else
#define MEASURE_PERIOD(j)			(1 << (8 + j))
#define RESULT_EXTRACENTERS			8
#define GROUP_COUNT					22
#define HAVE_POLES					1
#define FUSE_TRIGGERPERIOD			(-2)
#endif

#ifdef flagSKIP_IDLEACT
#define IDLEACT						0
#else
#define IDLEACT						1
#endif

#define FMLEVEL						0.6
#define BASE_FWDSTEP_BEGIN			5
#define OUTPUT_COUNT				GROUP_COUNT
#define INPUT_COUNT					150
#define TIME_SENSORS				3
#define INPUT_SENSORS				(3 * 5 * 2)
#define SENSOR_SIZE					(TIME_SENSORS + SYM_COUNT * INPUT_SENSORS)
#define RANDOM_TIMESTEPS			8
#define REWARD_AV_PERIOD			400
#define MAX_LEARNING_RATE			0.0100
#define MIN_LEARNING_RATE			0.0025
#define BREAK_INTERVAL_ITERS		40000
#define MAX_EXTRA_TIMESTEPS			3
#define VOLAT_DIV					0.001
#define CHANGE_DIV					0.0001
#define VOLINTMUL					10 // = VOLAT_DIV / CHANGE_DIV
#define DD_LIMIT					20.0

#define MEASURE_PERIODCOUNT			3
#define MEASURE_SIZE				(MEASURE_PERIODCOUNT * SYM_COUNT)
#define RESULT_SIZE					(SYM_COUNT * OUTPUT_COUNT)
#define RESULT_BYTES				(RESULT_SIZE / 8 + 1)
#define TARGET_COUNT				3
#define TARGET_SIZE					(SYM_COUNT * OUTPUT_COUNT * TARGET_COUNT)
#define TARGET_BYTES				(TARGET_SIZE / 8 + 1)

#define TRAINEE_RESULT_COUNT		1000
#define TRAINEE_COUNT				(GROUP_COUNT * SYM_COUNT)

#define SIGNAL_STATES				SENSOR_SIZE
#define SIGNAL_ACTIONCOUNT			(IDLEACT + TARGET_COUNT)
#define SIGNAL_PHASE_ITER_LIMIT		100000
#define SIGNAL_EPS_ITERS_STEP		10000

#define AMP_STATES					SENSOR_SIZE
#define AMP_MAXSCALES				3
#define AMP_MAXSCALE_MUL			2
#define AMP_ACTIONCOUNT				AMP_MAXSCALES
#define AMP_PHASE_ITER_LIMIT		100000
#define AMP_EPS_ITERS_STEP			10000

#define FUSE_BROKERCOUNT			4
#define FUSE_AVCOUNT				2
#define FUSE_TRIGGERCOUNT			(FUSE_BROKERCOUNT * FUSE_AVCOUNT)
#define FUSE_TRIGGERLOSS			10
#define FUSE_ALLOWZEROSIGNAL		false
#define FUSE_STATES					(2 * FUSE_BROKERCOUNT * (1 + FUSE_AVCOUNT))
#define FUSE_ACTIONCOUNT			((1 + FUSE_BROKERCOUNT) * FUSE_AVCOUNT)
#define FUSE_PHASE_ITER_LIMIT		400000
#define FUSE_EPS_ITERS_STEP			40000

class AgentSystem;

struct ResultTuple : Moveable<ResultTuple> {
	int16 change, volat;
	
	ResultTuple() : change(0), volat(0) {}
	ResultTuple(int16 c, int16 v) : change(c), volat(v) {}
	uint32 GetHashValue() const {
		CombineHash ch;
		ch << change << 1 << volat << 1;
		return ch;
	}
	
	void Serialize(Stream& s) {s % change % volat;}
	bool operator == (const ResultTuple& b) const {return b.change == change && b.volat == volat;}
	bool operator < (const ResultTuple& b) const {return volat < b.volat || (abs((int)change) < abs((int)b.change) && change == b.change);}
	bool operator()(const ResultTuple& a, const ResultTuple& b) const {return volat < b.volat;}
};

struct ResultTupleCounter : Moveable<ResultTupleCounter> {
	int16 change, volat;
	int count;
	
	ResultTupleCounter() : change(0), volat(0), count(0) {}
	ResultTupleCounter(const ResultTuple& t, int count) : change(t.change), volat(t.volat), count(count) {}
	void Serialize(Stream& s) {s % change % volat % count;}
	uint32 GetHashValue() const {
		CombineHash ch;
		ch << change << 1 << volat << 1 << count << 1;
		return ch;
	}
};


struct IndicatorTuple : Moveable<IndicatorTuple> {
	uint8 values				[SENSOR_SIZE];
	
	IndicatorTuple() {
		for(int i = 0; i < SENSOR_SIZE; i++)
			values[i] = 0;
	}
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(values, SENSOR_SIZE);
		}
		else if (s.IsStoring()) {
			s.Put(values, SENSOR_SIZE);
		}
	}
	uint32 GetHashValue() const {
		CombineHash ch;
		for(int i = 0; i < SENSOR_SIZE; i++)
			 ch << values[i] << 1;
		return ch;
	}
	String ToString() const {
		String s;
		for(int i = 0; i < SENSOR_SIZE; i++) {
			if (i) s << ", ";
			s << values[i] / 255.0;
		}
		return s;
	}
	uint32 GetDistance(const IndicatorTuple& it) const {
		uint32 sqr_sum = 0;
		for(int i = 0; i < SENSOR_SIZE; i++) {
			int dist = values[i] - it.values[i];
			sqr_sum += dist * dist;
		}
		return root(sqr_sum);
	}
};

#include "Snapshot.h"

struct SectorConnection : Moveable<SectorConnection> {
	int sym_id;
	int tf_id;
	int c_id;
	
	SectorConnection() : sym_id(0), tf_id(0), c_id(0) {}
	SectorConnection(int sym_id, int tf_id, int c_id) : sym_id(sym_id), tf_id(tf_id), c_id(c_id) {}
	void Serialize(Stream& s) {s % sym_id % tf_id % c_id;}
	uint32 GetHashValue() const {
		CombineHash ch;
		ch << sym_id << 1 << tf_id << 1 << c_id << 1;
		return ch;
	}
	
	bool operator == (const SectorConnection& b) const {return b.sym_id == sym_id && b.tf_id == tf_id && b.c_id == c_id;}
	bool operator < (const SectorConnection& b) const {return sym_id < b.sym_id || (sym_id == b.sym_id && tf_id < b.tf_id) || (sym_id == b.sym_id && tf_id == b.tf_id && c_id < b.c_id);}
	bool operator()(const SectorConnection& a, const SectorConnection& b) const {return *this < b;}
};

struct ResultSector : Moveable<ResultSector> {
	VectorMap<SectorConnection, int> src_conns;
	VectorMap<int, int> sector_conn_counts;
	AveragePoint pos[TARGET_COUNT];
	AveragePoint neg[TARGET_COUNT];
	AveragePoint pnd;
	int conn_total = 0;
	
	
	ResultSector() {}
	void Serialize(Stream& s) {
		for(int i = 0; i < TARGET_COUNT; i++) s % pos[i] % neg[i];
		s % src_conns % sector_conn_counts % pnd % conn_total;
	}
	void AddSource(int sym_id, int tf_id, int indi_c_id) {
		src_conns.GetAdd(SectorConnection(sym_id, tf_id, indi_c_id), 0)++;
		sector_conn_counts.GetAdd(indi_c_id, 0)++;
		conn_total++;
	}
	void Sort() {SortByValue(sector_conn_counts, StdGreater<int>());}
};

struct IndicatorSector : Moveable<IndicatorSector> {
	VectorMap<SectorConnection, int> dst_conns;
	VectorMap<int, int> sector_conn_counts;
	int conn_total = 0;
	int result_id = -1;
	
	
	IndicatorSector() {}
	void Serialize(Stream& s) {s % dst_conns % sector_conn_counts % conn_total % result_id;}
	void AddDestination(int sym_id, int tf_id, int result_c_id) {
		dst_conns.GetAdd(SectorConnection(sym_id, tf_id, result_c_id), 0)++;
		sector_conn_counts.GetAdd(result_c_id, 0)++;
		conn_total++;
	}
	void Sort() {SortByValue(sector_conn_counts, StdGreater<int>());}
	int GetResultSector() const {return sector_conn_counts.GetKey(0);}
	double GetResultProbability() const {return (double)sector_conn_counts[0] / conn_total;}
	bool IsEmpty() const {return sector_conn_counts.IsEmpty();}
};

#include "AgentGroup.h"


extern bool reset_signals;
extern bool reset_amps;
extern bool reset_fuse;

class AgentSystem {


public:

	// Persistent
	Vector<AgentGroup>			groups;
	Vector<FactoryDeclaration>	indi_ids;
	Vector<ResultTuple>			result_centroids;
	Vector<IndicatorTuple>		indi_centroids;
	Vector<ResultSector>		result_sectors;
	Vector<IndicatorSector>		indi_sectors;
	AgentFuse					fuse;
	Time created;
	int phase = 0;
	bool initial_result_clustering = true;
	bool initial_indicator_clustering = true;


	// Temp
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Snapshot> snaps;
	Vector<Core*> databridge_cores;
	Vector<double> spread_points;
	Vector<int> proxy_id, proxy_base_mul;
	VectorMap<String, int> allowed_symbols;
	Index<int> sym_ids;
	TimeStop last_store, last_datagather;
	System* sys;
	double signal_epsilon = 0.0, amp_epsilon = 0.0, fuse_epsilon = 0.0;
	double begin_equity = 10000.0;
	double leverage = 1000.0;
	double free_margin_level = FMLEVEL;
	int main_tf = -1;
	int data_begin = 0;
	int buf_count = 0;
	int sym_count = 0;
	int counted_bars = 0;
	int prev_shift = 0;
	int realtime_count = 0;
	int result_cluster_counter = 0;
	int indi_cluster_counter = 0;
	bool running = false, stopped = true;
	bool trigger_retrain = false;
	Mutex work_lock;
	
	
public:
	typedef AgentSystem CLASSNAME;
	AgentSystem(System* sys);
	~AgentSystem();


	void Init();
	void InitThread();
	void InitBrokerValues();
	void Start();
	void Stop();
	void StoreThis();
	void LoadThis();
	void SetSignalEpsilon(double d);
	void SetAmpEpsilon(double d);
	void SetFuseEpsilon(double d);
	void SetFreeMarginLevel(double d);
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
	double GetAverageSignalDrawdown();
	double GetAverageSignalIterations();
	double GetAverageSignalDeepIterations();
	double GetAverageAmpDrawdown();
	double GetAverageAmpIterations();
	double GetAverageAmpDeepIterations();
	double GetAverageAmpEpochs();
	double GetAverageFuseDrawdown() {return fuse.GetLastDrawdown();}
	double GetAverageFuseIterations() {return fuse.iter;}
	double GetAverageFuseDeepIterations() {return fuse.deep_iter;}
	double GetAverageIterations(int phase);
	double GetAverageDeepIterations(int phase);
	double GetSignalEpsilon() const {return signal_epsilon;}
	double GetAmpEpsilon() const {return amp_epsilon;}
	double GetFuseEpsilon() const {return fuse_epsilon;}
	double GetPhaseIters(int phase);
	void RefreshAgentEpsilon(int phase);
	void RefreshLearningRate(int phase);
	void TrainAgents(int phase);
	void TrainFuse();
	void MainReal();
	void Data();
	void RefreshSnapEquities();
	void LoopAgentSignals(int phase);
	void LoopAgentSignalsAll(bool from_begin);
	int FindActiveGroup(int sym_id);
	bool PutLatest(Brokerage& broker, Vector<Snapshot>& snaps);
	void Main();
	void SetAgentsTraining(bool b);
	void SetSingleFixedBroker(int sym_id, SingleFixedSimBroker& broker);
	void SetFixedBroker(int sym_id, FixedSimBroker& broker);
	void ReduceExperienceMemory(int phase);
	void RefreshClusters();
	void RefreshResultClusters();
	void RefreshIndicatorClusters();
	void RefreshClusterConnections();
	void RefreshPoleNavigation();
	void ResetAgents();
	void InitAgents();
	void AnalyzeSectorPoles(int snap_begin, int snap_end, int sym_id, int result_c_id);
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
	
};

}

#endif
#endif
