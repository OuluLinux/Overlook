#ifndef _Overlook_AgentSystem_h_
#define _Overlook_AgentSystem_h_

namespace Overlook {

#ifndef flagHAVE_ALLSYM
#define SYM_COUNT					6
#else
#define SYM_COUNT					24
#endif

#define FMLEVEL						0.6
#define BASE_FWDSTEP_BEGIN			5
#define GROUP_COUNT					24
#define INPUT_COUNT					(GROUP_COUNT * 5)
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

#define MEASURE_PERIODCOUNT			6
#define MEASURE_SIZE				(MEASURE_PERIODCOUNT * SYM_COUNT)

#define TRAINEE_RESULT_COUNT		1000
#define TRAINEE_COUNT				(GROUP_COUNT * SYM_COUNT)

#define SIGNAL_STATES				SENSOR_SIZE
#define SIGNAL_ACTIONCOUNT			2
#define SIGNAL_PHASE_ITER_LIMIT		300000
#define SIGNAL_EPS_ITERS_STEP		10000

#define AMP_STATES					SENSOR_SIZE
#define AMP_MAXSCALES				3
#define AMP_MAXSCALE_MUL			2
#define AMP_ACTIONCOUNT				2
#define AMP_PHASE_ITER_LIMIT		300000
#define AMP_EPS_ITERS_STEP			10000

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
	
	bool operator == (const ResultTuple& b) const {return b.change == change && b.volat == volat;}
	bool operator < (const ResultTuple& b) const {return volat < b.volat || (volat == b.volat && change < b.change);}
	bool operator()(const ResultTuple& a, const ResultTuple& b) const {return a.volat < b.volat;}
};

struct ResultTupleCounter : Moveable<ResultTupleCounter> {
	int16 change, volat;
	int count;
	
	ResultTupleCounter() : change(0), volat(0), count(0) {}
	ResultTupleCounter(const ResultTuple& t, int count) : change(t.change), volat(t.volat), count(count) {}
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


/*struct IndicatorTupleCounter : Moveable<IndicatorTupleCounter> {
	uint8 values				[SENSOR_SIZE];
	int count;
	
	IndicatorTupleCounter() {
		for(int i = 0; i < SENSOR_SIZE; i++)
			values[i] = 0;
		count = 0;
	}
	IndicatorTupleCounter(IndicatorTuple& t, int count) {
		memcpy(values, t.value, SENSOR_SIZE);
		this->count = count;
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
};*/

class Snapshot : Moveable<Snapshot> {

private:
	IndicatorTuple sensors;
	ResultTuple result_bwd		[MEASURE_SIZE];
	uint8 result_cluster_id		[MEASURE_SIZE];
	uint8 indi_cluster_id;
	
	double open					[SYM_COUNT];
	double change				[SYM_COUNT];
	
	int signal_broker_symsig	[TRAINEE_COUNT];
	int amp_broker_symsig		[TRAINEE_COUNT];
	int shift;

	inline int GetSignalOutput(int i) const {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		return signal_broker_symsig[i];
	}

	inline void SetSignalOutput(int i, int j) {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		signal_broker_symsig[i] = j;
	}

	inline int GetAmpOutput(int i) const {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		return amp_broker_symsig[i];
	}

	inline void SetAmpOutput(int i, int j) {
		ASSERT(i >= 0 && i < TRAINEE_COUNT);
		amp_broker_symsig[i] = j;
	}

public:
	void Reset() {
		for (int i = 0; i < TRAINEE_COUNT; i++)
			signal_broker_symsig[i]		= 0;

		for (int i = 0; i < TRAINEE_COUNT; i++)
			amp_broker_symsig[i]		= 0;
	}
	
	int GetResultCluster(int sym, int tf) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_cluster_id	[sym * MEASURE_PERIODCOUNT + tf];
	}
	
