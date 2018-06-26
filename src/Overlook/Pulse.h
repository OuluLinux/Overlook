#ifndef _Overlook_Pulse_h_
#define _Overlook_Pulse_h_

namespace Overlook {

class Pulse {
	
	
public:
	typedef Pulse CLASSNAME;
	Pulse();
	
	
};

inline Pulse& GetPulse() {return Single<Pulse>();}

class PulseCtrl : public ParentCtrl {
	
	
public:
	typedef PulseCtrl CLASSNAME;
	PulseCtrl();
	
	void Data();
	
	
};

}

#endif
