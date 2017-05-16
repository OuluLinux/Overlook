#ifndef _Overlook_SpreadUtils_h_
#define _Overlook_SpreadUtils_h_


namespace Overlook {


class SpreadStats : public Core {
	FloatVector mean_cost;
	BridgeAskBid* askbid;
	Vector<OnlineVariance> stats;
	
public:
	SpreadStats();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf, 1);
		//reg.AddOut(, , SymTf, 1);
	}
};

class SpreadMeanProfit : public Core {
	FloatVector mean_profit;
	BridgeAskBid* askbid;
	
public:
	SpreadMeanProfit();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf, 1);
		//reg.AddOut(, , SymTf, 1);
	}
};

class SpreadProfitDistribution : public Core {
	FloatVector mean, stddev;
	
public:
	SpreadProfitDistribution();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf, 1);
		//reg.AddOut(, , SymTf, 1);
	}
};

class SpreadProbability : public Core {
	FloatVector profit, real;
	
public:
	SpreadProbability();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf, 1);
		//reg.AddOut(, , SymTf, 1);
	}
};



class ValueChange : public Core {
	bool has_proxy;
	int proxy_id, proxy_factor;
	DataBridge* db;
	
public:
	ValueChange();
	
	virtual String GetStyle() const {return "";}
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, RealValue, SymTf, 1);
		reg.AddOut(IndiPhase, RealChangeValue, SymTf, 1);
		reg.AddOut(IndiPhase, RealProxyChangeValue, SymTf, 1);
		reg.AddOut(IndiPhase, SpreadProxyChangeValue, SymTf, 1);
		reg.AddOut(IndiPhase, RealLowChangeValue, SymTf, 1);
		reg.AddOut(IndiPhase, RealHighChangeValue, SymTf, 1);
	}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
};



class IdealOrders : public Core {
	SimBroker broker;
	
public:
	IdealOrders();
	
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealProxyChangeValue, SymTf, 1);
		reg.AddIn(IndiPhase, SpreadProxyChangeValue, SymTf, 1);
		reg.AddOut(IndiPhase, IdealOrderSignal, SymTf, 1);
	}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	
};

}

#endif
