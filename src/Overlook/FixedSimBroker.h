#if 0

#ifndef _Overlook_FixedSimBroker_h_
#define _Overlook_FixedSimBroker_h_

namespace Overlook {

struct FixedOrder {
	double open = 0.0;
	double close = 0.0;
	double volume = 0.0;
	double profit = 0.0;
	int type = -1;
	int is_open = 0;
	
	void Serialize(Stream& s) {
		s % open % close % volume % profit % type % is_open;
	}
};


struct FixedSimBroker {
	#define ORDERS_PER_SYMBOL	7
	#define MAX_ORDERS			(ORDERS_PER_SYMBOL * SYM_COUNT)
	
	FixedOrder order[MAX_ORDERS];
	double spread_points[SYM_COUNT];
	int signal[SYM_COUNT];
	int proxy_id[SYM_COUNT];
	int proxy_base_mul[SYM_COUNT];
	bool signal_freezed[SYM_COUNT];
	double balance = 0.0;
	double equity = 0.0;
	double part_balance = 0.0;
	double part_equity = 0.0;
	double begin_equity = 0.0;
	double profit_sum = 0.0;
	double loss_sum = 0.0;
	double free_margin_level = 0.80;
	double leverage = 1000.0;
	int order_count = 0;
	int part_sym_id = -1;
	bool init = false;
	
		
	FixedSimBroker();
	void Reset();
	double RealtimeBid(int pos, int sym_id) const;
	double RealtimeAsk(int pos, int sym_id) const;
	double GetCloseProfit(int sym_id, const FixedOrder& o, int pos) const;
	void OrderSend(int sym_id, int type, double volume, double price, int pos);
	void OrderClose(int sym_id, double lots, FixedOrder& order, int pos);
	void CloseAll(int pos);
	double GetMargin(int pos, int sym_id, double volume);
	bool Cycle(int pos);
	void RefreshOrders(int pos);
	double GetSpreadCost(int pos) const;
	void Serialize(Stream& s) {
		for(int i = 0; i < MAX_ORDERS; i++) s % order[i];
		for(int i = 0; i < SYM_COUNT; i++) s % spread_points[i];
		for(int i = 0; i < SYM_COUNT; i++) s % signal[i];
		for(int i = 0; i < SYM_COUNT; i++) s % proxy_id[i];
		for(int i = 0; i < SYM_COUNT; i++) s % proxy_base_mul[i];
		for(int i = 0; i < SYM_COUNT; i++) s % signal_freezed[i];
		s % balance % equity % part_balance % part_equity % begin_equity %
		    profit_sum % loss_sum % free_margin_level % leverage % order_count % part_sym_id;
	}
	
	double AccountEquity() const {return equity;}
	double PartialEquity() const {return part_equity;}
	double GetDrawdown() const {double sum = profit_sum + loss_sum; return sum > 0.0 ? loss_sum / sum : 1.0;}
	void SetSignal(int sym_id, int sig) {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); signal[sym_id] = sig;}
	void SetSignalFreeze(int sym_id, bool b) {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); signal_freezed[sym_id] = b;}
	int GetSignal(int sym_id) const {ASSERT(sym_id >= 0 && sym_id < SYM_COUNT); return signal[sym_id];}
};

}

#endif
#endif
