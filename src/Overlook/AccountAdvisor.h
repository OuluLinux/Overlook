#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;

class AccountAdvisor : public Core {
	
	struct MainOptimizationCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	// Persistent
	int							prev_counted		= 0;
	
	
	// Temp
	Vector<ConstBuffer*> inputs;
	Vector<double> spread_point;
	int realtime_count = 0;
	bool forced_optimizer_reset = false;
	bool once = true;
	
	
protected:
	virtual void Start();
	
	void SetRealArea();
	void RefreshMain();
	void MainReal();
	
	SimBroker sb;
	void RunSimBroker();
	
public:
	typedef AccountAdvisor CLASSNAME;
	AccountAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction0)
			% In<MainAdvisor>(&FilterFunction1)
			% Out(1, 1)
			% Mem(prev_counted);
	}
	
	static bool FilterFunction0(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf  == out_tf;
		
		if (in_sym == out_sym)
			return true;
		
		return ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
	static bool FilterFunction1(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf  == out_tf;
		
		return in_sym == out_sym;
	}
	
};


}

#endif
