#if 0

#ifndef _Overlook_EventConsole_h_
#define _Overlook_EventConsole_h_


namespace Overlook {


struct EventSnap {
	struct Stat : Moveable<Stat> {
		int net, src, signal, count, grade;
		double mean, cdf;
		bool inverse;
		
		bool operator() (const Stat& a, const Stat& b) const {return a.mean > b.mean;}
		void Serialize(Stream& s) {s % net % src % signal % count % grade % mean % cdf % inverse;}
	};
	Time time;
	String comment;
	int slot_id = -1;
	Vector<CalEvent> cal;
	Vector<Stat> stats;
	
	void Serialize(Stream& s) {s % time % comment % slot_id % cal % stats;}
};

class EventConsole : public Common {
	
	
protected:
	friend class EventConsoleCtrl;
	
	// Persistent
	Array<EventSnap> snaps;
	
	
	
public:
	typedef EventConsole CLASSNAME;
	EventConsole();
	~EventConsole();
	
	
	virtual void Init();
	virtual void Start();
	
	int GetSnapCount() const {return snaps.GetCount();}
	EventSnap& GetSnap(int i) {return snaps[i];}
	
	void Serialize(Stream& s) {s % snaps;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("EventConsole.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("EventConsole.bin"));}
	
};

inline EventConsole& GetEventConsole() {return GetSystem().GetCommon<EventConsole>();}


class EventConsoleCtrl : public CommonCtrl {
	Splitter hsplit;
	
	ArrayCtrl history;
	ArrayCtrl calendar;
	ArrayCtrl events;
	
	ParentCtrl actions;
	::Upp::Label actlbl;
	Option automation, against;
	Button skip, act;
	
public:
	typedef EventConsoleCtrl CLASSNAME;
	EventConsoleCtrl();
	
	virtual void Data();
	
	void LoadHistory();
	
};

}

#endif
#endif
