#include "libmt.h"

inline Time TimeFromTimestamp(int64 seconds) {
	return Time(1970, 1, 1) + seconds;
}

inline dword TimestampNow() {
	return GetSysTime().Get() - Time(1970,1,1).Get();
}

#define NEVER          ASSERT(0)


String Order::GetTypeString() const {
	switch (type) {
		case OP_BUY: return "Buy";
		case OP_SELL: return "Sell";
		case OP_BUYLIMIT: return "Buy Limit";
		case OP_SELLLIMIT: return "Sell Limit";
		case OP_BUYSTOP: return "Buy Stop";
		case OP_SELLSTOP: return "Sell Stop";
		default: return "Unknown";
	}
}







Symbol& Symbol::operator = (const Symbol& sym) {
	id = sym.id;
	name = sym.name;
	proxy_name = sym.proxy_name;
	
	tradeallowed	= sym.tradeallowed;
	is_skipping		= sym.is_skipping;
	
	lotsize = sym.lotsize;
	profit_calc_mode = sym.profit_calc_mode;
	margin_calc_mode = sym.margin_calc_mode;
	margin_hedged = sym.margin_hedged;
	margin_required = sym.margin_required;
	
	selected = sym.selected;
	visible = sym.visible;
	digits = sym.digits;
	spread_floating = sym.spread_floating;
	spread = sym.spread;
	calc_mode = sym.calc_mode;
	trade_mode = sym.trade_mode;
	start_time = sym.start_time;
	expiration_time = sym.expiration_time;
	stops_level = sym.stops_level;
	freeze_level = sym.freeze_level;
	execution_mode = sym.execution_mode;
	swap_mode = sym.swap_mode;
	swap_rollover3days = sym.swap_rollover3days;
	
	point = sym.point;
	tick_value = sym.tick_value;
	tick_size = sym.tick_size;
	contract_size = sym.contract_size;
	volume_min = sym.volume_min;
	volume_max = sym.volume_max;
	volume_step = sym.volume_step;
	swap_long = sym.swap_long;
	swap_short = sym.swap_short;
	margin_initial = sym.margin_initial;
	margin_maintenance = sym.margin_maintenance;
	
	currency_base = sym.currency_base;
	currency_profit = sym.currency_profit;
	currency_margin = sym.currency_margin;
	description = sym.description;
	path = sym.path;
	
	margin_factor = sym.margin_factor;
	
	proxy_id = sym.proxy_id;
	proxy_factor = sym.proxy_factor;
	
	virtual_type = sym.virtual_type;
	
	for(int i = 0; i < 7; i++) {
		quotes_begin_hours[i]		= sym.quotes_begin_hours[i];
		quotes_begin_minutes[i]		= sym.quotes_begin_minutes[i];
		quotes_end_hours[i]			= sym.quotes_end_hours[i];
		quotes_end_minutes[i]		= sym.quotes_end_minutes[i];
		trades_begin_hours[i]		= sym.trades_begin_hours[i];
		trades_begin_minutes[i]		= sym.trades_begin_minutes[i];
		trades_end_hours[i]			= sym.trades_end_hours[i];
		trades_end_minutes[i]		= sym.trades_end_minutes[i];
	}
	
	return *this;
}

String Symbol::GetCalculationMode(int mode) {
	switch (mode) {
		case CALCMODE_FOREX:		return "Forex";
		case CALCMODE_CDF:			return "CDF";
		case CALCMODE_FUTURES:		return "Futures";
		case CALCMODE_CDF_INDICES:	return "CDF Indicex";
		default:					return "";
	}
}

String Symbol::GetTradeMode(int mode) {
	switch (mode) {
		case SYMBOL_TRADE_MODE_DISABLED:		return "Disabled";
		case SYMBOL_TRADE_MODE_LONGONLY:		return "Long only";
		case SYMBOL_TRADE_MODE_SHORTONLY:		return "Short only";
		case SYMBOL_TRADE_MODE_CLOSEONLY:		return "Close only";
		case SYMBOL_TRADE_MODE_FULL:			return "Full access";
		default:								return "";
	}
}

String Symbol::GetExecutionMode(int mode) {
	switch (mode) {
		case SYMBOL_TRADE_EXECUTION_REQUEST:		return "Request";
		case SYMBOL_TRADE_EXECUTION_INSTANT:		return "Instant";
		case SYMBOL_TRADE_EXECUTION_MARKET:			return "Market";
		case SYMBOL_TRADE_EXECUTION_EXCHANGE:		return "Exchange";
		default:									return "";
	}
}

