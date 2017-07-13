#ifndef _Overlook_Manager_h_
#define _Overlook_Manager_h_

namespace Overlook {

class Manager {
	
	
public:
	typedef Manager CLASSNAME;
	Manager(System* sys);
	
	void Init();
	void Start();
	void Stop();
	
	
	
	Array<AgentGroup> groups;
	System* sys;
};

}

#endif
