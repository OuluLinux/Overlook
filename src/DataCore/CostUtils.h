#ifndef _DataCore_CostUtils_h_
#define _DataCore_CostUtils_h_

#include "Slot.h"

namespace DataCore {
using namespace Upp;

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
