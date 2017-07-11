#include "Overlook.h"
	
namespace Overlook {

BrokerCtrl::BrokerCtrl() {
	broker = NULL;
	
	CtrlLayout(*this);
	
	current.AddColumn("Symbol");
	current.AddColumn("Bid");
	current.AddColumn("Ask");
	current.ColumnWidths("3 2 2");
	current <<= THISBACK(PriceCursor);
	
	trade.AddColumn("Order");
	trade.AddColumn("Time");
	trade.AddColumn("Type");
	trade.AddColumn("Size");
	trade.AddColumn("Symbol");
	trade.AddColumn("Price");
	trade.AddColumn("S / L");
	trade.AddColumn("T / P");
	trade.AddColumn("Price");
	trade.AddColumn("Commission");
	trade.AddColumn("Swap");
	trade.AddColumn("Profit");
	trade.ColumnWidths("3 4 2 2 2 2 2 2 2 2 2 3");
	
	exposure.AddColumn("Asset");
	exposure.AddColumn("Volume");
	exposure.AddColumn("Rate");
	exposure.AddColumn("USD");
	exposure.AddColumn("Graph");
	exposure.ColumnWidths("1 1 1 1 1");
	
	history.AddColumn("Order");
	history.AddColumn("Time");
	history.AddColumn("Type");
	history.AddColumn("Size");
	history.AddColumn("Symbol");
	history.AddColumn("Price");
	history.AddColumn("S / L");
	history.AddColumn("T / P");
	history.AddColumn("Time");
	history.AddColumn("Price");
	history.AddColumn("Swap");
	history.AddColumn("Profit");
	history.ColumnWidths("3 4 2 2 2 2 2 2 4 2 2 3");
	
	volume.SetData(0.01);
	takeprofit.SetData(0.0);
	stoploss.SetData(0.0);
	
	close		<<= THISBACK(Close);
	closeall	<<= THISBACK(CloseAll);
	buy			<<= THISBACK1(OpenOrder, 0);
	sell		<<= THISBACK1(OpenOrder, 1);
	
	split.Vert();
	split << trade << exposure << history;
}

void BrokerCtrl::Init() {
	
	
	
	//Thread::Start(THISBACK(DummyRunner));
}

void BrokerCtrl::ReadOnly() {
	buy.Hide();
	sell.Hide();
	takeprofit.Hide();
	tplbl.Hide();
	stoploss.Hide();
	sllbl.Hide();
	volume.Hide();
	vollbl.Hide();
	close.Hide();
	closeall.Hide();
	info.Hide();
	watch.LeftPosZ(4, 172).TopPosZ(4, 104);
	current.LeftPosZ(4, 172).VSizePosZ(132, 4);
}

void BrokerCtrl::Data() {
	ASSERT_(broker, "Broker is not yet set to BrokerCtrl");
	
	Brokerage& b = *broker;
	
	b.Enter();
	
	SimBroker* sb = dynamic_cast<SimBroker*>(&b);
	if (sb) sb->RefreshOrders();
	
	String w;
	w =		"Market watch: "	+ Format("%",     b.GetTime()) + "\n"
			"Balance: "			+ Format("%2!,n", b.AccountBalance()) + "\n"
			"Equity: "			+ Format("%2!,n", b.AccountEquity()) + "\n"
			"Margin: "			+ Format("%2!,n", b.AccountMargin()) + "\n"
			"Free Margin: "		+ Format("%2!,n", b.AccountFreeMargin()) + "\n"
			"Open orders: "		+ Format("%d",    b.GetOpenOrders().GetCount()) + "\n"
			"History orders: "	+ Format("%d",    b.GetHistoryOrders().GetCount());
	watch.SetLabel(w);
	
	const Vector<Symbol>& symbols = b.GetSymbols();
	
	for(int i = 0; i < symbols.GetCount(); i++) {
		const Price& price = b.GetAskBid()[i];
		current.Set(i, 0, symbols[i].name);
		current.Set(i, 1, price.bid);
		current.Set(i, 2, price.ask);
	}
	
	
	const Array<Order>& orders = b.GetOpenOrders();
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		const Symbol& sym = b.GetSymbol(o.symbol);
		trade.Set(i, 0, o.ticket);
		trade.Set(i, 1, Format("%", o.begin));
		trade.Set(i, 2, o.type == 0 ? "Buy" : "Sell");
		trade.Set(i, 3, o.volume);
		trade.Set(i, 4, sym.name);
		trade.Set(i, 5, o.open);
		trade.Set(i, 6, o.stoploss);
		trade.Set(i, 7, o.takeprofit);
		trade.Set(i, 8, o.close);
		trade.Set(i, 9, o.commission);
		trade.Set(i, 10, o.swap);
		trade.Set(i, 11, o.profit);
	}
	trade.SetCount(orders.GetCount());
	
	
	const Vector<Asset>& assets = b.GetAssets();
	for(int i = 0; i < assets.GetCount(); i++) {
		const Asset& a = assets[i];
		if (a.sym == -1) continue;
		const String& name =
			a.sym < b.GetSymbolCount() ?
				b.GetSymbol(a.sym).name :
				b.GetCurrency(a.sym - b.GetSymbolCount()).name;
		exposure.Set(i, 0, name);
		exposure.Set(i, 1, a.volume);
		exposure.Set(i, 2, a.rate);
		exposure.Set(i, 3, a.base_value);
	}
	exposure.SetCount(assets.GetCount());
	
	
	const Array<Order>& horders = b.GetHistoryOrders();
	for(int i = 0; i < horders.GetCount(); i++) {
		const Order& o = horders[i];
		if (o.symbol < 0 || o.symbol >= b.GetSymbolCount()) continue;
		const Symbol& sym = b.GetSymbol(o.symbol);
		history.Set(i, 0, o.ticket);
		history.Set(i, 1, Format("%", o.begin));
		history.Set(i, 2, o.type == 0 ? "Buy" : "Sell");
		history.Set(i, 3, o.volume);
		history.Set(i, 4, sym.name);
		history.Set(i, 5, o.open);
		history.Set(i, 6, o.stoploss);
		history.Set(i, 7, o.takeprofit);
		history.Set(i, 8, Format("%", o.end));
		history.Set(i, 9, o.close);
		history.Set(i, 10, o.swap);
		history.Set(i, 11, o.profit);
	}
	history.SetCount(horders.GetCount());
	
	
	b.Leave();
}

