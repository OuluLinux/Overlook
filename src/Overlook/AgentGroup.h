#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

#include <amp.h>
using namespace concurrency;

namespace Overlook {

struct Snap {
	float year_time, week_time;
	float sensor[21];
	float signal[21];
	float open[21];
};

class AgentGroup {
	
	typedef DQNAgent<1,1,1,1> Agent;
	typedef DQNAgent<1,1,1,1> Joiner;
	typedef DQNAgent<1,1,1,1> Final;
	
public:
	
	// Persistent
	std::vector<Agent> agents;
	std::vector<Joiner> joiners;
	Final fin;
	Index<int> tf_ids, sym_ids, indi_ids;
	Time created;
	String name;
	double limit_factor;
	int phase;
	bool enable_training;
	
	
	
	
	// Temp
	TimeStop last_store, last_datagather;
	int main_tf;
	bool running, stopped;
	System* sys;
	
	enum {PHASE_SEEKSNAPS, PHASE_TRAINING, PHASE_WEIGHTS, PHASE_FINAL, PHASE_UPDATE, PHASE_REAL, PHASE_WAIT};
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup();
	~AgentGroup();
	
	
	void Init();
	void Start();
	void Stop();
	void StoreThis();
	void LoadThis();
	void SetEpsilon(double d);
	void SetMode(int i);
	void RefreshSnapshots();
	void Progress(int actual, int total, String desc);
	void SubProgress(int actual, int total);
	void Serialize(Stream& s);
	
	void Main();
	
	
	
	
	Callback3<int, int, String> WhenProgress;
	Callback2<int, int> WhenSubProgress;
	int a0, t0, a1, t1;
	String prog_desc;
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
	
};

}

#endif
