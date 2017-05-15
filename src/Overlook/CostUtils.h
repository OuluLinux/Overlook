#ifndef _Overlook_SpreadUtils_h_
#define _Overlook_SpreadUtils_h_


namespace Overlook {


class SpreadStats : public Pipe {
	FloatVector mean_cost;
	BridgeAskBid* askbid;
	Vector<OnlineVariance> stats;
	
public:
	SpreadStats();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class SpreadMeanProfit : public Pipe {
	FloatVector mean_profit;
	BridgeAskBid* askbid;
	
public:
	SpreadMeanProfit();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class SpreadProfitDistribution : public Pipe {
	FloatVector mean, stddev;
	
public:
	SpreadProfitDistribution();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class SpreadProbability : public Pipe {
	FloatVector profit, real;
	
public:
	SpreadProbability();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class ValueChange : public Pipe {
	bool has_proxy;
	int proxy_id, proxy_factor;
	DataBridge* db;
	
public:
	ValueChange();
	
	virtual String GetStyle() const {}
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(SourcePhase, RealValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, RealChangeValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, RealProxyChangeValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, SpreadProxyChangeValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, RealLowChangeValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, RealHighChangeValue, SymTf, 1));
	}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
};



class IdealOrders : public Pipe {
	SimBroker broker;
	
public:
	IdealOrders();
	
	virtual String GetStyle() const;
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, RealProxyChangeValue, SymTf, 1));
		in.Add(ValueType(IndiPhase, SpreadProxyChangeValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, IdealOrderSignal, SymTf, 1));
	}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	
};

}

#endif
