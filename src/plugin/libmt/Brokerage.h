#ifndef _plugin_libmt_Brokerage_h_
#define _plugin_libmt_Brokerage_h_

namespace libmt {

class Brokerage {
	
protected:
	Vector<Order> orders, history_orders;
	Index<String> skipped_currencies;
	Index<String>	symbol_idx;
	
	String account_name, account_server, account_currency;
	String last_error;
	double free_margin_level, min_free_margin_level, max_free_margin_level;
	double balance, equity, margin, margin_free, margin_call, margin_stop;
	double leverage, initial_balance;
	int cur_begin;
	int account_currency_id;
	int account_id;
	int selected;
	bool demo, connected, simulation;
	bool init_success;
	bool is_failed;
	
	Vector<Vector<int> > basket_symbols;
	Vector<Symbol> symbols;
	Vector<Price> askbid;
	Vector<PriceTf> pricetf;
	VectorMap<String, Currency> currencies;
	Vector<int> indices;
	Vector<int> signals;
	Vector<Asset> assets;
	Mutex current_price_lock;
	
	ArrayMap<int, String> periodstr;
	int tf_h1_id;
	
	// Exposure temp vars
	Vector<double> cur_volumes, idx_volumes, cur_rates, cur_base_values, idx_rates, idx_base_values;
	
	
public:
	enum {MODE_OPEN, MODE_LOW, MODE_HIGH, MODE_CLOSE, MODE_VOLUME, MODE_TIME};
	
	
	Brokerage();
	
	void operator=(const Brokerage& b);
	
	void ForwardExposure();
	void BackwardExposure();
	void PutSignal(int sym, int signal);
	void SetSignal(int sym, int signal) {signals[sym] = signal;}
	bool IsFailed() const {return is_failed;}
	void SetFailed(bool b=true) {is_failed = b;}
	
	const Vector<Order>&	GetOpenOrders() const {return orders;}
	const Vector<Order>&	GetHistoryOrders() const {return history_orders;}
	const Vector<Symbol>&	GetSymbols() const {return symbols;}
	const Vector<Price>&	GetAskBid() const {return askbid;}
	const Vector<PriceTf>&	GetTickData() const {return pricetf;}
	const Vector<Asset>&	GetAssets() const {return assets;}
	const Symbol& GetSymbol(int i) const {return symbols[i];}
	int GetTimeframe(int i) {return periodstr.GetKey(i);}
	const Currency& GetCurrency(int i) const {return currencies[i];}
	String GetTimeframeString(int i) {return periodstr[i];}
	int GetTimeframeCount() {return periodstr.GetCount();}
	int GetTimeframeIdH1() {return tf_h1_id;}
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetCurrencyCount() const {return currencies.GetCount();}
	int GetIndexId(int i) const {return indices[i];}
	int GetIndexCount() const {return indices.GetCount();}
	
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
	bool    IsDemo() {return demo;}
	bool    IsConnected() {return connected;}
	
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
	virtual int		RefreshRates() = 0;
	virtual Time	GetTime() const = 0;
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
