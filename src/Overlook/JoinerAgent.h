#if 0

#ifndef _Overlook_JoinerAgent_h_
#define _Overlook_JoinerAgent_h_

namespace Overlook {

class AgentGroup;

class JoinerAgent : public TraineeBase {
	
public:
	typedef JoinerAgent CLASSNAME;
	JoinerAgent();
	
	virtual void Create(int width, int height);
	virtual void Forward(Snapshot& snap, SimBroker& broker) {Forward(snap, (Brokerage&)broker);}
	virtual void Backward(double reward);
	
	bool PutLatest(Brokerage& broker);
	void Forward(Snapshot& snap, Brokerage& broker);
	
};

}

#endif
#endif
