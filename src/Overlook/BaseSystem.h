#ifndef _Overlook_BaseSystem_h_
#define _Overlook_BaseSystem_h_

#include <CtrlUtils/CtrlUtils.h>
#include <CoreUtils/CoreUtils.h>

namespace Overlook {
using namespace Upp;

class BaseSystem : public Core {
	
protected:
	
	Vector<int>		tfbars_in_slowtf;
	Vector<int>		bars;
	Index<String>	symbols;
	Index<int>		periods;
	Vector<String>	period_strings;
	String			addr;
	int64			memory_limit;
	int				port;
	
	
protected:
	
	// Time
	Time begin, end;
	int timediff;
	int base_period;
	int begin_ts, end_ts;
	
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	
	
public:
	
	int GetCount(int period) const;
	Time GetTime(int period, int pos) const {return begin + base_period * period * pos;}
	Time GetBegin() const {return begin;}
	Time GetEnd() const {return end;}
	int GetBeginTS() {return begin_ts;}
	int GetEndTS() {return end_ts;}
	int GetBasePeriod() const {return base_period;}
	int64 GetShift(int src_period, int dst_period, int shift);
	int64 GetShiftFromTime(int timestamp, int period);
	int GetTfFromSeconds(int period_seconds);
	
	void SetBegin(Time t)	{begin = t; begin_ts = (int)(t.Get() - Time(1970,1,1).Get());}
	void SetEnd(Time t)	{end = t; end_ts = (int)(t.Get() - Time(1970,1,1).Get()); timediff = (int)(end.Get() - begin.Get());}
	void SetBasePeriod(int period)	{base_period = period;}
	
	
public:
	
	void AddPeriod(String nice_str, int period);
	void AddSymbol(String sym);
	
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	
	int GetPeriod(int i) const {return periods[i];}
	String GetPeriodString(int i) const {return period_strings[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int FindPeriod(int period) const {return periods.Find(period);}
	
	
public:
	
	typedef BaseSystem CLASSNAME;
	BaseSystem();
	
	virtual void SetArguments(const VectorMap<String, Value>& args) {}
	virtual void Init();
	virtual void Start() {}
	
	
};


class BaseSystemCtrl : public CustomCtrl {
	
	
public:
	typedef BaseSystemCtrl CLASSNAME;
	BaseSystemCtrl();
	
	
};

inline BaseSystem& GetBaseSystem() {return Factory::GetCore<BaseSystem>();}
}

#endif
