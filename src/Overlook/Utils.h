#ifndef _Overlook_Utils_h_
#define _Overlook_Utils_h_

namespace Overlook {

class ValueChange : public AdvisorBase {
	
public:
	typedef ValueChange CLASSNAME;
	ValueChange();
	
	virtual void IO(ValueRegister& reg) {
		AdvisorBase::BaseIO(reg);
	}
	
	virtual void Init();
	virtual void Start();
	
};

}

#endif
