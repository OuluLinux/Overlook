#ifndef _plugin_libmt_libmt_h_
#define _plugin_libmt_libmt_h_

#include <Core/Core.h>
#include <Core/Rpc/Rpc.h>

using namespace Upp;

String GetProxy(String currency);

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
	
	void Serialize(Stream& s) {s
		% id
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


enum {TYPE_BUY, TYPE_SELL, TYPE_BUY_LIMIT, TYPE_SELL_LIMIT, TYPE_BUY_STOP, TYPE_SELL_STOP};
	
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

class MetaTrader {
	// Vars
	int port;
	Mutex lock;
	String mainaddr;
	//XmlRpcRequest socket;
	
	Index<String> blocked_currencies;
	
	String account_id, account_name, account_server, account_currency;
	String last_error;
	double balance, equity, margin, freemargin, leverage;
	bool demo, connected, simulation;
	
	
	//VectorMap<String, Symbol> symbols;
	Index<String> currencies;
	Vector<Price> current_prices;
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
	//const Symbol& GetSymbol(int i) const {return symbols[i];}
	
	// Backend API functions, must be implemented
	int		Init(String addr, int port=42000);
	int		Connect(int port);
	int		Check();
	void	Disconnect();
	
	double	AccountInfoDouble(int property_id);
	int		AccountInfoInteger(int property_id);
	String	AccountInfoString(int property_id);
	double	AccountBalance();
	double	AccountCredit();
	String	AccountCompany();
	String	AccountCurrency();
	double	AccountEquity();
	double	AccountFreeMargin();
	double	AccountFreeMarginCheck(String symbol, int cmd, double volume);
	double	AccountFreeMarginMode();
	int		AccountLeverage();
	double	AccountMargin();
	String	AccountName();
	int		AccountNumber();
	double	AccountProfit();
	String	AccountServer();
	int		AccountStopoutLevel();
	int		AccountStopoutMode();
	double	MarketInfo(String symbol, int type);
	int		SymbolsTotal(int selected);
	String	SymbolName(int pos, int selected);
	int		SymbolSelect(String name, int select);
	double	SymbolInfoDouble(String name, int prop_id);
	int		SymbolInfoInteger(String name, int prop_id);
	String	SymbolInfoString(String name, int prop_id);
	
	int		RefreshRates();
	
	int		iBars(String symbol, int timeframe);
	int		iBarShift(String symbol, int timeframe, int datetime);
	double	iClose(String symbol, int timeframe, int shift);
	double	iHigh(String symbol, int timeframe, int shift);
	double	iLow(String symbol, int timeframe, int shift);
	double	iOpen(String symbol, int timeframe, int shift);
	int		iHighest(String symbol, int timeframe, int type, int count, int start);
	int		iLowest(String symbol, int timeframe, int type, int count, int start);
	int		iTime(String symbol, int timeframe, int shift);
	int		iVolume(String symbol, int timeframe, int shift);
	
	int		OrderClose(int ticket, double lots, double price, int slippage);
	double	OrderClosePrice();
	int		OrderCloseTime();
	String	OrderComment();
	double	OrderCommission();
	int		OrderDelete(int ticket);
	int		OrderExpiration();
	double	OrderLots();
	int		OrderMagicNumber();
	int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration);
	double	OrderOpenPrice();
	int		OrderOpenTime();
	double	OrderProfit();
	int		OrderSelect(int index, int select, int pool);
	int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0);
	//int		OrderSend(const Order& id, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0);
	int		OrdersHistoryTotal();
	double	OrderStopLoss();
	int		OrdersTotal();
	double	OrderSwap();
	String	OrderSymbol();
	double	OrderTakeProfit();
	int		OrderTicket();
	int		OrderType();
	bool    IsDemo();
	bool    IsConnected();
	Vector<Symbol>	GetSymbols();
	Vector<Price>	GetAskBid();
	Vector<PriceTf>	GetTickData();
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
	
	
	
	String GetAccountCurrency() {return account_currency;}
	String GetAccountServer() {return account_server;}
	String GetAccountName() {return account_name;}
	String GetAccountNumber() {return account_id;}
	double GetAccountBalance() {return balance;}
	double GetAccountEquity() {return equity;}
	double GetAccountLeverage() {return leverage;}
	double GetAccountFreeMargin() {return freemargin;}
	bool IsDemoCached() {return demo;}
	bool IsConnectedCached() {return connected;}
	int GetTimeframeCount() {return periodstr.GetCount();}
	int GetTimeframe(int i) {return periodstr.GetKey(i);}
	String GetTimeframeString(int i) {return periodstr[i];}
	int GetTimeframeIdH1() {return tf_h1_id;}

	int input, output;
	
	int GetInputBytes() {return input;}
	int GetOutputBytes() {return output;}
	void AddOutputBytes(int i) {input += i;}
	void AddInputBytes(int i) {output += i;}

};


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
