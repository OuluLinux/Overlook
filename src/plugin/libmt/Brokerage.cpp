#include "libmt.h"

namespace libmt {
	
Brokerage::Brokerage() {
	cur_begin = -1;
	is_failed = false;
	min_free_margin_level = 0.65;
	max_free_margin_level = 0.95;
	free_margin_level = 0.98;
	margin_call = 0.5;
	margin_stop = 0.2;
	selected = -1;
	tf_h1_id = -1;
	account_currency_id = -1;
}

void Brokerage::operator=(const Brokerage& b) {
	orders <<= b.orders;
	history_orders <<= b.history_orders;
	skipped_currencies <<= b.skipped_currencies;
	symbol_idx <<= b.symbol_idx;
	account_name = b.account_name;
	account_server = b.account_server;
	account_currency = b.account_currency;
	account_company = b.account_company;
	last_error = b.last_error;
	free_margin_level = b.free_margin_level;
	min_free_margin_level = b.min_free_margin_level;
	max_free_margin_level = b.max_free_margin_level;
	balance = b.balance;
	equity = b.equity;
	margin = b.margin;
	margin_free = b.margin_free;
	margin_call = b.margin_call;
	margin_stop = b.margin_stop;
	leverage = b.leverage;
	initial_balance = b.initial_balance;
	cur_begin = b.cur_begin;
	account_currency_id = b.account_currency_id;
	account_id = b.account_id;
	selected = b.selected;
	demo = b.demo;
	connected = b.connected;
	simulation = b.simulation;
	init_success = b.init_success;
	is_failed = b.is_failed;
	basket_symbols <<= b.basket_symbols;
	symbols <<= b.symbols;
	askbid <<= b.askbid;
	pricetf <<= b.pricetf;
	currencies <<= b.currencies;
	indices <<= b.indices;
	signals <<= b.signals;
	signal_freezed <<= b.signal_freezed;
	periodstr <<= b.periodstr;
	tf_h1_id = b.tf_h1_id;
	cur_volumes <<= b.cur_volumes;
	idx_volumes <<= b.idx_volumes;
	cur_rates <<= b.cur_rates;
	cur_base_values <<= b.cur_base_values;
	idx_rates <<= b.idx_rates;
	idx_base_values <<= b.idx_base_values;
}

void Brokerage::SetSignal(int sym, int signal) {
	if (signals.IsEmpty()) {
		signals.SetCount(symbols.GetCount(), 0);
		signal_freezed.SetCount(symbols.GetCount(), 0);
	}
	signals[sym] = signal;
}

void Brokerage::SetSignalFreeze(int sym, bool freeze_signal) {
	signal_freezed[sym] = freeze_signal;
}

void Brokerage::PutSignal(int sym, int signal) {
	if (signals.IsEmpty()) signals.SetCount(symbols.GetCount(), 0);
	signals[sym] += signal;
}

void Brokerage::ForwardExposure() {
	String base = AccountCurrency();
	double leverage = AccountLeverage();
	double base_volume = AccountEquity() * leverage;
	
	int sym_count = symbols.GetCount();
	int cur_count = currencies.GetCount();
	int skip_account_cur = account_currency_id - sym_count;
	
	idx_rates.SetCount(0);
	idx_volumes.SetCount(0);
	idx_base_values.SetCount(0);
	cur_rates.SetCount(0);
	cur_volumes.SetCount(0);
	cur_base_values.SetCount(0);
	idx_rates.SetCount(sym_count, 0.0);
	idx_volumes.SetCount(sym_count, 0.0);
	idx_base_values.SetCount(sym_count, 0.0);
	cur_rates.SetCount(cur_count, 0.0);
	cur_volumes.SetCount(cur_count, 0.0);
	cur_base_values.SetCount(cur_count, 0.0);
	
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		
		if (o.symbol < 0 || o.symbol >= sym_count)
			continue;
		
		const Symbol& sym = GetSymbol(o.symbol);
		
		if (sym.margin_calc_mode == Symbol::CALCMODE_FOREX) {
			if (sym.base_mul == +1) {
				// For example: base=USD, sym=USDCHF, cmd=buy, lots=0.01, a=USD, b=CHF
				double& cur_volume = cur_volumes[sym.base_cur1];
				if (o.type == OP_BUY)
					cur_volume += -1 * o.volume * sym.lotsize * askbid[o.symbol].ask;
				else if (o.type == OP_SELL)
					cur_volume += o.volume * sym.lotsize * askbid[o.symbol].bid;
				else Panic("Type handling not implemented");
			}
			else if (sym.base_mul == -1) {
				// For example: base=USD, sym=AUDUSD, cmd=buy, lots=0.01, a=AUD, b=USD
				double& cur_volume = cur_volumes[sym.base_cur0];
				if (o.type == OP_BUY)
					cur_volume += o.volume * sym.lotsize;
				else if (o.type == OP_SELL)
					cur_volume += -1 * o.volume * sym.lotsize;
				else Panic("Type handling not implemented");
			}
			else {
				ASSERT(sym.proxy_id != -1);
				// For example: base=USD, sym=CHFJPY, cmd=buy, lots=0.01, s=USDCHF, a=CHF, b=JPY
				//  - CHF += 0.01 * 1000
				{
					double& cur_volume = cur_volumes[sym.base_cur1];
					if (o.type == OP_BUY)
						cur_volume += o.volume * sym.lotsize;
					else if (o.type == OP_SELL)
						cur_volume += -1 * o.volume * sym.lotsize;
					else Panic("Type handling not implemented");
				}
				//  - JPY += -1 * 0.01 * 1000 * open-price
				{
					double& cur_volume = cur_volumes[sym.base_cur0];
					if (o.type == OP_BUY)
						cur_volume += -1 * o.volume * sym.lotsize * o.open;
					else if (o.type == OP_SELL)
						cur_volume += o.volume * sym.lotsize * o.open;
					else Panic("Type handling not implemented");
				}
			}
		}
		else {
			double& idx_volume = idx_volumes[o.symbol];
			double value;
			if (o.type == OP_BUY)
				value = o.volume * sym.contract_size;
			else if (o.type == OP_SELL)
				value = -1 * o.volume * sym.contract_size;
			else Panic("Type handling not implemented");
			idx_volume += value;
		}
		
	}
	
