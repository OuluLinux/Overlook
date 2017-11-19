#include "Overlook.h"

namespace Overlook {

FixedSimBroker::FixedSimBroker() {
	
	
	begin_equity = 10000;
}

void FixedSimBroker::Reset() {
	equity = begin_equity;
	balance = begin_equity;
	part_balance = 0.0;
	part_equity = 0.0;
	order_count = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
	free_margin_level = 0.80;
	for(int i = 0; i < SYM_COUNT; i++) {
		signal[i] = 0;
		signal_freezed[i] = false;
	}
	for(int i = 0; i < MAX_ORDERS; i++)
		order[i].is_open = false;
	init = false;
}

double FixedSimBroker::RealtimeBid(int sym_id) const {
	return price[sym_id] - spread_points[sym_id];
}

double FixedSimBroker::RealtimeAsk(int sym_id) const {
	return price[sym_id];
}

double FixedSimBroker::GetCloseProfit(int sym_id, const FixedOrder& o) const {
	
	// NOTE: only for forex. Check SimBroker for other symbols too
	
	double volume = 10000 * o.volume; // lotsize * volume
	
	double close;
	if (o.type == OP_BUY)
		close = RealtimeBid(sym_id);
	else if (o.type == OP_SELL)
		close = RealtimeAsk(sym_id);
	else
		return 0.0;
	
	int proxy_id = this->proxy_id[sym_id];
	int proxy_base_mul = this->proxy_base_mul[sym_id];
	
	if (o.type == OP_BUY) {
		double change = volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0)
			;
		else if (proxy_base_mul == +1)
			change /= RealtimeBid(proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeAsk(proxy_id);
		
		return change;
	}
	else if (o.type == OP_SELL) {
		double change = -1.0 * volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0)
			;
		else if (proxy_base_mul == +1)
			change /= RealtimeAsk(proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeBid(proxy_id);
		
		return change;
	}
	else {
		Panic("Invalid type");
		return 0.0;
	}
}

void FixedSimBroker::OrderSend(int sym_id, int type, double volume, double price) {
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
		return;
	}
	
	// Close smallest order
	int order_id = -1;
	double order_volume = DBL_MAX;
	for(int i = 0; i < ORDERS_PER_SYMBOL; i++) {
		FixedOrder& order = this->order[sym_orders_begin + i];
		if (order.type == type && order.volume < order_volume) {
			order_id = i;
			order_volume = order.volume;
		}
	}
	
	if (order_id != -1) {
		FixedOrder& order = this->order[sym_orders_begin + order_id];
		double prev_vol = order.volume;
		OrderClose(sym_id, order.volume, order);
		order.type = type;
		order.volume = volume + prev_vol;
		order.open = price;
		order.close = price;
		order.profit = 0.0;
		order.is_open = true;
		order_count++;
		return;
	}
	
	// Last resort... all orders were against this, which is illegal anyway.
	CloseAll();
	OrderSend(sym_id, type, volume, price);
}

void FixedSimBroker::OrderClose(int sym_id, double lots, FixedOrder& order) {
	if (!order.is_open)
		return;
	double profit;
	if (lots < order.volume) {
		double remain = order.volume - lots;
		order.volume = lots; // hackish... switch volume for calculation
		profit = GetCloseProfit(sym_id, order);
		order.volume = remain;
		order.is_open = remain >= 0.01;
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
		if (sym_id == part_sym_id) part_balance += profit;
	} else {
		order.is_open = false;
		profit = GetCloseProfit(sym_id, order);
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
		if (sym_id == part_sym_id) part_balance += profit;
	}
	if (balance < 0.0) balance = 0.0;
}

void FixedSimBroker::CloseAll() {
	for(int i = 0; i < MAX_ORDERS; i++) {
		FixedOrder& order = this->order[i];
		if (!order.is_open)
			continue;
		int sym_id = i / ORDERS_PER_SYMBOL;
		order.is_open = false;
		double profit = GetCloseProfit(sym_id, order);
		balance += profit;
		if (profit > 0) profit_sum += profit;
		else            loss_sum   -= profit;
	}
	if (balance < 0.0) balance = 0.0;
}

