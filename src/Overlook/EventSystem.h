#ifndef _Overlook_EventSystem_h_
#define _Overlook_EventSystem_h_

#include "Optimizer.h"

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

class TimeSlot : Moveable<TimeSlot> {
	
public:
	String id;
	Vector<String> open, close;
	Vector<String> was_open, is_open;
	Index<String> is_open_cur;
	
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

struct EventError : Moveable<EventError> {
	String msg;
	byte level = 0;
	Time time;
	
	EventError() {}
	EventError(const EventError& e) {*this = e;}
	void operator=(const EventError& e) {msg = e.msg; level = e.level; time = e.time;}
	unsigned GetHashValue() const {return msg.GetHashValue();}
	void Serialize(Stream& s) {s % msg % level % time;}
	bool operator==(const EventError& e) const {return msg == e.msg;}
};

class EventSystem : public Common {
	
protected:
	friend class EventSystemCtrl;
	
	
	// Persistent
	Index<EventError> prev_errors;
	Vector<EventError> history_errors;
	int prev_sent_id = -1;
	
	// Temporary
	VectorMap<String, Currency> currencies;
	Index<String> pairs;
	bool running = false, stopped = true;
	
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	virtual void Init();
	virtual void Start();
	void Serialize(Stream& s) {s % prev_errors % history_errors % prev_sent_id;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("EventSystem.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("EventSystem.bin"));}
	void GetDaySlots(Date date, DaySlots& slot);
	void GetErrorList(const SentimentSnapshot& snap, Index<EventError>& errors);
	void PreNewsErrors(const CalEvent& ev, const SentimentSnapshot& snap, Index<EventError>& errors);
	void PostNewsErrors(const CalEvent& ev, const SentimentSnapshot& snap, Index<EventError>& errors);
	void GetSessionErrors(const SentimentSnapshot& snap, Index<EventError>& errors);
	void AddError(Index<EventError>& errors, String e);
	void AddWarning(Index<EventError>& errors, String e);
	void AddInfo(Index<EventError>& errors, String e);
	bool IsSessionOpen(String currency);
	bool IsSessionOpen(String currency, Date d);
	
	//void RunOptimizer();
	//void WriteIndexUpDown(int i, int pair);
	//void GetSentimentResponses(int i, int pair, Vector<SentimentResponse>& response, int signal);
	
	Callback1<EventError> WhenError;
	
};

inline EventSystem& GetEventSystem() {return GetSystem().GetCommon<EventSystem>();}

class EventSystemCtrl : public CommonCtrl {
	ArrayCtrl arr;
	
public:
	typedef EventSystemCtrl CLASSNAME;
	EventSystemCtrl();
	
	virtual void Data();
	
	
};

void TestEventSystem();

}

#endif