	// Get values in base currency: currencies
	for(int i = 0; i < cur_count; i++) {
		if (i == skip_account_cur) continue;
		const Currency& cur = currencies[i];
		
		double& rate = cur_rates[i];
		
		if (cur.base_mul == -1)
			rate = askbid[cur.base_pair].bid;
		else if (cur.base_mul == +1)
			rate = 1 / askbid[cur.base_pair].ask;
		else if (cur.base_mul == 0)
			rate = 1.0;
		else
			Panic("Currency has no known exchange pair");
		
		double volume = cur_volumes[i];
		double base_value = volume / leverage * rate;
		cur_base_values[i] = base_value;
	}
	
	// Get values in base currency: Indices and CFDs
	for(int i = 0; i < sym_count; i++) {
		const Symbol& sym = symbols[i];
		const String& cur = sym.currency_base;
		
		double& rate = idx_rates[i];
		double volume = idx_volumes[i];
		if (volume == 0.0) {
			idx_base_values[i] = 0.0;
			continue;
		}
		
		if (sym.is_base_currency) {
			if (volume < 0)		rate = askbid[i].ask;
			else				rate = askbid[i].bid;
			double base_value = volume * rate * sym.margin_factor;// * fabs(volume / sym.contract_size);
			idx_base_values[i] = base_value;
		} else {
			const Symbol& proxy_sym = symbols[sym.proxy_id];
			
			if (proxy_sym.base_mul == -1) {
				rate = askbid[sym.proxy_id].bid;
				if (volume < 0)		rate *= askbid[i].ask;
				else				rate *= askbid[i].bid;
			}
			else if (proxy_sym.base_mul == +1) {
				rate = 1 / askbid[sym.proxy_id].ask;
				if (volume < 0)		rate *= askbid[i].ask;
				else				rate *= askbid[i].bid;
			}
			else
				Panic("Invalid proxy symbol");
			
			double base_value = volume * rate * sym.margin_factor;// * abs(volume / bs.contract_size);
			idx_base_values[i] = base_value;
		}
	}
	
