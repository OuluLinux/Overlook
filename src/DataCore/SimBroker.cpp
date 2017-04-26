#include "DataCore.h"

namespace DataCore {

SimBroker::SimBroker() {
	min_free_margin_level = 0.65;
	max_free_margin_level = 0.95;
	free_margin_level = 0.90;
	margin_call = 0.5;
	margin_stop = 0.2;
	lotsize = 100000;
	order_counter = 1000000;
}

void SimBroker::Init(MetaTrader& mt) {
	
	symbols <<= mt.GetSymbols();
	currency = mt.GetAccountCurrency();
	
	for(int i = 0; i < symbols.GetCount(); i++)
		symbol_idx.Add(symbols[i].name);
	
	balance = 1000;
	equity = 1000;
	leverage = 1000;
	margin = 0;
	margin_free = equity;
	
	TimeVector& tv = GetTimeVector();
	src = tv.FindLinkSlot("/open");
	ASSERTEXC(src);
	change = tv.FindLinkSlot("/change");
	ASSERTEXC(change);
}

void SimBroker::Cycle() {
	
	
	
}

void SimBroker::ClearWorkingMemory() {
	
	
	
}

int SimBroker::FindSymbol(const String& symbol) const {
	return symbol_idx.Find(symbol);
}

bool SimBroker::IsZeroSignal() const {
	
	
	return false;
}

int SimBroker::GetSignal(int symbol) const {
	
	
	return 0;
}

int SimBroker::GetOpenOrderCount() const {
	return orders.GetCount();
}

double SimBroker::GetWorkingMemoryChange() const {
	
	
	return 0;
}

double SimBroker::GetPreviousCycleChange() const {
	
	
	return 0;
}

double SimBroker::GetFreeMarginLevel() const {
	
	
	return 0;
}

Time SimBroker::GetTime() const {
	
}

void SimBroker::SetSignal(int sym, int signal) {
	
	
	
}

void SimBroker::SetFreeMarginLevel(double d) {
	if (d < min_free_margin_level) d = min_free_margin_level;
	if (d > max_free_margin_level) d = max_free_margin_level;
	free_margin_level = d;
}








double	SimBroker::AccountInfoDouble(int property_id) {
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

int		SimBroker::AccountInfoInteger(int property_id) {
	switch (property_id) {
		case ACCOUNT_LOGIN:					return 123456;
		case ACCOUNT_TRADE_MODE:			return ACCOUNT_TRADE_MODE_DEMO;
		case ACCOUNT_LEVERAGE:				return leverage;
		case ACCOUNT_LIMIT_ORDERS:			return 0; // TODO pending orders
		case ACCOUNT_MARGIN_SO_MODE:		return ACCOUNT_STOPOUT_MODE_PERCENT;
		case ACCOUNT_TRADE_ALLOWED:			return true;
		case ACCOUNT_TRADE_EXPERT:			return true;
	}
}

String	SimBroker::AccountInfoString(int property_id) {
	switch (property_id) {
		case ACCOUNT_NAME:					return "<client name>";
		case ACCOUNT_SERVER:				return "<server name>";
		case ACCOUNT_CURRENCY:				return currency;
		case ACCOUNT_COMPANY:				return "<company name>";
	}
}


double	SimBroker::AccountBalance() {
	return balance;
}

double	SimBroker::AccountCredit() {
	return balance;
}

String	SimBroker::AccountCompany() {
	return AccountInfoString(ACCOUNT_COMPANY);
}

String	SimBroker::AccountCurrency() {
	return AccountInfoString(ACCOUNT_CURRENCY);
}

double	SimBroker::AccountEquity() {
	return AccountInfoDouble(ACCOUNT_EQUITY);
}

double	SimBroker::AccountFreeMargin() {
	return AccountInfoDouble(ACCOUNT_MARGIN_FREE);
}

double	SimBroker::AccountFreeMarginCheck(String symbol, int cmd, double volume) {
	
}

double	SimBroker::AccountFreeMarginMode() {
	return FREEMARGINMODE_WITHOPENORDERS; // all open orders affects the free margin
}

int		SimBroker::AccountLeverage() {
	return AccountInfoInteger(ACCOUNT_LEVERAGE);
}

double	SimBroker::AccountMargin() {
	return AccountInfoDouble(ACCOUNT_MARGIN);
}

String	SimBroker::AccountName() {
	return AccountInfoString(ACCOUNT_NAME);
}

int		SimBroker::AccountNumber() {
	return AccountInfoInteger(ACCOUNT_LOGIN);
}

double	SimBroker::AccountProfit() {
	return AccountInfoDouble(ACCOUNT_PROFIT);
}

String	SimBroker::AccountServer() {
	return AccountInfoString(ACCOUNT_SERVER);
}

int		SimBroker::AccountStopoutLevel() {
	return AccountInfoDouble(ACCOUNT_MARGIN_SO_SO);
}

int		SimBroker::AccountStopoutMode() {
	return AccountInfoInteger(ACCOUNT_MARGIN_SO_MODE);
}

double	SimBroker::MarketInfo(String symbol, int type) {
	TimeVector& tv = GetTimeVector();
	
	SlotProcessAttributes& attr = tv.GetCurrent();
	
	int sym = FindSymbol(symbol);
	
	switch (type) {
		case MODE_LOW:			return *src->GetValue<double>(
									1, // LOW
									sym,
									tv.FindPeriod(tv.GetTfFromSeconds(24*60*60)),
									0, attr);
		case MODE_HIGH:			return *src->GetValue<double>(
									2, // HIGH
									sym,
									tv.FindPeriod(tv.GetTfFromSeconds(24*60*60)),
									0, attr);
		case MODE_TIME:			return tv.GetTime(1, attr.GetCounted()).Get() - Time(1970,1,1).Get();
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
	}
}

int		SimBroker::SymbolsTotal(int selected) {
	ASSERT(!selected); // symbols in general list is allowed only
	return symbols.GetCount();
}

String	SimBroker::SymbolName(int pos, int selected) {
	ASSERT(!selected); // symbols in general list is allowed only
	return symbols[pos].name;
}

int		SimBroker::SymbolSelect(String name, int select) {
	return false; // can't implement
}

double	SimBroker::SymbolInfoDouble(String name, int prop_id) {
	TimeVector& tv = GetTimeVector();
	
	SlotProcessAttributes& attr = tv.GetCurrent();
	
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
	}
}

int		SimBroker::SymbolInfoInteger(String name, int prop_id) {
	TimeVector& tv = GetTimeVector();
	
	SlotProcessAttributes& attr = tv.GetCurrent();
	
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
		case SYMBOL_TIME:					return tv.GetTime(1, attr.GetCounted()).Get() - Time(1970,1,1).Get();
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
	}
}

