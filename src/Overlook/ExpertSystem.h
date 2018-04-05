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
	
	void Serialize(Stream& s) {s % name % tz % local;}
	
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
	unsigned GetHashValue() const {
		CombineHash ch;
		for(int i = 0; i < was_open.GetCount(); i++) ch << ::GetHashValue(was_open[i]) << 1;
		for(int i = 0; i < is_open.GetCount(); i++)  ch << ::GetHashValue(is_open[i])  << 1;
		return ch;
	}
	
	void Serialize(Stream& s) {s % id % open % close % was_open % is_open;}
	
};

class DaySlots {
	
	
public:
	VectorMap<Time, TimeSlot> time_points;
	
	
	void Clear() {time_points.Clear();}
};

class Slot : Moveable<Slot> {
	
	
public:
	Vector<Time> open_time, close_time;
	Index<String> currencies;
	String id;
	
public:
	
	void Serialize(Stream& s) {s % open_time % close_time % currencies % id;}
	
};

class ExchangeSlots  {
	
protected:
	friend class ExchangeSlotsCtrl;
	friend class SlotSignals;
	
	Vector<Currency> currencies;
	VectorMap<unsigned, Slot> slots;
	Time cursor;
	
	bool is_loaded = false;
	
public:
	typedef ExchangeSlots CLASSNAME;
	ExchangeSlots();
	~ExchangeSlots();
	
	void GetDaySlots(Date date, DaySlots& slot);
	
	void Refresh();
	
	String GetPath();
	void LoadThis() {LoadFromFile(*this, GetPath());}
	void StoreThis() {StoreToFile(*this, GetPath());}
	void Serialize(Stream& s) {s % currencies % slots % cursor;}
	
	Slot* FindCurrent();
	
};

inline ExchangeSlots& GetExchangeSlots() {return Single<ExchangeSlots>();}


}

#endif
