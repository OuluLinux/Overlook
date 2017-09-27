#ifndef _Overlook_AgentSystem_h_
#define _Overlook_AgentSystem_h_

namespace Overlook {

#ifndef flagHAVE_ALLSYM
	#define SYM_COUNT				6
#else
	#define SYM_COUNT				24
#endif

#define FMLEVEL						0.6
#define BASE_FWDSTEP_BEGIN			5
#define GROUP_COUNT					8
#define TIME_SENSORS				3
#define INPUT_SENSORS				(6 * 4)
#define SENSOR_SIZE					(SYM_COUNT * INPUT_SENSORS)
#define RANDOM_TIMESTEPS			8
#define REWARD_AV_PERIOD			400
#define MAX_LEARNING_RATE			0.0100
#define MIN_LEARNING_RATE			0.0025
#define BREAK_INTERVAL_ITERS		40000
#define MAX_EXTRA_TIMESTEPS			3

#define TRAINEE_RESULT_COUNT		1000
#define TRAINEE_COUNT				(GROUP_COUNT * SYM_COUNT)
#define FILTER_TRAINEE_COUNT		(TRAINEE_COUNT * FILTER_COUNT)

#define FILTER_COUNT				3
#define FILTER_SENSORS				2
#define FILTER_SIZE					(TRAINEE_COUNT * FILTER_SENSORS * FILTER_COUNT)
#define FILTER_LEVEL_SIZE			(TRAINEE_COUNT * FILTER_SENSORS)
#define FILTER_GROUP_SIZE			(SYM_COUNT * FILTER_SENSORS)
#define FILTER_STATES				(TIME_SENSORS + SENSOR_SIZE + FILTER_LEVEL_SIZE)
#define FILTER_POS_FWDSTEPS			5
#define FILTER_ZERO_FWDSTEPS		5
#define FILTER_ACTIONCOUNT			(FILTER_POS_FWDSTEPS + FILTER_ZERO_FWDSTEPS)
#define FILTER_PHASE_ITER_LIMIT		200000
#define FILTER_EPS_ITERS_STEP		10000

#define SIGNAL_SENSORS				2
#define SIGNAL_SIZE					(TRAINEE_COUNT * SIGNAL_SENSORS)
#define SIGNAL_GROUP_SIZE			(SYM_COUNT * SIGNAL_SENSORS)
#define SIGNAL_STATES				(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE)
#define SIGNAL_POS_FWDSTEPS			3
#define SIGNAL_NEG_FWDSTEPS			3
#define SIGNAL_ACTIONCOUNT			(SIGNAL_POS_FWDSTEPS + SIGNAL_NEG_FWDSTEPS)
#define SIGNAL_PHASE_ITER_LIMIT		300000
#define SIGNAL_EPS_ITERS_STEP		10000

#define FUSE_SENSORS				2
#define FUSE_SIZE					(TRAINEE_COUNT * FUSE_SENSORS)
#define FUSE_GROUP_SIZE				(SYM_COUNT * FUSE_SENSORS)
#define FUSE_STATES					(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE + FUSE_SIZE)
#define FUSE_FWDSTEPS				5
#define FUSE_ACTIONCOUNT			(2 * FUSE_FWDSTEPS)
#define FUSE_PHASE_ITER_LIMIT		300000
#define FUSE_EPS_ITERS_STEP			10000

#define AMP_SENSORS					3
#define AMP_SIZE					(TRAINEE_COUNT * AMP_SENSORS)
#define AMP_GROUP_SIZE				(SYM_COUNT * AMP_SENSORS)
#define AMP_STATES					(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE + FUSE_SIZE + AMP_SIZE)
#define AMP_FWDSTEP_BEGIN			BASE_FWDSTEP_BEGIN
#define AMP_FWDSTEPS				5
#define AMP_MAXSCALES				3
#define AMP_MAXSCALE_MUL			2
#define AMP_ACTIONCOUNT				(AMP_MAXSCALES * AMP_FWDSTEPS)
#define AMP_PHASE_ITER_LIMIT		300000
#define AMP_EPS_ITERS_STEP			10000

class AgentSystem;

class Snapshot : Moveable<Snapshot> {
	
private:
	double year_timesensor;
	double week_timesensor;
	double day_timesensor;
	double sensors				[SENSOR_SIZE];
	double open					[SYM_COUNT];
	double change				[SYM_COUNT];
	double signal_sensors		[SIGNAL_SIZE];
	double amp_sensors			[AMP_SIZE];
	double fuse_sensors			[FUSE_SIZE];
	double filter_sensors		[FILTER_SIZE];
	int signal_broker_symsig	[TRAINEE_COUNT];
	int amp_broker_symsig		[TRAINEE_COUNT];
	int fuse_broker_symsig		[TRAINEE_COUNT];
	int filter_broker_symsig	[FILTER_TRAINEE_COUNT];
	int shift;
	
