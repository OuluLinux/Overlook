#ifndef _Overlook_MarketWatch_h_
#define _Overlook_MarketWatch_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;


namespace Overlook {

class Overlook;

// Ctrl for current prices and events of market
class MarketWatch : public ParentCtrl {
	
	// Vars
	CtrlCallbacks<ArrayCtrl> list;
	Overlook* olook;
	Id sym_id;
	
public:
	
	
	// Ctors
	typedef MarketWatch CLASSNAME;
	MarketWatch(Overlook* olook);
	
	
	// Main funcs
	void Data();
	void PostRefreshData() {PostCallback(THISBACK(Data));}
	void OpenChart();
	virtual void RightDown(Point, dword) {MenuBar::Execute(THISBACK(LocalMenu));}
	void LocalMenu(Bar& bar);
	void MenuNewOrder();
	void MenuSpecification();
	
	
	// Get funcs
	int GetSelectedSymbol() {return list.GetCursor();}
	
	// Public vars
	Callback1<int> WhenOpenChart;
	
};

}

#endif