String Symbol::GetSwapMode(int mode) {
	switch (mode) {
		case SWAPTYPE_INPOINTS:					return "In points";
		case SWAPTYPE_INSYMBOLBASECURRENCY:		return "In symbol's base currency";
		case SWAPTYPE_BYINTEREST:				return "By interest";
		case SWAPTYPE_INMARGINCURRENCY:			return "In margin's currency";
		default:								return "";
	}
}
	
String Symbol::GetDayOfWeek(int wday) {
	switch (wday) {
		case SUNDAY:		return "Sunday";
		case MONDAY:		return "Monday";
		case TUESDAY:		return "Tuesday";
		case WEDNESDAY:		return "Wednesday";
		case THURSDAY:		return "Thursday";
		case FRIDAY:		return "Friday";
		case SATURDAY:		return "Saturday";
		default: return "";
	}
}
	
String Symbol::GetQuotesRange(int wday) const {
	return Format("%02d:%02d-%02d:%02d", 
		(int)quotes_begin_hours[wday],	(int)quotes_begin_minutes[wday],
		(int)quotes_end_hours[wday],	(int)quotes_end_minutes[wday]);
}

String Symbol::GetTradesRange(int wday) const {
	return Format("%02d:%02d-%02d:%02d", 
		(int)trades_begin_hours[wday],	(int)trades_begin_minutes[wday],
		(int)trades_end_hours[wday],	(int)trades_end_minutes[wday]);
}

bool Symbol::IsOpen(Time t) const {
	if (is_skipping) return false;
	int wday = DayOfWeek(t);
	Time begin(t.year, t.month, t.day, trades_begin_hours[wday], trades_begin_minutes[wday]);
	Time end(t.year, t.month, t.day, trades_end_hours[wday], trades_end_minutes[wday]);
	return t >= begin && t < end;
}















MetaTrader::MetaTrader() {
	init_success = false;
	port = 42000;
	
	tf_h1_id = 4;
	
	periodstr.Add(1, "M1");
	periodstr.Add(5, "M5");
	periodstr.Add(15, "M15");
	periodstr.Add(30, "M30");
	
	periodstr.Add(60, "H1");
	periodstr.Add(240, "H4");
	periodstr.Add(1440, "D");
	periodstr.Add(10080, "W");
	
	//periodstr.Add(43200, "M"); // Not actually fixed period, can't use this. Days in month: (28 / 30 / 31)
	
	
	// These are by default the most expensive currencies
	skipped_currencies.Add("RUR");
	skipped_currencies.Add("RUB");
	skipped_currencies.Add("CHF");
	skipped_currencies.Add("XAU");
	skipped_currencies.Add("XAG");
}

MetaTrader::~MetaTrader() {
	
}

int MetaTrader::Init(String addr, int port) {
	mainaddr = addr;
	this->port = port;
	try {
		if (mainaddr.IsEmpty() || !IsResponding()) {
			return 1;
		}
		
		// Refresh symbols
		init_success = true; // GetSymbols requires this temporarily
		GetSymbols();
		init_success = !symbols.IsEmpty();
	}
	catch (ConnectionError e) {
		init_success = false;
	}
	return !init_success;
}

#define MTFUNC0(code, rtype, name, gret) rtype MetaTrader::name() {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(0); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}


#define MTFUNC1(code, rtype, name, a1type, a1set, gret) rtype MetaTrader::name(a1type a1) {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(1); \
	p.a1set(0, a1); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}

#define MTFUNC2(code, rtype, name, a1type, a1set, a2type, a2set, gret) rtype MetaTrader::name(a1type a1, a2type a2) {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(2); \
	p.a1set(0, a1); \
	p.a2set(1, a2); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}

#define MTFUNC3(code, rtype, name, a1type, a1set, a2type, a2set, a3type, a3set, gret) rtype MetaTrader::name(a1type a1, a2type a2, a3type a3) {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(3); \
	p.a1set(0, a1); \
	p.a2set(1, a2); \
	p.a3set(2, a3); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}

#define MTFUNC4(code, rtype, name, a1type, a1set, a2type, a2set, a3type, a3set, a4type, a4set, gret) rtype MetaTrader::name(a1type a1, a2type a2, a3type a3, a4type a4) {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(4); \
	p.a1set(0, a1); \
	p.a2set(1, a2); \
	p.a3set(2, a3); \
	p.a4set(3, a4); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}