	// Currencies
	assets.SetCount(0);
	double base_value = 0.0;
	for(int i = 0; i < cur_count; i++) {
		if (i == skip_account_cur) continue;
		
		double volume = cur_volumes[i];
		if (volume == 0.0) continue;
		
		Asset& a		= assets.Add();
		a.sym			= sym_count + i;
		a.volume		= volume;
		a.rate			= cur_rates[i];
		a.base_value	= cur_base_values[i];
		
		// Freaky logic, but gives correct result, which have been checked many, many times.
		double base_volume_change = 0;
		if (a.base_value > 0)
			base_volume_change = -1.0 * a.base_value * leverage * 2;
		else
			base_volume_change = -1.0 * a.base_value * leverage;
		
		base_volume += base_volume_change;
		base_value  += a.volume * a.rate;
	}
	
	// Indices and CFDs
	for(int i = 0; i < sym_count; i++) {
		double volume = idx_volumes[i];
		if (volume == 0.0) continue;
		
		const Symbol& sym = symbols[i];
		
		Asset& a		= assets.Add();
		a.sym			= i;
		a.volume		= idx_volumes[i];
		a.rate			= idx_rates[i];
		a.base_value	= idx_base_values[i];
		
		if (sym.is_base_currency) {
			double base_volume_change = 0;
			if (a.base_value > 0)
				base_volume_change = -1.0 * a.base_value * leverage;
			else
				base_volume_change = +1.0 * a.base_value * leverage;
		
			base_volume += base_volume_change;
			base_value  += a.volume / a.rate;
		}
	}
	
	// Base currency
	Asset& a = assets.Add();
	a.sym = account_currency_id;
	a.volume = base_volume;
	a.rate = 1.00;
	a.base_value = base_volume / leverage;
	
	
}


void Brokerage::CloseAll() {
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		double close = o.type == OP_BUY ?
			RealtimeBid(o.symbol) :
			RealtimeAsk(o.symbol);
		int r = OrderClose(o.ticket, o.volume, close, 100);
		if (r) {
			LOG("Order closed");
			i--;
		} else {
			LOG("Order close failed");
		}
	}
}


