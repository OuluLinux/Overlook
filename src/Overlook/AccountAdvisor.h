#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;

#define ACCURACY

class WeekSlotAdvisor : public Core {
	
	struct MainOptimizationCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	// Persistent
	GeneticOptimizer optimizer;
	Vector<double> optimization_pts;
	double area_change_total[3];
	int prev_counted = 0;
	
	
	// Temp
	Vector<ConstBuffer*> inputs, signals, weights;
	Vector<double> trial;
	ForestArea area;
	int realtime_count = 0;
	int tfmins = 0, slotmins = 0;
	int weekslots = 0;
	int cols = 0;
	bool forced_optimizer_reset = false;
	
	
protected:
	virtual void Start();
	
	bool MainOptimizationBegin();
	bool MainOptimizationIterator();
	bool MainOptimizationEnd();
	bool MainOptimizationInspect();
	void NormalizeTrial();
	void RefreshMainBuffer(bool forced);
	void RefreshInputs();
	void RunMain();
	void SetTrainingArea();
	void SetRealArea();
	void RefreshMain();
	void MainReal();
	
	#ifdef ACCURACY
	SimBroker sb;
	void RunSimBroker();
	#else
	FixedSimBroker sb;
	#endif
	
public:
	typedef WeekSlotAdvisor CLASSNAME;
	WeekSlotAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction0)
			% In<DqnAdvisor>(&FilterFunction1)
			% Out(SYM_COUNT, SYM_COUNT)
			% Mem(optimizer)
			% Mem(optimization_pts)
			% Mem(area_change_total[0]) % Mem(area_change_total[1]) % Mem(area_change_total[2])
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
		
		return ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
};


}

#endif
