#include "Overlook.h"

namespace Overlook {

SimBroker::SimBroker() {
	order_counter = 1000000;
	lightweight = false;
	initial_balance = 1000;
	cur_begin = -1;
	close_sum = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
	collect_limit = 0.0;
	collected = 0.0;
	do_collect = false;
	
}

void SimBroker::SetCollecting(double d) {
	if (d <= 0.0)
		do_collect = false;
	else {
		do_collect = true;
		collect_limit = d;
	}
}

void SimBroker::Init() {
	Clear();
	
	close_sum = 0;
	lightweight = false;
	
	MetaTrader& mt = GetMetaTrader();
	symbols <<= mt.GetSymbols();
	currency = mt.AccountCurrency();
	
	for(int i = 0; i < symbols.GetCount(); i++)
		symbol_idx.Add(symbols[i].name);
	cur_begin = mt.GetSymbolCount();
}

void SimBroker::InitLightweight() {
	Clear();
	
	lightweight = true;
	
	if (symbol_idx.IsEmpty()) {
		for(int i = 0; i < symbols.GetCount(); i++)
			symbol_idx.Add(symbols[i].name);
		cur_begin = GetMetaTrader().GetSymbolCount();
	}
}

void SimBroker::Clear() {
	Enter();
	
	Brokerage::Clear();
	
	is_failed = false;
	order_counter = 1000000;
	cycle_time = Time(1970,1,1);
	close_sum = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
	collected = 0.0;
	
	
	signals.SetCount(symbols.GetCount(), 0);
	signal_freezed.SetCount(symbols.GetCount(), 0);
	symbol_profits.SetCount(symbols.GetCount(), 0);
	prev_symbol_profits.SetCount(symbols.GetCount(), 0);
	symbol_profit_diffs.SetCount(symbols.GetCount(), 0);
	for(int i = 0; i < symbols.GetCount(); i++) {
		signals[i] = 0;
		signal_freezed[i] = 0;
		symbol_profits[i] = 0;
		prev_symbol_profits[i] = 0;
		symbol_profit_diffs[i] = 0;
	}
	
	Leave();
}

double SimBroker::PopCloseSum() {
	double d = close_sum;
	close_sum = 0;
	return d;
}

void SimBroker::Cycle() {
	SignalOrders();
	if (do_collect) {
		if (balance > collect_limit) {
			collected += balance - collect_limit;
			balance = collect_limit;
		}
	}
}

void SimBroker::CycleChanges() {
	ASSERT(!symbol_profits.IsEmpty());
	
	
	// Move current symbol profits to previous and reset current values
	Swap(symbol_profits, prev_symbol_profits);
	for(int i = 0; i < symbol_profits.GetCount(); i++)
		symbol_profits[i] = 0;
	
	
	// Add order profits to sum
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		symbol_profits[o.symbol] += o.profit;
	}
	
	// Calculate difference between previous values
	for(int i = 0; i < symbol_profits.GetCount(); i++) {
		double diff = symbol_profits[i] - prev_symbol_profits[i];
		symbol_profit_diffs[i] = diff;
	}
}

void SimBroker::RefreshOrders() {
	double used_margin = 0;
	double equity = balance;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.profit = GetCloseProfit(o, o.volume);
		o.close = o.type == OP_BUY ? askbid[o.symbol].bid : askbid[o.symbol].ask;
		equity += o.profit;
		
		if (o.type == OP_BUY) {
			if (o.takeprofit != 0.0 && o.close >= o.takeprofit) {
				if (OrderClose(o.ticket, o.volume, o.close, 10)) i--;
			}
			else if (o.stoploss != 0.0 && o.close <= o.stoploss) {
				if (OrderClose(o.ticket, o.volume, o.close, 10)) i--;
			}
		}
		else if (o.type == OP_SELL) {
			if (o.takeprofit != 0.0 && o.close <= o.takeprofit) {
				if (OrderClose(o.ticket, o.volume, o.close, 10)) i--;
			}
			else if (o.stoploss != 0.0 && o.close >= o.stoploss) {
				if (OrderClose(o.ticket, o.volume, o.close, 10)) i--;
			}
		}
		
		double order_used_margin = GetMargin(o.symbol, o.volume);
		used_margin += order_used_margin;
	}
	this->margin = used_margin;
	this->margin_free = equity * free_margin_level - used_margin;
	this->equity = equity;
}


int SimBroker::FindSymbol(const String& symbol) const {
	return symbol_idx.Find(symbol);
}

double SimBroker::GetDrawdown() const {
	double total = profit_sum + loss_sum;
	if (total == 0.0)
		return 1.0;
	return loss_sum / total;
}

int SimBroker::GetSignal(int symbol) const {
	return signals[symbol];
}

int SimBroker::GetOpenOrderCount() const {
	return orders.GetCount();
}

Time SimBroker::GetTime() const {
	return cycle_time;
}

void SimBroker::SetPrice(int sym, double price) {
	Price& p = askbid[sym];
	double spread = p.ask / p.bid;
	p.bid = price;
	p.ask = price * spread;
}