#define MTFUNC5(code, rtype, name, a1type, a1set, a2type, a2set, a3type, a3set, a4type, a4set, a5type, a5set, gret) rtype MetaTrader::name(a1type a1, a2type a2, a3type a3, a4type a4, a5type a5) {\
	lock.Enter(); \
	if (!init_success) {lock.Leave(); throw ConnectionError();} \
	MTPacket p; \
	p.SetCode(code); \
	p.SetValuesCount(5); \
	p.a1set(0, a1); \
	p.a2set(1, a2); \
	p.a3set(2, a3); \
	p.a4set(3, a4); \
	p.a5set(4, a5); \
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();} \
	if(p.Import()) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();} \
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();} \
	lock.Leave(); \
	return p.gret(0); \
}

	
MTFUNC1(0,	double,		AccountInfoDouble,		int,	SetInt,		GetDbl);
MTFUNC1(1,	int,		AccountInfoInteger,		int,	SetInt,		GetInt);
MTFUNC1(2,	String,		AccountInfoString,		int,	SetInt,		GetStr);
MTFUNC0(3,	double,		AccountBalance,			GetDbl);
MTFUNC0(4,	double,		AccountCredit,			GetDbl);
MTFUNC0(5,	String,		AccountCompany,			GetStr);
MTFUNC0(6,	String,		AccountCurrency,		GetStr);
MTFUNC0(7,	double,		AccountEquity,			GetDbl);
MTFUNC0(8,	double,		AccountFreeMargin,		GetDbl);
MTFUNC3(9,	double,		AccountFreeMarginCheck, String, SetStr,	int, SetInt, double, SetDbl, GetDbl);
MTFUNC0(10,	double,		AccountFreeMarginMode,	GetDbl);
MTFUNC0(11,	int,		AccountLeverage,		GetInt);
MTFUNC0(12,	double,		AccountMargin,			GetDbl);
MTFUNC0(13,	String,		AccountName,			GetStr);
MTFUNC0(14,	int,		AccountNumber,			GetInt);
MTFUNC0(15,	double,		AccountProfit,			GetDbl);
MTFUNC0(16,	String,		AccountServer,			GetStr);
MTFUNC0(17,	int,		AccountStopoutLevel,	GetInt);
MTFUNC0(18,	int,		AccountStopoutMode,		GetInt);
MTFUNC2(19,	double,		MarketInfo,				String, SetStr,	int, SetInt, GetDbl);
MTFUNC1(20,	int,		SymbolsTotal,			int, SetInt, GetInt);
MTFUNC2(21,	String,		SymbolName,				int, SetInt, int, SetInt, GetStr);
MTFUNC2(22,	int,		SymbolSelect,			String, SetStr,	int, SetInt, GetInt);
MTFUNC2(23,	double,		SymbolInfoDouble,		String, SetStr,	int, SetInt, GetDbl);
MTFUNC2(24,	int,		SymbolInfoInteger,		String, SetStr,	int, SetInt, GetInt);
MTFUNC2(25,	String,		SymbolInfoString,		String, SetStr,	int, SetInt, GetStr);
MTFUNC0(26,	int,		RefreshRates,			GetInt);
MTFUNC2(27,	int,		iBars,					String, SetStr,	int, SetInt, GetInt);
MTFUNC3(28,	int,		iBarShift,				String, SetStr,	int, SetInt, int, SetInt, GetInt);
MTFUNC3(29,	double,		iClose,					String, SetStr,	int, SetInt, int, SetInt, GetDbl);
MTFUNC3(30,	double,		iHigh,					String, SetStr,	int, SetInt, int, SetInt, GetDbl);
MTFUNC3(31,	double,		iLow,					String, SetStr,	int, SetInt, int, SetInt, GetDbl);
MTFUNC3(32,	double,		iOpen,					String, SetStr,	int, SetInt, int, SetInt, GetDbl);
MTFUNC5(33,	int,		iHighest,				String, SetStr,	int, SetInt, int, SetInt, int, SetInt, int, SetInt, GetInt);
MTFUNC5(34,	int,		iLowest,				String, SetStr,	int, SetInt, int, SetInt, int, SetInt, int, SetInt, GetInt);
MTFUNC3(35,	int,		iTime,					String, SetStr,	int, SetInt, int, SetInt, GetInt);
MTFUNC3(36,	int,		iVolume,				String, SetStr,	int, SetInt, int, SetInt, GetInt);
MTFUNC4(37,	int,		OrderClose,				int, SetInt, double, SetDbl, double, SetDbl, int, SetInt, GetInt);
MTFUNC0(38,	double,		OrderClosePrice,		GetDbl);
MTFUNC0(39,	int,		OrderCloseTime,			GetInt);
MTFUNC0(40,	String,		OrderComment,			GetStr);
MTFUNC0(41,	double,		OrderCommission,		GetDbl);
MTFUNC1(42,	int,		OrderDelete,			int, SetInt, GetInt);
MTFUNC0(43,	int,		OrderExpiration,		GetInt);
MTFUNC0(44,	double,		OrderLots,				GetDbl);
MTFUNC0(45,	int,		OrderMagicNumber,		GetInt);
MTFUNC5(46,	int,		OrderModify,			int, SetInt, double, SetDbl, double, SetDbl, double, SetDbl, int, SetInt, GetInt);
MTFUNC0(47,	double,		OrderOpenPrice,			GetDbl);
MTFUNC0(48,	int,		OrderOpenTime,			GetInt);
MTFUNC0(49,	double,		OrderProfit,			GetDbl);
MTFUNC3(50,	int,		OrderSelect,			int, SetInt, int, SetInt, int, SetInt, GetInt);
MTFUNC0(52,	int,		OrdersHistoryTotal,		GetInt);
MTFUNC0(53,	double,		OrderStopLoss,			GetDbl);
MTFUNC0(54,	int,		OrdersTotal,			GetInt);
MTFUNC0(55,	double,		OrderSwap,				GetDbl);
MTFUNC0(56,	String,		OrderSymbol,			GetStr);
MTFUNC0(57,	double,		OrderTakeProfit,		GetDbl);
MTFUNC0(58,	int,		OrderTicket,			GetInt);
MTFUNC0(59,	int,		OrderType,				GetInt);
MTFUNC0(60,	String,		GetSymbolsRaw,			GetStr);
MTFUNC0(61,	String,		GetAskBidRaw,			GetStr);
MTFUNC0(62,	String,		GetPricesRaw,			GetStr);
MTFUNC1(63,	String,		GetHistoryOrdersRaw,	int,  SetInt,	GetStr);
MTFUNC1(64,	String,		GetOrdersRaw,			int,  SetInt,	GetStr);
MTFUNC0(66,	String,		GetLastError,			GetStr);
MTFUNC0(68,	bool,		IsDemo,					GetInt);
MTFUNC0(69,	bool,		IsConnected,			GetInt);
MTFUNC3(70,	int,		FindPriceTime,			int, SetInt, int, SetInt, dword, SetInt, GetInt);



