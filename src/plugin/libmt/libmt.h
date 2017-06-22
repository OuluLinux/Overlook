#ifndef _plugin_libmt_libmt_h_
#define _plugin_libmt_libmt_h_

#include <Core/Core.h>
#include <Core/Rpc/Rpc.h>

using namespace Upp;

String GetProxy(const String& currency);

inline void SyncToTimeframe(Time& t, int tf) {
	t.second = 0;
	int mins;
	switch (tf) {
		case 1:
			return;
		case 5:
		case 10:
		case 15:
		case 20:
		case 30:
			t.minute = t.minute - t.minute % tf;
			return;
		case 60:
			t.minute = 0;
			return;
		case 90:
			mins = t.hour * 60 + t.minute;
			mins -= mins % 90;
			t.hour = mins / 60;
			t.minute = mins % 60;
			return;
		case 120:
			t.minute = 0;
			t.hour = t.hour - t.hour % 2;
			return;
		case 180:
			t.minute = 0;
			t.hour = t.hour - t.hour % 3;
			return;
		case 240:
			t.minute = 0;
			t.hour = t.hour - t.hour % 4;
			return;
		case 360:
			t.minute = 0;
			t.hour = t.hour - t.hour % 6;
			return;
		case 720:
			t.minute = 0;
			t.hour = t.hour - t.hour % 12;
			return;
		case 1440:
			t.minute = 0;
			t.hour = 0;
			return;
		case 2880:
			Panic("This is more complex problem");
			return;
		case 10080:
			t.minute = 0;
			t.hour = 0;
			while (DayOfWeek(t)) t -= 60*60*24;
	}
	return;
}


enum ENUM_APPLIED_PRICE { PRICE_CLOSE, PRICE_OPEN, PRICE_HIGH, PRICE_LOW, PRICE_MEDIAN, PRICE_TYPICAL, PRICE_WEIGHTED, PRICE_VOLUME, PRICE_TIME, PRICE_SELLVOLUME};
enum VOLUME_TICK { VOLUME_TICK, VOLUME_REAL};
enum ORDER_POOL {MODE_TRADES, MODE_HISTORY};
enum SELECT_ORDER {SELECT_BY_POS, SELECT_BY_TICKET};
enum OPERATION_CMD {OP_BUY, OP_SELL, OP_BUYLIMIT, OP_BUYSTOP, OP_SELLLIMIT, OP_SELLSTOP};
enum SYMBOL_PROPERTIES {
	MODE_LOW=1, MODE_HIGH=2, MODE_TIME=5, MODE_BID=9, MODE_ASK=10, MODE_POINT=11, MODE_DIGITS=12,
	MODE_SPREAD=13, MODE_STOPLEVEL=14, MODE_LOTSIZE=15, MODE_TICKVALUE=16, MODE_TICKSIZE=17, MODE_SWAPLONG=18,
	MODE_SWAPSHORT=19, MODE_STARTING=20, MODE_EXPIRATION=21, MODE_TRADEALLOWED=22, MODE_MINLOT=23, MODE_LOTSTEP=24,
	MODE_MAXLOT=25, MODE_SWAPTYPE=26, MODE_PROFITCALCMODE=27, MODE_MARGINCALCMODE=28, MODE_MARGININIT=29, MODE_MARGINMAINTENANCE=30,
	MODE_MARGINHEDGED=31, MODE_MARGINREQUIRED=32, MODE_FREEZELEVEL=33, MODE_CLOSEBY_ALLOWED=34};
	