	inline int GetSignalOutput(int i) const		{ASSERT(i >= 0 && i < TRAINEE_COUNT); return signal_broker_symsig[i];}
	inline void SetSignalOutput(int i, int j)	{ASSERT(i >= 0 && i < TRAINEE_COUNT); signal_broker_symsig[i] = j;}
	inline int GetAmpOutput(int i) const		{ASSERT(i >= 0 && i < TRAINEE_COUNT); return amp_broker_symsig[i];}
	inline void SetAmpOutput(int i, int j)		{ASSERT(i >= 0 && i < TRAINEE_COUNT); amp_broker_symsig[i] = j;}
	inline int GetFuseOutput(int i) const		{ASSERT(i >= 0 && i < TRAINEE_COUNT); return fuse_broker_symsig[i];}
	inline void SetFuseOutput(int i, int j)		{ASSERT(i >= 0 && i < TRAINEE_COUNT); fuse_broker_symsig[i] = j;}
	inline int GetFilterOutput(int i) const		{ASSERT(i >= 0 && i < FILTER_TRAINEE_COUNT); return filter_broker_symsig[i];}
	inline void SetFilterOutput(int i, int j)	{ASSERT(i >= 0 && i < FILTER_TRAINEE_COUNT); filter_broker_symsig[i] = j;}
	
public:
	
	void Reset() {
		for(int i = 0; i < SIGNAL_SIZE; i++)			signal_sensors[i]			= 1.0;
		for(int i = 0; i < AMP_SIZE; i++)				amp_sensors[i]				= 1.0;
		for(int i = 0; i < FILTER_SIZE; i++)			filter_sensors[i]			= 1.0;
		for(int i = 0; i < TRAINEE_COUNT; i++)			signal_broker_symsig[i]		= 0;
		for(int i = 0; i < TRAINEE_COUNT; i++)			amp_broker_symsig[i]		= 0;
		for(int i = 0; i < TRAINEE_COUNT; i++)			fuse_broker_symsig[i]		= 0;
		for(int i = 0; i < FILTER_TRAINEE_COUNT; i++)	filter_broker_symsig[i]		= 0;
	}
	
	double GetYearSensor() const {return year_timesensor;}
	double GetWeekSensor() const {return week_timesensor;}
	double GetDaySensor()  const {return day_timesensor;}
	
	void SetYearSensor(double d) {year_timesensor = d;}
	void SetWeekSensor(double d) {week_timesensor = d;}
	void SetDaySensor(double d)  {day_timesensor = d;}
	
	inline double GetSensor(int sym, int in) const     {ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(in >= 0 && in < INPUT_SENSORS); return sensors[sym * INPUT_SENSORS + in];}
	inline void   SetSensor(int sym, int in, double d) {ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(in >= 0 && in < INPUT_SENSORS); sensors[sym * INPUT_SENSORS + in] = d;}
	inline double GetSensorUnsafe(int i) const {return sensors[i];}
	
	inline double GetOpen(int sym) const {ASSERT(sym >= 0 && sym < SYM_COUNT); return open[sym];}
	inline void   SetOpen(int sym, double d) {ASSERT(sym >= 0 && sym < SYM_COUNT); open[sym] = d;}
	
	inline double GetChange(int sym) const {ASSERT(sym >= 0 && sym < SYM_COUNT); return change[sym];}
	inline void   SetChange(int sym, double d) {ASSERT(sym >= 0 && sym < SYM_COUNT); change[sym] = d;}
	
