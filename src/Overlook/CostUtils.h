#ifndef _Overlook_SpreadUtils_h_
#define _Overlook_SpreadUtils_h_


namespace Overlook {


class SpreadStats : public Core {
	
public:
	SpreadStats();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, AskBidValue, SymTf);
		reg.AddOut(SourcePhase, SpreadValue, SymTf, 1, 1);
	}
};

class SpreadMeanProfit : public Core {
	Vector<double> mean_profit;
	BridgeAskBid* askbid;
	
public:
	SpreadMeanProfit();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class SpreadProfitDistribution : public Core {
	Vector<double> mean, stddev;
	
public:
	SpreadProfitDistribution();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class SpreadProbability : public Core {
	Vector<double> profit, real;
	
public:
	SpreadProbability();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
		reg.AddIn(SourcePhase, RealValue, Sym);
		reg.AddOut(IndiPhase, RealChangeValue, SymTf);
		reg.AddOut(IndiPhase, RealProxyChangeValue, SymTf);
		reg.AddOut(IndiPhase, SpreadProxyChangeValue, SymTf);
		reg.AddOut(IndiPhase, RealLowChangeValue, SymTf);
		reg.AddOut(IndiPhase, RealHighChangeValue, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};



class IdealOrders : public Core {
	SimBroker broker;
	
public:
	IdealOrders();
	
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealProxyChangeValue, SymTf);
		reg.AddIn(IndiPhase, SpreadProxyChangeValue, SymTf);
		reg.AddOut(IndiPhase, IdealOrderSignal, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};

}

#endif
