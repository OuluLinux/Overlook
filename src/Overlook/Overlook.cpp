#include "Overlook.h"

namespace Overlook {


#define TOPICFILE <Overlook/app.tpp/all.i>
#include <Core/topic_group.h>


Overlook::Overlook() : watch(this), backtest(this) {
	MaximizeBox().MinimizeBox().Sizeable();
	Icon(OverlookImg::icon());
	Title("Overlook");
	
	RealizeDirectory(ConfigFile("Advisors"));
	RealizeDirectory(ConfigFile("Scripts"));
	
	Add(cman.SizePos());
	
	AddFrame(menu);
	AddFrame(tool);
	AddFrame(status);
	
	menu.Set(THISBACK(MainMenu));
	tool.Set(THISBACK(SubBar));
	
	right_offset.SetImage(OverlookImg::right_offset());
	keep_at_end.SetImage(OverlookImg::keep_at_end());
	
	right_offset.WhenAction = THISBACK(ToggleRightOffset);
	keep_at_end.WhenAction = THISBACK(ToggleKeepAtEnd);
	
	status.AddFrame(account.Left(200));
	status.AddFrame(network.Right(200));
	
	cman.WhenActiveWindowChanges = THISBACK(ActiveWindowChanged);
	
	watch.WhenOpenChart = THISBACK(OpenChart);
	
	NewOrderWindow::WhenOrdersChanged = THISBACK(Data);
	
	trade.AddColumn ( "Order" );
	trade.AddColumn ( "Time" );
	trade.AddColumn ( "Type" );
	trade.AddColumn ( "Size" );
	trade.AddColumn ( "Symbol" );
	trade.AddColumn ( "Price" );
	trade.AddColumn ( "S / L" );
	trade.AddColumn ( "T / P" );
	trade.AddColumn ( "Price" );
	trade.AddColumn ( "Commission" );
	trade.AddColumn ( "Swap" );
	trade.AddColumn ( "Profit" );
	trade.AddIndex(sym);
	trade.ColumnWidths ( "5 5 3 3 3 3 3 3 3 3 3 5" );
	trade.WhenLeftDouble << THISBACK(OpenChartFromList);
	trade.WhenRightDown = THISBACK(TradeRightDown);
	
	exposure.AddColumn("Asset");
	exposure.AddColumn("Volume");
	exposure.AddColumn("Rate");
	exposure.AddColumn("USD");
	exposure.AddColumn("Graph");
	exposure.ColumnWidths("1 1 1 1 1");
	
	trade_history.AddColumn("Order");
	trade_history.AddColumn("Time");
	trade_history.AddColumn("Type");
	trade_history.AddColumn("Size");
	trade_history.AddColumn("Symbol");
	trade_history.AddColumn("Price");
	trade_history.AddColumn("S / L");
	trade_history.AddColumn("T / P");
	trade_history.AddColumn("Time");
	trade_history.AddColumn("Price");
	trade_history.AddColumn("Swap");
	trade_history.AddColumn("Profit");
	
	add_symindi.SetLabel("+");
	add_symindi.WhenAction = THISBACK(OpenSymbolMenu);
	
	add_symindi.Tip("Create a new chart");
	right_offset.Tip("Shift end of the chart from right border");
	keep_at_end.Tip("Scroll the chart to the end on tick incoming");
	
	cman.Init();
	
	nav.Init();
	nav.WhenIndicator = THISBACK(SetIndicator);
	
	PostRefreshData();
}

Overlook::~Overlook() {
	
}

void Overlook::DockInit() {
	DockLeft(Dockable(watch, "Market Watch").SizeHint(Size(300, 200)));
	DockLeft(Dockable(nav, "Navigator").SizeHint(Size(300, 200)));
	
	DockableCtrl& bt = Dockable(backtest, "Backtest").SizeHint(Size(300, 200));
	DockBottom(bt);
	Tabify(bt, Dockable(trade_history, "Account History").SizeHint(Size(300, 200)));
	Tabify(bt, Dockable(exposure, "Exposure").SizeHint(Size(300, 200)));
	Tabify(bt, Dockable(trade, "Terminal").SizeHint(Size(300, 200)));
}

int Overlook::GetTimeframeIndex() {
	for(int i = 0; i < tf_buttons.GetCount(); i++)
		if (tf_buttons[i].Get())
			return i;
	return 0;
}

void Overlook::SetMenuBar(MenuBar& bar) {
	bar.Set(THISBACK(MainMenu));
}

void Overlook::MainMenu(Bar& bar) {
	bar.Add("File", THISBACK(FileMenu));
	bar.Add("View", THISBACK(ViewMenu));
	bar.Add("Insert", THISBACK(InsertMenu));
	bar.Add("Charts", THISBACK(ChartMenu));
	bar.Add("Tools", THISBACK(ToolMenu));
	bar.Add("Window", THISBACK(WindowMenu));
	bar.Add("Help", THISBACK(HelpMenu));
}

void Overlook::FileMenu(Bar& bar) {
	
	bar.Add("Exit", Proxy(WhenExit));
}

void Overlook::ViewMenu(Bar& bar) {
	
	bar.Add("Full Screen", THISBACK(ToggleFullScreen)).Key(K_F11);
}

void Overlook::InsertMenu(Bar& bar) {
	
}

void Overlook::ChartMenu(Bar& bar) {
	
	bar.Add("Properties", THISBACK(ChartProperties));
}

void Overlook::ToolMenu(Bar& bar) {
	bar.Add("New Order", THISBACK(NewOrder)).Key(K_F9);
	bar.Separator();
	bar.Add("History Center", THISBACK(HistoryCenter)).Key(K_F2);
	bar.Separator();
	bar.Add("Options", THISBACK(Options)).Key(K_CTRL|K_O);
}

void Overlook::WindowMenu(Bar& bar) {
	bar.Add("Tile Windows", THISBACK(TileWindow)).Key(K_ALT|K_R);
	bar.Separator();
	bar.Add("Close Window", THISBACK(CloseWindow)).Key(K_CTRL|K_F4);
}

void Overlook::HelpMenu(Bar& bar) {
	bar.Add("Help Topics", THISBACK(HelpTopics)).Key(K_F1);
	bar.Separator();
	bar.Add("About", THISBACK(About));
	
}

void Overlook::SubBar(Bar& bar) {
	bar.Add(add_symindi, Size(40, 30));
	bar.Separator();
	
	bar.Add(right_offset, Size(30, 30));
	bar.Add(keep_at_end, Size(30, 30));
	bar.Separator();
	
	MetaTrader& mt = GetMetaTrader();
	if (tf_buttons.IsEmpty()) {
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			ButtonOption& bo = tf_buttons.Add();
			bo.SetLabel(mt.GetTimeframeString(i));
			bo <<= THISBACK1(SetTimeframe, i);
		}
		tf_buttons[0].Set(true);
	}
	for(int i = 0; i < tf_buttons.GetCount(); i++) {
		bar.Add(tf_buttons[i], Size(30, 30));
	}
	
}

