#ifndef _Overlook_SimBroker_h_
#define _Overlook_SimBroker_h_

#include <plugin/libmt/libmt.h>

namespace Overlook {

class SimBroker : public Brokerage, Moveable<SimBroker> {
	System* sys;
	String currency;
	Time prev_cycle_time;
	double free_margin_level, min_free_margin_level, max_free_margin_level;
	double balance, equity, margin, margin_free, margin_call, margin_stop;
	double leverage, initial_balance;
	int selected;
	int lotsize;
	int order_counter;
	int basket_begin, cur_begin;
	bool lightweight;
	bool is_failed;
	
public:
	SimBroker();
	
	void Init(MetaTrader& mt, System& sys);
	void InitLightweight();
	void Clear();
	void Cycle();
	
	int FindSymbol(const String& symbol) const;
	int GetSignal(int symbol) const;
	int GetOpenOrderCount() const;
	double GetFreeMarginLevel() const;
	double GetInitialBalance() const {return initial_balance;}
	Time GetTime() const;
	bool IsFailed() const {return is_failed;}
	
	void SetFreeMarginLevel(double d);
	void SetFailed(bool b=true) {is_failed = b;}
	void PutSignal(int sym, int signal);
	void SetSignal(int sym, int signal) {signals[sym] = signal;}
	
	// MT4-like functions
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
	
};


class Exposure {
	SimBroker* broker;
	
	// Input
	Vector<double> sym_lots;
	Vector<int> sym_types;
	
	// Mid
	Vector<double> cur_volumes, idx_volumes;
	
	// Output
	
public:
	
	Exposure(SimBroker& sb);
	void Forward();
	void Backward();
	
	
};

}

#endif
