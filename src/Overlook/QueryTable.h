#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory: http://www.saedsayad.com/decision_tree.htm

namespace Overlook {


class QueryTableForecaster : public Core {
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % Out(ForecastPhase, ForecastChangeValue, SymTf);
	}
};

class QueryTableHugeForecaster : public Core {
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % Out(ForecastPhase, ForecastChangeValue, SymTf);
	}
};

class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % In(ForecastPhase, ForecastChangeValue, Sym);
		reg % In(IndiPhase, ForecastChannelValue, Sym);
		reg % InOptional(ForecastPhase, ForecastChangeValue, Sym);
		reg % InOptional(IndiPhase, IndicatorValue, Sym);
		reg % Out(ForecastCombPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableAgent : public Core {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % In(ForecastCombPhase, ForecastChangeValue, SymTf);
		reg % In(IndiPhase, ForecastChannelValue, SymTf);
		reg % In(IndiPhase, IdealOrderSignal, SymTf);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableHugeAgent : public Core {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(ForecastCombPhase, ForecastChangeValue, All);
		reg % In(IndiPhase, ForecastChannelValue, All);
		reg % In(IndiPhase, IdealOrderSignal, All);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableMetaAgent : public Core {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, Sym);
		reg % In(AgentPhase, ForecastOrderSignal, Sym);
		reg % InOptional(AgentPhase, ForecastOrderSignal, Sym);
		reg % Out(AgentCombPhase, ForecastOrderSignal, SymTf);
	}
};

class QueryTableDoubleAgent : public Core {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(AgentCombPhase, ForecastOrderSignal, All);
		reg % Out(AgentCombPhase, ForecastOrderSignal, All);
	}
};
	
}

#endif