enum ENUM_ACCOUNT_INFO_INTEGER {
	ACCOUNT_LOGIN, ACCOUNT_TRADE_MODE, ACCOUNT_LEVERAGE, ACCOUNT_LIMIT_ORDERS, ACCOUNT_MARGIN_SO_MODE,
	ACCOUNT_TRADE_ALLOWED, ACCOUNT_TRADE_EXPERT
};
enum ENUM_ACCOUNT_INFO_DOUBLE {
	ACCOUNT_BALANCE, ACCOUNT_CREDIT, ACCOUNT_PROFIT, ACCOUNT_EQUITY,
	ACCOUNT_MARGIN, ACCOUNT_MARGIN_FREE, ACCOUNT_MARGIN_LEVEL, ACCOUNT_MARGIN_SO_CALL,
	ACCOUNT_MARGIN_SO_SO, ACCOUNT_MARGIN_INITIAL, ACCOUNT_MARGIN_MAINTENANCE, ACCOUNT_ASSETS,
	ACCOUNT_LIABILITIES, ACCOUNT_COMMISSION_BLOCKED
};
enum ENUM_ACCOUNT_INFO_STRING {
	ACCOUNT_NAME, ACCOUNT_SERVER, ACCOUNT_CURRENCY, ACCOUNT_COMPANY
};
enum ENUM_ACCOUNT_TRADE_MODE {
	ACCOUNT_TRADE_MODE_DEMO, ACCOUNT_TRADE_MODE_CONTEST, ACCOUNT_TRADE_MODE_REAL
};
enum ENUM_ACCOUNT_STOPOUT_MODE {
	ACCOUNT_STOPOUT_MODE_PERCENT, ACCOUNT_STOPOUT_MODE_MONEY
};
enum ENUM_FREEMARGIN_CALCMODE {
	FREEMARGINMODE_WITHOUTOPENORDERS, FREEMARGINMODE_WITHOPENORDERS, FREEMARGINMODE_WITHOPENPROFIT, FREEMARGINMODE_WITHOPENLOSS
};
enum ENUM_SYMBOL_INFO_INTEGER {
	SYMBOL_SELECT, SYMBOL_VISIBLE, SYMBOL_SESSION_DEALS, SYMBOL_SESSION_BUY_ORDERS, SYMBOL_SESSION_SELL_ORDERS, SYMBOL_VOLUME, SYMBOL_VOLUMEHIGH,
	SYMBOL_VOLUMELOW, SYMBOL_TIME, SYMBOL_DIGITS, SYMBOL_SPREAD_FLOAT, SYMBOL_SPREAD, SYMBOL_TRADE_CALC_MODE, SYMBOL_TRADE_MODE, SYMBOL_START_TIME,
	SYMBOL_EXPIRATION_TIME, SYMBOL_TRADE_STOPS_LEVEL, SYMBOL_TRADE_FREEZE_LEVEL, SYMBOL_TRADE_EXEMODE, SYMBOL_SWAP_MODE, SYMBOL_SWAP_ROLLOVER3DAYS,
	SYMBOL_EXPIRATION_MODE, SYMBOL_FILLING_MODE, SYMBOL_ORDER_MODE
};
enum ENUM_SYMBOL_INFO_DOUBLE {
	SYMBOL_BID, SYMBOL_BIDHIGH, SYMBOL_BIDLOW, SYMBOL_ASK, SYMBOL_ASKHIGH,
	SYMBOL_ASKLOW, SYMBOL_LAST, SYMBOL_LASTHIGH, SYMBOL_LASTLOW, SYMBOL_POINT,
	SYMBOL_TRADE_TICK_VALUE, SYMBOL_TRADE_TICK_VALUE_PROFIT, SYMBOL_TRADE_TICK_VALUE_LOSS,
	SYMBOL_TRADE_TICK_SIZE, SYMBOL_TRADE_CONTRACT_SIZE, SYMBOL_VOLUME_MIN, SYMBOL_VOLUME_MAX,
	SYMBOL_VOLUME_STEP, SYMBOL_VOLUME_LIMIT, SYMBOL_SWAP_LONG, SYMBOL_SWAP_SHORT,
	SYMBOL_MARGIN_INITIAL, SYMBOL_MARGIN_MAINTENANCE, SYMBOL_MARGIN_LONG, SYMBOL_MARGIN_SHORT,
	SYMBOL_MARGIN_LIMIT, SYMBOL_MARGIN_STOP, SYMBOL_MARGIN_STOPLIMIT, SYMBOL_SESSION_VOLUME,
	SYMBOL_SESSION_TURNOVER, SYMBOL_SESSION_INTEREST, SYMBOL_SESSION_BUY_ORDERS_VOLUME,
	SYMBOL_SESSION_SELL_ORDERS_VOLUME, SYMBOL_SESSION_OPEN, SYMBOL_SESSION_CLOSE,
	SYMBOL_SESSION_AW, SYMBOL_SESSION_PRICE_SETTLEMENT, SYMBOL_SESSION_PRICE_LIMIT_MIN,
	SYMBOL_SESSION_PRICE_LIMIT_MAX
};
enum ENUM_SYMBOL_INFO_STRING {
	SYMBOL_CURRENCY_BASE, SYMBOL_CURRENCY_PROFIT, SYMBOL_CURRENCY_MARGIN, SYMBOL_DESCRIPTION, SYMBOL_PATH
};

