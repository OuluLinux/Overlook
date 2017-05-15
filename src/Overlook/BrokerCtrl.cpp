#include "Overlook.h"
	
namespace Overlook {

BrokerCtrl::BrokerCtrl() {
	CtrlLayout(*this);
	
	current.AddColumn("Symbol");
	current.AddColumn("Bid");
	current.AddColumn("Ask");
	current.ColumnWidths("3 2 2");
	
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
	
	journal.AddColumn("Time");
	journal.AddColumn("Message");
	journal.ColumnWidths("1 6");
	
	volume.SetData(0.01);
	
	split.Vert();
	split << trade << exposure << history << journal;
}

void BrokerCtrl::SetArguments(const VectorMap<String, Value>& args) {
	/*BaseSystem& ol = Get<BaseSystem>();
	
	MetaNode::SetArguments(args);
	
	int i = args.Find("broker");
	if (i == -1) throw DataExc("No broker path set");
	broker = ol.FindLinkCore(args[i]);
	ASSERTEXC(broker);
	
	db = dynamic_cast<DataBridge*>(&*broker);
	ASSERT(db);*/
}

void BrokerCtrl::Init() {
	
	
	
	//Thread::Start(THISBACK(DummyRunner));
}

void BrokerCtrl::Refresher() {
	
}

void BrokerCtrl::Reset() {
	
}

void BrokerCtrl::DummyRunner() {
	#if 0
	Brokerage& b = db->demo;
	
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
			double stoploss = type == OP_BUY ? open * 0.99 : open * 1.01;
			double takeprofit = type == OP_SELL ? open * 0.99 : open * 1.01;
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
	#endif
}

}
