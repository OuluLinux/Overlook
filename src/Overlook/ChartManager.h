#ifndef _Overlook_ChartManager_h_
#define _Overlook_ChartManager_h_

#include <CtrlLib/CtrlLib.h>
#include <SubWindowCtrl/SubWindowCtrl.h>
using namespace Upp;



namespace Overlook {

// Main view. Shows subwindows of graphs and ctrls.
class ChartManager : public SubWindows {
	friend class Chart;
	
	// Vars
	bool backtest_broker;
	
public:

	// Ctors
	typedef ChartManager CLASSNAME;
	ChartManager();
	
	
	// Main funcs
	void Init();
	GraphGroupCtrl& AddChart();
	void CloseBacktestWindows(bool exclude_indicators=true);
	void RefreshBacktestWindows();
	void PostRefreshBacktestWindows() {PostCallback(THISBACK(RefreshBacktestWindows));}
	
	
	// Get funcs
	GraphGroupCtrl* GetVisibleChart() {return dynamic_cast<GraphGroupCtrl*>(GetVisibleSubWindowCtrl());}
	
	
	// Set funcs
	void UseBacktestBroker(bool b=true) {backtest_broker = b;}
	
};

}

#endif