struct Price : Moveable<Price> {
	double ask, bid, volume;
    dword time;
    
    Price() : ask(0), bid(0), volume(0), time(0) {}
    Price(const Price& p) {ask = p.ask; bid = p.bid; volume = p.volume; time = p.time;}
    
	void Set(dword time, double ask, double bid, double volume) {
		this->ask = ask;
		this->bid = bid;
		this->time = time;
		this->volume = volume;
	}
    
	String ToString() const {
		String s; s << time << " : " << ask << " : " << bid << " : " << volume; return s;
	}
	
	void Serialize(Stream &s) {
		s % time % ask % bid % volume;
	}
};

struct PriceTf : Moveable<PriceTf> {
	double open, low, high, close, volume;
	dword time;
	
	PriceTf() : open(0), low(0), high(0), close(0), volume(0), time(0) {}
	
	void Serialize(Stream &s) {
		s % open % low % high % close % volume % time;
	}
};

struct SpreadTick : Moveable<SpreadTick> {
	Time time;
	int sym;
	double spread;
	void Serialize(Stream& s) {s % time % sym % spread;}
};

struct Symbol : public Moveable<Symbol> {
	Symbol() {}
	Symbol(const Symbol& bs) {*this = bs;}
	Symbol& operator = (const Symbol& sym);
	
	// Old BrokerSymbol
	int id;
	String name, proxy_name;
	
	bool is_skipping;
	bool tradeallowed;
	
	double lotsize;
	int profit_calc_mode;
	int margin_calc_mode;
	double margin_hedged;
	double margin_required;
	
	int selected;
	int visible;
	int digits;
	int spread_floating;
	int spread;
	int calc_mode;
	int trade_mode;
	int start_time;
	int expiration_time;
	int stops_level;
	int freeze_level;
	int execution_mode;
	int swap_mode;
	int swap_rollover3days;
	int base_mul;
	
	double point;
	double tick_value;
	double tick_size;
	double contract_size;
	double volume_min;
	double volume_max;
	double volume_step;
	double swap_long;
	double swap_short;
	double margin_initial;
	double margin_maintenance;
	
	String currency_base;
	String currency_profit;
	String currency_margin;
	String description;
	String path;
	
	byte quotes_begin_hours[7], quotes_begin_minutes[7];
	byte quotes_end_hours[7], quotes_end_minutes[7];
	byte trades_begin_hours[7], trades_begin_minutes[7];
	byte trades_end_hours[7], trades_end_minutes[7];
	
	double margin_factor;
	int virtual_type;
	
	enum CALCMODE { CALCMODE_FOREX, CALCMODE_CDF, CALCMODE_FUTURES, CALCMODE_CDF_INDICES };
	enum SWAPTYPE { SWAPTYPE_INPOINTS, SWAPTYPE_INSYMBOLBASECURRENCY, SWAPTYPE_BYINTEREST, SWAPTYPE_INMARGINCURRENCY };
	enum SYMBOL_TRADE_MODE { SYMBOL_TRADE_MODE_DISABLED, SYMBOL_TRADE_MODE_LONGONLY, SYMBOL_TRADE_MODE_SHORTONLY, SYMBOL_TRADE_MODE_CLOSEONLY, SYMBOL_TRADE_MODE_FULL };
	enum SYMBOL_TRADE_EXECUTION { SYMBOL_TRADE_EXECUTION_REQUEST, SYMBOL_TRADE_EXECUTION_INSTANT, SYMBOL_TRADE_EXECUTION_MARKET, SYMBOL_TRADE_EXECUTION_EXCHANGE };
	enum DAY_OF_WEEK { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
	
	static String GetCalculationMode(int mode);
	static String GetTradeMode(int mode);
	static String GetExecutionMode(int mode);
	static String GetSwapMode(int mode);
	static String GetDayOfWeek(int wday);
	String GetQuotesRange(int wday) const;
	String GetTradesRange(int wday) const;
	
