#ifndef _DataCore_CostUtils_h_
#define _DataCore_CostUtils_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;

/*
	Brokers doesn't generally publish time-series data about their spreads, or even statistics.
	That is problematic for sophisticated algorithms, which might have very different long-term
	results with different spreads. Overlook tries to solve this problem by storing data from
	the user's broker and by making statistics from them. It is not accurate and fast events
	might have completely unrealistic spread, but it still better than nothing.
	
	The other problem is currency pairs without base currency and commodities based on different
	currency than base currency. The base currency must be converted to other currency and two
	rates will affect to the value change. In practice, those two-rates must be easily
	available at the same time, but that is a huge problem when two time series doesn't have
	same time-positions, and it requires finding the same time, which is very inefficient.
	Overlook has fixed time-position for prices to solve that problem. Even weekends are
	included to calculate position fast from time. ValueChange solves the two-rate problem, and
	shows the combined change with the proxy-currency.

*/

class SpreadStats : public Slot {
	struct SymTf : Moveable<SymTf> {
		Vector<OnlineVariance> stats;
		bool has_stats;
	};
	Vector<SymTf> data;
	
public:
	SpreadStats();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "SpreadStats";}
	virtual String GetShortName() const {return "spread";}
	
};



class ValueChange : public Slot {
	struct Sym : Moveable<Sym> {
		bool has_proxy;
		int proxy_id, proxy_factor;
	};
	SlotPtr src, spread;
	Vector<Sym> data;
	DataBridge* db;
	
public:
	ValueChange();
	
	virtual String GetName() {return "ValueChange";}
	virtual String GetShortName() const {return "change";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

}

#endif
