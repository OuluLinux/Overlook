#include "Overlook.h"

namespace Overlook {


MarketWatch::MarketWatch(Overlook* olook) : olook(olook) {
	
	Add(list.VSizePos().HSizePos());
	
	list.AddColumn("Symbol");
	list.AddColumn("Bid");
	list.AddColumn("Ask");
	list.AddIndex(sym_id);
	list.ColumnWidths("3 2 2");
	
	list.WhenLeftDouble = THISBACK(OpenChart);
	list.WhenRightDown = THISBACK(RightDown);
	
	PostRefreshData();
}

void MarketWatch::Data() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int count = sys.GetSymbolCount();
	int mtcount = mt.GetSymbolCount();
	for(int i = 0; i < count; i++) {
		list.Set(i, 0, sys.GetSymbol(i));
		if (i < mtcount) {
			const Price& p = mt.GetAskBid()[i];
			list.Set(i, 1, p.bid);
			list.Set(i, 2, p.ask);
		}
		list.Set(i, 3, i);
	}
	//list.SetSortColumn(0);
}

void MarketWatch::OpenChart() {
	int i = list.GetCursor();
	if (i < 0) return;
	i = list.Get(i, 3);
	WhenOpenChart(i);
}

void MarketWatch::LocalMenu(Bar& bar) {
	bar.Add("New Order", THISBACK(MenuNewOrder));
	bar.Add("Chart Window", [=] {olook->OpenChart(list.GetCursor());});
	bar.Add("Specification", THISBACK(MenuSpecification));
	
}

void MarketWatch::MenuNewOrder() {
	int sym = list.GetCursor();
	
	NewOrderWindow win;
	win.Init(sym);
	win.Run();
	
}

void MarketWatch::MenuSpecification() {
	int sym = list.GetCursor();
	
	Specification spec;
	spec.Init(sym);
	spec.Run();
	
}

}
