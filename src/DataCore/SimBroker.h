#ifndef _DataCore_SimBroker_h_
#define _DataCore_SimBroker_h_

#include <CoreUtils/CoreUtils.h>
#include <plugin/libmt/libmt.h>
#include <RefCore/RefCore.h>

namespace DataCore {
using namespace RefCore;

class SimBroker : public Brokerage, Moveable<SimBroker> {
	Vector<Order> orders, history_orders;
	Vector<Symbol> symbols;
	Index<String> symbol_idx;
	Vector<Price> askbid;
	Vector<PriceTf> pricetf;
	SlotPtr src, change;
	String currency;
	double free_margin_level, min_free_margin_level, max_free_margin_level;
	double balance, equity, margin, margin_free, margin_call, margin_stop;
	double leverage;
	int selected;
	int lotsize;
	int order_counter;
	
public:
	SimBroker();
	
	void Init(MetaTrader& mt);
	void Cycle();
	void ClearWorkingMemory();
	
	int FindSymbol(const String& symbol) const;
	bool IsZeroSignal() const;
	int GetSignal(int symbol) const;
	int GetOpenOrderCount() const;
	double GetWorkingMemoryChange() const;
	double GetPreviousCycleChange() const;
	double GetFreeMarginLevel() const;
	Time GetTime() const;
	
	void SetSignal(int sym, int signal);
	void SetFreeMarginLevel(double d);
	
	
	// MT4-like functions
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
	virtual String	SymbolName(int pos, int selected=0);
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
	virtual int		OrderSelect(int index, int select, int pool=MODE_TRADES);
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
	virtual void GetOrders(ArrayMap<int, Order>& orders, Vector<int>& open, int magic, bool force_history = false);
	
};




}

#endif
