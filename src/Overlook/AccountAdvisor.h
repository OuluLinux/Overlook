#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;


class WeekSlotAdvisor : public Core {
	
protected:
	virtual void Start();
	
public:
	WeekSlotAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1);
	}
};


}

#endif