String	SimBroker::SymbolInfoString(String name, int prop_id) {
	
	int sym = FindSymbol(name);
	if (sym == -1) return "";
	
	switch (prop_id) {
		case SYMBOL_CURRENCY_BASE:			return symbols[sym].currency_base;
		case SYMBOL_CURRENCY_PROFIT:		return symbols[sym].currency_profit;
		case SYMBOL_CURRENCY_MARGIN:		return symbols[sym].currency_margin;
		case SYMBOL_DESCRIPTION:			return symbols[sym].description;
		case SYMBOL_PATH:					return symbols[sym].path;
		default: return "";
	}
}

int		SimBroker::RefreshRates() {
	return false;
}

int		SimBroker::iBars(String symbol, int timeframe) {
	TimeVector& tv = GetTimeVector();
	int period = timeframe*60 / tv.GetBasePeriod();
	return tv.GetCount(period);
}

int		SimBroker::iBarShift(String symbol, int timeframe, int datetime) {
	Panic("TODO"); return 0;
}

double	SimBroker::iClose(String symbol, int timeframe, int shift) {
	TimeVector& tv = GetTimeVector();
	int sym = FindSymbol(symbol);
	SlotProcessAttributes& attr = tv.GetCurrent();
	return *src->GetValue<double>(
		0,
		sym,
		tv.FindPeriod(tv.GetTfFromSeconds(timeframe*60)),
		-1+shift, attr);
}