void SimBroker::SetPrice(int sym, double ask, double bid) {
	Price& p = askbid[sym];
	p.bid = bid;
	p.ask = ask;
}

int		SimBroker::RefreshRates() {
	return false;
}

int		SimBroker::iBars(String symbol, int timeframe) {
	Panic("no need to implement yet"); return 0;
}

int		SimBroker::iBarShift(String symbol, int timeframe, int datetime) {
	Panic("no need to implement yet"); return 0;
}

double	SimBroker::iClose(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

double	SimBroker::iHigh(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

double	SimBroker::iLow(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

double	SimBroker::iOpen(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

int		SimBroker::iHighest(String symbol, int timeframe, int type, int count, int start) {
	Panic("no need to implement yet"); return 0;
}

int		SimBroker::iLowest(String symbol, int timeframe, int type, int count, int start) {
	Panic("no need to implement yet"); return 0;
}

int		SimBroker::iTime(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

int		SimBroker::iVolume(String symbol, int timeframe, int shift) {
	Panic("no need to implement yet"); return 0;
}

double	SimBroker::RealtimeAsk(int sym) {
	return askbid[sym].ask;
}

double	SimBroker::RealtimeBid(int sym) {
	return askbid[sym].bid;
}

int		SimBroker::OrderClose(int ticket, double lots, double price, int slippage) {
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.ticket != ticket) continue;
		
		ASSERT(o.type == OP_BUY || o.type == OP_SELL);
		Symbol& sym = symbols[o.symbol];
		
		double close = o.type == OP_BUY ? askbid[o.symbol].bid : askbid[o.symbol].ask;
		
		// Inspect slippage
		double slippage_points = sym.point * slippage;
		double close_max = close + slippage_points * 0.5;
		double close_min = close - slippage_points * 0.5;
		if (!(close_min <= price && price <= close_max)) {
			last_error = "Invalid close price";
			return false;
		}
		
		// Reduce or close?
		if (lots > o.volume) {
			last_error = "Invalid close volume";
			return false;
		}
		
		
		// Close
		if (lots == o.volume) {
			o.close = o.type == OP_BUY ? askbid[o.symbol].bid : askbid[o.symbol].ask;
			o.profit = GetCloseProfit(o, o.volume);
			if (o.profit >= 0) profit_sum += o.profit;
			else               loss_sum   -= o.profit;
			balance += o.profit;
			close_sum += o.profit;
			
			if (!lightweight) {
				Order& ho = history_orders.Add(o);
				ho.end = GetTime();
			}
			orders.Remove(i);
		}
		
		// Reduce
		else {
			double profit = GetCloseProfit(o, lots);
			o.volume -= lots;
			balance -= profit;
			close_sum += profit;
			if (profit >= 0) profit_sum += profit;
			else             loss_sum   -= profit;
		}
		
		RefreshOrders();
		
		return true;
	}
	last_error = "Order not found";
	return false;
}

double SimBroker::GetCloseProfit(const Order& o, double volume) const {
	const Symbol& sym = symbols[o.symbol];
	volume *= sym.lotsize;
	if (!sym.IsForex())
		volume *= o.open;
	
	if (sym.is_base_currency) {
		if (o.type == OP_BUY) {
			if (askbid[o.symbol].ask == o.open) return 0.0; // normal data changes, broken data not
			return      volume * (askbid[o.symbol].bid / o.open - 1.0);
		}
		else if (o.type == OP_SELL) {
			if (askbid[o.symbol].bid == o.open) return 0.0; // normal data changes, broken data not
			return -1 * volume * (askbid[o.symbol].ask / o.open - 1.0);
		}
		else Panic("Type handling not implemented");
	}
	else {
		// return mt4 profits as is, because proxy_open is not set.
		if (o.proxy_open == 0.0)
			return o.profit;
		
		const Symbol& proxy = symbols[sym.proxy_id];
		ASSERT(proxy.base_mul != 0);
		if (o.type == OP_BUY) {
			if (askbid[o.symbol].ask == o.open) return 0.0; // normal data changes, broken data not
			double change = volume * (askbid[o.symbol].bid / o.open - 1.0);
			if (proxy.base_mul == +1) {
				change /= askbid[sym.proxy_id].bid;
			}
			else if (proxy.base_mul == -1) {
				change *= askbid[sym.proxy_id].ask;
			}
			else Panic("Invalid proxy sym");
			return change;
		}
		else if (o.type == OP_SELL) {
			if (askbid[o.symbol].bid == o.open) return 0.0; // normal data changes, broken data not
			double change = -1 * volume * (askbid[o.symbol].ask / o.open - 1.0);
			if (proxy.base_mul == +1) {
				change /= askbid[sym.proxy_id].ask;
			}
			else if (proxy.base_mul == -1) {
				change *= askbid[sym.proxy_id].bid;
			}
			else Panic("Invalid proxy sym");
			return change;
		}
		else Panic("Type handling not implemented");
	}
	Panic("NEVER"); return 0;
}

double SimBroker::OrderClosePrice() {
	return GetSelected().close;
}

Time SimBroker::OrderCloseTime() {
	return GetSelected().end;
}

String SimBroker::OrderComment() {
	return ""; // Not included for performance reasons. Match ticket to String and store separately if needed.
}

double SimBroker::OrderCommission() {
	return GetSelected().commission;
}

int SimBroker::OrderDelete(int ticket) {
	Panic("no need to implement yet"); return 0;
}

int SimBroker::OrderExpiration() {
	return (int)(GetSelected().expiration.Get() - Time(1970,1,1).Get());
}

double SimBroker::OrderLots() {
	return GetSelected().volume;
}

int SimBroker::OrderMagicNumber() {
	return GetSelected().magic;
}

int SimBroker::OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) {
	Panic("no need to implement yet"); return 0;
}

double SimBroker::OrderOpenPrice() {
	return GetSelected().open;
}

int SimBroker::OrderOpenTime() {
	return (int)(GetSelected().begin.Get() - Time(1970,1,1).Get());
}

double SimBroker::OrderProfit() {
	return GetSelected().profit;
}

int SimBroker::OrderSelect(int index, int select, int pool) {
	selected = -1;
	selected_pool = -1;
	if (select == SELECT_BY_POS) {
		if (pool == MODE_TRADES) {
			if (index >= 0 && index < orders.GetCount()) {
				selected = index;
				selected_pool = pool;
				return true;
			}
			return false;
		}
		else if (pool == MODE_HISTORY) {
			if (index >= 0 && index < history_orders.GetCount()) {
				selected = index;
				selected_pool = pool;
				return true;
			}
			return false;
		}
		else return false;
	}
	else if (select == SELECT_BY_TICKET) {
		if (pool == MODE_TRADES) {
			for(int i = 0; i < orders.GetCount(); i++) {
				if (orders[i].ticket == index) {
					selected = i;
					selected_pool = pool;
					return true;
				}
			}
			return false;
		}
		else if (pool == MODE_HISTORY) {
			for(int i = 0; i < history_orders.GetCount(); i++) {
				if (history_orders[i].ticket == index) {
					selected = i;
					selected_pool = pool;
					return true;
				}
			}
			return false;
		}
		else return false;
	}
	else return false;
	
	Panic("NEVER"); return 0;
}

int SimBroker::OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry) {
	return OrderSend(FindSymbol(symbol), cmd, volume, price, slippage, stoploss, takeprofit, magic, expiry);
}

int SimBroker::OrderSend(int symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry) {
	if (symbol == -1) return -1;
	if (cmd != OP_BUY && cmd != OP_SELL) {last_error = "Invalid command"; return -1;}
	Symbol& s = symbols[symbol];
	if (volume < s.volume_min) {last_error = "Invalid volume"; return -1;}
	// TODO: check slippage, stoploss and takeprofit
	
	if (cmd == OP_BUY || cmd == OP_SELL) {
		Order& o = orders.Add();
		o.begin = GetTime();
		o.type = cmd;
		o.volume = volume;
		o.symbol = symbol;
		o.open = cmd != OP_BUY ? askbid[o.symbol].bid : askbid[o.symbol].ask;
		if (!s.is_base_currency) {
			if (s.proxy_id == -1) {
				last_error = "Invalid proxy";
				return -1;
			}
			Symbol& proxy = symbols[s.proxy_id];
			o.proxy_open = proxy.base_mul == (cmd == OP_BUY ? -1 : +1) ? askbid[s.proxy_id].bid : askbid[s.proxy_id].ask;
		}
		o.stoploss = stoploss;
		o.takeprofit = takeprofit;
		o.ticket = order_counter++;
		o.is_open = true;
		o.profit = GetCloseProfit(o, volume);
		o.magic = magic;
		return o.ticket;
	}
	else
		Panic("Not implemented");
	return -1;
}

Order& SimBroker::GetSelected() {
	if (selected_pool == MODE_TRADES)
		return orders[selected];
	else
		return history_orders[selected];
}

int		SimBroker::OrdersHistoryTotal() {
	return history_orders.GetCount();
}

double	SimBroker::OrderStopLoss() {
	return GetSelected().stoploss;
}

int		SimBroker::OrdersTotal() {
	return orders.GetCount();
}

double	SimBroker::OrderSwap() {
	return GetSelected().swap;
}

String	SimBroker::OrderSymbol() {
	int sym = GetSelected().symbol;
	return symbols[sym].name;
}

double	SimBroker::OrderTakeProfit() {
	return GetSelected().takeprofit;
}

int		SimBroker::OrderTicket() {
	return GetSelected().ticket;
}

int		SimBroker::OrderType() {
	return GetSelected().type;
}

bool    SimBroker::IsDemo() {
	return AccountInfoInteger(ACCOUNT_TRADE_MODE) != ACCOUNT_TRADE_MODE_REAL;
}

bool    SimBroker::IsConnected() {
	return true;
}

}
