#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

namespace Overlook {

class Currency : Moveable<Currency> {
	typedef Tuple<int, int> OpenClose;
	
	String name, tz;
	Vector<OpenClose> local;
	
	
public:
	
	String GetTZ() const {return tz;}
	String GetName() const {return name;}
	int GetLocalCount() const {return local.GetCount();}
	void GetLocalOpen(int i, Time& t)  const {t.hour = local[i].a / 100; t.minute = local[i].a % 100;}
	void GetLocalClose(int i, Time& t) const {t.hour = local[i].b / 100; t.minute = local[i].b % 100;}
	
	Currency& AddOpenLocal(int open, int close) {local.Add(OpenClose(open, close)); return *this;}
	
	Currency& Set(String name) {this->name = name; return *this;}
	Currency& SetTZ(String tz) {this->tz = tz; return *this;}
	
};

class CurrencyPair {
	
};

class TimeSlot : Moveable<TimeSlot> {
	
public:
	String id;
	Vector<String> open, close;
	Vector<String> was_open, is_open;
	
public:
	
	String GetID() const {return id;}
	String ToString() const {return "open " + Join(open, ",") + " close " + Join(close, ",") + " was " + Join(was_open, ",") + " is " + Join(is_open, ",");}
	
	void Add(String s) {id << s;}
	void AddOpen(String s) {open.Add(s);}
	void AddClose(String s) {close.Add(s);}
	
	
};

class DaySlots {
	
	
public:
	VectorMap<Time, TimeSlot> time_points;
	
	
	void Clear() {time_points.Clear();}
};

class Slot : Moveable<Slot> {
	
	
public:
	
};

class ExpertSystem {
	
	Vector<Currency> currencies;
	Vector<Slot> slots;
	
public:
	typedef ExpertSystem CLASSNAME;
	ExpertSystem();
	~ExpertSystem();
	
	void GetDaySlots(Date date, DaySlots& slot);
	
	
};

inline ExpertSystem& GetExpertSystem() {return Single<ExpertSystem>();}

void TestExpertSystem();

}

#endif