double	SimBroker::iHigh(String symbol, int timeframe, int shift) {
	TimeVector& tv = GetTimeVector();
	int sym = FindSymbol(symbol);
	SlotProcessAttributes& attr = tv.GetCurrent();
	return *src->GetValue<double>(
		2,
		sym,
		tv.FindPeriod(tv.GetTfFromSeconds(timeframe*60)),
		shift, attr);
}

double	SimBroker::iLow(String symbol, int timeframe, int shift) {
	TimeVector& tv = GetTimeVector();
	int sym = FindSymbol(symbol);
	SlotProcessAttributes& attr = tv.GetCurrent();
	return *src->GetValue<double>(
		1,
		sym,
		tv.FindPeriod(tv.GetTfFromSeconds(timeframe*60)),
		shift, attr);
}

double	SimBroker::iOpen(String symbol, int timeframe, int shift) {
	TimeVector& tv = GetTimeVector();
	int sym = FindSymbol(symbol);
	SlotProcessAttributes& attr = tv.GetCurrent();
	return *src->GetValue<double>(
		0,
		sym,
		tv.FindPeriod(tv.GetTfFromSeconds(timeframe*60)),
		shift, attr);
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
}

int		SimBroker::OrderExpiration() {
	return orders[selected].expiration.Get() - Time(1970,1,1).Get();
}

double	SimBroker::OrderLots() {
	return orders[selected].volume;
}

int		SimBroker::OrderMagicNumber() {
	Panic("Magic numbers aren't allowed to use because they are used internally in databridge.");
}

int		SimBroker::OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) {
	Panic("TODO");
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

const Vector<Symbol>&	SimBroker::GetSymbols() {
	return symbols;
}

const Vector<Price>&	SimBroker::GetAskBid() {
	
	
	
	//return askbid;
}

const Vector<PriceTf>&	SimBroker::GetTickData() {
	
	
	
	// return pricetf;
}

void SimBroker::GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history) {
	
}


