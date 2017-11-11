#ifndef _Overlook_Backtest_h_
#define _Overlook_Backtest_h_

namespace Overlook {

// Ctrl for backtesting scripts, advisors and indicators
class BacktestCtrl : public WithBacktestLayout<ParentCtrl> {
	
protected:
	friend class BacktestSettingsDialog;
	
	// Vars
	/*One<Thread> thrd;
	Settings indi_settings;
	ForexCore* core;
	ChartWindow* win;
	Advisor* advisor;
	AdvisorCtrl* advisorctrl;
	Script* script;
	ScriptCtrl* scriptctrl;
	BrokerBacktest* backtester;*/
	Overlook* olook = NULL;
	GraphGroupCtrl* win = NULL;
	int id, tf;
	int begin_shift;
	int target_type, target_id;
	int sleep_ms = 0;
	bool running, stopped;
	
	
	// Protected main functions to prevent direct (wrong) usage
	void IndicatorProperties();
	void SymbolProperties();
	void OpenChart();
	void ModifyIndicator();
	void Start();
	void RefreshType();
	void RefreshTarget();
	void Loop();
	void PostProgressRefresh();
	void RefreshProgress();
	void Stop();
	
public:
	
	// Ctors
	typedef BacktestCtrl CLASSNAME;
	BacktestCtrl(Overlook* olook);
	
	
	// Main funcs
	void Init();
	
};


// Ctrl for settings of the backtester ctrl
class BacktestSettingsDialog : public WithBacktestSettingsLayout<TopWindow> {
	
	// Vars
	WithBacktestTestingLayout<ParentCtrl> testing;
	ArrayCtrl optimization;
	ParentCtrl inputs;
	ArrayCtrl input_values;
	Button input_load, input_save;
	Array<Option> input_opts;
	EditDouble edit_value, edit_start, edit_stop, edit_step;
	BacktestCtrl* bc;
	bool is_ok, is_changed;
	
	
protected:
	
	// Protected main functions to prevent direct (wrong) usage
	void SelectInputLine(int i) {is_changed = true; input_values.SetCursor(i);}
	void EditsValue() {is_changed = true;}
	
public:

	// Ctors
	typedef BacktestSettingsDialog CLASSNAME;
	BacktestSettingsDialog(BacktestCtrl* bc);
	
	// Main funcs
	void LoadInputs();
	void SaveInputs();
	void OK();
	void Cancel();
	void Reset();
	
	// Get funcs
	bool IsOK() {return is_ok;}
	bool IsChanged() {return is_changed;}
	//Settings GetSettings();
	
	// Set funcs
	//void SetSettings(int type, int id, Settings& is);
	
};

}

#endif
