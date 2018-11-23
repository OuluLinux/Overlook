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
	
}

Time TimeFromStr(const String& s) {
	Vector<String> v = Split(s, ",");
	return Time(StrInt(v[0]), StrInt(v[1]), StrInt(v[2]), StrInt(v[3]), StrInt(v[4]));
}

void Automation::Start() {
	RLOG("Automation::Start");

	Time now = GetUtcTime();
	
	MetaTrader& mt = GetMetaTrader();
	
	if (last_update <= now - 30) {
		TcpSocket s;
		
		if (!s.Connect("127.0.0.1", 17777)) return;
		s.Blocking();
		Vector<String> lines;
		for(int i = 0; i < 9 * 7; i++) {
			lines.Add(s.GetLine());
		}
		if (lines.IsEmpty())
			return;
		
		for(int i = 0; i < lines.GetCount();) {
			
			String sym = lines[i++];
			String from = lines[i++];
			String till = lines[i++];
			String sig = lines[i++];
			String sig_at = lines[i++];
			String tp = lines[i++];
			String sl = lines[i++];
			if (sym == "")
				continue;
			
			Time f = TimeFromStr(from);
			Time t = TimeFromStr(till);
			
			int sym_id = GetSystem().FindSymbol(sym);
			double stop_points = mt._MarketInfo(sym, MODE_STOPLEVEL);
			double point = mt.GetSymbols()[sym_id].point;
			
			mt.DataEnter();
			const Vector<Order>& open_orders = mt.GetOpenOrders();
			
			if (now >= f && now < t && tp.GetCount() && sl.GetCount()) {
				
				bool have_open = false;
				for(int j = 0; j < open_orders.GetCount(); j++) {
					const Order& o = open_orders[j];
					String osym = mt.GetSymbols()[o.symbol];
					if (osym == sym)
						have_open = true;
				}
				
				if (!have_open) {
					double open = StrDbl(sig_at);
					double takeprofit = StrDbl(tp);
					double stoploss = StrDbl(sl);
					double ask = mt.RealtimeAsk(sym_id);
					double bid = mt.RealtimeBid(sym_id);
					double lots = mt.AccountBalance() * 0.001;
					int cmd;
					if (sig == "Buy") {
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
					
					if (mt.OrderSend(sym, cmd, lots, open, 0, stoploss, takeprofit, "Automation", 0) < 0) {
						String err = mt._GetLastError();
						ReleaseLog("Pending order open failed " + sym + ": " + err);
						if (err == "no error") {
							double imopen;
							if (sig == "Buy") {
								imopen = mt.RealtimeAsk(sym_id);
								cmd = OP_BUY;
							}
							else {
								imopen = mt.RealtimeBid(sym_id);
								cmd = OP_SELL;
							}
							int slippage = fabs((imopen - open) / point);
							if (slippage < 3)
								mt.OrderSend(sym, cmd, lots, imopen, 3, stoploss, takeprofit, "Automation", 0);
						}
						if (err == "invalid stops") {
							int ticket = mt.OrderSend(sym, cmd, lots, open, 1, 0, 0, "", 0);
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
					if (osym == sym) {
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
				}
			}
			
			mt.DataLeave();
			
			
		}
		
		mt.Data();
	}
}










AutomationCtrl::AutomationCtrl() {
	
}

void AutomationCtrl::Data() {
	
}

}
