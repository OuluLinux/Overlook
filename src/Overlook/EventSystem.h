#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

namespace Overlook {

class EventSystem {
	
	
	enum {OPEN, HURST, MA, BB, VA, VS, PC, CAL, BUF_COUNT};
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > bufs;
	Vector<Calendar*> cals;
	Index<int> sym_ids, tf_ids, fac_ids;
	
	bool running = false, stopped = true;
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	void Data();
	void PrintNetCode();
	
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
};

inline EventSystem& GetEventSystem() {return Single<EventSystem>();}

}

#endif
