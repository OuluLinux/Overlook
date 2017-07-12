#ifndef _Overlook_Manager_h_
#define _Overlook_Manager_h_

namespace Overlook {

struct AgentGroup {
	
	// Persistent
	Array<Agent> agents;
	Vector<double> latest_signals;
	Index<int> tf_ids;
	Time created;
	String name;
	
	// Temp
	SimBroker broker;
	
	
	AgentGroup();
	void Init();
	void StoreThis();
	void LoadThis();
	void Serialize(Stream& s);
	
	int GetSignalPos(int shift);
	int GetSignalPos(int shift, int group_id);
	
};

class Manager {
	
	
public:
	typedef Manager CLASSNAME;
	Manager();
	
	void Init();
	void Start();
	void Stop();
	
	
	
	Array<AgentGroup> groups;
	
};

}

#endif