void Brokerage::SignalOrders() {
	ASSERT(signals.GetCount() == symbols.GetCount());
	double leverage = AccountLeverage();
	int sym_count = symbols.GetCount();
	int cur_count = currencies.GetCount();
	
	
	// Get maximum margin sum
	ASSERT(free_margin_level >= 0.20 && free_margin_level <= 1.0);
	if (free_margin_level == 1.0)
		free_margin_level = 0.8;
	double max_margin_sum = AccountEquity() * (1.0 - free_margin_level);
	
	
	// Get long/short signals
	// Shortcomings:
	//  - volumes should match, signals might not have correct value
	buy_signals.SetCount(0);
	buy_signals.SetCount(signals.GetCount(), 0);
	sell_signals.SetCount(0);
	sell_signals.SetCount(signals.GetCount(), 0);
	buy_lots.SetCount(0);
	buy_lots.SetCount(signals.GetCount(), 0);
	sell_lots.SetCount(0);
	sell_lots.SetCount(signals.GetCount(), 0);
	for(int i = 0; i < sym_count; i++) {
		const Symbol& sym = symbols[i];
		int signal = signals[i];
		if (!signal) continue;
		if (signal > 0) {
			buy_signals[i] += signal;
			if (sym.proxy_id != -1) {
				const Symbol& proxy = symbols[sym.proxy_id];
				if (proxy.base_mul == +1)
					buy_signals[sym.proxy_id] += signal;
				else
					sell_signals[sym.proxy_id] += signal;
			}
		}
		else {
			sell_signals[i] -= signal;
			if (sym.proxy_id != -1) {
				const Symbol& proxy = symbols[sym.proxy_id];
				if (proxy.base_mul == +1)
					sell_signals[sym.proxy_id] -= signal;
				else
					buy_signals[sym.proxy_id] -= signal;
			}
		}
	}
	
	// Balance signals
	for(int i = 0; i < sym_count; i++) {
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
	for(int i = 0; i < sym_count; i++) {
		int buy = buy_signals[i];
		int sell = sell_signals[i];
		if (buy  > 0 && buy < min_sig) min_sig = buy;
		if (sell > 0 && sell < min_sig) min_sig = sell;
		sig_abs_total += buy + sell;
	}
	if (!sig_abs_total) {
		CloseAll();
		return;
	}
	
	double minimum_margin_sum = 0;
	for(int i = 0; i < sym_count; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0) continue;
		const Symbol& sym = symbols[i];
		const Price& p = askbid[i];
		double buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01;
		double sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01;
		double buy_used_margin = p.ask * buy_lots * sym.contract_size * sym.margin_factor;
		double sell_used_margin = p.bid * sell_lots * sym.contract_size * sym.margin_factor;
		if (sym.IsForex()) {
			buy_used_margin  /= leverage;
			sell_used_margin /= leverage;
		}
		minimum_margin_sum += buy_used_margin;
		minimum_margin_sum += sell_used_margin;
	}
	
	double lot_multiplier = max_margin_sum / minimum_margin_sum;
	if (lot_multiplier < 1.0) {
		last_error = "Total margin is too much";
		CloseAll();
		return;
	}
	
	for(int i = 0; i < sym_count; i++) {
		if (buy_signals[i] == 0 && sell_signals[i] == 0) continue;
		const Symbol& sym = symbols[i];
		const Price& p = askbid[i];
		double sym_buy_lots  = (double)buy_signals[i]  / (double)min_sig * 0.01 * lot_multiplier;
		double sym_sell_lots = (double)sell_signals[i] / (double)min_sig * 0.01 * lot_multiplier;
		buy_lots[i]  = ((int)(sym_buy_lots  / 0.01)) * 0.01;
		sell_lots[i] = ((int)(sym_sell_lots / 0.01)) * 0.01;
	}
	
	for(int i = 0; i < orders.GetCount(); i++) {
		const Order& o = orders[i];
		if (signal_freezed[o.symbol])
			continue;
		if (o.type == OP_BUY) {
			double& lots = buy_lots[o.symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				for(int j = 0; j < 3; j++) {
					int success = OrderClose(o.ticket, reduce, RealtimeBid(o.symbol), 100);
					if (success) break;
				}
				lots = 0;
			}
		}
		else if (o.type == OP_SELL) {
			double& lots = sell_lots[o.symbol];
			if (o.volume <= lots) {
				lots -= o.volume;
			} else {
				double reduce = o.volume - lots;
				for(int j = 0; j < 3; j++) {
					int success = OrderClose(o.ticket, reduce, RealtimeAsk(o.symbol), 100);
					if (success) break;
				}
				lots = 0;
			}
		}
	}
	
	for(int i = 0; i < sym_count; i++) {
		if (signal_freezed[i])
			continue;
		if (buy_lots[i] < 0.01 && sell_lots[i] < 0.01)
			continue;
		double sym_buy_lots = buy_lots[i];
		double sym_sell_lots = sell_lots[i];
		const Symbol& sym = symbols[i];
		if (sym_buy_lots > 0.0) {
			double price = RealtimeAsk(i);
			int r = OrderSend(i, OP_BUY, sym_buy_lots, price, 100, price * 0.99, price * 1.01, 0, 0);
			if (r == -1) {
				LOG("Brokerage::SignalOrders: OrderSend faild with buy " + sym.name + " lots=" + DblStr(sym_buy_lots));
			}
		}
		if (sym_sell_lots > 0.0) {
			double price = RealtimeBid(i);
			int r = OrderSend(i, OP_SELL, sym_sell_lots, price, 100, price * 1.01, price * 0.99, 0, 0);
			if (r == -1) {
				LOG("Brokerage::SignalOrders: OrderSend faild with sell " + sym.name + " lots=" + DblStr(sym_sell_lots));
			}
		}
	}
}









double	Brokerage::AccountInfoDouble(int property_id) {
	switch (property_id) {
		case ACCOUNT_BALANCE:				return balance;
		case ACCOUNT_CREDIT:				return balance;
		case ACCOUNT_PROFIT:				return equity - balance;
		case ACCOUNT_EQUITY:				return equity;
		case ACCOUNT_MARGIN:				return margin;
		case ACCOUNT_MARGIN_FREE:			return margin_free;
		case ACCOUNT_MARGIN_LEVEL:			return (1.0 - free_margin_level) * 100.0;
		case ACCOUNT_MARGIN_SO_CALL:		return margin_call;
		case ACCOUNT_MARGIN_SO_SO:			return margin_stop;
		case ACCOUNT_MARGIN_INITIAL:		return 0.0;
		case ACCOUNT_MARGIN_MAINTENANCE:	return 0.0;
		case ACCOUNT_ASSETS:				return 0.0;
		case ACCOUNT_LIABILITIES:			return 0.0;
		case ACCOUNT_COMMISSION_BLOCKED:	return 0.0;
		default: return 0.0;
	}
}

