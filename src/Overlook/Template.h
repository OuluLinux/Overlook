#ifndef _Overlook_Template_h_
#define _Overlook_Template_h_

/*
	Template for all advanced custom core classes
	
	Features:
	 - takes slower tf instances as inputs (higher priority) because the slower has higher
	   probability succeed in the longer run (which is common knowledge and also measured).
	 - takes all other symbols as inputs, to handle better the web effect of market tickers
*/

namespace Overlook {

class Template : public Core {
	DecisionTreeNode tree;
	QueryTable qt;
	int corr_period, max_timesteps, steps, peek;
	int arg_learningmode, arg_targetperiod, arg_usetime, arg_usetrend, arg_usecorr, arg_usetrad, arg_usechan;
	
public:
	typedef Template CLASSNAME;
	Template();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		for(int i = 0; i < max_sources; i++)
			reg % In<Template>(&TemplateIn);
		
		for(int i = 0; i < max_traditional; i++)
			reg % InOptional();
		
		reg % In<DataBridge>()
			% In<ValueChange>()
			% Out(3, 3)
		
			// 4 main arguments
			% Arg("Learning template (decision tree vs neural network vs similarity)", arg_learningmode, 0, 15)
			% Arg("Target period", arg_targetperiod, 0, 31)
			% Arg("Use time", arg_usetime, 0, 1)
			% Arg("Use trend", arg_usetrend, 0, 1)
			% Arg("Use correlation", arg_usecorr, 0, 1)
			% Arg("Use traditional indicators", arg_usetrad, 0, 1)
			% Arg("Use channel", arg_usechan, 0, 1)
			
			% Arg("Correlation period", corr_period, 2, 16);
	}
	
	static bool TemplateIn(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		
		
		
		return false;
	}
};

}

#endif
