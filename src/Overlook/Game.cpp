#include "Overlook.h"
#include <Mmsystem.h>

namespace Overlook {

Game::Game() {
	for(int i = 0; i < USEDSYMBOL_COUNT; i++)
		signal[i] = 0;
}

void Game::Refresh() {
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
		
		go.signal     = sa.GetSignal();
		go.level      = sa.GetLevel();
		opp_sum.Add(i, go.level);
		
		max_level = max(max_level, go.level);
		
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
	
	
	GetExchangeSlots().Refresh();
	Slot* slot = GetExchangeSlots().FindCurrent();
	
	for(int i = 0; i < a.sym_count; i++) {
		GameOpportunity& go = opps[i];
		SlowAutomation& sa  = a.slow[i];
		
		go.created    = GetUtcTime();
		go.vol_idx    = volat_sum.Find(i);
		go.spread_idx = spread_sum.Find(i);
		go.trend_idx  = trend_sum.Find(i);
		go.opp_idx    = opp_sum.Find(i);
		go.bcase_idx  = best_cases.Find(i);
		
		
		if (slot) {
			int sys_sym = sys.used_symbols_id[i];
			String sym = sys.GetSymbol(sys_sym);
			String a = sym.Left(3);
			String b = sym.Right(3);
			
			go.is_active = false;
			if (slot->currencies.Find(a) != -1) go.is_active = true;
			if (slot->currencies.Find(b) != -1) go.is_active = true;
		}
		
	}
	
	
	if (max_level != prev_max_level && max_level > 0)
		PlaySound(TEXT("alert.wav"), NULL, SND_ASYNC | SND_FILENAME);
	prev_max_level = max_level;
}

}
