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
		reg.AddIn(SourcePhase, TimeValue, SymTf);
		reg.AddIn(SourcePhase, AskBidValue, SymTf);
		reg.AddOut(SourcePhase, SpreadValue, SymTf, 1, 1);
	}
};

class SpreadMeanProfit : public Core {
	BridgeAskBid* askbid;
	
public:
	SpreadMeanProfit();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, TimeValue, SymTf);
		reg.AddIn(SourcePhase, RealValue, SymTf);
		reg.AddIn(SourcePhase, AskBidValue, SymTf);
		reg.AddOut(SourcePhase, SpreadOvercomeValue, SymTf, 1, 1);
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
		reg.AddIn(SourcePhase, TimeValue, SymTf);
		reg.AddIn(SourcePhase, AskBidValue, SymTf);
		reg.AddIn(IndiPhase, SubTfValue, SymTf);
		reg.AddOut(SourcePhase, SpreadOvercomeDistValue, SymTf, 2, 2);
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
		reg.AddIn(SourcePhase, TimeValue, SymTf);
		reg.AddIn(SourcePhase, RealValue, SymTf);
		reg.AddIn(SourcePhase, SpreadOvercomeDistValue, SymTf);
		reg.AddOut(SourcePhase, SpreadProbValue, SymTf, 2, 2);
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
		reg.AddIn(SourcePhase, SpreadValue, Tf);
		reg.AddOut(IndiPhase, RealChangeValue, SymTf, 7, 7);
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
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddOut(IndiPhase, IdealOrderSignal, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};

}

#endif
