#ifndef _Overlook_FastEventConsole_h_
#define _Overlook_FastEventConsole_h_



namespace Overlook {


struct FastEventSnap {
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
	int prev_check_slot_id = -1, neg_count = 0;
	bool is_finished = false;
	
	void Serialize(Stream& s) {s % time % comment % slot_id % cal % stats % prev_check_slot_id % neg_count % is_finished;}
};

class FastEventConsole : public Common {
	
	
protected:
	friend class FastEventConsoleCtrl;
	
	// Persistent
	Array<FastEventSnap> snaps;
	
	
	
public:
	typedef FastEventConsole CLASSNAME;
	FastEventConsole();
	~FastEventConsole();
	
	
	virtual void Init();
	virtual void Start();
	
	int GetSnapCount() const {return snaps.GetCount();}
	FastEventSnap& GetSnap(int i) {return snaps[i];}
	
	void Serialize(Stream& s) {s % snaps;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("FastEventConsole.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("FastEventConsole.bin"));}
	
};

inline FastEventConsole& GetFastEventConsole() {return GetSystem().GetCommon<FastEventConsole>();}


class FastEventConsoleCtrl : public CommonCtrl {
	Splitter hsplit;
	
	ArrayCtrl history;
	ArrayCtrl calendar;
	ArrayCtrl FastEvents;
	
	ParentCtrl actions;
	::Upp::Label actlbl;
	Option automation, against;
	Button skip, act;
	
public:
	typedef FastEventConsoleCtrl CLASSNAME;
	FastEventConsoleCtrl();
	
	virtual void Data();
	
	void LoadHistory();
	
};

}

#endif
