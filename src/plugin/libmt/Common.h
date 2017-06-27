#ifndef _plugin_libmt_Common_h_
#define _plugin_libmt_Common_h_


#include <Core/Core.h>
#include <Core/Rpc/Rpc.h>

namespace libmt {
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
	Symbol() : base_cur0(-1), base_cur1(-1) {}
	Symbol(const Symbol& bs) {*this = bs;}
	Symbol& operator = (const Symbol& sym);
	
	// Old BrokerSymbol
	int id;
	String name, proxy_name;
	
	bool is_skipping;
	bool tradeallowed;
	bool is_base_currency;
	
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
	int base_cur0, base_cur1;
	
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
	
	enum CALCMODE { CALCMODE_FOREX, CALCMODE_CFD, CALCMODE_FUTURES, CALCMODE_CFD_INDICES };
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
	bool IsCFD() const {return calc_mode == CALCMODE_CFD;}
	bool IsCFDIndex() const {return calc_mode == CALCMODE_CFD_INDICES;}
	bool IsFuture() const {return calc_mode == CALCMODE_FUTURES;}
	
	operator String() const {return name;}
	String GetId() const {return name;}
	
	void Serialize(Stream& s) {
		s	% id
			% name % proxy_name
			% is_skipping
			% tradeallowed
			% is_base_currency
			
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
			% base_cur0
			% base_cur1
			
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
	Index<int> pairs0, pairs1, all_pairs;
	int base_mul, base_pair;
	String name;
	
	Currency() : base_mul(0), base_pair(-1) {}
	Currency(const Currency& c) {
		pairs0 <<= c.pairs0;
		pairs1 <<= c.pairs1;
		all_pairs <<= c.all_pairs;
		base_mul = c.base_mul;
		base_pair = c.base_pair;
		name = c.name;
	}
};

struct Asset : Moveable<Asset> {
	int sym;
	double volume, rate, base_value;
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
	Order() {
		open = 0.0;			close = 0.0;		volume = 0.0;		profit = 0.0;
		stoploss = 0.0;		takeprofit = 0.0;	swap = 0.0;			commission = 0.0;
		symbol = -1;		ticket = -1;		type = -1;			is_open = false;
	}
	Order(const Order& o) {
		*this = o;
	}
	Order& operator = (const Order& o) {
		begin = o.begin;
		end = o.end;
		expiration = o.expiration;
		open = o.open;
		close = o.close;
		stoploss = o.stoploss;
		takeprofit = o.takeprofit;
		volume = o.volume;
		profit = o.profit;
		commission = o.commission;
		swap = o.swap;
		symbol = o.symbol;
		ticket = o.ticket;
		type = o.type;
		is_open = o.is_open;
		return *this;
	}
	
	
	Time begin, end;
	Time expiration;
	double open, close;
	double takeprofit, stoploss;
	double volume, profit, commission, swap;
	int symbol;
	int ticket;
	int type;
	bool is_open;
	
	// Main funcs
	void Serialize(Stream& s) {
		s	% begin % end
			% expiration
			% open % close
			% stoploss % takeprofit
			% volume
			% profit
			% commission
			% swap
			% symbol
			% ticket
			% type
			% is_open;
	}
	String GetTypeString() const;
	
};



struct ConnectionError : public Exc { ConnectionError() {*(String*)this = "MT4 Connection Error";} };




}

#endif