int		Brokerage::AccountInfoInteger(int property_id) {
	switch (property_id) {
		case ACCOUNT_LOGIN:					return account_id;
		case ACCOUNT_TRADE_MODE:			return ACCOUNT_TRADE_MODE_DEMO;
		case ACCOUNT_LEVERAGE:				return leverage;
		case ACCOUNT_LIMIT_ORDERS:			return 0; // TODO pending orders
		case ACCOUNT_MARGIN_SO_MODE:		return ACCOUNT_STOPOUT_MODE_PERCENT;
		case ACCOUNT_TRADE_ALLOWED:			return true;
		case ACCOUNT_TRADE_EXPERT:			return true;
	}
}

String	Brokerage::AccountInfoString(int property_id) {
	switch (property_id) {
		case ACCOUNT_NAME:					return account_name;
		case ACCOUNT_SERVER:				return account_server;
		case ACCOUNT_CURRENCY:				return account_currency;
		case ACCOUNT_COMPANY:				return account_company;
	}
}


double	Brokerage::AccountBalance() {
	return balance;
}

double	Brokerage::AccountCredit() {
	return balance;
}

String	Brokerage::AccountCompany() {
	return AccountInfoString(ACCOUNT_COMPANY);
}

String	Brokerage::AccountCurrency() {
	return AccountInfoString(ACCOUNT_CURRENCY);
}

double	Brokerage::AccountEquity() {
	return AccountInfoDouble(ACCOUNT_EQUITY);
}

double	Brokerage::AccountFreeMargin() {
	return AccountInfoDouble(ACCOUNT_MARGIN_FREE);
}

double	Brokerage::AccountFreeMarginCheck(String symbol, int cmd, double volume) {
	
	Panic("TODO"); return 0;
}

double	Brokerage::AccountFreeMarginMode() {
	return FREEMARGINMODE_WITHOPENORDERS; // all open orders affects the free margin
}

int		Brokerage::AccountLeverage() {
	return AccountInfoInteger(ACCOUNT_LEVERAGE);
}

double	Brokerage::AccountMargin() {
	return AccountInfoDouble(ACCOUNT_MARGIN);
}

String	Brokerage::AccountName() {
	return AccountInfoString(ACCOUNT_NAME);
}

int		Brokerage::AccountNumber() {
	return AccountInfoInteger(ACCOUNT_LOGIN);
}

double	Brokerage::AccountProfit() {
	return AccountInfoDouble(ACCOUNT_PROFIT);
}

String	Brokerage::AccountServer() {
	return AccountInfoString(ACCOUNT_SERVER);
}

int		Brokerage::AccountStopoutLevel() {
	return AccountInfoDouble(ACCOUNT_MARGIN_SO_SO);
}

int		Brokerage::AccountStopoutMode() {
	return AccountInfoInteger(ACCOUNT_MARGIN_SO_MODE);
}

