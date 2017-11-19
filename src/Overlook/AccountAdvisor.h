#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;


class WeekSlotAdvisor : public Core {
	
	enum {WS_IDLE, WS_TRAINING, WS_REAL};
	
	
	// Persistent
	GeneticOptimizer optimizer;
	int prev_counted = 0;
	int phase = WS_IDLE;
	int weekslots = 0;
	
	
	// Temp
	Vector<ConstBuffer*> inputs, signals, weights;
	Vector<double> trial;
	ForestArea area;
	FixedSimBroker sb;
	double area_change_total[3];
	int realtime_count = 0;
	bool running = false;
	bool forced_optimizer_reset = false;
	
	
protected:
	virtual void Start();
	
	void MainTraining();
	
public:
	typedef WeekSlotAdvisor CLASSNAME;
	WeekSlotAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction0)
			% In<RandomForestAdvisor>(&FilterFunction1)
			% Out(SYM_COUNT, SYM_COUNT)
			% Mem(optimizer)
			% Mem(phase)
			% Mem(weekslots);
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
	
	void SearchSources();
	void MainOptimizer();
	void RunMain();
	void SetTrainingArea();
	void SetRealArea();
	void RefreshMainBuffer(bool forced);
	void RefreshMain();
	void NormalizeTrial();
	void RefreshInputs();
	void MainReal();
	
};


}

#endif
