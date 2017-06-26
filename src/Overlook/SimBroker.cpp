#include "Overlook.h"

namespace Overlook {

SimBroker::SimBroker() {
	order_counter = 1000000;
	lightweight = false;
	initial_balance = 1000;
	cur_begin = -1;
	sys = NULL;
}

void SimBroker::Init(MetaTrader& mt, System& sys) {
	this->sys = &sys;
	lightweight = false;
	
	symbols <<= mt.GetSymbols();
	currency = mt.AccountCurrency();
	
	for(int i = 0; i < symbols.GetCount(); i++)
		symbol_idx.Add(symbols[i].name);
	
	cur_begin = mt.GetSymbolCount();
	
	Clear();
	
	/*System& bs = GetSystem();
	src = bs.FindLinkCore("/open");
	ASSERTEXC(src);
	change = bs.FindLinkCore("/change");
	ASSERTEXC(change);*/
}

void SimBroker::InitLightweight() {
	lightweight = true;
	
	Clear();
	
	/*System& bs = GetSystem();
	src = bs.FindLinkCore("/open");
	ASSERTEXC(src);
	change = bs.FindLinkCore("/change");
	ASSERTEXC(change);*/
}

void SimBroker::Clear() {
	orders.Clear();
	history_orders.Clear();
	
	for(int i = 0; i < signals.GetCount(); i++) signals[i] = 0;
	
	balance = initial_balance;
	equity = initial_balance;
	leverage = 1000;
	margin = 0;
	margin_free = equity;
	is_failed = false;
	
	prev_cycle_time = Time(1970,1,1);
}

void SimBroker::Cycle() {
	BackwardExposure();
}

int SimBroker::FindSymbol(const String& symbol) const {
	return symbol_idx.Find(symbol);
}

int SimBroker::GetSignal(int symbol) const {
	return signals[symbol];
}

int SimBroker::GetOpenOrderCount() const {
	return orders.GetCount();
}

double SimBroker::GetFreeMarginLevel() const {
	
	
	return 0;
}

Time SimBroker::GetTime() const {
	
	Panic("TODO"); return Time();
}

void SimBroker::SetFreeMarginLevel(double d) {
	if (d < min_free_margin_level) d = min_free_margin_level;
	if (d > max_free_margin_level) d = max_free_margin_level;
	free_margin_level = d;
}







int		SimBroker::RefreshRates() {
	return false;
}

int		SimBroker::iBars(String symbol, int timeframe) {
	/*System& bs = GetSystem();
	int period = timeframe*60 / bs.GetBasePeriod();
	return bs.GetCount(period);*/
	
	Panic("TODO"); return 0;
}

int		SimBroker::iBarShift(String symbol, int timeframe, int datetime) {
	Panic("TODO"); return 0;
}

double	SimBroker::iClose(String symbol, int timeframe, int shift) {
	/*System& bs = GetSystem();
	int sym = FindSymbol(symbol);
	CoreProcessAttributes& attr = bs.GetCurrent();
	return *src->GetValue<double>(
		0,
		sym,
		ol.FindPeriod(bs.GetTfFromSeconds(timeframe*60)),
		-1+shift, attr);*/
		
	Panic("TODO"); return 0;
}

double	SimBroker::iHigh(String symbol, int timeframe, int shift) {
	/*System& bs = GetSystem();
	int sym = FindSymbol(symbol);
	CoreProcessAttributes& attr = bs.GetCurrent();
	return *src->GetValue<double>(
		2,
		sym,
		ol.FindPeriod(bs.GetTfFromSeconds(timeframe*60)),
		shift, attr);*/
		
	Panic("TODO"); return 0;
}

double	SimBroker::iLow(String symbol, int timeframe, int shift) {
	/*System& bs = GetSystem();
	int sym = FindSymbol(symbol);
	CoreProcessAttributes& attr = bs.GetCurrent();
	return *src->GetValue<double>(
		1,
		sym,
		ol.FindPeriod(bs.GetTfFromSeconds(timeframe*60)),
		shift, attr);*/
		
	Panic("TODO"); return 0;
}

double	SimBroker::iOpen(String symbol, int timeframe, int shift) {
	/*System& bs = GetSystem();
	int sym = FindSymbol(symbol);
	CoreProcessAttributes& attr = bs.GetCurrent();
	return *src->GetValue<double>(
		0,
		sym,
		ol.FindPeriod(bs.GetTfFromSeconds(timeframe*60)),
		shift, attr);*/
		
	Panic("TODO"); return 0;
}

int		SimBroker::iHighest(String symbol, int timeframe, int type, int count, int start) {
	Panic("TODO"); return 0;
}

int		SimBroker::iLowest(String symbol, int timeframe, int type, int count, int start) {
	Panic("TODO"); return 0;
}

int		SimBroker::iTime(String symbol, int timeframe, int shift) {
	Panic("TODO"); return 0;
}

int		SimBroker::iVolume(String symbol, int timeframe, int shift) {
	Panic("TODO"); return 0;
}

double	SimBroker::RealtimeAsk(int sym) {
	Panic("TODO"); return 0;
}

double	SimBroker::RealtimeBid(int sym) {
	Panic("TODO"); return 0;
}

int		SimBroker::OrderClose(int ticket, double lots, double price, int slippage) {
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.ticket != ticket) continue;
		