int MetaTrader::OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry) {
	lock.Enter();
	if (!init_success) {lock.Leave(); throw ConnectionError();}
	MTPacket p;
	int code;
	code = 51;
	p.SetCode(code);
	p.SetValuesCount(9);
	p.SetStr(0, symbol);
	p.SetInt(1, cmd);
	p.SetDbl(2, volume);
	p.SetDbl(3, price);
	p.SetInt(4, slippage);
	p.SetDbl(5, stoploss);
	p.SetDbl(6, takeprofit);
	p.SetInt(7, magic);
	p.SetInt(8, expiry);
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();}
	if(p.Import()) {lock.Leave(); throw ConnectionError();}
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();}
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();}
	lock.Leave();
	return p.GetInt(0);
}




Vector<Vector<Time> > MetaTrader::GetLatestPriceTimes() {
	lock.Enter();
	if (!init_success) {lock.Leave(); throw ConnectionError();}
	MTPacket p;
	int code;
	code = 67;
	p.SetCode(code);
	p.SetValuesCount(0);
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();}
	if(p.Import()) {lock.Leave(); throw ConnectionError();}
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();}
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();}
	lock.Leave();
	String content = p.GetStr(0);
	Vector<String> data = Split(content, ";");
	Vector<Vector<Time> > out;
	for(int i = 0; i < data.GetCount(); i++) {
		String& line = data[i];
		Vector<String> line_data = Split(line, ",");
		Vector<Time>& out_line = out.Add();
		for(int j = 0; j < line_data.GetCount(); j++) {
			dword time = StrInt(line_data[j]);
			ASSERT(time != 0);
			Time t = TimeFromTimestamp(time);
			ASSERT(t.IsValid());
			out_line.Add(t);
		}
	}
	return out;
}
	
Vector<Vector<Time> > MetaTrader::GetEarliestPriceTimes() {
	lock.Enter();
	if (!init_success) {lock.Leave(); throw ConnectionError();}
	MTPacket p;
	int code;
	code = 71;
	p.SetCode(code);
	p.SetValuesCount(0);
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();}
	if(p.Import()) {lock.Leave(); throw ConnectionError();}
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();}
	if(p.GetCode() != code) {lock.Leave(); throw ConnectionError();}
	lock.Leave();
	String content = p.GetStr(0);
	Vector<String> data = Split(content, ";");
	Vector<Vector<Time> > out;
	for(int i = 0; i < data.GetCount(); i++) {
		String& line = data[i];
		Vector<String> line_data = Split(line, ",");
		Vector<Time>& out_line = out.Add();
		for(int j = 0; j < line_data.GetCount(); j++) {
			dword time = StrInt(line_data[j]);
			ASSERT(time != 0);
			Time t = TimeFromTimestamp(time);
			ASSERT(t.IsValid());
			out_line.Add(t);
		}
	}
	return out;
}