	void SetResultCluster(int sym, int tf, int cl) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_cluster_id	[sym * MEASURE_PERIODCOUNT + tf] = cl;
	}
	
	int GetIndicatorCluster() {
		return indi_cluster_id;
	}
	
	void SetIndicatorCluster(int cl) {
		indi_cluster_id = cl;
	}
	
	const ResultTuple& GetResultTuple(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf];
	}
	
	const IndicatorTuple& GetIndicatorTuple() const {
		return sensors;
	}
	
	void   SetPeriodChange(int sym, int tf, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_bwd[sym * MEASURE_PERIODCOUNT + tf].change = d / CHANGE_DIV;
	}

	double GetPeriodChange(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].change * CHANGE_DIV;
	}

	int    GetPeriodChangeInt(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].change;
	}

	void   SetPeriodVolatility(int sym, int tf, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat = d / VOLAT_DIV;
	}

	double GetPeriodVolatility(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat * VOLAT_DIV;
	}

	int    GetPeriodVolatilityInt(int sym, int tf) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(tf >= 0 && tf < MEASURE_PERIODCOUNT);
		return result_bwd[sym * MEASURE_PERIODCOUNT + tf].volat;
	}
	
	inline double GetSensor(int sym, int in) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(in >= 0 && in < INPUT_SENSORS);
		return sensors.values[TIME_SENSORS + sym * INPUT_SENSORS + in] / 255.0;
	}

	inline void SetSensor(int sym, int in, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		ASSERT(in >= 0 && in < INPUT_SENSORS);
		sensors.values[TIME_SENSORS + sym * INPUT_SENSORS + in] = Upp::max(0, Upp::min(255, (int)(d * 255.0)));
	}

	inline double GetSensorUnsafe(int i) const {
		return sensors.values[i] / 255.0;
	}

	inline double GetTimeSensor(int i) const {
		ASSERT(i >= 0 && i < TIME_SENSORS);
		return sensors.values[i] / 255.0;
	}
	
	inline void SetTimeSensor(int i, double d) {
		ASSERT(i >= 0 && i < TIME_SENSORS);
		sensors.values[i] = Upp::max(0, Upp::min(255, (int)(d * 255.0)));
	}
	
	inline double GetOpen(int sym) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return open[sym];
	}

	inline void   SetOpen(int sym, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		open[sym] = d;
	}

	inline double GetChange(int sym) const {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return change[sym];
	}

	inline void   SetChange(int sym, double d) {
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		change[sym] = d;
	}


	inline int GetSignalOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetSignalOutput(group * SYM_COUNT + sym);
	}

	inline void SetSignalOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetSignalOutput(group * SYM_COUNT + sym, i);
	}

	inline int GetAmpOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetAmpOutput(group * SYM_COUNT + sym);
	}

	inline void SetAmpOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT);
		ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetAmpOutput(group * SYM_COUNT + sym, i);
	}


	int GetShift() const {
		return shift;
	}

	void SetShift(int shift) {
		this->shift = shift;
	}

};


#include "AgentGroup.h"


extern bool reset_signals;
extern bool reset_amps;

class AgentSystem {


public:

	// Persistent
	Vector<AgentGroup> groups;
	Vector<FactoryDeclaration> indi_ids;
	Time created;
	int phase = 0;


	// Temp
	Vector<ResultTuple > result_centroids;
	Vector<IndicatorTuple> indi_centroids;
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
	double signal_epsilon = 0.0, amp_epsilon = 0.0;
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
	bool initial_result_clustering = true;
	bool initial_indicator_clustering = true;
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
	double GetAverageIterations(int phase);
	double GetAverageDeepIterations(int phase);

	double GetSignalEpsilon() const {
		return signal_epsilon;
	}

	double GetAmpEpsilon() const {
		return amp_epsilon;
	}

	double GetPhaseIters(int phase);
	void RefreshAgentEpsilon(int phase);
	void RefreshLearningRate(int phase);
	void TrainAgents(int phase);
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
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
	
};

}

#endif
