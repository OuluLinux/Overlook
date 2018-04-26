#include "Overlook.h"

namespace Overlook {

void RealtimeMatchCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Automation& a = GetAutomation();
	
	if (sym >= 0 && sym < a.sym_count) {
		SymAutomation& sa = a.slow[sym];
		int count = 6;
		int begin = sa.most_matching_pos;
		int end = min(begin + count, sa.processbits_cursor);
		
		tmp.SetCount(end - begin);
		for(int i = begin, j = 0; i < end; i++, j++) {
			tmp[j] = sa.open_buf[i];
		}
		DrawVectorPolyline(id, sz, tmp, polyline);
	}
	
	w.DrawImage(0, 0, id);
}

void RealtimeOrderCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Realtime& game = GetRealtime();
	if (sym >= 0 && sym < game.opps.GetCount()) {
		RealtimeOpportunity& go = game.opps[sym];
		DrawVectorPolyline(id, sz, go.open_profits, polyline);
	}
	
	w.DrawImage(0, 0, id);
}

RealtimeCtrl::RealtimeCtrl() {
	Add(split.SizePos());
	split << opplist << view;
	split.Horz();
	split.SetPos(3400);
	CtrlLayout(view);
	
	view.split << match << order;
	view.split.Horz();
	
	view.start << THISBACK(Start);
	view.stop << THISBACK(Stop);
	view.signal.Add("Suggested");
	view.signal.Add("Long");
	view.signal.Add("Short");
	view.signal.SetIndex(0);
	
	view.fmlevel << THISBACK(SetValues);
	view.maxsym << THISBACK(SetValues);
	view.tp << THISBACK(SetValues);
	view.sl << THISBACK(SetValues);
	view.timelimit << THISBACK(SetValues);
	view.spreadlimit << THISBACK(SetValues);
	view.autostart << THISBACK(SetValues);
	view.inversesig << THISBACK(SetValues);
	
	opplist.AddIndex();
	opplist.AddColumn("Symbol");
	opplist.AddColumn("Volatility");
	opplist.AddColumn("Spread");
	opplist.AddColumn("Trend");
	opplist.AddColumn("Best case");
	opplist.AddColumn("Average");
	opplist.AddColumn("Level");
	opplist.AddColumn("Signal");
	opplist.AddColumn("Active");
	opplist.AddColumn("Profit");
	
	opplist << THISBACK(SetView);
	
	GetValues();
}

void RealtimeCtrl::SetValues() {
	Realtime& game = GetRealtime();
	
	game.free_margin_level   = view.fmlevel.GetData();
	game.max_symbols         = view.maxsym.GetData();
	game.tplimit             = view.tp.GetData();
	game.sllimit             = view.sl.GetData();
	game.timelimit           = view.timelimit.GetData();
	game.spread_limit        = view.spreadlimit.GetData();
	game.autostart           = view.autostart.Get();
	game.inversesig          = view.inversesig.Get();
}

void RealtimeCtrl::GetValues() {
	Realtime& game = GetRealtime();
	
	view.fmlevel			.SetData(game.free_margin_level);
	view.maxsym				.SetData(game.max_symbols);
	view.tp					.SetData(game.tplimit);
	view.sl					.SetData(game.sllimit);
	view.timelimit			.SetData(game.timelimit);
	view.spreadlimit		.SetData(game.spread_limit);
	view.autostart			.Set(game.autostart);
	view.inversesig			.Set(game.inversesig);
}

void RealtimeCtrl::Data() {
	Realtime& game = GetRealtime();
	System& sys = GetSystem();
	
	game.Refresh();
	
	int cursor = opplist.GetCursor();
	int sym = -1;
	if (cursor >= 0 && cursor < opplist.GetCount())
		sym = opplist.Get(cursor, 0);
	
	for(int i = 0; i < game.opps.GetCount(); i++) {
		RealtimeOpportunity& o = game.opps[i];
		int sys_sym = GetSystem().System::used_symbols_id[i];
		
		opplist.Set(i, 0, i);
		opplist.Set(i, 1, sys.GetSymbol(sys_sym));
		opplist.Set(i, 2, o.vol_idx);
		opplist.Set(i, 3, o.spread_idx);
		opplist.Set(i, 4, o.trend_idx);
		opplist.Set(i, 5, o.bcase_idx);
		opplist.Set(i, 6, o.GetAverageIndex());
		opplist.Set(i, 7, o.level);
		opplist.Set(i, 8, o.signal ? "Short" : "Long");
		opplist.Set(i, 9, o.is_active ? "Is active" : "");
		opplist.Set(i, 10, o.profit);
	}
	opplist.SetSortColumn(5, false);
	
	
	opplist.WhenAction.Clear();
	if (sym != -1)
		for(int i = 0; i < opplist.GetCount(); i++)
			if (opplist.Get(i, 0) == sym)
				opplist.SetCursor(i);
	opplist << THISBACK(SetView);
	
	order.sym = sym;
	order.Refresh();
	
	match.sym = sym;
	match.Refresh();
}