	bool IsOpen(Time t) const;
	
	
	// Old MT4Symbol
	int proxy_id, proxy_factor;
	
	bool IsForex() const {return calc_mode == CALCMODE_FOREX;}
	bool IsCDF() const {return calc_mode == CALCMODE_CDF;}
	bool IsCDFIndex() const {return calc_mode == CALCMODE_CDF_INDICES;}
	bool IsFuture() const {return calc_mode == CALCMODE_FUTURES;}
	
	operator String() const {return name;}
	String GetId() const {return name;}
	
	void Serialize(Stream& s) {
		s	% id
			% name % proxy_name
			% is_skipping
			% tradeallowed
			
			% lotsize
			% profit_calc_mode
			% margin_calc_mode
			% margin_hedged
			% margin_required
			
			% selected
			% visible
			% digits
			% spread_floating
			% spread
			% calc_mode
			% trade_mode
			% start_time
			% expiration_time
			% stops_level
			% freeze_level
			% execution_mode
			% swap_mode
			% swap_rollover3days
			% base_mul
			
			% point
			% tick_value
			% tick_size
			% contract_size
			% volume_min
			% volume_max
			% volume_step
			% swap_long
			% swap_short
			% margin_initial
			% margin_maintenance
			
			% currency_base
			% currency_profit
			% currency_margin
			% description
			% path;
			
		for(int i = 0; i < 7; i++) {
			s % quotes_begin_hours[i] % quotes_begin_minutes[i];
			s % quotes_end_hours[i]   % quotes_end_minutes[i];
			s % trades_begin_hours[i] % trades_begin_minutes[i];
			s % trades_end_hours[i]   % trades_end_minutes[i];
		}
		
		s	% margin_factor
			% proxy_id % proxy_factor
			% virtual_type;
	}
	String ToString() const {Panic("Symbol ToString still TODO"); return "";}
	
};

struct Currency : Moveable<Currency> {
	Index<int> pairs0, pairs1;
	String name;
};

	
struct DbgDouble {
	double value;
	Vector<double> past_values;
	DbgDouble() : value(0) {past_values.Add(0);}
	DbgDouble(const DbgDouble& src) : value(0) {past_values.Add(0); *this = src;}
	bool operator==(double d) const {return value == d;}
	bool operator!=(double d) const {return value != d;}
	operator double() const {return value;}
	operator Value()  const {return value;}
	void operator=(double d) {ASSERT(value == 0); value = d; past_values.Add(d);}
	void operator=(const DbgDouble& d) {ASSERT(value == 0); value = d; past_values.Append(d.past_values);}
	String ToString() const {return DblStr(value);}
};

struct Order : Moveable<Order> {
	
	// Ctors
	Order() : open(0), close(0), takeprofit(0), stoploss(0), id(-1) {}
	Order(const Order& o) {*this = o;}
	Order& operator = (const Order& o) {
		begin = o.begin;
		end = o.end;
		price_change = o.price_change;
		spread = o.spread;
		volume = o.volume;
		profit = o.profit;
		best  = o.best;
		worst = o.worst;
		id = o.id;
		sym = o.sym;
		tf = o.tf;
		type = o.type;
		proxy = o.proxy;
		is_open = o.is_open;
		
		symbol = o.symbol;
		comment = o.comment;
		expiration = o.expiration;
		open = o.open;
		close = o.close;
		open_proxy = o.open_proxy;
		close_proxy = o.close_proxy;
		stoploss = o.stoploss;
		takeprofit = o.takeprofit;
		commission = o.commission;
		swap = o.swap;
		ticket = o.ticket;
		
		shift = o.shift;
		
		return *this;
	}
	
	
	//
	Time begin, end;
	double price_change;
	double spread;
	double volume;
	double profit;
	double best, worst;
	int id;
	int sym;
	int tf;
	int type;
	int proxy;
	bool is_open;
	
	// Public vars
	String symbol;
	String comment;
	Time expiration;
	double open, close;
	double open_proxy, close_proxy;
	double stoploss, takeprofit;
	double commission;
	double swap;
	int ticket;
	int shift;
	
