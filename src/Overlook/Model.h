#ifndef _Overlook_Model_h_
#define _Overlook_Model_h_

/*
	Model for all advanced custom core classes
	
	Features:
	 - takes slower tf instances as inputs (higher priority) because the slower has higher
	   probability succeed in the longer run (which is common knowledge and also measured).
	 - takes all other symbols as inputs, to handle better the web effect of market tickers
*/

namespace Overlook {

class Model : public Core {
	DecisionTreeNode tree;
	QueryTable qt;
	int corr_period, max_timesteps, steps, peek;
	int arg_priority, arg_targetmode, arg_reason, arg_level;
	
public:
	typedef Model CLASSNAME;
	Model();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, Sym)
			% In(IndiPhase, RealChangeValue, Sym)
			% In(IndiPhase, CorrelationValue, SymTf)
			% InHigherPriority()
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastRealValue, SymTf, 3, 3)
			
			// 4 main arguments
			% Arg("Learning model (decision tree vs neural network vs similarity)", arg_priority, 0, 15)
			% Arg("Priority (moment vs probable target)", arg_priority, 0, 15)
			% Arg("Target (heuristic vs scheduled)", arg_targetmode, 0, 15)
			% Arg("Reason (match past vs match more probable)", arg_reason, 0, 15)
			% Arg("Level (low level vs high level, immediate values vs derived indicator values)", arg_level, 0, 15)
			
			% Arg("Correlation period", corr_period, 2, 16);
	}
	
};

}

#endif
