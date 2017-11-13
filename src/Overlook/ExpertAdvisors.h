#ifndef _Overlook_ExpertAdvisors_h_
#define _Overlook_ExpertAdvisors_h_

namespace Overlook {
using namespace Upp;


class RandomForestAdvisor : public Core {
	
protected:
	virtual void Start();
	
public:
	RandomForestAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1);
	}
};


}

#endif
