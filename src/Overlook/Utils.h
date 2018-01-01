#ifndef _Overlook_Utils_h_
#define _Overlook_Utils_h_

namespace Overlook {

class ValueChange : public Core {
	
	OnlineAverage1 change_av;
	
public:
	typedef ValueChange CLASSNAME;
	ValueChange();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 2)
			% Mem(change_av);
	}
	
	virtual void Init();
	virtual void Start();
	
};

}

#endif
