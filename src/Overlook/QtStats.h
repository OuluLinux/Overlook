#ifndef _Overlook_QtStats_h_
#define _Overlook_QtStats_h_

namespace Overlook {

class QtStats : public Core {
	DecisionTreeNode tree;
	QueryTable qt;
	Vector<ConstBuffer*> input_template_buffers, input_traditional_buffers, input_slower_buffers;
	int corr_period, max_timesteps, steps, peek;
	int arg_usetime, arg_usetrend, arg_usecorr;
	int input_template_begin;
	
public:
	typedef QtStats CLASSNAME;
	QtStats();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<ValueChange>()
			% Out(3, 3)
		
			// 4 main arguments
			% Arg("Use time", arg_usetime, 0, 1)
			% Arg("Use trend", arg_usetrend, 0, 1)
			% Arg("Use correlation", arg_usecorr, 0, 1)
			
			% Arg("Correlation period", corr_period, 2, 16);
	}
	
	static bool QtStatsIn(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		
		
		
		return false;
	}
};

}

#endif
