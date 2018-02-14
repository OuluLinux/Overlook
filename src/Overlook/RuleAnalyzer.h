#ifndef _Overlook_RuleAnalyzer_h_
#define _Overlook_RuleAnalyzer_h_

namespace Overlook {

struct BitStats : Moveable<BitStats> {
	double prob_av = 0.0, succ_idx = 0.0;
	
	void Serialize(Stream& s) {s % prob_av % succ_idx;}
};

struct BeginStats : Moveable<BeginStats> {
	bool type;
	BitStats initial;
	Index<int> begin_bits, sust_bits, end_bits;
	Vector<BitStats> begins;
	Vector<BitStats> sustains;
	Vector<BitStats> ends;
	
	void Serialize(Stream& s) {s % type % initial % begin_bits % sust_bits % end_bits % begins % sustains % ends;}
};



class OrganizationAgent {
	
	bool is_init = false;
	
	
public:
	typedef OrganizationAgent CLASSNAME;
	OrganizationAgent();
	void Init();
	
	bool IsInit() const {return is_init;}
	
	
	
	void Serialize(Stream& s) {}
	
};

class OrganizationAgentCtrl : public ParentCtrl {
	
};

class RuleAnalyzer : public WithRuleAnalyzer<TopWindow> {
	
protected:
	struct RADataCtrl : public Ctrl {
		RuleAnalyzer* ra = NULL;
		int cursor = 0;
		virtual void Paint(Draw& d);
	};
	
	struct Order : Moveable<Order> {
		bool label;
		int start, stop, len;
		double av_change, err;
		double av_idx, err_idx, len_idx;
		double idx, idx_norm;
		void Serialize(Stream& s) {s % label % start % stop % len % av_change % err % av_idx % err_idx % len_idx % idx % idx_norm;}
	};
	
	struct Snap : Moveable<Snap> {
		dword[4] data;
	};
	typedef Vector<Snap> VectorSnap;
	
	
	// Persistent
	Vector<Vector<OnlineAverageWindow1> > av_wins;
	Vector<Vector<Vector<double> > > volat_divs;
	Vector<Vector<VectorMap<int,int> > > median_maps;
	OrganizationAgent agent;
	Vector<VectorSnap> data;
	Vector<VectorSnap> rtdata;
	int cursor = 0;
	int phase = 0;
	bool is_enabled = false;
	
	
	// Temporary
	SliderCtrl data_slider;
	RADataCtrl data_ctrl;
	Array<Thread> thrds;
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > ci_queue;
	OrganizationAgentCtrl agentctrl;
	Atomic not_stopped, thrd_current, waiting;
	double prev_speed = 1.0;
	int perc = 0, actual = 0, total = 1;
	int processed_cursor = 0, data_cursor = 0, realtime_count = 0;
	bool is_prepared = false;
	bool running = false, stopped = true;
	
	
	void StartProcess();
	void StopProcess();
	void PostStartProcess() {PostCallback(THISBACK(StartProcess));}
	void PostStopProcess() {PostCallback(THISBACK(StopProcess));}
	void Process(int thrd_id);
	void ProcessIteration(int thrd_id);
	void Prepare();
	void ProcessData();
	void ProcessSignalInitial();
	void ProcessSignalPrediction();
	void IterateTrain(int current);
	void IterateTest(int current);
	void IterateJoining();
	
	String GetSignalString(int i);
	
public:
	typedef RuleAnalyzer CLASSNAME;
	RuleAnalyzer();
	~RuleAnalyzer();
	
	void ProcessRealtime();
	void Refresh();
	bool RefreshReal();
	void Data();
	void PostData() {PostCallback(THISBACK(Data));}
	void SetCursor() {data_ctrl.cursor = data_slider.GetData(); data_ctrl.Refresh();}
	void SetEnable() {is_enabled = enable.Get();}
	
	void Serialize(Stream& s) {s % av_wins % volat_divs % median_maps % agent % data % rtdata % cursor % phase % is_enabled;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ruleanalyzer.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ruleanalyzer.bin"));}
	
	
	// Constants
	enum {TRAIN, TEST, FINISHED};
	enum {OPEN, CLOSE, SUSTAIN, BLOCK, ATTENTION, signal_count};
	enum {RT_SIGNAL, RT_ENABLED, rt_row_size};
	#ifdef flagDEBUG
	const int period_count = 1;
	#else
	const int period_count = 6;
	#endif
	enum {ONLINEMINLAB, TRENDINDEX,
		#ifndef flagDEBUG
		VOLATCTX0, VOLATCTX1, VOLATCTX2, VOLATCTX3, VOLATCTX4, VOLATCTX5, MA, MOM, OTREND, HTREND, LTREND, RTRENDUP, RTRENDDOWN,
		#endif
		COUNT
	};
	const int trsh_bits = 5;
	const int row_size = period_count * COUNT;
	const int volat_div = 6;
	
	static const String GetTypeString(int id) {
		switch (id) {
			case ONLINEMINLAB:	return "Online Minimum Label";
			case TRENDINDEX:	return "Trend Index";
			#ifndef flagDEBUG
			case VOLATCTX0:		return "Volatility Ctx #1";
			case VOLATCTX1:		return "Volatility Ctx #2";
			case VOLATCTX2:		return "Volatility Ctx #3";
			case VOLATCTX3:		return "Volatility Ctx #4";
			case VOLATCTX4:		return "Volatility Ctx #5";
			case VOLATCTX5:		return "Volatility Ctx #6";
			case MA:			return "Moving Average";
			case MOM:			return "Momentum";
			case OTREND:		return "Open-trend";
			case HTREND:		return "High-trend";
			case LTREND:		return "Low-trend";
			case RTRENDUP:		return "Up trend reversal";
			case RTRENDDOWN:	return "Down trend reversal";
			#endif
			default:			return "INVALID";
		}
	}
};


inline RuleAnalyzer& GetRuleAnalyzer() {return Single<RuleAnalyzer>();}

}

#endif
