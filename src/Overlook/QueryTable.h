#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory: http://www.saedsayad.com/decision_tree.htm

namespace Overlook {


class QueryTableForecaster : public Pipe {
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, SymTf, 1));
		out.Add(ValueType(ForecastPhase, ForecastChangeValue, SymTf, 1));
	}
};

class QueryTableHugeForecaster : public Pipe {
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, All, 1));
		out.Add(ValueType(ForecastPhase, ForecastChangeValue, SymTf, 1));
	}
};

class QueryTableMetaForecaster : public Pipe {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, SymTf, 1));
		in.Add(ValueType(ForecastPhase, ForecastChangeValue, Sym, 1));
		in.Add(ValueType(IndiPhase, ForecastChannelValue, Sym, 1));
		in_opt.Add(ValueType(ForecastPhase, ForecastChangeValue, Sym, 5));
		in_opt.Add(ValueType(IndiPhase, IndicatorValue, Sym, 5));
		out.Add(ValueType(ForecastCombPhase, ForecastChangeValue, SymTf, 1));
	}
};


class QueryTableAgent : public Pipe {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, SymTf, 1));
		in.Add(ValueType(ForecastCombPhase, ForecastChangeValue, SymTf, 1));
		in.Add(ValueType(IndiPhase, ForecastChannelValue, SymTf, 1));
		in.Add(ValueType(IndiPhase, IdealOrderSignal, SymTf, 1));
		out.Add(ValueType(AgentPhase, ForecastOrderSignal, SymTf, 1));
	}
};


class QueryTableHugeAgent : public Pipe {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, All, 1));
		in.Add(ValueType(ForecastCombPhase, ForecastChangeValue, All, 1));
		in.Add(ValueType(IndiPhase, ForecastChannelValue, All, 1));
		in.Add(ValueType(IndiPhase, IdealOrderSignal, All, 1));
		out.Add(ValueType(AgentPhase, ForecastOrderSignal, SymTf, 1));
	}
};


class QueryTableMetaAgent : public Pipe {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, Sym, 1));
		in.Add(ValueType(AgentPhase, ForecastOrderSignal, Sym, 1));
		in_opt.Add(ValueType(AgentPhase, ForecastOrderSignal, Sym, 5));
		out.Add(ValueType(AgentCombPhase, ForecastOrderSignal, SymTf, 1));
	}
};

class QueryTableDoubleAgent : public Pipe {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealChangeValue, All, 1));
		in.Add(ValueType(AgentCombPhase, ForecastOrderSignal, All, 1));
		out.Add(ValueType(AgentCombPhase, ForecastOrderSignal, All, 1));
	}
};
	
}

#endif
