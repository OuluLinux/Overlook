#ifndef _Overlook_AccountAdvisor_h_
#define _Overlook_AccountAdvisor_h_

namespace Overlook {
using namespace Upp;


class WeekSlotAdvisor : public Core {
	
	enum {RF_IDLE, RF_TRAINING, RF_OPTIMIZING, RF_IDLEREAL, RF_TRAINREAL, RF_REAL};
	
	
	// Persistent
	GeneticOptimizer optimizer;
	int phase = 0;
	
	// Temp
	Vector<double> trial;
	bool running = false;
	
	
protected:
	virtual void Start();
	
	void Optimizing();
	void RefreshOutputBuffers();
	
public:
	typedef WeekSlotAdvisor CLASSNAME;
	WeekSlotAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<RandomForestAdvisor>(&FilterFunction)
			% Out(1, 1)
			% Mem(optimizer)
			% Mem(phase);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf  == out_tf;
		
		return ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
	
	void RunMain();
	
};


}

#endif