bool MetaTrader::IsResponding() {
	lock.Enter();
	MTPacket p;
	p.SetCode(100);
	p.SetValuesCount(0);
	if(p.Export(*this, mainaddr, port)) {lock.Leave(); throw ConnectionError();}
	if(p.Import()) {lock.Leave(); throw ConnectionError();}
	if(p.GetCount() == 0) {lock.Leave(); throw ConnectionError();}
	if(p.GetCode() != 100) {lock.Leave(); throw ConnectionError();}
	lock.Leave();
	return p.GetInt(0) == 123456;
}

String GetProxy(const String& currency) {
	if      (currency == "AUD")
		return "AUDUSD";
	else if (currency == "CAD")
		return "USDCAD";
	else if (currency == "CHF")
		return "USDCHF";
	else if (currency == "EUR")
		return "EURUSD";
	else if (currency == "GBP")
		return "GBPUSD";
	else if (currency == "NZD")
		return "NZDUSD";
	else if (currency == "RUR")
		return "USDRUR";
	else
		return "USD" + currency;
}

const Vector<Symbol>& MetaTrader::GetSymbols() {
	symbols.SetCount(0);
	
	// Get symbols from Broker
	String s;
	while (1) {
		try {
			s = GetSymbolsRaw();
			if (s.GetCount()) break;
		}
		catch (ConnectionError e) {
			continue;
		}
	}
	
	Vector<String> lines = Split(s, ";");
	
	String account_currency = GetAccountCurrency();
	if (!account_currency.GetCount())
		account_currency = "USD";
	
	VectorMap<String, int> currencies;
	VectorMap<int,int> postfix_counts;
	
	// Parse symbol lines
	int c1 = lines.GetCount();
	for(int i = 0; i < c1; i++) {
		lines[i].Replace("\r","");
		Vector<String> line = Split(lines[i], ",", false);
		Symbol sym;
		
		int j = 0;
		#define GET_DBL(x) x = StrDbl(line[j]); j++;
		#define GET_INT(x) x = StrInt(line[j]); j++;
		#define GET_STR(x) x = line[j]; j++;
	
		sym.id = i;
		
		GET_STR(sym.name);
		
		GET_DBL(sym.lotsize);
		GET_INT(sym.tradeallowed);			//MarketInfo(symbols[i], MODE_TRADEALLOWED)
		GET_DBL(sym.profit_calc_mode);		//MarketInfo(symbols[i], MODE_PROFITCALCMODE)
		GET_DBL(sym.margin_calc_mode);		//MarketInfo(symbols[i], MODE_MARGINCALCMODE)
		GET_DBL(sym.margin_hedged);			//MarketInfo(symbols[i], MODE_MARGINHEDGED)
		GET_DBL(sym.margin_required);		//MarketInfo(symbols[i], MODE_MARGINREQUIRED)
		
		GET_INT(sym.selected);				//SymbolInfoInteger(symbols[i], SYMBOL_SELECT)
		GET_INT(sym.visible);				//SymbolInfoInteger(symbols[i], SYMBOL_VISIBLE)
		GET_INT(sym.digits);				//SymbolInfoInteger(symbols[i], SYMBOL_DIGITS)
		GET_INT(sym.spread_floating);		//SymbolInfoInteger(symbols[i], SYMBOL_SPREAD_FLOAT)
		GET_INT(sym.spread);				//SymbolInfoInteger(symbols[i], SYMBOL_SPREAD)
		GET_INT(sym.calc_mode);				//SymbolInfoInteger(symbols[i], SYMBOL_TRADE_CALC_MODE)
		GET_INT(sym.trade_mode);			//SymbolInfoInteger(symbols[i], SYMBOL_TRADE_MODE)
		GET_INT(sym.start_time);			//SymbolInfoInteger(symbols[i], SYMBOL_START_TIME)
		GET_INT(sym.expiration_time);		//SymbolInfoInteger(symbols[i], SYMBOL_EXPIRATION_TIME)
		GET_INT(sym.stops_level);			//SymbolInfoInteger(symbols[i], SYMBOL_TRADE_STOPS_LEVEL)
		GET_INT(sym.freeze_level);			//SymbolInfoInteger(symbols[i], SYMBOL_TRADE_FREEZE_LEVEL)
		GET_INT(sym.execution_mode);		//SymbolInfoInteger(symbols[i], SYMBOL_TRADE_EXEMODE)
		GET_INT(sym.swap_mode);				//SymbolInfoInteger(symbols[i], SYMBOL_SWAP_MODE)
		GET_INT(sym.swap_rollover3days);	//SymbolInfoInteger(symbols[i], SYMBOL_SWAP_ROLLOVER3DAYS)

		GET_DBL(sym.point);					//SymbolInfoDouble(symbols[i], SYMBOL_POINT)
		GET_DBL(sym.tick_value);			//SymbolInfoDouble(symbols[i], SYMBOL_TRADE_TICK_VALUE)
		GET_DBL(sym.tick_size);				//SymbolInfoDouble(symbols[i], SYMBOL_TRADE_TICK_SIZE)
		GET_DBL(sym.contract_size);			//SymbolInfoDouble(symbols[i], SYMBOL_TRADE_CONTRACT_SIZE)
		GET_DBL(sym.volume_min);			//SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_MIN)
		GET_DBL(sym.volume_max);			//SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_MAX)
		GET_DBL(sym.volume_step);			//SymbolInfoDouble(symbols[i], SYMBOL_VOLUME_STEP)
		GET_DBL(sym.swap_long);				//SymbolInfoDouble(symbols[i], SYMBOL_SWAP_LONG)
		GET_DBL(sym.swap_short);			//SymbolInfoDouble(symbols[i], SYMBOL_SWAP_SHORT)
		GET_DBL(sym.margin_initial);		//SymbolInfoDouble(symbols[i], SYMBOL_MARGIN_INITIAL)
		GET_DBL(sym.margin_maintenance);	//SymbolInfoDouble(symbols[i], SYMBOL_MARGIN_MAINTENANCE)
		
		GET_STR(sym.currency_base);			//SymbolInfoString(symbols[i], SYMBOL_CURRENCY_BASE)
		GET_STR(sym.currency_profit);		//SymbolInfoString(symbols[i], SYMBOL_CURRENCY_PROFIT)
		GET_STR(sym.currency_margin);		//SymbolInfoString(symbols[i], SYMBOL_CURRENCY_MARGIN)
		GET_STR(sym.path);					//SymbolInfoString(symbols[i], SYMBOL_PATH) + ";" ;
		GET_STR(sym.description);			//SymbolInfoString(symbols[i], SYMBOL_DESCRIPTION)
		
		sym.virtual_type = 0;
		
		Vector<Time> quotes_begins, quotes_ends;
		Vector<Time> trades_begins, trades_ends;
		for(int k = 0; k < 7; k++) {
			dword from, to;
			GET_INT(from);
			GET_INT(to);
			sym.quotes_begin_hours[k] = from / (60*60);
			sym.quotes_begin_minutes[k] = (from % (60*60)) / 60;
			sym.quotes_end_hours[k] = to / (60*60);
			sym.quotes_end_minutes[k] = (to % (60*60)) / 60;
		}
		for(int k = 0; k < 7; k++) {
			dword from, to;
			GET_INT(from);
			GET_INT(to);
			sym.trades_begin_hours[k] = from / (60*60);
			sym.trades_begin_minutes[k] = (from % (60*60)) / 60;
			sym.trades_end_hours[k] = to / (60*60);
			sym.trades_end_minutes[k] = (to % (60*60)) / 60;
		}
		
		ASSERT(j == line.GetCount());
		sym.is_skipping = false;
		
		// Get proxy symbols
		if (sym.IsForex()) {
			const String& name = sym.name;
			
			// TODO: handle symbols like EURUSDm, EUR/USD, etc.
			ASSERT_(	name.GetCount() == 6 ||
						(name.GetCount() == 7 && name.Right(1) == "."),
						"Only 'EURUSD', 'AUDJPYm', 'EURCHF.' format is allowed, not 'EUR/USD'. Hack this or change broker.");
			
			// Count postfixes
			if (name.GetCount() == 6)
				postfix_counts.GetAdd(0, 0)++;
			else if (name.GetCount() == 7)
				postfix_counts.GetAdd(name.Right(1)[0], 0)++;
			
			
			
			String a = name.Left(3);
			String b = name.Mid(3,3);
			if (a == account_currency || b == account_currency) {
				// No need for proxy
			} else {
				if (account_currency == "USD") {
					if (a != account_currency)
						sym.proxy_name = GetProxy(a);
				}
				else Panic("TODO: add proxies for this currency");
			}
			if (skipped_currencies.Find(a) != -1)
				sym.is_skipping = true;
			if (skipped_currencies.Find(b) != -1)
				sym.is_skipping = true;
		}
		else if (sym.IsCDF() || sym.IsCDFIndex() || sym.IsFuture()) {
			const String& a = sym.currency_margin;
			if (account_currency == "USD") {
				if (a != account_currency)
					sym.proxy_name = GetProxy(a);
			}
			else Panic("TODO: add proxies for this currency");
			if (skipped_currencies.Find(a) != -1)
				sym.is_skipping = true;
		}
		else Panic("What type " + sym.name + " has?");
		
		// Find currencies
		if (sym.IsForex()) {
			String a = sym.name.Left(3);
			String b = sym.name.Mid(3,3);
			currencies.GetAdd(a, 0)++;
			currencies.GetAdd(b, 0)++;
		}
		
		symbols.Add(sym);
	}
	
	// Sort postfix stats
	SortByValue(postfix_counts, StdGreater<int>());
	int postfix = postfix_counts.GetKey(0);
	
	// Find proxy ids
	for(int i = 0; i < symbols.GetCount(); i++) {
		Symbol& sym = symbols[i];
		if (sym.proxy_name.IsEmpty()) {
			sym.proxy_id = -1;
			continue;
		}
		bool found = false;
		
		
		// Add postfix to the proxy symbol
		if (postfix)
			sym.proxy_name.Cat(postfix);
		
		
		for(int j = 0; j < symbols.GetCount(); j++) {
			if (j == i) continue;
			if (symbols[j].name == sym.proxy_name) {
				sym.proxy_id = j;
				
				String a = sym.proxy_name.Left(3);
				String b = sym.proxy_name.Mid(3,3);
				bool base_dest = b == account_currency; // base USD, buy AUDJPY, proxy AUDUSD.. USD->AUD->JPY ... AUDUSD selling
				sym.proxy_factor = base_dest ? -1 : 1;
				
				found = true;
				break;
			}
		}
		if (!found) {
			LOG("Warning: no proxy was found for symbol " << sym.name);
		}
	}
	
	// Add currencies
	SortByValue(currencies, StdGreater<int>()); // sort by must pairs having currency
	this->currencies.Clear();
	for(int i = 0; i < currencies.GetCount(); i++) {
		const String& symbol = currencies.GetKey(i);
		Currency& c = this->currencies.Add(symbol);
		c.name = symbol;
		for(int j = 0; j < symbols.GetCount(); j++) {
			Symbol& s = symbols[j];
			if (!s.IsForex()) continue;
			const String& key = s.name;
			
			String k0 = key.Left(3);
			String k1 = key.Mid(3,3);
			
			if (k0 == symbol) {
				c.pairs0.Add(j);
			}
			else if (k1 == symbol) {
				c.pairs1.Add(j);
			}
		}
	}
	
	return symbols;
}

