#include "Overlook.h"
#include <plugin/tidy/tidy.h>

namespace Overlook {

Automation::Automation() {
	last_update = Time(1970,1,1);
	
	symbols.Add("AUDUSD");
	symbols.Add("EURCHF");
	symbols.Add("EURGBP");
	symbols.Add("EURJPY");
	symbols.Add("GBPJPY");
	symbols.Add("EURUSD");
	symbols.Add("GBPJPY");
	symbols.Add("GBPUSD");
	symbols.Add("NZDUSD");
	symbols.Add("USDCAD");
	symbols.Add("USDJPY");
}

void Automation::Init() {
	LoadThis();
}

Time TimeFromStr(const String& s) {
	Vector<String> v = Split(s, ",");
	return Time(StrInt(v[0]), StrInt(v[1]), StrInt(v[2]), StrInt(v[3]), StrInt(v[4]));
}

void Automation::Start() {
	RLOG("Automation::Start");

	Time now = GetUtcTime();
	
	MetaTrader& mt = GetMetaTrader();
	

	Index<String> symbols;
	
	for(int i = signals.GetCount() - 1; i >= 0; i--) {
		AutomationSignal& signal = signals[i];
		
		if (symbols.Find(signal.symbol) != -1)
			continue;
		symbols.Add(signal.symbol);
		
		
		int sym_id = GetSystem().FindSymbol(signal.symbol);
		double stop_points = mt._MarketInfo(signal.symbol, MODE_STOPLEVEL);
		double point = mt.GetSymbols()[sym_id].point;
		
		mt.DataEnter();
		const Vector<Order>& open_orders = mt.GetOpenOrders();
		
		if (now >= signal.from && now < signal.till) {
			bool try_win = now + 60*60 >= signal.till;
			
			bool have_open = false;
			for(int j = 0; j < open_orders.GetCount(); j++) {
				const Order& o = open_orders[j];
				String osym = mt.GetSymbols()[o.symbol];
				if (osym == signal.symbol) {
					bool sig;
					switch (o.type) {
						case OP_BUY:		sig = 0; break;
						case OP_SELL:		sig = 1; break;
						case OP_BUYLIMIT:	sig = 0; break;
						case OP_BUYSTOP:	sig = 0; break;
						case OP_SELLLIMIT:	sig = 1; break;
						case OP_SELLSTOP:	sig = 1; break;
					}
					if (sig != signal.sig) {
						RemoveOrder(o);
					} else {
						have_open = true;
						
						if (!signal.sig) {
							double close = mt.RealtimeBid(sym_id);
							if (close >= signal.tp || close <= signal.sl ||
								(try_win && close >= o.open && close < o.open + 10 * point)) {
								RemoveOrder(o);
								have_open = false;
							}
						}
						else {
							double close = mt.RealtimeAsk(sym_id);
							if (close <= signal.tp || close >= signal.sl ||
								(try_win && close <= o.open && close > o.open - 10 * point)) {
								RemoveOrder(o);
								have_open = false;
							}
						}
						
						
					}
				}
			}
			
			if (!have_open) {
				double lot_factor = 0.0002;
				double open = signal.open;
				double takeprofit = signal.tp;
				double stoploss = signal.sl;
				double ask = mt.RealtimeAsk(sym_id);
				double bid = mt.RealtimeBid(sym_id);
				double lots = mt.AccountBalance() * lot_factor;
				int cmd;
				if (signal.sig == false) {
					if (open >= ask)
						cmd = OP_BUYSTOP;
					else
						cmd = OP_BUYLIMIT;
					
					double min_tp = open + stop_points * point;
					if (takeprofit < min_tp)
						takeprofit = min_tp;
					double min_sl = open - stop_points * point;
					if (stoploss > min_sl)
						stoploss = min_sl;
				} else {
					if (open <= bid)
						cmd = OP_SELLSTOP;
					else
						cmd = OP_SELLLIMIT;
					
					double min_tp = open - stop_points * point;
					if (takeprofit > min_tp)
						takeprofit = min_tp;
					double min_sl = open + stop_points * point;
					if (stoploss < min_sl)
						stoploss = min_sl;
				}
				
				open = ((int64)(open / point)) * point;
				stoploss = ((int64)(stoploss / point)) * point;
				takeprofit = ((int64)(takeprofit / point)) * point;
				
				if (mt.OrderSend(signal.symbol, cmd, lots, open, 0, stoploss, takeprofit, "Automation", 0) < 0) {
					String err = mt._GetLastError();
					ReleaseLog("Pending order open failed " + signal.symbol + ": " + err);
					if (err == "no error") {
						double imopen;
						if (signal.sig == false) {
							imopen = mt.RealtimeAsk(sym_id);
							cmd = OP_BUY;
						}
						else {
							imopen = mt.RealtimeBid(sym_id);
							cmd = OP_SELL;
						}
						int slippage = fabs((imopen - open) / point);
						if (slippage < 3)
							mt.OrderSend(signal.symbol, cmd, lots, imopen, 3, stoploss, takeprofit, "Automation", 0);
					}
					if (err == "invalid stops") {
						int ticket = mt.OrderSend(signal.symbol, cmd, lots, open, 1, 0, 0, "", 0);
						if (ticket >= 0) {
							mt.OrderModify(ticket, open, stoploss, takeprofit, 0);
						}
					}
				}
			}
		}
		else {
			
			for(int j = 0; j < open_orders.GetCount(); j++) {
				const Order& o = open_orders[j];
				String osym = mt.GetSymbols()[o.symbol];
				if (osym == signal.symbol) {
					RemoveOrder(o);
				}
			}
		}
		
		mt.DataLeave();
	}
	
	mt.Data();
}

void Automation::RemoveOrder(const Order& o) {
	MetaTrader& mt = GetMetaTrader();
	int sym_id = o.symbol;
	
	if (o.type >= OP_BUYLIMIT) {
		mt.OrderDelete(o.ticket);
	}
	else {
		for(int j = 0; j < 10; j++) {
			double close;
			if (o.type == OP_BUY)
				close = mt.RealtimeBid(sym_id);
			else
				close = mt.RealtimeAsk(sym_id);
			if (mt.OrderClose(o.ticket, o.volume, close, 3))
				break;
		}
	}
}









AutomationCtrl::AutomationCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	split << list << parent;
	
