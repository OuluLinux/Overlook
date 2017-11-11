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


}


#endif
