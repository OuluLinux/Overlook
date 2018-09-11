#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_



#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include <plugin/libmt/libmt.h>
#include <SubWindowCtrl/SubWindowCtrl.h>
#include <PushNotification/PushNotification.h>
#include <MailClient/MAILClient.h>
#include <MailClient/IMAPClient.h>
using namespace Upp;


#define LAYOUTFILE <Overlook/Overlook.lay>
#include <CtrlCore/lay.h>

#include "DQN.h"
#include "Common.h"
#include "Optimizer.h"
#include "BeamSearchOptimization.h"
#include "SimBroker.h"
#include "System.h"
#include "CoreList.h"
#include "Calendar.h"
#include "Core.h"
#include "DataBridge.h"
#include "Utils.h"
#include "AutoChartist.h"
#include "Indicators.h"
#include "Advisors.h"
#include "Net.h"
#include "GraphCtrl.h"
#include "Chart.h"
#include "Dialogs.h"
#include "MarketWatch.h"
#include "Navigator.h"
#include "ChartManager.h"
#include "Myfxbook.h"
#include "Arbitrage.h"
#include "Analyzer.h"
#include "Scalper.h"
#include "AdvisorSystem.h"
#include "Sentiment.h"


namespace Overlook {
using namespace libmt;


struct AssetGraphDislay : public Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const;
};


class OptionWindow : public WithOptions<TopWindow> {
	
	
public:
	OptionWindow();
	
	
};


struct ProfileGroup : Moveable<ProfileGroup> {
	FactoryDeclaration decl;
	Rect rect;
	int shift = 0, symbol = 0, tf = 0, type = 0;
	bool right_offset = 0, keep_at_end = 0, is_maximized = 0;
	
	void Serialize(Stream& s) {s % decl % rect % shift % symbol % tf % type % right_offset % keep_at_end % is_maximized;}
};

struct Profile : Moveable<Profile> {
	Vector<ProfileGroup>	charts;
	
	void Serialize(Stream& s) {s % charts;}
};

// Class for main gui of forex
class Overlook : public DockWindow {
	
	
protected:
	
	// Vars
	Array<CommonCtrl> commons;
	Profile current_profile;
	VectorMap<String, Vector<int> > symbol_menu_items;
	ChartManager cman;
	Navigator nav;
	MarketWatch watch;
	CtrlCallbacks<ArrayCtrl> trade, trade_history, exposure, joblist, debuglist;
	CtrlCallbacks<Splitter> jobs_hsplit;
	CtrlCallbacks<ArbitrageCtrl> arb;
	NetCtrl net;
	ParentCtrl job_ctrl;
	Ctrl* prev_job_ctrl = NULL;
	StatusBar status;
	InfoCtrl account, network;
	ToolBar tool;
	ButtonOption right_offset, keep_at_end;
	Array<ButtonOption> tf_buttons;
	TimeCallback statusbar_cb;
	Button add_symindi;
	Vector<int> symindi_args;
	MenuBar menu;
	TimeStop mt_refresh, runtime;
	Id thrd_id, thrd_job_id;
	Id sym;
	
	// Protected main functions to prevent direct (wrong) usage
	void ToggleRightOffset();
	void ToggleKeepAtEnd();
	void ActiveWindowChanged();
	void PriceUpdated();
	void OpenSymbolMenu();
	void SymbolMenu(Bar& bar);
	int  GetTimeframeIndex();
	void DeepRefresh();
	
public:
	
	// Ctors
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	void Deinit();
	
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
	void TileWindowVert();
	void CloseWindow();
	void HelpTopics();
	void About();
	
	// Core functions
	virtual void DockInit();
	virtual void TradeRightDown(Point, dword) {MenuBar::Execute(THISBACK(TradeMenu));}
	void RefreshData();
	void DeepRefreshData();
	void PostRefreshData() {PostCallback(THISBACK(RefreshData));}
	void Data();
	Chart& OpenChart(int symbol, int indi=0, int tf=-1);
	Chart& OpenChart(int symbol, const FactoryDeclaration& decl, int tf=-1);
	Chart* GetChart(int i);
	void OpenChartFromList() {OpenChart(trade.GetCursor());}
	void OpenNet();
	void SetFactory(int f);
	void SetTimeframe(int tf_id);
	void RefreshCalendar();
	void RefreshTrades();
	void RefreshExposure();
	void RefreshTradesHistory();
	void RefreshJobs();
	void RefreshDebug();
	void MenuNewOrder();
	void MenuCloseOrder();
	void MenuModifyOrder();
	void ToggleFullScreen() {TopWindow::FullScreen(!IsFullScreen());}
	void LoadPreviousProfile();
	void StorePreviousProfile();
	void SaveProfile();
	void LoadAdvisorProfile();
	void LoadMajorPairProfile(int tf);
	void LoadMajorCurrencyProfile(int tf);
	void LoadMajorActiveNetProfile(int tf);
	void LoadOpenOrderCharts();
	void LoadSymbolProfile(int sym, int tf, int factory=-1);
	void LoadProfile(Profile& profile);
	void StoreProfile(Profile& profile);
	void SetProfileFromFile(String path);
	void LoadProfileFromFile(Profile& profile, String path);
	void StoreProfileToFile(Profile& profile, String path);
	void OpenChartBars(int symbol) {OpenChart(symbol);}
	void RefreshCommon();
	
	// Public vars
	Callback WhenExit;
	
};

inline ::Overlook::Overlook& GetOverlook() {return Single<::Overlook::Overlook>();}

}

#endif
	