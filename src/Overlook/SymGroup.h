#if 0

#ifndef _Overlook_SymGroup_h_
#define _Overlook_SymGroup_h_

#include <amp.h>
using namespace concurrency;

namespace Overlook {



class SymGroup {
	AgentGroup* group;
	System* sys;
	
public:
	
	// Persistent
	Array<Agent> agents;
	double tf_limit;
	int input_width, input_height;
	int tf_id;
	
	
	// Temp
	Array<Snapshot> snaps;
	int tf, tf_minperiod, tf_period, tf_type;
	int data_begin;
	
	
	
	typedef SymGroup CLASSNAME;
	SymGroup();
	
	void SetEpsilon(double d);
	
	void Main(SnapshotView& snaps);
	
	
};

}

#endif
#endif