void Overlook::TradeMenu(Bar& bar) {
	bar.Add("New Order", THISBACK(MenuNewOrder));
	if (trade.GetCount()) {
		bar.Add("Close Order", THISBACK(MenuCloseOrder));
		bar.Add("Modify Order", THISBACK(MenuModifyOrder));
	}
}

void Overlook::MenuNewOrder() {
	int sym = 0;
	int cursor = trade.GetCursor();
	if (cursor != -1)
		sym = trade.Get(cursor, 12);
	
	NewOrderWindow win;
	win.Init(sym);
	win.OpenMain();
}

void Overlook::MenuCloseOrder() {
	MetaTrader& mt = GetMetaTrader();
	const Vector<Order>& orders = mt.GetOpenOrders();
	
	int cursor = trade.GetCursor();
	if (cursor < 0 || cursor >= orders.GetCount()) return;
	
	const libmt::Order& o = orders[cursor];
	if (o.type == -1) return;
	
	
	if (o.type < 2) {
		double close = o.type == OP_BUY ?
			mt.RealtimeBid(o.symbol) :
			mt.RealtimeAsk(o.symbol);
		int r = mt.OrderClose(o.ticket, o.volume, close, 100);
		if (r) {
			mt.Data();
			NewOrderWindow::WhenOrdersChanged();
		}
	}
}

void Overlook::MenuModifyOrder() {
	MetaTrader& mt = GetMetaTrader();
	const Vector<Order>& orders = mt.GetOpenOrders();
	
	int cursor = trade.GetCursor();
	if (cursor == -1) return;
	
	const libmt::Order& o = orders[cursor];
	if (o.type == -1) return;
	
	NewOrderWindow win;
	win.Init(o.symbol);
	win.Modify(o);
	win.Run();
}

