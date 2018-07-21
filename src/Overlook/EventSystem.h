#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

namespace Overlook {

class EventSystem {
	
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Index<int> sym_ids, tf_ids;
	
	bool running = false, stopped = true;
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	void Data();
	
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
};

inline EventSystem& GetEventSystem() {return Single<EventSystem>();}

}

#endif