double FixedSimBroker::GetSpreadCost() const {
	double spread_cost_sum = 0.0;
	for(int i = 0; i < MAX_ORDERS; i++) {
		const FixedOrder& order = this->order[i];
		if (!order.is_open)
			continue;
		int sym_id = i / ORDERS_PER_SYMBOL;
		// TODO solution for other than pairs
		double volsum = order.volume * 10000;
		double ask = RealtimeAsk(sym_id);
		double spread_factor = (spread_points[sym_id] + ask) / ask - 1.0;
		spread_cost_sum += volsum * spread_factor;
	}
	return spread_cost_sum;
}

double FixedSimBroker::GetMargin(int sym_id, double volume) {
	ASSERT(leverage > 0);
	double used_margin = 0.0;
	if (proxy_id[sym_id] < 0) {
		used_margin = //RealtimeAsk(snap, sym_id) *
			volume * 10000;
	} else {
		if (proxy_base_mul[sym_id] == -1)
			used_margin = RealtimeAsk(proxy_id[sym_id]) * volume * 10000;
		else
			used_margin = (1.0 / RealtimeAsk(proxy_id[sym_id])) * volume * 10000;
	}
	
	
	#if 0
	// Sort of realistic. Leverage is always limited and limits varies between brokers.
	// The leverage is based on equity, but it's actually in steps in the real world, not linear.
	// Warning: will cause agents to stop trading at 10000 equity with current settings!
	// Fix idea: weight reward with available leverage... but that it just a difficult way to train the same result.
	if (equity < 100000.0) {
		if (equity < 1000.0)
			used_margin /= leverage;
		else
			used_margin /= (1.0 - (equity - 1000.0) / (100000.0 - 1000.0)) * leverage;
	}
	#else
	used_margin /= leverage;
	#endif
	
	ASSERT(volume == 0.0 || used_margin > 0.0);
	ASSERT(IsFin(used_margin));
	return used_margin;
}

