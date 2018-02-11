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

class RuleAnalyzer : public WithRuleAnalyzer<TopWindow> {
	
protected:
	friend class RADataCtrl;
	
	// Persistent
	Vector<Vector<BeginStats> > stats;
	Vector<VectorBool> data;
	int cursor = 0;
	int phase = 0, joinlevel = 0;
	bool is_enabled = false;
	
	
	// Temporary
	Array<Thread> thrds;
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > ci_queue;
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
	void RealizeStats();
	void Iterate(int current, int type);
	void IterateJoining();
	
	double GetBitProbBegin(int symbol, int begin_id);
	double GetBitProbTest(int symbol, int begin_id, int type, int sub_id, bool inv_action);
	
public:
	typedef RuleAnalyzer CLASSNAME;
	RuleAnalyzer();
	~RuleAnalyzer();
	
	void ProcessRealtime();
	void Refresh();
	bool RefreshReal();
	void Data();
	void PostData() {PostCallback(THISBACK(Data));}
	void SetCursor() {data_check.cursor = datactrl_cursor.GetData(); data_check.Refresh();}
	void SetEnable() {is_enabled = enable.Get();}
	
	void Serialize(Stream& s) {s % stats % data % cursor % phase % joinlevel % is_enabled;}
	void LoadThis() {LoadFromFile(*this, ConfigFile("ruleanalyzer.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("ruleanalyzer.bin"));}
	
	
	// Constants
	enum {BEGIN, SUSTAIN, END, FINISHED};
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
	const int row_size = period_count * COUNT;
	const int rt_row_size = 2;
	const int JOINLEVEL_COUNT = 3;
	const int volat_div = 6;
	const int LOOP_COUNT = 10;
	const int BEGIN_PEEK = 2;
	const int SUSTAIN_PEEK = 6;
	const int END_PEEK = 6;
	
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
