#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

namespace Overlook {

class EventSystem {
	
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	
	void Data();
	
	
};

inline EventSystem& GetEventSystem() {return Single<EventSystem>();}

}

#endif