const Vector<Price>& MetaTrader::GetAskBid() {
	String content;
	while (1) {
		try {
			content = GetAskBidRaw();
			if (content.GetCount()) break;
		}
		catch (ConnectionError e) {
			continue;
		}
	}
	
	dword time = TimestampNow();
	Vector<String> lines = Split(content, ";");
	int c1 = lines.GetCount();
	
	askbid.SetCount(c1);
	for(int i = 0; i < c1; i++) {
		Vector<String> line = Split(lines[i], ",");
		double ask = StrDbl(line[1]);
		double bid = StrDbl(line[2]);
		double volume = 0;
		if (line.GetCount() > 3)
			volume = StrDbl(line[3]);
		Price& p = askbid[i];
		p.time = time;
		p.ask = ask;
		p.bid = bid;
		p.volume = volume;
	}
	return askbid;
}


#undef GET_INT
#undef GET_DBL
#define GET() v[pos++]
#define GET_INT() StrInt(v[pos++])
#define GET_DBL() StrDbl(v[pos++])

void MetaTrader::LoadOrderFile(String content, Vector<MTOrder>& orders, bool is_open) {
	
	// Clear old data
	orders.Clear();
	
	Vector<String> v = Split(content, ",", false);
	int pos = 0;
	
	if (v[0] == "1234") return;
	
	// Read data
	bool skip;
	while (pos < v.GetCount()) {
		skip = 0;
		int ticket = GET_INT();
		int i = orders.GetCount();
		MTOrder& o = orders.Add();
		int strsize, t;
		o.ticket = ticket;
		o.symbol = GET();
		o.open = GET_DBL();
		o.close = GET_DBL();
		int ts = GET_INT();
		o.begin = TimeFromTimestamp(ts);
		//if (o.begin < realbegin) skip = 1;
		o.end = TimeFromTimestamp(GET_INT());
		o.type = GET_INT();
		o.takeprofit = GET_DBL();
		o.stoploss = GET_DBL();
		o.volume = GET_DBL();
		o.profit = GET_DBL();
		o.commission = GET_DBL();
		o.swap = GET_DBL();
		o.expiration = TimeFromTimestamp(GET_INT());
		o.is_open = is_open;
		
		if (!ts) {
			orders.Remove(i);
		}
		if (skip) break;
		
		if (v[pos] == "1234") return;
	}
}


