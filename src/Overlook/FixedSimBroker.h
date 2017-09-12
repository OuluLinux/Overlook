#ifndef _Overlook_FixedSimBroker_h_
#define _Overlook_FixedSimBroker_h_


struct FixedOrder {
	double open = 0.0;
	double close = 0.0;
	double volume = 0.0;
	double profit = 0.0;
	int type = -1;
	int is_open = 0;
};

struct SingleFixedSimBroker {
	FixedOrder order;
	double balance = 0.0;
	double equity = 0.0;
	double begin_equity = 0.0;
	double spread_points = 0.001;
	double profit_sum = 0.0;
	double loss_sum = 0.0;
	int order_count = 0;
	int sym_id = -1;
	int proxy_id = -1;
	int proxy_base_mul = 0;
	
		
	SingleFixedSimBroker();
	void Reset();
	double RealtimeBid(const Snapshot& snap, int sym_id) const;
	double RealtimeAsk(const Snapshot& snap, int sym_id) const;
	double GetCloseProfit(const Snapshot& snap) const;
	void OrderSend(int type, double volume, double price);
	void OrderClose(const Snapshot& snap);
	void Cycle(int signal, const Snapshot& snap);
	void RefreshOrders(const Snapshot& snap);
	
	double AccountEquity() const {return equity;}
	double GetDrawdown() const {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
};


struct FixedSimBroker {
	#define ORDERS_PER_SYMBOL	7
	#define MAX_ORDERS			(ORDERS_PER_SYMBOL * SYM_COUNT)
	
	FixedOrder order[MAX_ORDERS];
	int signal[SYM_COUNT];
	int proxy_id[SYM_COUNT];
	int proxy_base_mul[SYM_COUNT];
	long double balance = 0.0;
	long double equity = 0.0;
	long double part_balance = 0.0;
	long double part_equity = 0.0;
	long double begin_equity = 0.0;
	long double profit_sum = 0.0;
	long double loss_sum = 0.0;
	double spread_points[SYM_COUNT];
	double free_margin_level = 0.80;
	double leverage = 1000.0;
	int order_count = 0;
	int part_sym_id = -1;
	int min_active_symbols = 4;
	
		
	FixedSimBroker();
	void Reset();
	double RealtimeBid(const Snapshot& snap, int sym_id) const;
	double RealtimeAsk(const Snapshot& snap, int sym_id) const;
	long double GetCloseProfit(int sym_id, const FixedOrder& o, const Snapshot& snap) const;
	void OrderSend(int sym_id, int type, double volume, double price, const Snapshot& snap);
	void OrderClose(int sym_id, double lots, FixedOrder& order, const Snapshot& snap);
	void CloseAll(const Snapshot& snap);
	double GetMargin(const Snapshot& snap, int sym_id, double volume);
	bool Cycle(const Snapshot& snap);
	void RefreshOrders(const Snapshot& snap);
	
	long double AccountEquity() const {return equity;}
	long double PartialEquity() const {return part_equity;}
	double GetDrawdown() const {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
	void SetSignal(int sym_id, int sig) {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); signal[sym_id] = sig;}
	int GetSignal(int sym_id) const {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); return signal[sym_id];}
};


#endif
