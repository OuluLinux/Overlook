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

struct SentimentResponseProto : Moveable<SentimentResponseProto> {
	int int_value0 = 0, int_value1 = 0;
	
};

struct SentimentResponse : Moveable<SentimentResponse> {
	char type;
	bool is_goal;
	
};

enum {
	RESP_ISOPEN, RESP_CALPRE, RESP_IDXUPDOWN, RESP_COUNT
};

class EventSystem : public Common {
	
	// Persistent
	Vector<SentimentResponseProto> responses;
	VectorMap<int, CoreList> corelists;
	Index<String> pairs;
	Optimizer opt;
	int cal_counted = 0;
	Date ses_counted;
	
	// Temporary
	VectorMap<String, Currency> currencies;
	bool running = false, stopped = true;
	
	
public:
	typedef EventSystem CLASSNAME;
	EventSystem();
	~EventSystem();
	
	virtual void Init();
	virtual void Start();
	
	void RunOptimizer();
	void GetDaySlots(Date date, DaySlots& slot);
	
	void WriteIndexUpDown(int i, int pair);
	void GetSentimentResponses(int i, int pair, Vector<SentimentResponse>& response, int signal);
	
	
};

inline EventSystem& GetEventSystem() {return GetSystem().GetCommon<EventSystem>();}

class EventSystemCtrl : public CommonCtrl {
	
public:
	typedef EventSystemCtrl CLASSNAME;
	EventSystemCtrl();
	
	virtual void Data();
	
	
};

void TestEventSystem();

}

#endif