void Overlook::ChartProperties() {
	
}

void Overlook::NewOrder() {
	int sym = 0;
	int cursor = watch.GetSelectedSymbol();
	if (cursor != -1)
		sym = cursor;
	
	NewOrderWindow win;
	win.Init(sym);
	win.Run();
}

void Overlook::HistoryCenter() {
	int sym = 0;
	int cursor = watch.GetSelectedSymbol();
	if (cursor != -1)
		sym = cursor;
	
	GraphGroupCtrl* chart = cman.GetVisibleChart();
	int tf = 0;
	if (chart) tf = chart->GetTf();
	
	class HistoryCenter hc;
	hc.Init(sym, tf);
	hc.Run();
}

/*void Overlook::AnalyzeIndicator() {
	GraphGroupCtrl* chart = cman.GetVisibleChart();
	IndicatorAnalyzer& ia = cman.AddSubWindow<IndicatorAnalyzer>();
	ia.Init(&core);
	if (chart) {
		ia.Set(chart->GetIndicatorId(), chart->GetId(), chart->GetTf());
	}
	ia.RefreshData();
	//ia.Run();
	
}*/

/*void Overlook::SearchIndicatorComb() {
	GraphGroupCtrl* chart = cman.GetVisibleChart();
	StatisticalEventPickerCtrl& ics = cman.AddSubWindow<StatisticalEventPickerCtrl>();
	ics.Init(&core);
	if (chart) {
		ics.Set(chart->GetIndicatorId(), chart->GetId(), chart->GetTf());
	}
	ics.RunSearch();
	//ics.Run();
	
}*/

/*void Overlook::OrderDebugger() {
	OrderCtrl& oc = cman.AddSubWindow<OrderCtrl>();
	oc.Init();
}*/

void Overlook::Options() {
	OptionWindow opt;
	opt.Run();
}

void Overlook::TileWindow() {
	cman.OrderTileWindows();
}

void Overlook::CloseWindow() {
	cman.CloseWindow(cman.GetActiveWindowId());
}

void Overlook::HelpTopics() {
	HelpWindow help;
	help.GoTo("topic://Overlook/app/main$en-us");
	help.Execute();
}

void Overlook::About() {
	AboutDialog().Run();
}

void Overlook::OpenSymbolMenu() {
	MenuBar::Execute(THISBACK(SymbolMenu));
}

void Overlook::SymbolMenu(Bar& bar) {
	symbol_menu_items.Clear();
	
	MetaTrader& mt = GetMetaTrader();
	
	for(int i = 0; i < mt.GetSymbolCount(); i++) {
		const Symbol& sym = mt.GetSymbol(i);
		
		if (sym.IsForex()) {
			symbol_menu_items.GetAdd(sym.currency_margin).Add(i);
		} else {
			symbol_menu_items.GetAdd("Others").Add(i);
		}
	}
	
	SortByKey(symbol_menu_items, StdLess<String>());
	
	for(int i = 0; i < symbol_menu_items.GetCount(); i++) {
		bar.Sub(symbol_menu_items.GetKey(i), [=](Bar& bar) {
			MetaTrader& mt = GetMetaTrader();
			const Vector<int>& list = symbol_menu_items[i];
			for(int j = 0; j < list.GetCount(); j++) {
				int id = list[j];
				bar.Add(mt.GetSymbol(id).name, THISBACK1(OpenChart, id));
			}
		});
	}
}

void Overlook::OpenChart(int id) {
	System& sys = GetSystem();
	
	if (id < 0 || id >= sys.GetSymbolCount()) return;
	
	GraphGroupCtrl& view = cman.AddChart();
	
	view.indi = 0;
	view.symbol = id;
	view.tf = GetTimeframeIndex();
	
	Core* core = sys.CreateSingle(view.indi, view.symbol, view.tf);
	ASSERT(core);
	
	view.Init(core);
	view.Data();
}

void Overlook::SetIndicator(int indi) {
	GraphGroupCtrl* c = cman.GetVisibleChart();
	if (c)
		c->SetIndicator(indi);
}


void Overlook::SetTimeframe(int tf_id) {
	for(int i = 0; i < tf_buttons.GetCount(); i++) {
		if (i == tf_id) continue;
		tf_buttons[i].Set(false);
	}
	
	GraphGroupCtrl* c = cman.GetVisibleChart();
	if (c) {
		c->SetTimeframe(tf_id);
		Refresh();
	}
}

