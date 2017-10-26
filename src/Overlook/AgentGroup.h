#if 0

#ifndef _Overlook_AgentGroup_h_
#define _Overlook_AgentGroup_h_

#include "FixedSimBroker.h"
#include "Agent.h"


class AgentSystem;


class AgentGroup : Moveable<AgentGroup> {
	
public:
	
	// Persistent
	Vector<Agent> agents;
	
	
	// Temporary
	AgentSystem* sys = NULL;
	
	
public:
	typedef AgentGroup CLASSNAME;
	AgentGroup();
	~AgentGroup();
	
	void Serialize(Stream& s);
	
};


#endif
#endif
