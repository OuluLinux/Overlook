#ifndef _DataCore_EventManager_h_
#define _DataCore_EventManager_h_

#include <CoreUtils/CoreUtils.h>
#include <RefCore/RefCore.h>

namespace DataCore {
using namespace RefCore;

struct Event : public Moveable<Event> {
	int id, timestamp;
	char impact, direction;
	String title, unit, currency, forecast, previous, actual, market;
	
	Event() : id(-1), timestamp(0), direction(0), impact(0) {}
	Event(const Event& e) {*this = e;}
	Event& operator=(const Event& e);
	String ToInlineString() const;
	int GetSignalDirection();
	void Serialize(Stream& s);
	
	bool operator () (const Event& a, const Event& b) const {return a.timestamp < b.timestamp;}
	
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

void RefreshCalendarBusEventManager();
CalendarSignal CalendarBus(const String& currency, int tf, bool direct);
bool IsHighImpactSpeechEventManager(int begin_offset, int end_offset);

const XmlNode& TryOpenLocation(String pos, const XmlNode& root, int& errorcode);



class EventManager : public MetaNode {
	
	Vector<Event> events;
	
	int errorcode;
	dword prevtime;
	Date prevdate;
	int thisyear, thismonth;
	dword lastupdate;
	
	void ParseEvent(const XmlNode& n, Event& event);
	
	void RefreshProbabilities();
	
	Mutex lock;
	
	int addevents_count;
	int  Update(String postfix = "", bool force_update=false);
	void UpdateLastWeek();
	void UpdateNextWeek();
	void SetTimeNow();
	
	Thread thrd;
	
	int GetDirection(Event& fce);
	
	
public:
	
	enum {ID, TITLE, TIMESTAMP, UNIT, CURRENCY, FORECAST, PREVIOUS, ACTUAL, MARKET, IMPACT, DIRECTION};
	
	typedef EventManager CLASSNAME;
	EventManager();
	
	virtual String GetName() {return "EventManager";}
	virtual String GetShortName() const {return "events";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	void StoreThis();
	void LoadThis();
	void Serialize(Stream& s) {
		s % events % lastupdate;
	}
	
	//void Init();
	void Deinit();
	void Updater();
	void Refresh();
	void UpdateHistory();
	void Dump();
	
	int GetCount();
	Event& GetEvent(int i);
	bool IsCalendarNeedingUpdate();
	bool HasError() {return errorcode != 0;}
	
	void SetEventActualDiff(int i, double diff);
	void SetEvent(Event& ce);
	
	Callback1<Event&> AddEvent;
	
};

int GetCalendarCacheCount();
const Event& GetCalendarCacheEvent(int i);
Mutex& GetCalendarCacheLock();

}

#endif