void Overlook::RefreshData() {
	Data();
	
	if (!Thread::IsShutdownThreads())
		statusbar_cb.Set(1000, THISBACK(RefreshData));
}

void Overlook::Data() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	
	String info;
	
	info << "Balance: " << mt.AccountBalance()
		<< " Equity: " << mt.AccountEquity()
		<< " Free margin: " << mt.AccountFreeMargin();
	
	status.Set(info);
	
	
	String ninfo;
	ninfo = IntStr(mt.GetInputBytes() / 1024) + "k / " + IntStr(mt.GetOutputBytes());
	ninfo += mt.IsDemo() ? " Demo " : " REAL ";
	ninfo += mt.IsConnected() ? "Connected" : "Disconnect";
	network.Set(ninfo);
	
	watch.Data();
	
	RefreshTrades();
	RefreshExposure();
	RefreshTradesHistory();
}

void Overlook::RefreshTrades() {
	MetaTrader& mt = GetMetaTrader();
	int cursor = trade.GetCursor();
	
	const Vector<Order>& orders = mt.GetOpenOrders();
	int count = orders.GetCount();
	for(int i = 0; i < count; i++) {
		const libmt::Order& o = orders[i];
		if (o.type == -1) break;
		trade.Set(i, 0, o.ticket);
		trade.Set(i, 1, o.begin);
		trade.Set(i, 2, o.GetTypeString());
		trade.Set(i, 3, o.volume);
		trade.Set(i, 4, o.symbol);
		trade.Set(i, 5, o.open);
		trade.Set(i, 6, o.stoploss);
		trade.Set(i, 7, o.takeprofit);
		trade.Set(i, 8, o.close);
		trade.Set(i, 9, o.commission);
		trade.Set(i, 10, o.swap);
		trade.Set(i, 11, o.profit);
		trade.Set(i, 12, mt.GetSymbol(o.symbol).name);
	}
	trade.SetCount(count);
	
	if (cursor >= 0 && cursor < trade.GetCount())
		trade.SetCursor(cursor);
	
}