double	Brokerage::MarketInfo(String symbol, int type) {
	/*
	System& bs = GetSystem();
	CoreProcessAttributes& attr = bs.GetCurrent();
	
	int sym = FindSymbol(symbol);
	
	switch (type) {
		case MODE_LOW:			return *src->GetValue<double>(
									1, // LOW
									sym,
									bs.FindPeriod(bs.GetTfFromSeconds(24*60*60)),
									0, attr);
		case MODE_HIGH:			return *src->GetValue<double>(
									2, // HIGH
									sym,
									bs.FindPeriod(bs.GetTfFromSeconds(24*60*60)),
									0, attr);
		case MODE_TIME:			return bs.GetTime(1, GetBars()).Get() - Time(1970,1,1).Get();
		case MODE_BID:			return askbid[sym].bid;
		case MODE_ASK:			return askbid[sym].ask;
		case MODE_POINT:		return symbols[sym].point;
		case MODE_DIGITS:		return symbols[sym]. digits;
		case MODE_SPREAD:		return (askbid[sym].ask - askbid[sym].bid) / symbols[sym].point;
		case MODE_STOPLEVEL:	return 0.0;
		case MODE_LOTSIZE:		return lotsize;
		case MODE_TICKVALUE:	return symbols[sym].tick_value;
		case MODE_TICKSIZE:		return symbols[sym].tick_size;
		case MODE_SWAPLONG:		return symbols[sym].swap_long;
		case MODE_SWAPSHORT:	return symbols[sym].swap_short;
		case MODE_STARTING:		return symbols[sym].start_time;
		case MODE_EXPIRATION:	return symbols[sym].expiration_time;
		case MODE_TRADEALLOWED:	return symbols[sym].tradeallowed;
		case MODE_MINLOT:		return symbols[sym].volume_min;
		case MODE_LOTSTEP:		return symbols[sym].volume_step;
		case MODE_MAXLOT:		return symbols[sym].volume_max;
		case MODE_SWAPTYPE:		return symbols[sym].swap_mode;
		case MODE_PROFITCALCMODE:	return symbols[sym].profit_calc_mode;
		case MODE_MARGINCALCMODE:	return symbols[sym].margin_calc_mode;
		case MODE_MARGININIT:		return symbols[sym].margin_initial;
		case MODE_MARGINMAINTENANCE:	return symbols[sym].margin_maintenance;
		case MODE_MARGINHEDGED:			return symbols[sym].margin_hedged;
		case MODE_MARGINREQUIRED:		return symbols[sym].margin_required;
		case MODE_FREEZELEVEL:			return symbols[sym].freeze_level;
		case MODE_CLOSEBY_ALLOWED:		return false;
	}*/
	
	Panic("TODO"); return 0;
}

int		Brokerage::SymbolsTotal(int selected) {
	ASSERT(!selected); // symbols in general list is allowed only
	return symbols.GetCount();
}

String	Brokerage::SymbolName(int pos, int selected) {
	ASSERT(!selected); // symbols in general list is allowed only
	return symbols[pos].name;
}

int		Brokerage::SymbolSelect(String name, int select) {
	return false; // can't implement
}

double	Brokerage::SymbolInfoDouble(String name, int prop_id) {
	/*System& bs = GetSystem();
	
	CoreProcessAttributes& attr = bs.GetCurrent();
	
	int sym = FindSymbol(name);
	if (sym == -1) return 0;
	
	switch (prop_id) {
		case SYMBOL_BID:							return askbid[sym].bid;
		case SYMBOL_BIDHIGH:						return 0;
		case SYMBOL_BIDLOW:							return 0;
		case SYMBOL_ASK:							return askbid[sym].ask;
		case SYMBOL_ASKHIGH:						return 0;
		case SYMBOL_ASKLOW:							return 0;
		case SYMBOL_LAST:							return 0;
		case SYMBOL_LASTHIGH:						return 0;
		case SYMBOL_LASTLOW:						return 0;
		case SYMBOL_POINT:							return symbols[sym].point;
		case SYMBOL_TRADE_TICK_VALUE:				return symbols[sym].tick_value;
		case SYMBOL_TRADE_TICK_VALUE_PROFIT:		return 0;
		case SYMBOL_TRADE_TICK_VALUE_LOSS:			return 0;
		case SYMBOL_TRADE_TICK_SIZE:				return symbols[sym].tick_size;
		case SYMBOL_TRADE_CONTRACT_SIZE:			return symbols[sym].contract_size;
		case SYMBOL_VOLUME_MIN:						return symbols[sym].volume_min;
		case SYMBOL_VOLUME_MAX:						return symbols[sym].volume_max;
		case SYMBOL_VOLUME_STEP:					return symbols[sym].volume_step;
		case SYMBOL_VOLUME_LIMIT:					return 0;
		case SYMBOL_SWAP_LONG:						return symbols[sym].swap_long;
		case SYMBOL_SWAP_SHORT:						return symbols[sym].swap_short;
		case SYMBOL_MARGIN_INITIAL:					return symbols[sym].margin_initial;
		case SYMBOL_MARGIN_MAINTENANCE:				return symbols[sym].margin_maintenance;
		case SYMBOL_MARGIN_LONG:					return 0;
		case SYMBOL_MARGIN_SHORT:					return 0;
		case SYMBOL_MARGIN_LIMIT:					return 0;
		case SYMBOL_MARGIN_STOP:					return 0;
		case SYMBOL_MARGIN_STOPLIMIT:				return 0;
		case SYMBOL_SESSION_VOLUME:					return 0;
		case SYMBOL_SESSION_TURNOVER:				return 0;
		case SYMBOL_SESSION_INTEREST:				return 0;
		case SYMBOL_SESSION_BUY_ORDERS_VOLUME:		return 0;
		case SYMBOL_SESSION_SELL_ORDERS_VOLUME:		return 0;
		case SYMBOL_SESSION_OPEN:					return 0;
		case SYMBOL_SESSION_CLOSE:					return 0;
		case SYMBOL_SESSION_AW:						return 0;
		case SYMBOL_SESSION_PRICE_SETTLEMENT:		return 0;
		case SYMBOL_SESSION_PRICE_LIMIT_MIN:		return 0;
		case SYMBOL_SESSION_PRICE_LIMIT_MAX:		return 0;
		default: return 0;
	}*/
	
	Panic("TODO"); return 0;
}

