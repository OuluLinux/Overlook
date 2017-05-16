#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory: http://www.saedsayad.com/decision_tree.htm

namespace Overlook {


class QueryTableForecaster : public Core {
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf, 1);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf, 1);
	}
};

class QueryTableHugeForecaster : public Core {
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All, 1);
		reg.AddOut(ForecastPhase, ForecastChangeValue, SymTf, 1);
	}
};

class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf, 1);
		reg.AddIn(ForecastPhase, ForecastChangeValue, Sym, 1);
		reg.AddIn(IndiPhase, ForecastChannelValue, Sym, 1);
		reg.AddInOptional(ForecastPhase, ForecastChangeValue, Sym, 5);
		reg.AddInOptional(IndiPhase, IndicatorValue, Sym, 5);
		reg.AddOut(ForecastCombPhase, ForecastChangeValue, SymTf, 1);
	}
};


class QueryTableAgent : public Core {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, SymTf, 1);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, SymTf, 1);
		reg.AddIn(IndiPhase, ForecastChannelValue, SymTf, 1);
		reg.AddIn(IndiPhase, IdealOrderSignal, SymTf, 1);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf, 1);
	}
};


class QueryTableHugeAgent : public Core {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All, 1);
		reg.AddIn(ForecastCombPhase, ForecastChangeValue, All, 1);
		reg.AddIn(IndiPhase, ForecastChannelValue, All, 1);
		reg.AddIn(IndiPhase, IdealOrderSignal, All, 1);
		reg.AddOut(AgentPhase, ForecastOrderSignal, SymTf, 1);
	}
};


class QueryTableMetaAgent : public Core {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, Sym, 1);
		reg.AddIn(AgentPhase, ForecastOrderSignal, Sym, 1);
		reg.AddInOptional(AgentPhase, ForecastOrderSignal, Sym, 5);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, SymTf, 1);
	}
};

class QueryTableDoubleAgent : public Core {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(IndiPhase, RealChangeValue, All, 1);
		reg.AddIn(AgentCombPhase, ForecastOrderSignal, All, 1);
		reg.AddOut(AgentCombPhase, ForecastOrderSignal, All, 1);
	}
};
	
}

#endif
