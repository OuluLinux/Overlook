#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define GROUP_COUNT				4 //TODO
#define SYM_COUNT				21
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
#define JOINER_COUNT			10 //TODO
#define JOINERSIGNAL_SENSORS	5
#define JOINER_RESULT_COUNT		1000
#define JOINERSIGNAL_SIZE		(JOINER_COUNT * JOINERSIGNAL_SENSORS)
#define JOINER_STATES			(TIME_SENSORS + SENSOR_SIZE + SIGNAL_SIZE + JOINERSIGNAL_SIZE)
#define JOINER_NORMALACTS		(3*3*3*3)
#define JOINER_IDLEACTS			8
#define JOINER_ACTIONCOUNT		(JOINER_NORMALACTS + JOINER_IDLEACTS)
#define NEXT_PHASE_ITER_LIMIT	200000


class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	float year_timesensor;
	float week_timesensor;
	float day_timesensor;
	float sensor			[SENSOR_SIZE];
	float signal			[SIGNAL_SIZE];
	float open				[SYM_COUNT];
	float joiner_signal		[JOINERSIGNAL_SIZE];
	int shift;
};

#include "FixedSimBroker.h"
#include "Agent.h"
#include "Joiner.h"

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
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue;
	Vector<Snapshot> snaps;
	Vector<Core*> databridge_cores;
	Vector<double> agent_equities, joiner_equities;
	Index<String> allowed_symbols;
	Index<int> sym_ids;
	TimeStop last_store, last_datagather;
	System* sys;
	double agent_epsilon = 0.0, joiner_epsilon = 0.0;
	int main_tf = -1;
	int data_begin = 0;
	int buf_count = 0;
	int sym_count = 0;
	int max_memory_size = 0, agents_total_size = 0, memory_for_snapshots = 0, snaps_per_phase = 0, snap_phase_count = 0;
	int snap_phase_id = 0;
	int counted_bars = 0;
	int snap_begin = 0, snap_count = 0, snap_end = 0;
	int prev_shift = 0;
	int realtime_count = 0;
	int agent_equities_count = 0;
	int joiner_equities_count = 0;
	bool running = false, stopped = true;
	Mutex work_lock;
	
	enum {PHASE_TRAINING, PHASE_JOINER, PHASE_REAL};
	
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
	void SetAgentEpsilon(double d);
	void SetJoinerEpsilon(double d);
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
	double GetAverageAgentDrawdown();
	double GetAverageAgentIterations();
	double GetAverageJoinerDrawdown();
	double GetAverageJoinerIterations();
	double GetAverageJoinerEpochs();
	double GetAgentEpsilon()	{return agent_epsilon;}
	double GetJoinerEpsilon()	{return joiner_epsilon;}
	void RefreshAgentEpsilon();
	void RefreshJoinerEpsilon();
	void UpdateAmpSnaps(bool put_latest);
	void TrainAgents();
	void TrainJoiners();
	void MainReal();
	void LoopAgentSignals(bool from_begin);
	void LoopJoinerSignals(bool from_begin);
	Joiner* GetBestJoiner();
	
	void Main();
	void SetAgentsTraining(bool b);
	void SetJoinersTraining(bool b);
	
	
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
