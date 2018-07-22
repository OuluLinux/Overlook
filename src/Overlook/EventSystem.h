#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

namespace Overlook {

struct Event : Moveable<Event> {
	int ev = -1, tf = -1, sym = -1, sig = -1;
};

class EventSystem {
	
public:
	enum {OPEN, HURST, MA, BB, PSAR, VA, ANOM, VS, PC, CAL, BUF_COUNT};
	
	enum {
		HIHURST, HIMA, HIBB, NEWSAR, HIVOLAV, ANOMALY,
		HICHANGE, HIPERCH,
		HIVOLSLOT,
		HINEWS
	};
	
protected:
	
	// Persistent
	Vector<Event> events;
	int active_net = -1;
	
	
	// Temporary
	Vector<Event> tmp_events;
	Index<String> symbols;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > work_queue;
	Vector<Vector<Vector<Vector<ConstBuffer*> > > > bufs;
	Vector<Calendar*> cals;
	Index<int> sym_ids, tf_ids, fac_ids;
	bool running = false, stopped = true;
	
	
	
	void SetEvent(int ev, int tf, int sym, int sig);
	int GetOpenSig(int sym, int tf);
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	void Data();
	void PrintNetCode();
	void UpdateEvents();
	
	int GetCount() {return events.GetCount();}
	Event& Get(int i) {return events[i];}
	String GetEventString(int i);
	
	bool IsRunning() const {return running && !Thread::IsShutdownThreads();}
	
};

inline EventSystem& GetEventSystem() {return Single<EventSystem>();}

}

#endif