void MetaTrader::GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history) {
	
	// Load open orders data from Broker
	String content;
	while (1) {
		try {
			content = GetOrdersRaw(magic);
			if (content.GetCount()) break;
		}
		catch (ConnectionError) {
			continue;
		}
	}
	
	// Load MTOrder from data
	Vector<MTOrder> open_orders;
	LoadOrderFile(content, open_orders, true);
	
	// Compare current open orders to MT open orders
	bool all_equal = true;
	if (open.GetCount() != open_orders.GetCount())
		all_equal = false;
	open.Clear();
	for(int i = 0; i < open_orders.GetCount(); i++) {
		MTOrder& mto = open_orders[i];
		
		// Update existing orders
		int j = orders.Find(mto.ticket);
		if (j != -1) {
			Order& bo = orders[j];
			bo = mto;
			// Add pointer
			open.Add(bo.ticket);
		}
		
		// Add new orders
		else {
			Order& bo = orders.Add(mto.ticket);
			bo = mto;
			// Add pointer
			open.Add(bo.ticket);
			all_equal = false;
		}
	}
	
	// Don't update history files if open orders are not changed
	if (all_equal && !force_history) return;
	
	// Load history orders data from Broker
	content.Clear();
	while (1) {
		try {
			content = GetHistoryOrdersRaw(magic);
			if (content.GetCount()) break;
		}
		catch (ConnectionError) {
			continue;
		}
	}
	
	// Load stored file
	Vector<MTOrder> history_orders;
	LoadOrderFile(content, history_orders, false);
	//LOG(content);
	
	// Update history data
	for(int i = 0; i < history_orders.GetCount(); i++) {
		MTOrder& mto = history_orders[i];
		
		// Update existing orders
		int j = orders.Find(mto.ticket);
		if (j != -1) {
			Order& bo = orders[j];
			bo = mto;
			// Add pointer
			open.Add(bo.ticket);
		}
		
		// Add new orders
		else {
			Order& bo = orders.Add(mto.ticket);
			bo = mto;
			// Add pointer
			open.Add(bo.ticket);
		}
	}
}