void BrokerCtrl::PriceCursor() {
	int cursor = current.GetCursor();
	if (cursor == -1) {info.SetLabel(""); return;}
	const Price& price = broker->GetAskBid()[cursor];
	info.SetLabel(DblStr(price.bid) + "/" + DblStr(price.ask));
}

void BrokerCtrl::Refresher() {
	
}

void BrokerCtrl::Reset() {
	
}

void BrokerCtrl::Close() {
	int order_id = trade.GetCursor();
	if (order_id == -1) return;
	
	Brokerage& b = *broker;
	b.ForwardExposure();
	
	const Order& o = b.GetOpenOrders()[order_id];
	double close = o.type == OP_BUY ?
		b.RealtimeBid(o.symbol) :
		b.RealtimeAsk(o.symbol);
	int r = b.OrderClose(o.ticket, o.volume, close, 100);
	if (r) {
		//LOG("Order closed");
		b.ForwardExposure();
		Data();
	} else {
		//DLOG("Order close failed");
		info.SetLabel(b.GetLastError());
	}
}

void BrokerCtrl::CloseAll() {
	Brokerage& b = *broker;
	b.CloseAll();
	if (b.GetOpenOrders().GetCount()) {
		info.SetLabel(b.GetLastError());
	}
	b.ForwardExposure();
	Data();
}

void BrokerCtrl::OpenOrder(int type) {
	int sym_id = current.GetCursor();
	if (sym_id == -1) return;
	
	Brokerage& b = *broker;
	
	double lots = this->volume.GetData();
	double open = type == OP_BUY ?
		b.RealtimeAsk(sym_id) :
		b.RealtimeBid(sym_id);
	int slippage = 100;
	double stoploss = this->stoploss.GetData();
	double takeprofit = this->takeprofit.GetData();
	if (stoploss   == 0.0) stoploss   = type == OP_BUY  ? open * 0.99 : open * 1.01;
	if (takeprofit == 0.0) takeprofit = type == OP_SELL ? open * 0.99 : open * 1.01;
	int magic = 0;
	int ticket = b.OrderSend(
		sym_id,
		type,
		lots,
		open,
		slippage,
		stoploss,
		takeprofit,
		magic,
		0);
	if (ticket != -1) {
		LOG("Order opened successfully: ticket=" << ticket);
	} else {
		LOG("ERROR: opening order failed: " + b.GetLastError());
		info.SetLabel(b.GetLastError());
	}
	b.ForwardExposure();
	Data();
}

void BrokerCtrl::DummyRunner() {
	if (!broker)
		return;
	
	Brokerage& b = *broker;
	
	Vector<Symbol> symbols;
	symbols <<= b.GetSymbols();
	int magic = 513251;
	
	while (!Thread::IsShutdownThreads()) {
		bool open = Random(2);
		
		if (open) {
			int sym_id = Random(symbols.GetCount());
			String sym = symbols[sym_id].name;
			int type = Random(2); // 0 = OP_BUY, 1 = OP_SELL
			double lots = 0.01;
			double open = type == OP_BUY ?
				b.MarketInfo(sym, MODE_ASK) :
				b.MarketInfo(sym, MODE_BID);
			int slippage = 100;
			double stoploss = this->stoploss.GetData();
			double takeprofit = this->takeprofit.GetData();
			if (stoploss   == 0.0) stoploss   = type == OP_BUY  ? open * 0.99 : open * 1.01;
			if (takeprofit == 0.0) takeprofit = type == OP_SELL ? open * 0.99 : open * 1.01;
			
			int ticket = b.OrderSend(
				sym,
				type,
				lots,
				open,
				slippage,
				stoploss,
				takeprofit,
				magic,
				0);
			if (ticket == -1) {
				LOG("Order opened successfully: ticket=" << ticket);
			} else {
				LOG("ERROR: opening order failed");
			}
		}
		
		else {
			int count = b.OrdersTotal();
			if (!count) continue;
			
			if (b.OrderSelect(0, SELECT_BY_POS, MODE_TRADES)) {
				int ticket = b.OrderTicket();
				double lots = b.OrderLots();
				int type = b.OrderType();
				ASSERT(type == OP_BUY || type == OP_SELL); // others shouldn't use this
				String sym = b.OrderSymbol();
				double close = type == OP_BUY ?
					b.MarketInfo(sym, MODE_BID) :
					b.MarketInfo(sym, MODE_ASK);
				
				int slippage = 100;
				
				if (b.OrderClose(ticket, lots, close, slippage)) {
					LOG("Order close succeeded: " << sym << " " << ticket);
				} else {
					LOG("ERROR: Order close failed: " << sym << " " << ticket);
				}
			} else {
				LOG("ERROR: couldn't select open order pos=0");
			}
		}
		
		Sleep(500);
	}
}





















}
