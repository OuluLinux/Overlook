#if 0


class IdealOrders : public Core {
	SimBroker broker;
	
public:
	IdealOrders();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % Out(IndiPhase, IdealOrderSignal, SymTf);
	}
	
	virtual void Init();
	virtual void Start();
	
};





































class QueryTableForecaster : public Core {
	QueryTable qt;
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(IndiPhase, RealChangeValue, SymTf)
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastChangeValue, SymTf, 1, 1);
	}
};


class QueryTableHugeForecaster : public Core {
	Array<DecisionTreeNode> tree;
	QueryTable qt;
	int corr_period;
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction)
			% InDynamic(IndiPhase, RealChangeValue, &FilterFunction)
			% InDynamic(IndiPhase, CorrelationValue, &FilterFunctionSym)
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastChangeValue, SymTf, 6, 6)
			% Arg("Correlation period", corr_period);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return out_tf >= in_tf;
		return true;
	}
	static bool FilterFunctionSym(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return out_tf >= in_tf;
		return out_sym == in_sym;
	}
};


class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf)
			% In(ForecastPhase, ForecastChangeValue, Sym)
			% In(IndiPhase, ForecastChannelValue, Sym)
			% InOptional(ForecastPhase, ForecastChangeValue, Sym)
			% InOptional(IndiPhase, IndicatorValue, Sym)
			% Out(ForecastCombPhase, ForecastChangeValue, SymTf);
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
		///reg % In(AgentPhase, ForecastOrderSignal, Sym);
		//reg % InOptional(AgentPhase, ForecastOrderSignal, Sym);
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


#endif
