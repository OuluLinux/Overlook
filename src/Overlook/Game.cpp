#include "Overlook.h"
#include <Mmsystem.h>

namespace Overlook {

Game::Game() {
	for(int i = 0; i < USEDSYMBOL_COUNT; i++)
		signal[i] = 0;
}

void Game::Refresh() {
	lock.Enter();
	
	Automation& a = GetAutomation();
	MetaTrader& mt = GetMetaTrader();
	System& sys = GetSystem();
	
	int count = 12;
	
	VectorMap<int, double> volat_sum, spread_sum, trend_sum, opp_sum, best_cases;
	int max_level = -10;
	
	opps.SetCount(a.sym_count);
	for(int i = 0; i < a.sym_count; i++) {
		SlowAutomation& sa = a.slow[i];
		GameOpportunity& go = opps[i];
		int end = sa.dqn_cursor - 1;
		int begin = max(0, end - count);
		
		int sys_sym = sys.used_symbols_id[i];
		const Price& askbid = mt.GetAskBid()[sys_sym];
		double spread = askbid.ask - askbid.bid;
		double volat = 0;
		int prev_sig = 0;
		int trend = 0;
		for(int j = begin; j < end; j++) {
			
			double open  = sa.open_buf[j];
			double close = sa.open_buf[j+1];
			double diff = close - open;
			double absdiff = fabs(diff);
			
			volat += absdiff;
			int sig = close > open ? +1 : -1;
			if (sig == prev_sig)
				trend++;
			prev_sig = sig;
		}
		
		volat_sum.Add(i, volat / sa.point);
		spread_sum.Add(i, volat / spread);
		trend_sum.Add(i, trend);
		
		/*go.signal      = sa.GetSignal();
		go.slow_signal = sa.GetSlowSignal();
		go.level       = sa.GetLevel();*/
		sa.GetOutputValues(go.signal, go.level, go.slow_signal);
		opp_sum.Add(i, go.level);
		
		
		{
			double open  = sa.open_buf[end-2];
			double close = sa.open_buf[end];
			double best_case;
			if (open >= close) best_case = +(close / (open + spread) - 1.0);
			else               best_case = -(close / (open - spread) - 1.0);
			best_cases.Add(i, best_case);
		}
	}
	
	SortByValue(volat_sum,  StdGreater<double>());
	SortByValue(spread_sum, StdGreater<double>());
	SortByValue(best_cases, StdGreater<double>());
	SortByValue(trend_sum,  StdGreater<int>());
	SortByValue(opp_sum,    StdGreater<int>());
	
	GetMetaTrader()._GetOrders(0, false);
	
	GetExchangeSlots().Refresh();
	Slot* slot = GetExchangeSlots().FindCurrent();
	bool signal_changed = false;
	
	for(int i = 0; i < a.sym_count; i++) {
		GameOpportunity& go = opps[i];
		SlowAutomation& sa  = a.slow[i];
		
		go.vol_idx    = volat_sum.Find(i);
		go.spread_idx = spread_sum.Find(i);
		go.trend_idx  = trend_sum.Find(i);
		go.opp_idx    = opp_sum.Find(i);
		go.bcase_idx  = best_cases.Find(i);
		
		if (go.spread_idx < 6)
			max_level = max(max_level, go.level);
		
		int sys_sym = sys.used_symbols_id[i];
		if (slot) {
			String sym = sys.GetSymbol(sys_sym);
			String a = sym.Left(3);
			String b = sym.Right(3);
			
			go.is_active = false;
			if (slot->currencies.Find(a) != -1) go.is_active = true;
			if (slot->currencies.Find(b) != -1) go.is_active = true;
		}
		
		double lots = 0.0, profit = 0.0;
		const auto& open_orders = mt.GetOpenOrders();
		for(int j = 0; j < open_orders.GetCount(); j++) {
			const Order& o = open_orders[j];
			if (o.symbol == sys_sym) {
				lots += o.volume;
				profit = o.profit;
			}
		}
		go.profit = lots > 0.0 ? profit / (lots / 0.01) : 0.0;
		
		
		if (go.prev_lots == 0.0 && lots > 0.0) {
			
			// Clear drawer
			go.open_profits.SetCount(0);
			
			// Set opening time
			go.opened = GetUtcTime();
		}
		go.prev_lots = lots;
		
		
		if (lots > 0.0) {
			// Check SL/TP limits
			if (go.profit > tplimit || go.profit < sllimit) {
				signal[i] = 0;
				signal_changed = true;
			}
			
			// Check time limits
			int length = (GetUtcTime().Get() - go.opened.Get()) / 60;
			if (length >= timelimit) {
				signal[i] = 0;
				signal_changed = true;
			}
			
			go.open_profits.Add(go.profit);
		}
		else {
			if (autostart && go.spread_idx < spread_limit && go.level > 0) {
				signal[i] = go.signal;
				signal_changed = true;
			}
		}
	}
	
	if (signal_changed)
		sys.RefreshReal();
	
	
	if (max_level != prev_max_level && max_level >= 10)
		PlaySound(TEXT("alert.wav"), NULL, SND_ASYNC | SND_FILENAME);
	prev_max_level = max_level;
	
	lock.Leave();
}

}
