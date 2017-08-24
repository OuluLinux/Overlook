#if 0

#ifndef _Overlook_Manager_h_
#define _Overlook_Manager_h_

namespace Overlook {

class Manager {
	Time prev_update;
	int prev_shift;
	
public:
	typedef Manager CLASSNAME;
	Manager(System* sys);
	
	void Init();
	void Start();
	void Stop();
	void Main();
	
	AgentGroup* GetBestGroup();
	
	Array<AgentGroup> groups;
	System* sys;
};

}

#endif
#endif
