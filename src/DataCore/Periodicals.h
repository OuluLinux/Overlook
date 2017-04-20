#ifndef _DataCore_Periodicals_h_
#define _DataCore_Periodicals_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;


class WdayHourStats : public Slot {
	
protected:
	struct SymTf : Moveable<SymTf> {
		MovingOnlineVariance total;
		Vector<MovingOnlineVariance> wdayhour, wday, hour;
	};
	Vector<SymTf> data;
	SlotPtr src;
	int var_period;
	
public:
	WdayHourStats();
	
	virtual String GetName() {return "WdayHourStats";}
	virtual String GetShortName() const {return "whstat";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class WdayHourDiff : public Slot {
	
protected:
	
	int var_period_fast, var_period_diff;
	SlotPtr whstat_fast, whstat_slow;
	
public:
	WdayHourDiff();
	
	virtual String GetName() {return "WdayHourDiff";}
	virtual String GetShortName() const {return "whd";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class ChannelStats : public Slot {
	SlotPtr whstat_slow;
	
public:
	ChannelStats();
	
	virtual String GetName() {return "ChannelStats";}
	virtual String GetShortName() const {return "chstat";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

class ChannelPredicter : public Slot {
	
protected:
	
	SlotPtr src, chstat;
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetName() {return "ChannelPredicter";}
	virtual String GetShortName() const {return "chp";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};















class EventOsc : public Slot {
	struct Sym : Moveable<Sym> {
		Index<String> keys;
		int counted_events;
	};
	Vector<Sym> data;
	
	SlotPtr src;
	DataBridge* db;
	EventManager* emgr;
	double mul;
	
public:
	EventOsc();
	
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "EventOsc";}
	virtual String GetShortName() const {return SHORTNAME0("eosc");}
	virtual void SetArguments(const VectorMap<String, Value>& args);
};

}

#endif