	inline double GetSignalSensor(int group, int sym, int sens) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < SIGNAL_SENSORS);
		return signal_sensors[(group * SYM_COUNT + sym) * SIGNAL_SENSORS + sens];
	}
	inline void   SetSignalSensor(int group, int sym, int sens, double d) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < SIGNAL_SENSORS);
		signal_sensors[(group * SYM_COUNT + sym) * SIGNAL_SENSORS + sens] = d;
	}
	inline double GetSignalSensorUnsafe(int group_id, int i) const {return signal_sensors[(group_id * SYM_COUNT * SIGNAL_SENSORS) + i];}
	
	inline double GetAmpSensor(int group, int sym, int sens) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < AMP_SENSORS);
		return amp_sensors[(group * SYM_COUNT + sym) * AMP_SENSORS + sens];
	}
	inline void   SetAmpSensor(int group, int sym, int sens, double d) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < AMP_SENSORS);
		amp_sensors[(group * SYM_COUNT + sym) * AMP_SENSORS + sens] = d;
	}
	inline double GetAmpSensorUnsafe(int group_id, int i) const {return amp_sensors[(group_id * SYM_COUNT * AMP_SENSORS) + i];}
	
	inline double GetFuseSensor(int group, int sym, int sens) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < FUSE_SENSORS);
		return fuse_sensors[(group * SYM_COUNT + sym) * FUSE_SENSORS + sens];
	}
	inline void   SetFuseSensor(int group, int sym, int sens, double d) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(sens >= 0 && sens < FUSE_SENSORS);
		fuse_sensors[(group * SYM_COUNT + sym) * FUSE_SENSORS + sens] = d;
	}
	inline double GetFuseSensorUnsafe(int group_id, int i) const {return fuse_sensors[(group_id * SYM_COUNT * FUSE_SENSORS) + i];}
	
	inline double GetFilterSensor(int group, int level, int sym, int sens) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(level >= 0 && level < FILTER_COUNT); ASSERT(sens >= 0 && sens < FILTER_SENSORS);
		return filter_sensors[((group * FILTER_COUNT + level) * SYM_COUNT + sym) * FILTER_SENSORS + sens];
	}
	inline void   SetFilterSensor(int group, int level, int sym, int sens, double d) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(level >= 0 && level < FILTER_COUNT); ASSERT(sens >= 0 && sens < FILTER_SENSORS);
		filter_sensors[((group * FILTER_COUNT + level) * SYM_COUNT + sym) * FILTER_SENSORS + sens] = d;
	}
	inline double GetFilterSensorUnsafe(int group_id, int level, int i) const {return filter_sensors[((group_id * FILTER_COUNT + level) * SYM_COUNT * FILTER_SENSORS) + i];}
	
	inline int GetSignalOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetSignalOutput(group * SYM_COUNT + sym);
	}
	inline void SetSignalOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetSignalOutput(group * SYM_COUNT + sym, i);
	}
	
	inline int GetAmpOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetAmpOutput(group * SYM_COUNT + sym);
	}
	inline void SetAmpOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetAmpOutput(group * SYM_COUNT + sym, i);
	}
	
	inline int GetFuseOutput(int group, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		return GetFuseOutput(group * SYM_COUNT + sym);
	}
	inline void SetFuseOutput(int group, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT);
		SetFuseOutput(group * SYM_COUNT + sym, i);
	}
	
	inline int GetFilterOutput(int group, int level, int sym) const {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(level >= 0 && level < FILTER_COUNT);
		return GetFilterOutput((group * FILTER_COUNT + level) * SYM_COUNT + sym);
	}
	inline void SetFilterOutput(int group, int level, int sym, int i) {
		ASSERT(group >= 0 && group < GROUP_COUNT); ASSERT(sym >= 0 && sym < SYM_COUNT); ASSERT(level >= 0 && level < FILTER_COUNT);
		SetFilterOutput((group * FILTER_COUNT + level) * SYM_COUNT + sym, i);
	}
	
	int GetShift() const {return shift;}
	void SetShift(int shift) {this->shift = shift;}
	
};

#include "AgentGroup.h"


extern bool reset_signals;
extern bool reset_amps;
extern bool reset_fuses;
extern bool reset_filters;

class AgentSystem {
	
	
public:
	
	// Persistent
	Vector<AgentGroup> groups;
	Vector<FactoryDeclaration> indi_ids;
	Time created;
	int phase = 0;
	
	
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
	double signal_epsilon = 0.0, amp_epsilon = 0.0, fuse_epsilon = 0.0, filter_epsilon[FILTER_COUNT];
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
	bool running = false, stopped = true;
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
	void SetFilterEpsilon(int level, double d);
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
	double GetAverageFuseDrawdown();
	double GetAverageFuseIterations();
	double GetAverageFilterDrawdown(int level);
	double GetAverageFilterIterations(int level);
	double GetAverageIterations(int phase);
	double GetAverageDeepIterations(int phase);
	double GetSignalEpsilon() const			{return signal_epsilon;}
	double GetAmpEpsilon() const			{return amp_epsilon;}
	double GetFuseEpsilon() const			{return fuse_epsilon;}
	double GetFilterEpsilon(int level) const	{return filter_epsilon[level];}
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
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
