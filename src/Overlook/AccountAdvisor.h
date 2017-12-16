#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;

class WeekSlotAdvisor : public Core {
	
	struct MainOptimizationCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	// Persistent
	int prev_counted = 0;
	
	
	// Temp
	Vector<Vector<ConstVectorBool*> > signals, enabled;
	Vector<ConstBuffer*> inputs;
	Vector<double> spread_point;
	Vector<Vector<int> > time_slots;
	Index<int> tf_ids;
	Vector<int> ratios;
	ForestArea area;
	int realtime_count = 0;
	int tfmins = 0;
	int weekslots = 0;
	int cols = 0;
	bool forced_optimizer_reset = false;
	bool once = true;
	
	
protected:
	virtual void Start();
	
	void SetRealArea();
	void RefreshMain();
	void MainReal();
	void OptimizeLimit(double chg_limit, double slow_limit);
	
	SimBroker sb;
	void RunSimBroker();
	
public:
	typedef WeekSlotAdvisor CLASSNAME;
	WeekSlotAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction0)
			% In<VolatilitySlots>(&FilterFunction1)
			% In<DqnAdvisor>(&FilterFunction1)
			% Out(1, 1)
			% Mem(prev_counted);
	}
	
	static bool FilterFunction0(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf  == out_tf || IsTfUsed(::Overlook::GetSystem().GetPeriod(out_tf));
		
		if (in_sym == out_sym)
			return true;
		
		return ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
	static bool IsTfUsed(int tf_mins) {return tf_mins == 15 || tf_mins == 60 || tf_mins == 240;}
	
	static bool FilterFunction1(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1) {
			if (out_tf < in_tf)
				return false;
			
			int tf_mins = ::Overlook::GetSystem().GetPeriod(out_tf);
			if (IsTfUsed(tf_mins))
				return true;
			
			return false;
		}
		
		return ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
};


}

#endif
