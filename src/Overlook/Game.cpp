#include "Overlook.h"

namespace Overlook {

Game::Game() {
	
}

void Game::Refresh() {
	Automation& a = GetAutomation();
	MetaTrader& mt = GetMetaTrader();
	System& sys = GetSystem();
	
	int count = 12;
	
	VectorMap<int, double> volat_sum, spread_sum, trend_sum;
	
	for(int i = 0; i < a.sym_count; i++) {
		SlowAutomation& sa = a.slow[i];
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
	}
	
	SortByValue(volat_sum,  StdGreater<double>());
	SortByValue(spread_sum, StdGreater<double>());
	SortByValue(trend_sum,  StdGreater<int>());
	
	
	opps.SetCount(a.sym_count);
	for(int i = 0; i < a.sym_count; i++) {
		GameOpportunity& go = opps[i];
		SlowAutomation& sa  = a.slow[i];
		
		go.created    = GetUtcTime();
		go.vol_idx    = volat_sum.Find(i);
		go.spread_idx = spread_sum.Find(i);
		go.trend_idx  = trend_sum.Find(i);
		
		go.signal     = sa.GetSignal();
		go.level      = sa.GetLevel();
	}
	
	
}

}
