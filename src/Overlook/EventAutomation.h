#ifndef _Overlook_EventAutomation_h_
#define _Overlook_EventAutomation_h_

namespace Overlook {


class EventAutomation {
	
	
protected:
	
	enum {OPEN, BB};
	
	// Persistent
	VectorMap<String, int> auto_sig;
	int active_net = -1;
	
	
	// Temporary
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<ConstBuffer*> > > bufs;
	Vector<Calendar*> cals;
	Index<int> sym_ids, tf_ids, fac_ids;
	bool running = false, stopped = true;
	
	
public:
	typedef EventAutomation CLASSNAME;
	EventAutomation();
	~EventAutomation();
	
	void Data();
	void UpdateEvents();
	
	int GetAutoSig(String s) {return auto_sig.GetAdd(s, 0);}
	
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
};

inline EventAutomation& GetEventAutomation() {return Single<EventAutomation>();}

}
#endif
