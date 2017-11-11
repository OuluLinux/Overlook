#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_

#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <plugin/libmt/libmt.h>

using namespace Upp;
#define LAYOUTFILE <Overlook/Overlook.lay>
#include <CtrlCore/lay.h>

#include "Common.h"
#include "SimBroker.h"
#include "System.h"
#include "Core.h"
#include "ExposureTester.h"
#include "DataBridge.h"
#include "Indicators.h"
#include "GraphCtrl.h"
#include "GraphGroupCtrl.h"
#include "ExportCtrl.h"
#include "Dialogs.h"
#include "MarketWatch.h"
#include "Navigator.h"
#include "ChartManager.h"
#include "Backtest.h"


namespace Overlook {
using namespace libmt;


struct AssetGraphDislay : public Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
};


class OptionWindow : public WithOptions<TopWindow> {
	
	
public:
	OptionWindow();
	
	
};

// Class for main gui of forex
class Overlook : public DockWindow {
	
	
protected:
	friend class BacktestCtrl;
	
	// Vars
	VectorMap<String, Vector<int> > symbol_menu_items;
	ChartManager cman;
	Navigator nav;
	MarketWatch watch;
	BacktestCtrl backtest;
	CtrlCallbacks<ArrayCtrl>  trade, trade_history, exposure;
	StatusBar status;
	InfoCtrl account, network;
	ToolBar tool;
	ButtonOption right_offset, keep_at_end;
	Array<ButtonOption> tf_buttons;
	TimeCallback statusbar_cb;
	Button add_symindi;
	Vector<int> symindi_args;
	MenuBar menu;
	Id sym;
	
	// Protected main functions to prevent direct (wrong) usage
	void ToggleRightOffset();
	void ToggleKeepAtEnd();
	void ActiveWindowChanged();
	void PriceUpdated();
	void OpenSymbolMenu();
	void SymbolMenu(Bar& bar);
	int  GetTimeframeIndex();
	
public:
	
	// Ctors
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	
	
	// Main funcs
	void SetMenuBar(MenuBar& bar);
	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void InsertMenu(Bar& bar);
	void ChartMenu(Bar& bar);
	void ToolMenu(Bar& bar);
	void WindowMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	void SubBar(Bar& bar);
	void TradeMenu(Bar& bar);
	
	// Menu items
	void Exit();
	void FullScreen();
	void ChartProperties();
	void NewOrder();
	void HistoryCenter();
	void Options();
	void TileWindow();
	void CloseWindow();
	void HelpTopics();
	void About();
	
	// Core functions
	virtual void DockInit();
	virtual void TradeRightDown(Point, dword) {MenuBar::Execute(THISBACK(TradeMenu));}
	void RefreshData();
	void PostRefreshData() {PostCallback(THISBACK(RefreshData));}
	void Data();
	void OpenChart(int id);
	void OpenChartFromList() {OpenChart(trade.GetCursor());}
	void SetIndicator(int indi);
	void SetTimeframe(int tf_id);
	void RefreshTrades();
	void RefreshExposure();
	void RefreshTradesHistory();
	void MenuNewOrder();
	void MenuCloseOrder();
	void MenuModifyOrder();
	void ToggleFullScreen() {TopWindow::FullScreen(!IsFullScreen());}
	
	// Public vars
	Callback WhenExit;
	
};

}

#endif
	