int		Brokerage::SymbolInfoInteger(String name, int prop_id) {
	/*System& bs = GetSystem();
	
	CoreProcessAttributes& attr = bs.GetCurrent();
	
	int sym = FindSymbol(name);
	if (sym == -1) return 0;
	
	switch (prop_id) {
		case SYMBOL_SELECT:					return sym != -1;
		case SYMBOL_VISIBLE:				return sym != -1;
		case SYMBOL_SESSION_DEALS:			return 0;
		case SYMBOL_SESSION_BUY_ORDERS:		return 0;
		case SYMBOL_SESSION_SELL_ORDERS:	return 0;
		case SYMBOL_VOLUME:					return 0;
		case SYMBOL_VOLUMEHIGH:				return 0;
		case SYMBOL_VOLUMELOW:				return 0;
		case SYMBOL_TIME:					return bs.GetTime(1, GetBars()).Get() - Time(1970,1,1).Get();
		case SYMBOL_DIGITS:					return symbols[sym].digits;
		case SYMBOL_SPREAD_FLOAT:			return symbols[sym].spread_floating;
		case SYMBOL_SPREAD:					return symbols[sym].spread;
		case SYMBOL_TRADE_CALC_MODE:		return symbols[sym].calc_mode;
		case SYMBOL_TRADE_MODE:				return symbols[sym].trade_mode;
		case SYMBOL_START_TIME:				return symbols[sym].start_time;
		case SYMBOL_EXPIRATION_TIME:		return symbols[sym].expiration_time;
		case SYMBOL_TRADE_STOPS_LEVEL:		return symbols[sym].stops_level;
		case SYMBOL_TRADE_FREEZE_LEVEL:		return symbols[sym].freeze_level;
		case SYMBOL_TRADE_EXEMODE:			return symbols[sym].execution_mode;
		case SYMBOL_SWAP_MODE:				return symbols[sym].swap_mode;
		case SYMBOL_SWAP_ROLLOVER3DAYS:		return symbols[sym].swap_rollover3days;
		case SYMBOL_EXPIRATION_MODE:		return symbols[sym].execution_mode;
		case SYMBOL_FILLING_MODE:			return 0;
		case SYMBOL_ORDER_MODE:				return 0;
		default: return 0;
	}*/
	
	Panic("TODO"); return 0;
}

String	Brokerage::SymbolInfoString(String name, int prop_id) {
	/*
	int sym = FindSymbol(name);
	if (sym == -1) return "";
	
	switch (prop_id) {
		case SYMBOL_CURRENCY_BASE:			return symbols[sym].currency_base;
		case SYMBOL_CURRENCY_PROFIT:		return symbols[sym].currency_profit;
		case SYMBOL_CURRENCY_MARGIN:		return symbols[sym].currency_margin;
		case SYMBOL_DESCRIPTION:			return symbols[sym].description;
		case SYMBOL_PATH:					return symbols[sym].path;
		default: return "";
	}*/
	
	Panic("TODO"); return 0;

}

int Brokerage::OrderSend(int symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry) {
	return OrderSend(
		symbols[symbol].name,
		cmd,
		volume,
		price,
		slippage,
		stoploss,
		takeprofit,
		magic,
		expiry);
}

}