	CtrlLayout(parent);
	
	list.AddColumn("Symbol");
	list.AddColumn("From");
	list.AddColumn("Till");
	list.AddColumn("Signal");
	list.AddColumn("Open");
	list.AddColumn("Take-profit");
	list.AddColumn("Stop-loss");
	
	parent.save <<= THISBACK(Save);
	
	symbols.Add("EURUSD");
	symbols.Add("USDCHF");
	symbols.Add("GBPUSD");
	symbols.Add("USDJPY");
	symbols.Add("USDCAD");
	symbols.Add("AUDUSD");
	symbols.Add("EURJPY");
	symbols.Add("NZDUSD");
	symbols.Add("GBPCHF");
}

void AutomationCtrl::Data() {
	Automation& a = GetAutomation();
	
	if (parent.symbol.GetCount() == 0) {
		for(int i = 0; i < symbols.GetCount(); i++)
			parent.symbol.Add(symbols[i]);
		parent.symbol.SetIndex(0);
		
		parent.signal.Add("Buy");
		parent.signal.Add("Sell");
		parent.signal.SetIndex(0);
	}
	
	for(int i = list.GetCount(); i < a.signals.GetCount(); i++) {
		AutomationSignal& sig = a.signals[i];
		
		list.Set(i, 0, sig.symbol);
		list.Set(i, 1, sig.from);
		list.Set(i, 2, sig.till);
		list.Set(i, 3, sig.sig ? "Sell" : "Buy");
		list.Set(i, 4, sig.open);
		list.Set(i, 5, sig.tp);
		list.Set(i, 6, sig.sl);
		
		list.ScrollEnd();
	}
	
}

void AutomationCtrl::Save() {
	Automation& a = GetAutomation();
	
	AutomationSignal& sig = a.signals.Add();
	
	double hours = parent.length.GetData();
	
	sig.symbol = symbols[parent.symbol.GetIndex()];
	sig.from = GetUtcTime();
	sig.till = sig.from + hours * 60*60;
	sig.sig = parent.signal.GetIndex();
	sig.open = parent.open.GetData();
	sig.tp = parent.tp.GetData();
	sig.sl = parent.sl.GetData();
	
	a.StoreThis();
}

}