void Overlook::RefreshExposure() {
	MetaTrader& mt = GetMetaTrader();
	
	exposure.Clear();
	
	String base = mt.AccountCurrency();
	double leverage = mt.AccountLeverage();
	double base_volume = mt.AccountEquity() * leverage;
	
	VectorMap<String, double> cur_volumes;
	VectorMap<int, double> idx_volumes;
	
	const Vector<Order>& orders = mt.GetOpenOrders();
	int count = orders.GetCount();
	int sym_count = mt.GetSymbolCount();
	
	for(int i = 0; i < count; i++) {
		const libmt::Order& bo = orders[i];
		
		const Symbol& sym = mt.GetSymbol(bo.symbol);
		
		if (sym.margin_calc_mode == Symbol::CALCMODE_FOREX) {
			ASSERT(sym.name.GetCount() == 6); // TODO: allow EUR/USD and EURUSDm style names
			String a = sym.name.Left(3);
			String b = sym.name.Right(3);
			
			if (a == base) {
				// For example: base=USD, sym=USDCHF, cmd=buy, lots=0.01, a=USD, b=CHF
				int j = cur_volumes.Find(b);
				double& cur_volume = j == -1 ? cur_volumes.Add(b, 0) : cur_volumes[j];
				if (bo.type == OP_BUY)
					cur_volume += -1 * bo.volume * sym.lotsize * mt.RealtimeAsk(bo.symbol);
				else if (bo.type == OP_SELL)
					cur_volume += bo.volume * sym.lotsize * mt.RealtimeBid(bo.symbol);
			}
			else if (b == base) {
				// For example: base=USD, sym=AUDUSD, cmd=buy, lots=0.01, a=AUD, b=USD
				int j = cur_volumes.Find(a);
				double& cur_volume = j == -1 ? cur_volumes.Add(a, 0) : cur_volumes[j];
				if (bo.type == OP_BUY)
					cur_volume += bo.volume * sym.lotsize;
				else if (bo.type == OP_SELL)
					cur_volume += -1 * bo.volume * sym.lotsize;
			}
			else {
				// Find proxy symbol
				//for (int side = 0; side < 2 && !found; side++) 
				for(int k = 0; k < sym_count; k++) {
					String s = mt.GetSymbol(k).name;
					if ( (s.Left(3)  == base && (s.Right(3) == a || s.Right(3) == b)) ||
						 (s.Right(3) == base && (s.Left(3)  == a || s.Left(3)  == b)) ) {
						// For example: base=USD, sym=CHFJPY, cmd=buy, lots=0.01, s=USDCHF, a=CHF, b=JPY
						//  - CHF += 0.01 * 1000
						{
							int j = cur_volumes.Find(a);
							double& cur_volume = j == -1 ? cur_volumes.Add(a, 0) : cur_volumes[j];
							if (bo.type == OP_BUY)
								cur_volume += bo.volume * sym.lotsize;
							else if (bo.type == OP_SELL)
								cur_volume += -1 * bo.volume * sym.lotsize;
						}
						//  - JPY += -1 * 0.01 * 1000 * open-price
						{
							int j = cur_volumes.Find(b);
							double& cur_volume = j == -1 ? cur_volumes.Add(b, 0) : cur_volumes[j];
							if (bo.type == OP_BUY)
								cur_volume += -1 * bo.volume * sym.lotsize * bo.open;
							else if (bo.type == OP_SELL)
								cur_volume += bo.volume * sym.lotsize * bo.open;
						}
						break;
					}
				}
				
				
				
			}
		}
		else {
			int j = idx_volumes.Find(bo.symbol);
			double& idx_volume = j == -1 ? idx_volumes.Add(bo.symbol, 0) : idx_volumes[j];
			double value = bo.volume * sym.contract_size;
			if (bo.type == OP_SELL) value *= -1;
			idx_volume += value;
		}
		
	}
	
	
	// Get values in base currency: currencies
	VectorMap<String, double> cur_rates, cur_base_values;
	for(int i = 0; i < cur_volumes.GetCount(); i++) {
		String cur = cur_volumes.GetKey(i);
		bool found = false;
		double rate;
		for(int j = 0; j < sym_count; j++) {
			String s = mt.GetSymbol(j).name;
			
			if (s.Left(3) == cur && s.Right(3) == base) {
				rate = mt.RealtimeBid(j);
				cur_rates.Add(cur, rate);
				found = true;
				break;
			}
			else if (s.Right(3) == cur && s.Left(3) == base) {
				rate = 1 / mt.RealtimeAsk(j);
				cur_rates.Add(cur, rate);
				found = true;
				break;
			}
		}
		
		if (!found) Panic("not found");
		
		double volume = cur_volumes[i];
		double base_value = volume / leverage * rate;
		cur_base_values.Add(cur, base_value);
	}
	
	// Get values in base currency: Indices and CDFs
	VectorMap<int, double> idx_rates, idx_base_values;
	for(int i = 0; i < idx_volumes.GetCount(); i++) {
		int id = idx_volumes.GetKey(i);
		Symbol bs = mt.GetSymbol(id);
		String cur = bs.currency_base;
		
		double rate;
		double volume = idx_volumes[i];
		if (cur == base) {
			if (volume < 0)		rate = mt.RealtimeAsk(id);
			else				rate = mt.RealtimeBid(id);
			idx_rates.Add(id, rate);
			double base_value = volume * rate * fabs(volume / bs.contract_size);
			idx_base_values.Add(id, base_value);
		} else {
			bool found = false;
			for(int j = 0; j < sym_count; j++) {
				String s = mt.GetSymbol(j).name;
				
				if (s.Left(3) == cur && s.Right(3) == base) {
					rate = mt.RealtimeBid(j);
					if (volume < 0)		rate *= mt.RealtimeAsk(id);
					else				rate *= mt.RealtimeBid(id);
					idx_rates.Add(id, rate);
					found = true;
					break;
				}
				else if (s.Right(3) == cur && s.Left(3) == base) {
					rate = 1 / mt.RealtimeAsk(j);
					if (volume < 0)		rate *= mt.RealtimeAsk(id);
					else				rate *= mt.RealtimeBid(id);
					idx_rates.Add(id, rate);
					found = true;
					break;
				}
			}
			
			if (!found) Panic("not found");
			
			double base_value = volume * rate * bs.margin_factor;// * abs(volume / bs.contract_size);
			idx_base_values.Add(id, base_value);
		}
		
		
	}
	
	
	// Display currencies
	Vector<double> base_values;
	for(int i = 0; i < cur_volumes.GetCount(); i++) {
		int j;
		const String& cur = cur_volumes.GetKey(i);
		double rate = 0;
		double base_value = 0;
		j = cur_rates.Find(cur);
		if (j != -1) rate = cur_rates[j];
		j = cur_base_values.Find(cur);
		if (j != -1) base_value = cur_base_values[j];
		exposure.Add(cur_volumes.GetKey(i), Format("%0!,n", cur_volumes[i]), Format("%4!,n", rate), Format("%2!,n", base_value));
		base_values.Add(base_value);
		
		if (base_value > 0)
			base_volume -= base_value * leverage * 2;
		else
			base_volume -= base_value * leverage;
	}
	
	// Display indices and CDFs
	for(int i = 0; i < idx_volumes.GetCount(); i++) {
		int j;
		int id = idx_volumes.GetKey(i);
		double rate = 0;
		double base_value = 0;
		j = idx_rates.Find(id);
		if (j != -1) rate = idx_rates[j];
		j = idx_base_values.Find(id);
		if (j != -1) base_value = idx_base_values[j];
		const Symbol& bs = mt.GetSymbol(idx_volumes.GetKey(i));
		exposure.Add(bs.name, Format("%0!,n", idx_volumes[i]), Format("%4!,n", rate), Format("%2!,n", base_value), base_value);
		base_values.Add(base_value);
		
		if (bs.currency_base != base) {
			if (base_value > 0)
				base_volume -= base_value * leverage;
			else
				base_volume += base_value * leverage;
		}
	}
	
	// Display base currency
	base_values.Add(base_volume);
	exposure.Add(base, Format("%0!,n", base_volume), 1.00, Format("%2!,n", base_volume / leverage), base_volume);
	
	
	for(int i = 0; i < exposure.GetCount(); i++) {
		exposure.Set(i, 4, base_values[i] / base_volume);
		exposure.SetDisplay(i, 4, Single<AssetGraphDislay>());
	}
}

