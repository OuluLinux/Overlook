#ifndef _DataCore_Periodicals_h_
#define _DataCore_Periodicals_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;

class WdayHourChanges : public Slot {
	
protected:
	Vector<OnlineVariance> wdayhour;
	
	//Data32f stddev, mean;
	
public:
	WdayHourChanges();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "WdayHourChanges";}
	virtual String GetShortName() const {return "whch";}
	
	const OnlineVariance& GetOnlineVariance(int shift);
	
};

class WdayHourStats : public Slot {
	
protected:
	int var_period;
	MovingOnlineVariance total;
	Vector<MovingOnlineVariance> wdayhour, wday, hour;
	
	//Data32f t_pre_stddev, t_pre_mean;
	//Data32f h_pre_stddev, h_pre_mean;
	//Data32f d_pre_stddev, d_pre_mean;
	//Data32f dh_pre_stddev, dh_pre_mean;
	
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
	//Data32f t_pre_stddev, t_pre_mean;
	//Data32f h_pre_stddev, h_pre_mean;
	//Data32f d_pre_stddev, d_pre_mean;
	//Data32f dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourDiff();
	
	virtual String GetName() {return "WdayHourDiff";}
	virtual String GetShortName() const {return "whd";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};
















class EventOsc : public Slot {
	//Data32f info, low, med, high;
	double mul;
	Index<String> keys;
	EventManager* emgr;
	int counted_events;
	
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
