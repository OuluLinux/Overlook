#ifndef _plugin_libmt_MetaTrader_h_
#define _plugin_libmt_MetaTrader_h_

namespace libmt {

class MetaTrader : public Brokerage {
	Mutex lock;
	Mutex current_price_lock;
	String mainaddr;
	int input, output;
	int port;
	

	
public:
	
	
	MetaTrader();
	~MetaTrader();
	
	void Serialize(Stream& s) {s % port % mainaddr;}
	int Init(String addr, int port=42000);
	const String& GetAddr() const {return mainaddr;}
	int GetPort() const {return port;}

	
	// Brokerage functions without caching
	//  - function wrapper is needed, because remote calls are implemented with macros and
	//    some calls would intersect with Brokerage class methods, if the "_" prefix wouldn't
	//    be used. This is not a perfect solution, but it's best what could be thought while
	//    implementing.
	//  - improvement would be to add variable prefix to macros
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
	virtual int		RefreshRates();
	virtual Time	GetTime() const {return GetSysTime();}
	virtual double	RealtimeAsk(int sym) {return _MarketInfo(symbols[sym].name, MODE_ASK);}
	virtual double	RealtimeBid(int sym) {return _MarketInfo(symbols[sym].name, MODE_BID);}
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
		
	// Remote calling statistics
	int GetInputBytes() {return input;}
	int GetOutputBytes() {return output;}
	void AddOutputBytes(int i) {input += i;}
	void AddInputBytes(int i) {output += i;}

	// Remote calls without equal MQL counterpart
	bool _IsResponding();
	Vector<Vector<Time> > _GetLatestPriceTimes();
	Vector<Vector<Time> > _GetEarliestPriceTimes();
	void _GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history = false);
	String	_GetSymbolsRaw();
	String	_GetAskBidRaw();
	String	_GetPricesRaw();
	String	_GetHistoryOrdersRaw(int magic);
	String	_GetOrdersRaw(int magic);
	void LoadOrderFile(String content, Vector<MTOrder>& orders, bool is_open);
	const Vector<Symbol>&	_GetSymbols();
	const Vector<Price>&	_GetAskBid();
	const Vector<PriceTf>&	_GetTickData();
	
	// Remote calls
	double	_AccountInfoDouble(int property_id);
	int		_AccountInfoInteger(int property_id);
	String	_AccountInfoString(int property_id);
	double	_AccountBalance();
	double	_AccountCredit();
	String	_AccountCompany();
	String	_AccountCurrency();
	double	_AccountEquity();
	double	_AccountFreeMargin();
	double	_AccountFreeMarginCheck(String symbol, int cmd, double volume);
	double	_AccountFreeMarginMode();
	int		_AccountLeverage();
	double	_AccountMargin();
	String	_AccountName();
	int		_AccountNumber();
	double	_AccountProfit();
	String	_AccountServer();
	int		_AccountStopoutLevel();
	int		_AccountStopoutMode();
	double	_MarketInfo(String symbol, int type);
	int		_SymbolsTotal(int selected);
	String	_SymbolName(int pos, int selected);
	int		_SymbolSelect(String name, int select);
	double	_SymbolInfoDouble(String name, int prop_id);
	int		_SymbolInfoInteger(String name, int prop_id);
	String	_SymbolInfoString(String name, int prop_id);
	int		_RefreshRates();
	int		_iBars(String symbol, int timeframe);
	int		_iBarShift(String symbol, int timeframe, int datetime);
	double	_iClose(String symbol, int timeframe, int shift);
	double	_iHigh(String symbol, int timeframe, int shift);
	double	_iLow(String symbol, int timeframe, int shift);
	double	_iOpen(String symbol, int timeframe, int shift);
	int		_iHighest(String symbol, int timeframe, int type, int count, int start);
	int		_iLowest(String symbol, int timeframe, int type, int count, int start);
	int		_iTime(String symbol, int timeframe, int shift);
	int		_iVolume(String symbol, int timeframe, int shift);
	int		_OrderClose(int ticket, double lots, double price, int slippage);
	double	_OrderClosePrice();
	int		_OrderCloseTime();
	String	_OrderComment();
	double	_OrderCommission();
	int		_OrderDelete(int ticket);
	int		_OrderExpiration();
	double	_OrderLots();
	int		_OrderMagicNumber();
	int		_OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration);
	double	_OrderOpenPrice();
	int		_OrderOpenTime();
	double	_OrderProfit();
	int		_OrderSelect(int index, int select, int pool);
	int		_OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0);
	int		_OrdersHistoryTotal();
	double	_OrderStopLoss();
	int		_OrdersTotal();
	double	_OrderSwap();
	String	_OrderSymbol();
	double	_OrderTakeProfit();
	int		_OrderTicket();
	int		_OrderType();
	bool    _IsDemo();
	bool    _IsConnected();
	String	_GetLastError();
	int		_FindPriceTime(int sym, int tf, dword ts);
	
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

}

#endif
