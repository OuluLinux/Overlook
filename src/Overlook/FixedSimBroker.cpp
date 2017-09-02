#include "Overlook.h"

namespace Overlook {

SingleFixedSimBroker::SingleFixedSimBroker() {
	
	
	begin_equity = 10000;
}

void SingleFixedSimBroker::Reset() {
	
	
	
	equity = begin_equity;
	balance = begin_equity;
	order.is_open = false;
	order_count = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
}

float SingleFixedSimBroker::RealtimeBid(const Snapshot& snap, int sym_id) const {
	return snap.open[sym_id] - spread_points;
}

float SingleFixedSimBroker::RealtimeAsk(const Snapshot& snap, int sym_id) const {
	return snap.open[sym_id];
}

double SingleFixedSimBroker::GetCloseProfit(const Snapshot& snap) const {
	const FixedOrder& o = order;
	
	// NOTE: only for forex. Check SimBroker for other symbols too
	
	double volume = 100000 * o.volume; // lotsize * volume
	
	float close;
	if (o.type == OP_BUY)
		close = RealtimeBid(snap, sym_id);
	else if (o.type == OP_SELL)
		close = RealtimeAsk(snap, sym_id);
	else
		return 0.0;
	
	// Some time ranges can be invalid, and they have same price constantly...
	// Don't let them affect. This is the easiest and safest way to avoid that problem.
	// In normal and meaningful environment, open and close price is never the same.
	if (o.open == close)
		return 0.0;
	
	
	if (o.type == OP_BUY) {
		double change = volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeBid(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeAsk(snap, proxy_id);
		return change;
	}
	else if (o.type == OP_SELL) {
		double change = -1.0 * volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeAsk(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeBid(snap, proxy_id);
		return change;
	}
	else return 0.0;
}

void SingleFixedSimBroker::OrderSend(int type, float volume, float price) {
	ASSERT(!order.is_open);
	order.type = type;
	order.volume = volume;
	order.open = price;
	order.is_open = true;
	order_count++;
}

void SingleFixedSimBroker::OrderClose(const Snapshot& snap) {
	ASSERT(order.is_open);
	order.is_open = false;
	double profit = GetCloseProfit(snap);
	balance += profit;
	equity = balance;
	if (profit > 0) profit_sum += profit;
	else            loss_sum   -= profit;
}

void SingleFixedSimBroker::Cycle(int signal, const Snapshot& snap) {
	ASSERT(sym_id >= 0 && sym_id < SYM_COUNT);
	
	FixedOrder& o = order;
	
	// Close order
	if (o.is_open) {
		if (o.type == OP_BUY) {
			if (signal <= 0)
				OrderClose(snap);
			else
				return;
		}
		else if (o.type == OP_SELL) {
			if (signal >= 0)
				OrderClose(snap);
			else
				return;
		}
	}
	
	
	if      (signal > 0)
		OrderSend(OP_BUY,  0.01f, RealtimeAsk(snap, sym_id));
	else if (signal < 0)
		OrderSend(OP_SELL, 0.01f, RealtimeBid(snap, sym_id));
	
}

void SingleFixedSimBroker::RefreshOrders(const Snapshot& snap) {
	FixedOrder& o = order;
	
	if (o.is_open) {
		equity = balance + GetCloseProfit(snap);
	} else {
		equity = balance;
	}
}
























FixedSimBroker::FixedSimBroker() {
	
	
	begin_equity = 10000;
}

void FixedSimBroker::Reset() {
	equity = begin_equity;
	balance = begin_equity;
	order_count = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
	free_margin_level = 0.80;
	for(int i = 0; i < SYM_COUNT; i++)
		signal[i] = 0;
	for(int i = 0; i < MAX_ORDERS; i++)
		order[i].is_open = false;
}

float FixedSimBroker::RealtimeBid(const Snapshot& snap, int sym_id) const {
	return snap.open[sym_id] - spread_points[sym_id];
}

float FixedSimBroker::RealtimeAsk(const Snapshot& snap, int sym_id) const {
	return snap.open[sym_id];
}

double FixedSimBroker::GetCloseProfit(int sym_id, const FixedOrder& o, const Snapshot& snap) const {
	
	// NOTE: only for forex. Check SimBroker for other symbols too
	
	double volume = 100000 * o.volume; // lotsize * volume
	
	float close;
	if (o.type == OP_BUY)
		close = RealtimeBid(snap, sym_id);
	else if (o.type == OP_SELL)
		close = RealtimeAsk(snap, sym_id);
	else
		return 0.0;
	
	// Some time ranges can be invalid, and they have same price constantly...
	// Don't let them affect. This is the easiest and safest way to avoid that problem.
	// In normal and meaningful environment, open and close price is never the same.
	if (o.open == close)
		return 0.0;
	
	int proxy_id = this->proxy_id[sym_id];
	int proxy_base_mul = this->proxy_base_mul[sym_id];
	
	if (o.type == OP_BUY) {
		double change = volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeBid(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeAsk(snap, proxy_id);
		return change;
	}
	else if (o.type == OP_SELL) {
		double change = -1.0 * volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeAsk(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeBid(snap, proxy_id);
		return change;
	}
	else return 0.0;
}

void FixedSimBroker::OrderSend(int sym_id, int type, float volume, float price) {
	ASSERT(sym_id >= 0 && sym_id < SYM_COUNT);
	int sym_orders_begin = sym_id * ORDERS_PER_SYMBOL;
	
	for(int i = 0; i < ORDERS_PER_SYMBOL; i++) {
		FixedOrder& order = this->order[sym_orders_begin + i];
		if (order.is_open)
			continue;
		order.type = type;
		order.volume = volume;
		order.open = price;
		order.close = price;
		order.profit = 0.0f;
		order.is_open = true;
		order_count++;
		break;
	}
}

void FixedSimBroker::OrderClose(int sym_id, double lots, FixedOrder& order, const Snapshot& snap) {
	if (!order.is_open)
		return;
	if (lots < order.volume) {
		double remain = order.volume - lots;
		order.volume = lots; // hackish... switch volume for calculation
		double profit = GetCloseProfit(sym_id, order, snap);
		order.volume = remain;
		order.is_open = remain >= 0.01;
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
	} else {
		order.is_open = false;
		double profit = GetCloseProfit(sym_id, order, snap);
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
	}
}
/*
void FixedSimBroker::OrderClose(int sym_id, const Snapshot& snap) {
	ASSERT(sym_id >= 0 && sym_id < SYM_COUNT);
	int sym_orders_begin = sym_id * ORDERS_PER_SYMBOL;
	
	for(int i = 0; i < ORDERS_PER_SYMBOL; i++) {
		FixedOrder& order = this->order[sym_orders_begin + i];
		if (!order.is_open)
			continue;
		order.is_open = false;
		double profit = GetCloseProfit(sym_id, order, snap);
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
	}
}
*/
void FixedSimBroker::CloseAll(const Snapshot& snap) {
	for(int i = 0; i < MAX_ORDERS; i++) {
		FixedOrder& order = this->order[i];
		if (!order.is_open)
			continue;
		int sym_id = i / ORDERS_PER_SYMBOL;
		order.is_open = false;
		double profit = GetCloseProfit(sym_id, order, snap);
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
	}
}

double FixedSimBroker::GetMargin(const Snapshot& snap, int sym_id, double volume) {
	ASSERT(leverage > 0);
	double used_margin = 0.0;
	if (proxy_id[sym_id] == -1) {
		used_margin = RealtimeAsk(snap, sym_id) * volume * 100000;
	} else {
		if (proxy_base_mul[sym_id] == -1)
			used_margin = RealtimeAsk(snap, proxy_id[sym_id]) * volume * 100000;
		else
			used_margin = (1.0 / RealtimeAsk(snap, proxy_id[sym_id])) * volume * 100000;
	}
	used_margin /= leverage;
	ASSERT(volume == 0.0 || used_margin > 0.0);
	return used_margin;
}

bool FixedSimBroker::Cycle(const Snapshot& snap) {
	
	// Get maximum margin sum
	ASSERT(free_margin_level >= 0.60 && free_margin_level <= 1.0);
	if (free_margin_level == 1.0)
		free_margin_level = 0.8;
	double max_margin_sum = AccountEquity() * (1.0 - free_margin_level);
	
	
	// Get long/short signals
	// Shortcomings:
	//  - volumes should match, signals might not have correct value
	for(int i = 0; i < SYM_COUNT; i++) {
		buy_signals[i] = 0;
		sell_signals[i] = 0;
		buy_lots[i] = 0.0;
		sell_lots[i] = 0.0;
	}
	
	for(int i = 0; i < SYM_COUNT; i++) {
		int signal = this->signal[i];
		if (!signal) continue;
		if (signal > 0)
			buy_signals[i] += signal;
		else
			sell_signals[i] -= signal;
	}
	
	
	// Balance signals
	for(int i = 0; i < SYM_COUNT; i++) {
		int& buy  =  buy_signals[i];
		int& sell = sell_signals[i];
		if (buy == 0 || sell == 0) continue;
		if (buy > sell) {
			buy -= sell;
			sell = 0;
		}
		else {
			sell -= buy;
			buy = 0;
		}
	}
	
	
	int sig_abs_total = 0;
	int min_sig = INT_MAX;
	for(int i = 0; i < SYM_COUNT; i++) {
		int buy = buy_signals[i];
		int sell = sell_signals[i];
		if (buy  > 0 && buy < min_sig)
			min_sig = buy;
		if (sell > 0 && sell < min_sig)
			min_sig = sell;
		sig_abs_total += buy + sell;
	}
	if (!sig_abs_total) {
		CloseAll(snap);
		return true;
	}
	
	
	double minimum_margin_sum = 0;
	for(int i = 0; i < SYM_COUNT; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0)
			continue;
		double buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01;
		double sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01;
		double buy_used_margin = GetMargin(snap, i, buy_lots);
		double sell_used_margin = GetMargin(snap, i, sell_lots);
		minimum_margin_sum += buy_used_margin;
		minimum_margin_sum += sell_used_margin;
	}
	
	
	double lot_multiplier = max_margin_sum / minimum_margin_sum;
	if (lot_multiplier < 1.0)
		return false;
	
	
	for(int i = 0; i < SYM_COUNT; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0)
			continue;
		double sym_buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01 * lot_multiplier;
		double sym_sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01 * lot_multiplier;
		buy_lots[i]  = ((long)(sym_buy_lots  / 0.01)) * 0.01;
		sell_lots[i] = ((long)(sym_sell_lots / 0.01)) * 0.01;
	}
	
	
	for(int i = 0; i < MAX_ORDERS; i++) {
		FixedOrder& o = order[i];
		if (!o.is_open)
			continue;
		
		int symbol = i / ORDERS_PER_SYMBOL;
		
		if (o.type == OP_BUY) {
			double& lots = buy_lots[symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				OrderClose(symbol, reduce, o, snap);
				lots = 0;
			}
		}
		else if (o.type == OP_SELL) {
			double& lots = sell_lots[symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				OrderClose(symbol, reduce, o, snap);
				lots = 0;
			}
		}
	}
	
	
	for(int i = 0; i < SYM_COUNT; i++) {
		if (buy_lots[i] < 0.01 && sell_lots[i] < 0.01)
			continue;
		double sym_buy_lots = buy_lots[i];
		double sym_sell_lots = sell_lots[i];
		
		if (sym_buy_lots > 0.0) {
			double price = RealtimeAsk(snap, i);
			OrderSend(i, OP_BUY, sym_buy_lots, price);
		}
		if (sym_sell_lots > 0.0) {
			double price = RealtimeBid(snap, i);
			OrderSend(i, OP_SELL, sym_sell_lots, price);
		}
	}
	
	return true;
}

void FixedSimBroker::RefreshOrders(const Snapshot& snap) {
	double e = balance;
	for(int i = 0; i < MAX_ORDERS; i++) {
		FixedOrder& o = order[i];
		if (o.is_open) {
			int sym = i / ORDERS_PER_SYMBOL;
			double p = GetCloseProfit(sym, o, snap);
			o.close = snap.open[sym];
			o.profit = p;
			e += p;
		}
	}
	equity = e;
}

}