		ASSERT(o.type == OP_BUY || o.type == OP_SELL);
		Symbol& sym = symbols[o.sym];
		
		Order& ho = history_orders.Add(o);
		orders.Remove(i);
		ho.close = o.type == OP_BUY ? askbid[o.sym].bid : askbid[o.sym].ask;
		// TODO: check slippage
		
		ho.end = GetTime();
		
		Panic("TODO");
		return true;
	}
	return false;
}

double	SimBroker::OrderClosePrice() {
	return orders[selected].close;
}

int		SimBroker::OrderCloseTime() {
	return orders[selected].end.Get() - Time(1970,1,1).Get();
}

String	SimBroker::OrderComment() {
	return orders[selected].comment;
}

double	SimBroker::OrderCommission() {
	return orders[selected].commission;
}

int		SimBroker::OrderDelete(int ticket) {
	Panic("TODO: pending orders");
	return 0;
}

int		SimBroker::OrderExpiration() {
	return orders[selected].expiration.Get() - Time(1970,1,1).Get();
}

double	SimBroker::OrderLots() {
	return orders[selected].volume;
}

int		SimBroker::OrderMagicNumber() {
	Panic("Magic numbers aren't allowed to use because they are used internally in databridge.");
	return 0;
}

int		SimBroker::OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) {
	
	Panic("TODO"); return 0;
}

double	SimBroker::OrderOpenPrice() {
	return orders[selected].open;
}

int		SimBroker::OrderOpenTime() {
	return orders[selected].begin.Get() - Time(1970,1,1).Get();
}

double	SimBroker::OrderProfit() {
	return orders[selected].profit;
}

int		SimBroker::OrderSelect(int index, int select, int pool) {
	selected = -1;
	if (select == SELECT_BY_POS) {
		if (pool == MODE_TRADES) {
			if (index >= 0 && index < orders.GetCount()) {
				selected = index;
				return true;
			}
		}
		else if (pool == MODE_HISTORY) {
			if (index >= 0 && index < history_orders.GetCount()) {
				selected = index;
				return true;
			}
		}
		else return false;
	}
	else if (select == SELECT_BY_TICKET) {
		if (pool == MODE_TRADES) {
			for(int i = 0; i < orders.GetCount(); i++) {
				if (orders[i].ticket == index) {
					selected = i;
					return true;
				}
			}
		}
		else if (pool == MODE_HISTORY) {
			for(int i = 0; i < history_orders.GetCount(); i++) {
				if (history_orders[i].ticket == index) {
					selected = i;
					return true;
				}
			}
		}
		else return false;
	}
	else return false;
}

int		SimBroker::OrderSend(
			String symbol, int cmd, double volume, double price, int slippage, double stoploss,
			double takeprofit, int magic, int expiry) {
	int sym = FindSymbol(symbol);
	if (sym == -1) return -1;
	if (cmd != OP_BUY && cmd != OP_SELL) return -1;
	Symbol& s = symbols[sym];
	if (volume < s.volume_min || volume > s.volume_max) return -1;
	// TODO: check slippage, stoploss and takeprofit
	Order& o = orders.Add();
	o.begin = GetTime();
	o.type = cmd;
	o.symbol = symbol;
	o.sym = sym;
	o.open = cmd != OP_BUY ? askbid[o.sym].bid : askbid[o.sym].ask;
	o.stoploss = stoploss;
	o.takeprofit = takeprofit;
	//o.magic = magic;
	//o.expiration = expiry;
	o.ticket = order_counter++;
	o.is_open = true;
	return o.ticket;
}

int		SimBroker::OrdersHistoryTotal() {
	return history_orders.GetCount();
}

double	SimBroker::OrderStopLoss() {
	return orders[selected].stoploss;
}

int		SimBroker::OrdersTotal() {
	return orders.GetCount();
}

double	SimBroker::OrderSwap() {
	return orders[selected].swap;
}

String	SimBroker::OrderSymbol() {
	return orders[selected].symbol;
}

double	SimBroker::OrderTakeProfit() {
	return orders[selected].takeprofit;
}

int		SimBroker::OrderTicket() {
	return orders[selected].ticket;
}

int		SimBroker::OrderType() {
	return orders[selected].type;
}

bool    SimBroker::IsDemo() {
	return AccountInfoInteger(ACCOUNT_TRADE_MODE) != ACCOUNT_TRADE_MODE_REAL;
}

bool    SimBroker::IsConnected() {
	return true;
}

}