#if 0
void SimBroker::RefreshData() {
	rows.Clear();
	
	
	lock.Enter();
	
	String base = GetAccountCurrency();
	double leverage = GetAccountLeverage();
	double base_volume = GetAccountEquity() * leverage;
	
	VectorMap<String, double> cur_volumes;
	VectorMap<int, double> idx_volumes;
	
	int count = GetOpenOrderCount();
	int sym_count = GetSymbolCount();
	
	for(int i = 0; i < count; i++) {
		const Order& bo = GetOpenOrder(i);
		
		if (bo.sym < 0 || bo.sym >= GetSymbolCount())
			continue;
		
		Symbol sym = GetSymbol(bo.sym);
		
		if (sym.margin_calc_mode == Symbol::CALCMODE_FOREX) {
			ASSERT(sym.name.GetCount() == 6); // TODO: allow EUR/USD and EURUSDm style names
			String a = sym.name.Left(3);
			String b = sym.name.Right(3);
			
			if (a == base) {
				// For example: base=USD, sym=USDCHF, cmd=buy, lots=0.01, a=USD, b=CHF
				int j = cur_volumes.Find(b);
				double& cur_volume = j == -1 ? cur_volumes.Add(b, 0) : cur_volumes[j];
				if (bo.type == TYPE_BUY)
					cur_volume += -1 * bo.volume * sym.lotsize * GetAsk(bo.sym);
				else if (bo.type == TYPE_SELL)
					cur_volume += bo.volume * sym.lotsize * GetBid(bo.sym);
			}
			else if (b == base) {
				// For example: base=USD, sym=AUDUSD, cmd=buy, lots=0.01, a=AUD, b=USD
				int j = cur_volumes.Find(a);
				double& cur_volume = j == -1 ? cur_volumes.Add(a, 0) : cur_volumes[j];
				if (bo.type == TYPE_BUY)
					cur_volume += bo.volume * sym.lotsize;
				else if (bo.type == TYPE_SELL)
					cur_volume += -1 * bo.volume * sym.lotsize;
			}
			else {
				// Find proxy symbol
				//for (int side = 0; side < 2 && !found; side++)
				for(int k = 0; k < sym_count; k++) {
					String s = GetSymbol(k).name;
					if ( (s.Left(3)  == base && (s.Right(3) == a || s.Right(3) == b)) ||
						 (s.Right(3) == base && (s.Left(3)  == a || s.Left(3)  == b)) ) {
						// For example: base=USD, sym=CHFJPY, cmd=buy, lots=0.01, s=USDCHF, a=CHF, b=JPY
						//  - CHF += 0.01 * 1000
						{
							int j = cur_volumes.Find(a);
							double& cur_volume = j == -1 ? cur_volumes.Add(a, 0) : cur_volumes[j];
							if (bo.type == TYPE_BUY)
								cur_volume += bo.volume * sym.lotsize;
							else if (bo.type == TYPE_SELL)
								cur_volume += -1 * bo.volume * sym.lotsize;
						}
						//  - JPY += -1 * 0.01 * 1000 * open-price
						{
							int j = cur_volumes.Find(b);
							double& cur_volume = j == -1 ? cur_volumes.Add(b, 0) : cur_volumes[j];
							if (bo.type == TYPE_BUY)
								cur_volume += -1 * bo.volume * sym.lotsize * bo.open;
							else if (bo.type == TYPE_SELL)
								cur_volume += bo.volume * sym.lotsize * bo.open;
						}
						break;
					}
				}
				
				
				
			}
		}
		else {
			int j = idx_volumes.Find(bo.sym);
			double& idx_volume = j == -1 ? idx_volumes.Add(bo.sym, 0) : idx_volumes[j];
			double value = bo.volume * sym.contract_size;
			if (bo.type == TYPE_SELL) value *= -1;
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
			String s = GetSymbol(j).name;
			
			if (s.Left(3) == cur && s.Right(3) == base) {
				rate = GetBid(j);
				cur_rates.Add(cur, rate);
				found = true;
				break;
			}
			else if (s.Right(3) == cur && s.Left(3) == base) {
				rate = 1 / GetAsk(j);
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
		Symbol bs = GetSymbol(id);
		String cur = bs.currency_base;
		
		double rate;
		double volume = idx_volumes[i];
		if (cur == base) {
			if (volume < 0)		rate = GetAsk(id);
			else				rate = GetBid(id);
			idx_rates.Add(id, rate);
			double base_value = volume * rate * fabs(volume / bs.contract_size);
			idx_base_values.Add(id, base_value);
		} else {
			bool found = false;
			for(int j = 0; j < sym_count; j++) {
				String s = GetSymbol(j).name;
				
				if (s.Left(3) == cur && s.Right(3) == base) {
					rate = GetBid(j);
					if (volume < 0)		rate *= GetAsk(id);
					else				rate *= GetBid(id);
					idx_rates.Add(id, rate);
					found = true;
					break;
				}
				else if (s.Right(3) == cur && s.Left(3) == base) {
					rate = 1 / GetAsk(j);
					if (volume < 0)		rate *= GetAsk(id);
					else				rate *= GetBid(id);
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
	for(int i = 0; i < cur_volumes.GetCount(); i++) {
		int j;
		const String& cur = cur_volumes.GetKey(i);
		double rate = 0;
		double base_value = 0;
		j = cur_rates.Find(cur);
		if (j != -1) rate = cur_rates[j];
		j = cur_base_values.Find(cur);
		if (j != -1) base_value = cur_base_values[j];
		
		Row& row = rows.Add();
		row.asset = cur_volumes.GetKey(i);
		row.volume = cur_volumes[i];
		row.rate = rate;
		row.base_value = base_value;
		
		if (base_value > 0)
			base_volume -= base_value * leverage * 2; // Freaky bit, but gives correct result. Checked many, many times.
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
		Symbol bs = GetSymbol(idx_volumes.GetKey(i));
		
		Row& row = rows.Add();
		row.asset = bs.name;
		row.volume = idx_volumes[i];
		row.rate = rate;
		row.base_value = base_value;
		
		if (bs.currency_base != base) {
			if (base_value > 0)
				base_volume -= base_value * leverage;
			else
				base_volume += base_value * leverage;
		}
	}
	
	// Base currency
	Row& row = rows.Add();
	row.asset = base;
	row.volume = base_volume;
	row.rate = 1.00;
	row.base_value = base_volume / leverage;
	
		
	lock.Leave();
}
#endif

}
