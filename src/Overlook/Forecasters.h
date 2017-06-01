#ifndef _Overlook_Forecasters_h_
#define _Overlook_Forecasters_h_

namespace Overlook {

class Edge : public Core {
	QueryTable qt;
	
public:
	Edge();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Ideal : public Core {
	QueryTable qt;
	
public:
	Ideal();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Custom : public Core {
	Array<DecisionTreeNode> tree;
	QueryTable qt;
	int corr_period, max_timesteps, steps;
	
public:
	typedef Custom CLASSNAME;
	Custom();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction)
			% InDynamic(IndiPhase, RealChangeValue, &FilterFunction)
			% InDynamic(IndiPhase, CorrelationValue, &FilterFunctionSym)
			% InSlower()
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

class Feature : public Core {
	QueryTable qt;
	
public:
	Feature();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Channel : public Core {
	QueryTable qt;
	
public:
	Channel();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class SlowTarget : public Core {
	QueryTable qt;
	
public:
	SlowTarget();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Average : public Core {
	QueryTable qt;
	
public:
	Average();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Trending : public Core {
	QueryTable qt;
	
public:
	Trending();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class Combiner : public Core {
	QueryTable qt;
	
public:
	Combiner();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InSlower()
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

}

#endif
