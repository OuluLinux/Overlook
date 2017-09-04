#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

namespace Overlook {

#define SYM_COUNT				10
#define GROUP_COUNT				4
#define TIME_SENSORS			3
#define INPUT_SENSORS			(5 * 4 * 2)
#define SENSOR_SIZE				(SYM_COUNT * INPUT_SENSORS)

#define TRAINEE_RESULT_COUNT	1000
#define TRAINEE_COUNT			(GROUP_COUNT * SYM_COUNT)

#define AGENT_SIGNAL_SENSORS	3
#define AGENT_GROUP_SIGNAL_SIZE	(SYM_COUNT * AGENT_SIGNAL_SENSORS)
#define AGENT_SIGNAL_SIZE		(TRAINEE_COUNT * AGENT_SIGNAL_SENSORS)
#define AGENT_STATES			(TIME_SENSORS + SENSOR_SIZE + AGENT_SIGNAL_SIZE)
#define AGENT_NORMALACTS		14
#define AGENT_IDLEACTS			8
#define AGENT_ACTIONCOUNT		(AGENT_NORMALACTS + AGENT_IDLEACTS)
#define AGENT_PHASE_ITER_LIMIT	400000
#define AGENT_EPS_ITERS_STEP	10000

#define JOINER_SIGNAL_SENSORS	3
#define JOINER_SIGNAL_SIZE		(TRAINEE_COUNT * JOINER_SIGNAL_SENSORS)
#define JOINER_STATES			(TIME_SENSORS + SENSOR_SIZE + AGENT_SIGNAL_SIZE + JOINER_SIGNAL_SIZE)
#define JOINER_ACTIONCOUNT		(3*6)
#define JOINER_PHASE_ITER_LIMIT	1000000
#define JOINER_EPS_ITERS_STEP	100000


class AgentGroup;

struct Snapshot : Moveable<Snapshot> {
	double year_timesensor;
	double week_timesensor;
	double day_timesensor;
	double sensor				[SENSOR_SIZE];
	double open					[SYM_COUNT];
	double agent_signal			[AGENT_SIGNAL_SIZE];
	double joiner_signal		[JOINER_SIGNAL_SIZE];
	int agent_broker_signal		[TRAINEE_COUNT];
	int joiner_broker_signal	[TRAINEE_COUNT];
	int shift;
};

#include "FixedSimBroker.h"
#include "Agent.h"
#include "Joiner.h"


extern bool reset_agents;
extern bool reset_joiners;

class AgentGroup {
	
	
public:
	
	// Persistent
	Vector<Agent> agents;
	Vector<Joiner> joiners;
	Vector<FactoryDeclaration> indi_ids;
	Time created;
	int phase = 0;
	
	
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
	double joiner_fmlevel = 0.8;
	int main_tf = -1;
	int data_begin = 0;
	int buf_count = 0;
	int sym_count = 0;
	int counted_bars = 0;
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
	void TrainAgents();
	void TrainJoiners();
	void MainReal();
	void Data();
	void LoopAgentSignals(bool from_begin);
	void LoopJoinerSignals(bool from_begin);
	Joiner* GetBestJoiner(int sym_id);
	bool PutLatest(Brokerage& broker, Vector<Snapshot>& snaps);
	
	void Main();
	void SetAgentsTraining(bool b);
	void SetJoinersTraining(bool b);
	
	
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