void Overlook::RefreshTradesHistory() {
	MetaTrader& mt = GetMetaTrader();
	const Vector<Order>& orders = mt.GetHistoryOrders();
	for(int i = 0; i < orders.GetCount(); i++) {
		const libmt::Order& bo = orders[i];
		if (bo.is_open) continue;
		trade_history.Set(i, 0, bo.ticket);
		trade_history.Set(i, 1, bo.begin);
		trade_history.Set(i, 2, bo.GetTypeString());
		trade_history.Set(i, 3, bo.volume);
		trade_history.Set(i, 4, bo.symbol);
		trade_history.Set(i, 5, bo.open);
		trade_history.Set(i, 6, bo.stoploss);
		trade_history.Set(i, 7, bo.takeprofit);
		trade_history.Set(i, 8, bo.end);
		trade_history.Set(i, 9, bo.close);
		trade_history.Set(i, 10, bo.swap);
		trade_history.Set(i, 11, bo.profit);
	}
	trade_history.SetSortColumn(8, true);
}


void Overlook::ToggleRightOffset() {
	bool b = right_offset.Get();
	GraphGroupCtrl* c = cman.GetVisibleChart();
	if (c) {
		c->SetRightOffset(b);
	}
}

void Overlook::ToggleKeepAtEnd() {
	bool b = keep_at_end.Get();
	GraphGroupCtrl* c = cman.GetVisibleChart();
	if (c) {
		c->SetKeepAtEnd(b);
	}
}

void Overlook::ActiveWindowChanged() {
	GraphGroupCtrl* c = cman.GetVisibleChart();
	if (c) {
		bool b = c->GetRightOffset();
		right_offset.Set(b);
		
		int tf_id = c->GetTf();
		if (tf_id != -1)
			for(int i = 0; i < tf_buttons.GetCount(); i++)
				tf_buttons[i].Set(i == tf_id);
		
		b = c->GetKeepAtEnd();
		keep_at_end.Set(b);
	}
}










void AssetGraphDislay::Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const {
	w.DrawRect(r, paper);
	Rect g = r;
	g.top += 2;
	g.bottom -= 2;
	double d = q;
	if (d < 0) {
		Color clr = Color(135, 22, 0);
		g.left  += (int)(g.Width() * (1.0 + d));
		w.DrawRect(g, clr);
	} else {
		Color clr = Color(0, 134, 0);
		g.right -= (int)(g.Width() * (1.0 - d));
		w.DrawRect(g, clr);
	}
}










OptionWindow::OptionWindow() {
	Title("Options");
	CtrlLayout(*this);
	
	ok << [=]{Close();};
}

}