bool FixedSimBroker::Cycle() {
	double buy_lots[SYM_COUNT];
	double sell_lots[SYM_COUNT];
	int buy_signals[SYM_COUNT];
	int sell_signals[SYM_COUNT];
	
	
	int loop_begin, loop_end;
	int orderloop_begin, orderloop_end;
	if (part_sym_id != -1) {
		loop_begin = part_sym_id;
		loop_end = part_sym_id + 1;
		orderloop_begin = part_sym_id * ORDERS_PER_SYMBOL;
		orderloop_end = (part_sym_id + 1) * ORDERS_PER_SYMBOL;
	} else {
		loop_begin = 0;
		loop_end = SYM_COUNT;
		orderloop_begin = 0;
		orderloop_end = MAX_ORDERS;
	}
	
	
	// Get maximum margin sum
	ASSERT(free_margin_level >= 0.55 && free_margin_level <= 1.0);
	if (free_margin_level >= 1.0)
		free_margin_level = 0.8;
	
	
	// Get long/short signals
	// Shortcomings:
	//  - volumes should match, signals might not have correct value
	for(int i = loop_begin; i < loop_end; i++) {
		buy_signals[i] = 0;
		sell_signals[i] = 0;
		buy_lots[i] = 0.0;
		sell_lots[i] = 0.0;
	}
	
	int signal_sum = 0;
	for(int i = loop_begin; i < loop_end; i++) {
		int signal = this->signal[i];
		if (!signal) continue;
		if (signal > 0) {
			buy_signals[i] += signal;
			signal_sum += signal;
		} else {
			sell_signals[i] -= signal;
			signal_sum -= signal;
		}
	}
	
	
	// Balance signals
	for(int i = loop_begin; i < loop_end; i++) {
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
	for(int i = loop_begin; i < loop_end; i++) {
		int buy = buy_signals[i];
		int sell = sell_signals[i];
		if (buy  > 0 && buy < min_sig)
			min_sig = buy;
		if (sell > 0 && sell < min_sig)
			min_sig = sell;
		sig_abs_total += buy + sell;
	}
	if (!sig_abs_total) {
		CloseAll();
		return true;
	}
	
	
	double minimum_margin_sum = 0;
	int active_symbols = 0;
	for(int i = loop_begin; i < loop_end; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0)
			continue;
		double buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01;
		double sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01;
		double buy_used_margin = GetMargin(i, buy_lots);
		double sell_used_margin = GetMargin(i, sell_lots);
		ASSERT(IsFin(buy_used_margin));
		ASSERT(IsFin(sell_used_margin));
		minimum_margin_sum += buy_used_margin;
		minimum_margin_sum += sell_used_margin;
		active_symbols++;
	}
	
	
	const int fmscale = (MULT_MAXSCALES-1) * MULT_MAXSCALE_MUL * SYM_COUNT;
	if (signal_sum > fmscale) signal_sum = fmscale;
	double max_margin_sum = AccountEquity() * (1.0 - free_margin_level) * ((double)signal_sum / (double)fmscale);
	
	
	double lot_multiplier = max_margin_sum / minimum_margin_sum;
	if (lot_multiplier < 1.0)
		return false;
	
	
	for(int i = loop_begin; i < loop_end; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0)
			continue;
		double sym_buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01 * lot_multiplier;
		double sym_sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01 * lot_multiplier;
		buy_lots[i]  = ((long)(sym_buy_lots  / 0.01)) * 0.01;
		sell_lots[i] = ((long)(sym_sell_lots / 0.01)) * 0.01;
	}
	
	
	for(int i = orderloop_begin; i < orderloop_end; i++) {
		FixedOrder& o = order[i];
		if (!o.is_open)
			continue;
		
		int symbol = i / ORDERS_PER_SYMBOL;
		
		if (signal_freezed[symbol])
			continue;
		
		if (o.type == OP_BUY) {
			double& lots = buy_lots[symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				OrderClose(symbol, reduce, o);
				lots = 0;
			}
		}
		else if (o.type == OP_SELL) {
			double& lots = sell_lots[symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				OrderClose(symbol, reduce, o);
				lots = 0;
			}
		}
	}
	
	
	for(int i = loop_begin; i < loop_end; i++) {
		if (signal_freezed[i])
			continue;
		
		if (buy_lots[i] < 0.01 && sell_lots[i] < 0.01)
			continue;
		double sym_buy_lots = buy_lots[i];
		double sym_sell_lots = sell_lots[i];
		
		if (sym_buy_lots > 0.0) {
			double price = RealtimeAsk(i);
			OrderSend(i, OP_BUY, sym_buy_lots, price);
		}
		if (sym_sell_lots > 0.0) {
			double price = RealtimeBid(i);
			OrderSend(i, OP_SELL, sym_sell_lots, price);
		}
	}
	
	return true;
}

void FixedSimBroker::RefreshOrders() {
	double e = balance;
	double pe = part_balance;
	int orderloop_begin, orderloop_end;
	if (part_sym_id != -1) {
		orderloop_begin = part_sym_id * ORDERS_PER_SYMBOL;
		orderloop_end = (part_sym_id + 1) * ORDERS_PER_SYMBOL;
	} else {
		orderloop_begin = 0;
		orderloop_end = MAX_ORDERS;
	}
	for(int i = orderloop_begin; i < orderloop_end; i++) {
		FixedOrder& o = order[i];
		if (o.is_open) {
			int sym_id = i / ORDERS_PER_SYMBOL;
			double p = GetCloseProfit(sym_id, o);
			o.close = RealtimeAsk(sym_id);
			o.profit = p;
			e += p;
			if (sym_id == part_sym_id) pe += p;
		}
	}
	equity = e;
	part_equity = pe;
}

}

