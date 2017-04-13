#ifndef _DataCore_CostUtils_h_
#define _DataCore_CostUtils_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;

class CostStats : public Slot {
	//Data32f mean_cost;
	//BridgeAskBid* askbid;
	Vector<OnlineVariance> stats;
	
public:
	CostStats();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	virtual String GetName() {return "CostStats";}
	virtual String GetShortName() const {return "cost";}
	
};



class ValueChange : public Slot {
	bool has_proxy;
	int proxy_id, proxy_factor;
	/*Data32f inc, dec, cost;
	Data32f best_inc, best_dec, worst_inc, worst_dec;
	Data32f value_change;
	
	#ifdef PRICE_CHANGE_DEBUG
	Data32f proxy_status;
	Data32f proxy_cost, proxy_inc, proxy_dec;
	#endif
	
	BarData *src_a, *src_b;*/
	
public:
	ValueChange();
	
	/*virtual void Serialize(Stream& s) {
		BarDataContainer::Serialize(s);
		s % has_proxy % proxy_id % proxy_factor
		  % inc % dec % cost
		  % best_inc % best_dec % worst_inc % worst_dec
		  % value_change;
		#ifdef PRICE_CHANGE_DEBUG
		s % proxy_status % proxy_cost % proxy_inc % proxy_dec;
		#endif
	}*/
	
	virtual String GetName() {return "ValueChange";}
	virtual String GetShortName() const {return "valc";}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const SlotProcessAttributes& attr);
	
};

}

#endif