void RealtimeCtrl::SetView() {
	Realtime& game = GetRealtime();
	
	int cursor = opplist.GetCursor();
	if (cursor >= 0 && cursor < opplist.GetCount()) {
		int sym = opplist.Get(cursor, 0);
		int sys_sym = GetSystem().System::used_symbols_id[sym];
		GetOverlook().LoadDefaultProfile(sys_sym);
		
		RealtimeOpportunity& o = game.opps[sym];
		view.profit.SetLabel(DblStr(o.profit));
	}
}

void RealtimeCtrl::Start() {
	Realtime& game = GetRealtime();
	System& sys = GetSystem();
	int cursor = opplist.GetCursor();
	if (cursor < 0) return;
	int sym = opplist.Get(cursor, 0);
	RealtimeOpportunity& o = game.opps[sym];
	
	int signal = 0;
	switch (view.signal.GetIndex()) {
		case 0: signal = o.signal ? -1 : +1; break;
		case 1: signal = +1; break;
		case 2: signal = -1; break;
	}
	
	if (game.sllimit > -0.5) {
		game.sllimit = -0.5;
		GetValues();
	}
	
	game.free_margin_level = view.fmlevel.GetData();
	game.free_margin_scale = view.maxsym.GetData();
	game.signal[sym] = signal;
	
	sys.RefreshReal();
}

void RealtimeCtrl::Stop() {
	Realtime& game = GetRealtime();
	System& sys = GetSystem();
	int cursor = opplist.GetCursor();
	if (cursor < 0) return;
	int sym = opplist.Get(cursor, 0);
	RealtimeOpportunity& o = game.opps[sym];
	
	game.signal[sym] = 0;
	
	sys.RefreshReal();
}




void RealtimeBrokerCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Realtime& game = GetRealtime();
	DrawVectorPolyline(id, sz, game.sb_equity, game.sb_ma1, game.sb_ma2, polyline);
	
	w.DrawImage(0, 0, id);
}

PerformanceCtrl::PerformanceCtrl() {
	Add(split.SizePos());
	split.Horz();
	
	split << view << broker;
	split.SetPos(2000);
	CtrlLayout(view);
	
	GetValues();
	view.ma1 << THISBACK(SetValues);
	view.ma2 << THISBACK(SetValues);
	view.ma1.Disable();
	view.ma2.Disable();
	view.optimize << THISBACK(Optimize);
	
}

void PerformanceCtrl::Data() {
	Realtime& game = GetRealtime();
	game.Refresh();
	
	
	broker.Refresh();
}

void PerformanceCtrl::GetValues() {
	Realtime& game = GetRealtime();
	
	view.ma1.SetData(game.ma1);
	view.ma2.SetData(game.ma2);
}

void PerformanceCtrl::SetValues() {
	Realtime& game = GetRealtime();
	
	game.ma1 = view.ma1.GetData();
	game.ma2 = view.ma2.GetData();
}

void PerformanceCtrl::Optimize() {
	Realtime& game = GetRealtime();
	game.ma_lock.Enter();
	
	const Vector<double>& sb_equity = game.sb_equity;
	Vector<double>& sb_ma1 = game.sb_ma1;
	Vector<double>& sb_ma2 = game.sb_ma2;
	int last = sb_equity.GetCount() - 1;
	
	double max_sum = -DBL_MAX;
	int max_a, max_b;
	
	for(int i = 6; i <= 10; i++) {
		int a = 1 << i;
		for(int j = i+1; j <= 11; j++) {
			int b = 1 << j;
			
			OnlineAverageWindow1 a_ma, b_ma;
			a_ma.SetPeriod(a);
			b_ma.SetPeriod(b);
			
			bool prev_allow_real = false;
			double eq_open;
			double sum = 0.0;
			double prev_b_mean = 0.0;
			for(int k = 0; k <= last; k++) {
				double d = sb_equity[k];
				
				
				a_ma.Add(d);
				b_ma.Add(d);
				
				double a_mean = a_ma.GetMean();
				double b_mean = b_ma.GetMean();
				
				bool allow_real = a_mean >= b_mean && b_mean >= prev_b_mean;
				if (k == last) allow_real = false;
				
				if (allow_real) {
					if (!prev_allow_real)
						eq_open = d;
				} else {
					if (prev_allow_real)
						sum += d - eq_open;
				}
				
				prev_b_mean = b_mean;
				prev_allow_real = allow_real;
			}
			
			if (sum > max_sum) {
				max_a = a;
				max_b = b;
				max_sum = sum;
			}
		}
	}
	
	game.ma1 = max_a;
	game.ma2 = max_b;
	view.ma1.SetData(max_a);
	view.ma2.SetData(max_b);
	
	{
		game.ma1_av.Clear();
		game.ma2_av.Clear();
		
		game.ma1_av.SetPeriod(max_a);
		game.ma2_av.SetPeriod(max_b);
		
		double sum = 0.0;
		for(int k = 0; k <= last; k++) {
			double d = sb_equity[k];
			
			game.ma1_av.Add(d);
			game.ma2_av.Add(d);
			
			double a_mean = game.ma1_av.GetMean();
			double b_mean = game.ma2_av.GetMean();
			
			if (k >= max_a)
				sb_ma1[k] = a_mean;
			else
				sb_ma1[k] = d;
			
			if (k >= max_b)
				sb_ma2[k] = b_mean;
			else
				sb_ma2[k] = d;
		}
	}
	
	
	game.ma_lock.Leave();
}

}