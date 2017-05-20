#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory: http://www.saedsayad.com/decision_tree.htm

namespace Overlook {


class QueryTableForecaster : public Core {
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf);
	}
};

class QueryTableHugeForecaster : public Core {
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf);
	}
};

class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddIn(IndiPhase, ForecastChannelValue, Sym);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym);
		reg.AddOut(ForecastCombPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableAgent : public Core {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, SymTf);
		reg.AddIn(IndiPhase, ForecastChannelValue, SymTf);
		reg.AddIn(IndiPhase, IdealOrderSignal, SymTf);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableHugeAgent : public Core {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, All);
		reg.AddIn(IndiPhase, ForecastChannelValue, All);
		reg.AddIn(IndiPhase, IdealOrderSignal, All);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableMetaAgent : public Core {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, Sym);
		reg.AddIn(AgentPhase, ForecastOrderSignal, Sym);
		reg.AddInOptional(AgentPhase, ForecastOrderSignal, Sym);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, SymTf);
	}
};

class QueryTableDoubleAgent : public Core {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All);
		reg.AddIn(AgentCombPhase, ForecastOrderSignal, All);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, All);
	}
};
	
}

#endif
