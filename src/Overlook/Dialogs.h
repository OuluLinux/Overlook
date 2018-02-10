#ifndef _Overlook_Dialogs_h_
#define _Overlook_Dialogs_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include <plugin/libmt/libmt.h>


namespace Overlook {


// Required forward declarations
class ChartManager;
class Chart;
class BacktestSettingsDialog;


// Dialog for manual opening of the new order
class NewOrderWindow : public TopWindow {
	
protected:
	
	// Vars
	enum {VIEW_MARKETEXECUTION, VIEW_PENDINGORDER, VIEW_MODIFY, VIEW_ERROR};
	TimeCallback bidask_timer;
	double mod_point;
	double ask, bid;
	int view;
	libmt::Order modify_order;
	
	// Protected main functions to prevent direct (wrong) usage
	void RefreshBidAsk();
	void DoModifying();
	void DoModifyingPending();
	void DoDeleting();
	void HandleReturnValue(bool success);
	
public:
	
	// Ctors
	typedef NewOrderWindow CLASSNAME;
	NewOrderWindow();
	
	// Main functions
	void Init(int id);
	void PlaceOrder();
	void NewOrder(int type);
	void SetView(int i);
	void SetTypeView();
	void Data();
	void Modify(const libmt::Order& o);
	void RefreshModifyPointCopyValues();
	void CopyStopLoss();
	void CopyTakeProfit();
	void PostClose() {PostCallback(THISBACK(Close0));}
	void Close0() {Close();}
	virtual bool Key(dword key, int count);
	
	
	// Ctrls for part of the dialog
	struct Dialog : public WithNewOrderWindow<Ctrl> {Dialog() {CtrlLayout(*this);}};
	struct MarketExecution : public WithMarketExecution<Ctrl> {MarketExecution() {CtrlLayout(*this);}};
	struct PendingOrder : public WithPendingOrder<Ctrl> {PendingOrder() {CtrlLayout(*this);}};
	struct ModifyOrder : public WithModifyOrder<Ctrl> {ModifyOrder() {CtrlLayout(*this);}};
	struct ModifyPendingOrder : public WithModifyPendingOrder<Ctrl> {ModifyPendingOrder() {CtrlLayout(*this);}};
	struct ErrorMessage : public WithErrorMessage<Ctrl> {ErrorMessage() {CtrlLayout(*this);}};
	
	
	// Public vars
	Dialog diag;
	MarketExecution me;
	PendingOrder po;
	ModifyOrder mod;
	ModifyPendingOrder modp;
	ErrorMessage err;
	static Callback WhenOrdersChanged;
	
};


// Class for showing detailed info about selected symbol
class Specification : public WithSpecification<TopWindow> {
	
	// Vars
	int sym;
	
public:
	
	// Ctors
	typedef Specification CLASSNAME;
	Specification();
	
	// Main funcs
	void Init(int id);
	void Data();
	void DoClose() {Close();}
	
};


// About dialog. Info about the program.
class AboutDialog : public WithAbout<TopWindow> {
	
public:
	AboutDialog() {Title("About"); CtrlLayout(*this);}
	virtual bool Key(dword key, int count) {if (key == K_ESCAPE) {Close(); return true;} return false;}
};


// Detailed ctrl for price data
class HistoryCenter : public WithHistoryCenter<TopWindow> {
	
	// Vars
	ScrollBar sb;
	
public:
	
	// Ctors
	typedef HistoryCenter CLASSNAME;
	HistoryCenter();
	
	// Main funcs
	virtual bool Key(dword key, int count);
	void Init(int id, int tf_id);
	void Data();
	void DoClose() {Close();}
	
};

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
	int current = 0, phase = 0, joinlevel = 0;
	bool is_enabled = false;
	
	
	// Temporary
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	Vector<Ptr<CoreItem> > ci_queue;
	int processed_cursor = 0, data_cursor = 0, realtime_count = 0;
	bool is_prepared = false;
	
	
	void Prepare();
	void ProcessData();
	void RealizeStats();
	void Iterate(int type);
	void IterateJoining();
	
	double GetBitProbBegin(int symbol, int begin_id);
	double GetBitProbTest(int symbol, int begin_id, int type, int sub_id, bool inv_action);
	
public:
	typedef RuleAnalyzer CLASSNAME;
	RuleAnalyzer();
	~RuleAnalyzer();
	
	void Process();
	void ProcessRealtime();
	void Refresh();
	bool RefreshReal();
	void Data();
	void SetCursor() {data_check.cursor = datactrl_cursor.GetData(); data_check.Refresh();}
	void SetEnable() {is_enabled = enable.Get();}
	
	void Serialize(Stream& s) {s % stats % data % cursor % current % phase % joinlevel % is_enabled;}
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
