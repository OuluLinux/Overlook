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

struct Snap : Moveable<Snap> {
	uint64 data[2] = {0,0};
	
	void Copy(uint64* src) {data[0] = src[0]; data[1] = src[1];}
	void Set(int bit, bool b) {int s = bit/64; if (b) data[s] |= 1 << (bit-s*64); else data[s] &= ~(1 << (bit-s*64));}
	bool Get(int bit) const {int s = bit/64; return data[s] & (1 << (bit-s*64));}
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(data, sizeof(data)); else  s.Put(data, sizeof(data));}
	int GetDistance(const Snap& s) const {return PopCount64(data[0]) + PopCount64(data[1]);}
	
};
typedef Vector<Snap> VectorSnap;






class RuleAnalyzer : public WithRuleAnalyzer<TopWindow> {
	
protected:
	struct RADataCtrl : public Ctrl {
		RuleAnalyzer* ra = NULL;
		int cursor = 0;
		virtual void Paint(Draw& d);
	};
	
	struct OrganizationAgentCtrl : public Ctrl {
		RuleAnalyzer* ra = NULL;
		Vector<Point> pts;
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
	
	
	
	// Persistent
	Vector<Vector<OnlineAverageWindow1> > av_wins;
	Vector<Vector<Vector<double> > > volat_divs;
	Vector<Vector<VectorMap<int,int> > > median_maps;
	Vector<double> training_pts;
	BitOptimizer opt;
	Vector<VectorSnap> data_in, data_out;
	int cursor = 0;
	int phase = 0;
	bool is_enabled = false;
	
	
	// Temporary
	SliderCtrl data_slider;
	RADataCtrl data_ctrl;
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > ci_queue;
	OrganizationAgentCtrl agentctrl;
	double prev_speed = 1.0;
	int current = 0;
	int perc = 0, actual = 0, total = 1;
	int processed_cursor = 0, data_cursor = 0, realtime_count = 0;
	bool is_prepared = false;
	bool running = false, stopped = true;
	
	
	void StartProcess();
	void StopProcess();
	void PostStartProcess() {PostCallback(THISBACK(StartProcess));}
	void PostStopProcess() {PostCallback(THISBACK(StopProcess));}
	void Process();
	void ProcessIteration();
	void Prepare();
	void ProcessData();
	void ProcessSignalInitial();
	void ProcessSignalPrediction();
	void IterateTrain();
	void IterateTest();
	void IterateJoining();
	
	double RunAgent(bool test);
	String GetSignalString(int i);
	
public:
	typedef RuleAnalyzer CLASSNAME;
	RuleAnalyzer();
	~RuleAnalyzer();
	
	void Refresh();
	bool RefreshReal();
	void Data();
	void PostData() {PostCallback(THISBACK(Data));}
	void SetCursor() {data_ctrl.cursor = data_slider.GetData(); data_ctrl.Refresh();}
	void SetEnable() {is_enabled = enable.Get();}
	
	void Serialize(Stream& s) {
		s % av_wins % volat_divs % median_maps % training_pts % opt % data_in % data_out % cursor % phase % is_enabled;
	}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ruleanalyzer.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ruleanalyzer.bin"));}
	
	
	// Constants
	enum {TRAIN, TEST, FINISHED};
	enum {OPEN_LONG, OPEN_SHORT, CLOSE_LONG, CLOSE_SHORT, SUSTAIN, BLOCK, ATTENTION1, ATTENTION5, ATTENTION15, signal_count};
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
		INPUT_COUNT
	};
	const int trsh_bits = 5;
	const int row_size = period_count * INPUT_COUNT;
	const int volat_div = 6;
	const int opt_row_size = (row_size + trsh_bits) * 2;
	
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