#define GET_INT() StrInt(v[pos++])
#define GET_DBL() StrDbl(v[pos++])

const Vector<PriceTf>&	MetaTrader::GetTickData() {
	pricetf.SetCount(0);
	
	// Send a message to Broker to store data
	String s;
	while (1) {
		try {
			s = GetPricesRaw();
			if (s.GetCount()) break;
		}
		catch (ConnectionError e) {
			continue;
		}
	}
	
	// Open data file
	Vector<String> v = Split(s, ",");
	int pos = 0;
	
	// Load header
	int tf_count = GET_INT();
	
	Time now_time = GetSysTime();
	
	
	// Load data per timeframe
	for(int i = 0; i < tf_count; i++) {
		
		// Load timeframe header
		int tf = GET_INT(); ASSERT(GetTimeframe(i) == tf);
		
		// Load timeframe symbol header
		int symbol_count = GET_INT();
			
		// Load data per timeframe symbol
		for(int j = 0; j < symbol_count; j++) {
			
			// Load timeframe symbol header
			int count = GET_INT();
			ASSERT(count == 1);
			
			// Load the actual data
			dword time;
			double high, low, open, close, volume;
			for(int k = 0; k < count; k++) {
				// Common data
				time = GET_INT();
				high = GET_DBL();
				
				low = GET_DBL();
				open = GET_DBL();
				close = GET_DBL();
				volume = GET_DBL();
			}
			
			PriceTf& p = pricetf.Add();
			p.time = time;
			p.high = high;
			p.low = low;
			p.open = open;
			p.close = close;
			p.volume = volume;
		}
	}
	
	// Check that everything went as expected: require known magic value at the end.
	int magic = GET_INT();
	ASSERT(magic == 1234);
	
	return pricetf;
}


#define TEST(x) if (!(x)) {LOG("MTPacket error: " #x); return 1;}

bool MTPacket::Export(MetaTrader& mt, const String& addr, int port) {
	String str;
	for(int i = 0; i < values.GetCount(); i++) {
		if (i) str.Cat(',');
		str += values[i];
	}
	mt.AddOutputBytes(4 + str.GetCount());
	result.Clear();
	
	TcpSocket sock;
	sock.Timeout(3000);
	sock.Blocking();
	TEST(sock.Connect(addr, port));
	int got, len;
	len = str.GetCount();
	got = sock.Put(&len, 4);
	TEST(got == 4);
	got = sock.Put(str.Begin(), len);
	TEST(got == len);
	
	got = sock.Get(&len, 4);
	TEST(got == 4);
	TEST(len >= 0 && len < 256*256);
	if (len == 0)
		result.Clear();
	else
		result = sock.Get(len);
	TEST(result.GetCount() == len);
	
	mt.AddInputBytes(4 + result.GetCount());
	
	return 0;
}

bool MTPacket::Import() {
	values.SetCount(2);
	values[0] = latest_code;
	values[1] = result;
	return 0;
}

