#ifndef _Overlook_JoinerAgent_h_
#define _Overlook_JoinerAgent_h_

namespace Overlook {

class AgentGroup;

class JoinerAgent : public TraineeBase {
	AgentGroup* group;
	
public:
	typedef JoinerAgent CLASSNAME;
	JoinerAgent();
	
	
	bool PutLatest(Brokerage& broker);
	
};

}

#endif
