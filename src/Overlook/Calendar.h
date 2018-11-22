#ifndef _Overlook_Calendar_h_
#define _Overlook_Calendar_h_


namespace Overlook {

struct CalEvent : public Moveable<CalEvent> {
	int id = -1;
	Time timestamp = Null;
	char impact = 0, direction = 0;
	String title, unit, currency, forecast, previous, actual;
	
	CalEvent() {}
	CalEvent(const CalEvent& e) {*this = e;}
	CalEvent& operator=(const CalEvent& e);
	String ToInlineString() const;
	String GetImpactString() const;
	int GetSignalDirection();
	void Serialize(Stream& s);
	
	bool operator () (const CalEvent& a, const CalEvent& b) const {return a.timestamp < b.timestamp;}
	
	bool IsNegative() const {return title.Find("Unemploy") != -1;}
};


struct CalendarSignal : public Moveable<CalendarSignal> {
	CalendarSignal() {}
	CalendarSignal(const CalendarSignal& cs);
	Vector<int> id, time, dir, impact;
	Vector<String> title;
	int count;
	int GetCount() {return count;}
	
	void Add() {id.Add(0); time.Add(0); dir.Add(0); impact.Add(0); title.Add();}
};


void RefreshCalendarBusCalendar();
CalendarSignal CalendarBus(const String& currency, int tf, bool direct);
bool IsHighImpactSpeechCalendar(int begin_offset, int end_offset);
const XmlNode& TryOpenLocation(String pos, const XmlNode& root, int& errorcode);


class CalendarCommon : public Common {
	
	// Persistent
	Vector<CalEvent> events;
	Time last_week_update = Null, last_update = Null;
	
	
	// Temporary
	Mutex lock;
	Time prevtime;
	Date prevdate;
	int errorcode = 0;
	int thisyear, thismonth;
	bool ready = false;
	bool initial_update = false;
	
	void ParseEvent(const XmlNode& n, CalEvent& event);
	void RefreshProbabilities();
	int  Update(String postfix = "", bool force_update=false);
	void UpdateLastWeek();
	void UpdateNextWeek();
	void SetTimeNow();
	void TrimDuplicateEvents();
	
	
	
public:
	
	enum {ID, TITLE, TIMESTAMP, UNIT, CURRENCY, FORECAST, PREVIOUS, ACTUAL, MARKET, IMPACT, DIRECTION};
	
	typedef CalendarCommon CLASSNAME;
	CalendarCommon();
	
	virtual void Init();
	virtual void Start();
	
	void StoreThis();
	void LoadThis();
	void Refresh();
	void UpdateHistory();
	void Dump();
	
	int GetCount();
	const CalEvent& GetEvent(int i);
	void GetEvents(Vector<CalEvent>& ev, Time begin, Time end, int min_impact_level=0) const;
	bool IsCalendarNeedingUpdate();
	bool IsMajorSpeak();
	bool IsError() {return errorcode != 0;}
	bool IsReady() {return ready;}
	
	
	void SetEventActualDiff(int i, double diff);
	void SetEvent(CalEvent& ce);
	
	void Serialize(Stream& s) {
		s % events % last_week_update % last_update;
	}
	
	Callback1<CalEvent&> AddEvent;
	
};


inline CalendarCommon& GetCalendar() {return GetSystem().GetCommon<CalendarCommon>();}

void BasicHeaders(HttpRequest& h);
String XmlTreeString(const XmlNode& node, int indent=0, String prev_addr="");
void XmlFix(String& c);


class CalendarCtrl : public CommonCtrl {
	ArrayCtrl calendar;
	
public:
	typedef CalendarCtrl CLASSNAME;
	CalendarCtrl();
	
	virtual void Data();
	
};

}

#endif
