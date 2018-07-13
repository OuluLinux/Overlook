#ifndef _Overlook_MultiMultiExpertAdvisor_h_
#define _Overlook_MultiMultiExpertAdvisor_h_



namespace Overlook {


class MultiExpertAdvisor : public Core {
	
	struct OptimizationCtrl : public Ctrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
		int type = 0;
		JobCtrl* job;
	};
	
	struct TrainingCtrl : public JobCtrl {
		TabCtrl tabs;
		OptimizationCtrl opt0, opt1, opt2;
		
		TrainingCtrl();
	};
	
	struct Setting : Moveable<Setting> {
		int start, stop, timeofday;
	};
	
	static const int args_per_ea = 2;
	
	
	// Persistent
	Optimizer opt;
	Vector<double> training_pts, besteq_pts, cureq_pts;
	Vector<int> cursors;
	SimBroker sb;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	int input_size = 0;
	
	
	// Temporary
	TimeStop save_elapsed;
	Vector<double> tmp_mat;
	double point = 0.0001;
	int max_rounds = 0;
	int pos = 0;
	bool once = true;
	
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	void DumpTest();
	
public:
	typedef MultiExpertAdvisor CLASSNAME;
	MultiExpertAdvisor();
	virtual ~MultiExpertAdvisor() {StoreCache();}
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
		
		#if 0
			% In<Snake>()
			% In<ChannelScalper>()
			% In<Eagle>()
			% In<Salmon>()
			% In<Explorer>()
			% In<Mari>()
			% In<SuperMart>()
			% In<Spiral>()
			% In<MoneyTime>()
			% In<StochPower>()
			% In<Gainer>()
			% In<Cashier>()
			% In<ModestTry>()
			% In<Gaia>()
			% In<Squirter>()
			% In<Julia>()
			% In<Outsider>()
			% In<Foster>()
			% In<Spectrum>()
			% In<Maverick>()
			% In<President>()
			% In<Gatherer>()
			% In<Musk>()
			% In<Puma>()
			% In<Rose>()
			% In<Thief>()
			% In<Sculptor>()
			% In<Starter>()
			% In<Turtle>()
			% In<Unreal>()
			% In<ProfitChance>()
			% In<Rabbit>()
			% In<FxOne>()
			% In<Sunrise>()
			% In<YetAnother>()
			% In<Hornet>()
		#elif 0
		// M15
			% In<Eagle>()
			% In<Thief>()
			% In<Starter>()
		#elif 0
		// Clearly martingale only
			% In<Salmon>()
			% In<SuperMart>()
			% In<Gainer>()
			% In<ModestTry>()
			% In<Julia>()
			% In<Musk>()
			% In<Rose>()
			% In<Thief>()
			% In<Sculptor>()
			% In<Starter>()
			% In<Turtle>()
			% In<ProfitChance>()
			% In<Rabbit>()
			% In<Sunrise>()
		#else
		// M1 list
			% In<Eagle>()
			% In<Explorer>()
			% In<ModestTry>()
			% In<Thief>()
		#endif
			% Lbl(1)
			% Mem(opt)
			% Mem(training_pts)
			% Mem(besteq_pts)
			% Mem(cureq_pts)
			% Mem(cursors)
			% Mem(sb)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted)
			% Mem(input_size)
			;
	}
	
};

}

#endif