	// Main funcs
	void Serialize(Stream& s) {
		s
			% begin % end
			% price_change
			% spread
			% volume
			% profit
			% best % worst
			% id
			% sym
			% tf
			% type
			% proxy
			% is_open
			
			% symbol
			% comment
			% expiration
			% open % close
			% open_proxy % close_proxy
			% stoploss % takeprofit
			% commission
			% swap
			% ticket
			% shift;
	}
	String GetTypeString() const;
	
};



struct ConnectionError : public Exc { ConnectionError() {*(String*)this = "MT4 Connection Error";} };



// Class for temp order data
struct MTOrder : Moveable<MTOrder> {
	int ticket;
	String symbol;
	double open, close;
	Time begin, end;
	int type;
	double takeprofit, stoploss;
	double volume, profit, commission, swap;
	Time expiration;
	bool is_open;
	
	operator Order() {
		Order out;
		out.ticket = ticket;
		out.symbol = symbol;
		out.open = open;
		out.close = close;
		out.begin = begin;
		out.end = end;
		out.type = type;
		out.takeprofit = takeprofit;
		out.stoploss = stoploss;
		out.volume = volume;
		out.profit = profit;
		out.commission = commission;
		out.swap = swap;
		out.expiration = expiration;
		out.is_open = is_open;
		return out;
	}
};

class Brokerage {
	
	
public:
	Brokerage() {}
	
	virtual double	AccountInfoDouble(int property_id) = 0;
	virtual int		AccountInfoInteger(int property_id) = 0;
	virtual String	AccountInfoString(int property_id) = 0;
	virtual double	AccountBalance() = 0;
	virtual double	AccountCredit() = 0;
	virtual String	AccountCompany() = 0;
	virtual String	AccountCurrency() = 0;
	virtual double	AccountEquity() = 0;
	virtual double	AccountFreeMargin() = 0;
	virtual double	AccountFreeMarginCheck(String symbol, int cmd, double volume) = 0;
	virtual double	AccountFreeMarginMode() = 0;
	virtual int		AccountLeverage() = 0;
	virtual double	AccountMargin() = 0;
	virtual String	AccountName() = 0;
	virtual int		AccountNumber() = 0;
	virtual double	AccountProfit() = 0;
	virtual String	AccountServer() = 0;
	virtual int		AccountStopoutLevel() = 0;
	virtual int		AccountStopoutMode() = 0;
	virtual double	MarketInfo(String symbol, int type) = 0;
	virtual int		SymbolsTotal(int selected) = 0;
	virtual String	SymbolName(int pos, int selected) = 0;
	virtual int		SymbolSelect(String name, int select) = 0;
	virtual double	SymbolInfoDouble(String name, int prop_id) = 0;
	virtual int		SymbolInfoInteger(String name, int prop_id) = 0;
	virtual String	SymbolInfoString(String name, int prop_id) = 0;
	virtual int		RefreshRates() = 0;
	virtual int		iBars(String symbol, int timeframe) = 0;
	virtual int		iBarShift(String symbol, int timeframe, int datetime) = 0;
	virtual double	iClose(String symbol, int timeframe, int shift) = 0;
	virtual double	iHigh(String symbol, int timeframe, int shift) = 0;
	virtual double	iLow(String symbol, int timeframe, int shift) = 0;
	virtual double	iOpen(String symbol, int timeframe, int shift) = 0;
	virtual int		iHighest(String symbol, int timeframe, int type, int count, int start) = 0;
	virtual int		iLowest(String symbol, int timeframe, int type, int count, int start) = 0;
	virtual int		iTime(String symbol, int timeframe, int shift) = 0;
	virtual int		iVolume(String symbol, int timeframe, int shift) = 0;
	virtual int		OrderClose(int ticket, double lots, double price, int slippage) = 0;
	virtual double	OrderClosePrice() = 0;
	virtual int		OrderCloseTime() = 0;
	virtual String	OrderComment() = 0;
	virtual double	OrderCommission() = 0;
	virtual int		OrderDelete(int ticket) = 0;
	virtual int		OrderExpiration() = 0;
	virtual double	OrderLots() = 0;
	virtual int		OrderMagicNumber() = 0;
	virtual int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) = 0;
	virtual double	OrderOpenPrice() = 0;
	virtual int		OrderOpenTime() = 0;
	virtual double	OrderProfit() = 0;
	virtual int		OrderSelect(int index, int select, int pool) = 0;
	virtual int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0) = 0;
	virtual int		OrdersHistoryTotal() = 0;
	virtual double	OrderStopLoss() = 0;
	virtual int		OrdersTotal() = 0;
	virtual double	OrderSwap() = 0;
	virtual String	OrderSymbol() = 0;
	virtual double	OrderTakeProfit() = 0;
	virtual int		OrderTicket() = 0;
	virtual int		OrderType() = 0;
	virtual bool    IsDemo() = 0;
	virtual bool    IsConnected() = 0;
	virtual const Vector<Symbol>&	GetSymbols() = 0;
	virtual const Vector<Price>&	GetAskBid() = 0;
	virtual const Vector<PriceTf>&	GetTickData() = 0;
	virtual const Vector<Order>&	GetOpenOrders() = 0;
	virtual const Vector<Order>&	GetHistoryOrders() = 0;
	
};

