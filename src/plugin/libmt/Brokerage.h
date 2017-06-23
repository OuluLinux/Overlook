#ifndef _plugin_libmt_Brokerage_h_
#define _plugin_libmt_Brokerage_h_

namespace libmt {

class Brokerage {
	
	Vector<Order> orders, history_orders;
	Index<String> skipped_currencies;
	Index<String>	symbol_idx;
	
	String account_id, account_name, account_server, account_currency;
	String last_error;
	double balance, equity, margin, freemargin, leverage;
	bool demo, connected, simulation;
	bool init_success;
	
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
	
	
	Brokerage() {}
	
	void ForwardExposure();
	void BackwardExposure();
	
	const Vector<Order>&	GetOpenOrders() {return orders;}
	const Vector<Order>&	GetHistoryOrders() {return history_orders;}
	const Vector<Symbol>&	GetSymbols();
	const Vector<Price>&	GetAskBid();
	const Vector<PriceTf>&	GetTickData();
	const Symbol& GetSymbol(int i) const {return symbols[i];}
	
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
	String	SymbolName(int pos, int selected=0);
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
	bool    IsDemo();
	bool    IsConnected();
	
	virtual double	RealtimeAsk(int sym) = 0;
	virtual double	RealtimeBid(int sym) = 0;
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
	
};

}

#endif
