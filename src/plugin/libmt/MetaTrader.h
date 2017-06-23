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
	
	
	/*
	const String& GetAddr() const {return mainaddr;}
	int GetPort() const {return port;}
	String GetLastError();
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
	*/

	
	// Brokerage functions without caching
	virtual double	RealtimeAsk(int sym) {}
	virtual double	RealtimeBid(int sym) {}
	virtual int		OrderClose(int ticket, double lots, double price, int slippage) {}
	virtual double	OrderClosePrice() {}
	virtual int		OrderCloseTime() {}
	virtual String	OrderComment() {}
	virtual double	OrderCommission() {}
	virtual int		OrderDelete(int ticket) {}
	virtual int		OrderExpiration() {}
	virtual double	OrderLots() {}
	virtual int		OrderMagicNumber() {}
	virtual int		OrderModify(int ticket, double price, double stoploss, double takeprofit, int expiration) {}
	virtual double	OrderOpenPrice() {}
	virtual int		OrderOpenTime() {}
	virtual double	OrderProfit() {}
	virtual int		OrderSelect(int index, int select, int pool) {}
	virtual int		OrderSend(String symbol, int cmd, double volume, double price, int slippage, double stoploss, double takeprofit, int magic, int expiry=0) {}
	virtual int		OrdersHistoryTotal() {}
	virtual double	OrderStopLoss() {}
	virtual int		OrdersTotal() {}
	virtual double	OrderSwap() {}
	virtual String	OrderSymbol() {}
	virtual double	OrderTakeProfit() {}
	virtual int		OrderTicket() {}
	virtual int		OrderType() {}
	
	// Remote calling statistics
	int GetInputBytes() {return input;}
	int GetOutputBytes() {return output;}
	void AddOutputBytes(int i) {input += i;}
	void AddInputBytes(int i) {output += i;}

	// Remote calls without equal MQL counterpart
	bool IsResponding();
	int FindPriceTime(int sym, int tf, dword ts);
	Vector<Vector<Time> > GetLatestPriceTimes();
	Vector<Vector<Time> > GetEarliestPriceTimes();
	void GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history = false);
	String	GetSymbolsRaw();
	String	GetAskBidRaw();
	String	GetPricesRaw();
	String	GetHistoryOrdersRaw(int magic);
	String	GetOrdersRaw(int magic);
	void LoadOrderFile(String content, Vector<MTOrder>& orders, bool is_open);
	
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