class MetaTrader : public Brokerage {
	// Vars
	int port;
	Mutex lock;
	String mainaddr;
	
	
	Index<String> skipped_currencies;
	
	String account_id, account_name, account_server, account_currency;
	String last_error;
	double balance, equity, margin, freemargin, leverage;
	bool demo, connected, simulation;
	bool init_success;
	
	Vector<Order> orders, history_orders;
	Vector<Price> current_prices;
	Vector<Symbol> symbols;
	Vector<Price> askbid;
	Vector<PriceTf> pricetf;
	VectorMap<String, Currency> currencies;
	Vector<int> indices;
	Mutex current_price_lock;
	
	ArrayMap<int, String> periodstr;
	int tf_h1_id;
	
public:
	enum {MODE_OPEN, MODE_LOW, MODE_HIGH, MODE_CLOSE, MODE_VOLUME, MODE_TIME};
	
	
	MetaTrader();
	~MetaTrader();
	
	void Serialize(Stream& s) {s % port % mainaddr;}
	
	// Old functions, convert
	void LoadOrderFile(String content, Vector<MTOrder>& orders, bool is_open);
	
	const String& GetAddr() const {return mainaddr;}
	int GetPort() const {return port;}
	
	// Backend API functions, must be implemented
	int		Init(String addr, int port=42000);
	
	virtual double	AccountInfoDouble(int property_id);
	virtual int		AccountInfoInteger(int property_id);
	virtual String	AccountInfoString(int property_id);
	virtual double	AccountBalance();
	virtual double	AccountCredit();
	virtual String	AccountCompany();
	virtual String	AccountCurrency();
	virtual double	AccountEquity();
	virtual double	AccountFreeMargin();
	virtual double	AccountFreeMarginCheck(String symbol, int cmd, double volume);
	virtual double	AccountFreeMarginMode();
	virtual int		AccountLeverage();
	virtual double	AccountMargin();
	virtual String	AccountName();
	virtual int		AccountNumber();
	virtual double	AccountProfit();
	virtual String	AccountServer();
	virtual int		AccountStopoutLevel();
	virtual int		AccountStopoutMode();
	virtual double	MarketInfo(String symbol, int type);
	virtual int		SymbolsTotal(int selected);
	virtual String	SymbolName(int pos, int selected);
	virtual int		SymbolSelect(String name, int select);
	virtual double	SymbolInfoDouble(String name, int prop_id);
	virtual int		SymbolInfoInteger(String name, int prop_id);
	virtual String	SymbolInfoString(String name, int prop_id);
	virtual int		RefreshRates();
	virtual int		iBars(String symbol, int timeframe);
	virtual int		iBarShift(String symbol, int timeframe, int datetime);
	virtual double	iClose(String symbol, int timeframe, int shift);
	virtual double	iHigh(String symbol, int timeframe, int shift);
	virtual double	iLow(String symbol, int timeframe, int shift);
	virtual double	iOpen(String symbol, int timeframe, int shift);
	virtual int		iHighest(String symbol, int timeframe, int type, int count, int start);
	virtual int		iLowest(String symbol, int timeframe, int type, int count, int start);
	virtual int		iTime(String symbol, int timeframe, int shift);
	virtual int		iVolume(String symbol, int timeframe, int shift);
	virtual int		OrderClose(int ticket, double lots, double price, int slippage);
	virtual double	OrderClosePrice();
	virtual int		OrderCloseTime();
	virtual String	OrderComment();
	virtual double	OrderCommission();
	virtual int		OrderDelete(int ticket);
	virtual int		OrderExpiration();
	virtual double	OrderLots();
	virtual int		OrderMagicNumber();
	virtual int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration);
	virtual double	OrderOpenPrice();
	virtual int		OrderOpenTime();
	virtual double	OrderProfit();
	virtual int		OrderSelect(int index, int select, int pool);
	virtual int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0);
	virtual int		OrdersHistoryTotal();
	virtual double	OrderStopLoss();
	virtual int		OrdersTotal();
	virtual double	OrderSwap();
	virtual String	OrderSymbol();
	virtual double	OrderTakeProfit();
	virtual int		OrderTicket();
	virtual int		OrderType();
	virtual bool    IsDemo();
	virtual bool    IsConnected();
	virtual const Vector<Symbol>&	GetSymbols();
	virtual const Vector<Price>&	GetAskBid();
	virtual const Vector<PriceTf>&	GetTickData();
	virtual const Vector<Order>&	GetOpenOrders() {return orders;}
	virtual const Vector<Order>&	GetHistoryOrders() {return history_orders;}
	
	void GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history = false);
	
	String	GetSymbolsRaw();
	String	GetAskBidRaw();
	String	GetPricesRaw();
	String	GetHistoryOrdersRaw(int magic);
	String	GetOrdersRaw(int magic);
	
	Vector<Vector<Time> > GetLatestPriceTimes();
	Vector<Vector<Time> > GetEarliestPriceTimes();
	
	int FindPriceTime(int sym, int tf, dword ts);
	
	String GetLastError();
	
	bool IsResponding();
	
	
	
	const Vector<Symbol>& GetCacheSymbols() const {return symbols;}
	const Symbol& GetSymbol(int i) const {return symbols[i];}
	const Currency& GetCurrency(int i) const {return currencies[i];}
	String GetAccountCurrency() {return account_currency;}
	String GetAccountServer() {return account_server;}
	String GetAccountName() {return account_name;}
	String GetAccountNumber() {return account_id;}
	String GetTimeframeString(int i) {return periodstr[i];}
	double GetAccountBalance() {return balance;}
	double GetAccountEquity() {return equity;}
	double GetAccountLeverage() {return leverage;}
	double GetAccountFreeMargin() {return freemargin;}
	bool IsDemoCached() {return demo;}
	bool IsConnectedCached() {return connected;}
	int GetTimeframeCount() {return periodstr.GetCount();}
	int GetTimeframe(int i) {return periodstr.GetKey(i);}
	int GetTimeframeIdH1() {return tf_h1_id;}
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetCurrencyCount() const {return currencies.GetCount();}
	int GetIndexId(int i) const {return indices[i];}
	int GetIndexCount() const {return indices.GetCount();}
	

	int input, output;
	
	int GetInputBytes() {return input;}
	int GetOutputBytes() {return output;}
	void AddOutputBytes(int i) {input += i;}
	void AddInputBytes(int i) {output += i;}

};

inline MetaTrader& GetMetaTrader() {return Single<MetaTrader>();}


class MTPacket : Moveable<MTPacket> {
	
	// Vars
	Vector<String> values;
	String latest_code;
	String result;
	int code;
	
public:
	// Set value functions
	void	SetValuesCount(int i)	{values.SetCount(i+1);}
	void	SetDbl(int i, double d) {values[i+1] = DblStr(d);}
	void	SetInt(int i, int v)	{values[i+1] = IntStr(v);}
	void	SetStr(int i, String s) {values[i+1] = s;}
	void	SetCode(int i)			{if (values.GetCount() == 0) values.SetCount(1); values[0] = IntStr(i); latest_code = values[0];}
	
	// Get value functions
	String	GetStr(int i)			{return values[i+1];}
	int		GetInt(int i)			{return StrInt(values[i+1]);}
	double	GetDbl(int i)			{return StrDbl(values[i+1]);}
	int		GetCount()				{return values.GetCount()+1;}
	int		GetCode()				{if (values.GetCount() == 0) return -1; return StrInt(values[0]);}
	
	// Transfer functions
	bool Export(MetaTrader& mt, const String& addr, int port);
	bool Import();
	
};


#endif
