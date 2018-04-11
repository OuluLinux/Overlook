#include "Overlook.h"

namespace Overlook {

void GameMatchCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Automation& a = GetAutomation();
	
	if (sym >= 0 && sym < a.sym_count) {
		SlowAutomation& sa = a.slow[sym];
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

void GameOrderCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	Game& game = GetGame();
	if (sym >= 0 && sym < game.opps.GetCount()) {
		GameOpportunity& go = game.opps[sym];
		DrawVectorPolyline(id, sz, go.open_profits, polyline);
	}
	
	w.DrawImage(0, 0, id);
}

GameCtrl::GameCtrl() {
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
	opplist.AddColumn("Slow Signal");
	opplist.AddColumn("Active");
	opplist.AddColumn("Profit");
	
	opplist << THISBACK(SetView);
	
	GetValues();
}

void GameCtrl::SetValues() {
	Game& game = GetGame();
	
	game.free_margin_level   = view.fmlevel.GetData();
	game.max_symbols         = view.maxsym.GetData();
	game.tplimit             = view.tp.GetData();
	game.sllimit             = view.sl.GetData();
	game.timelimit           = view.timelimit.GetData();
	game.spread_limit        = view.spreadlimit.GetData();
	game.autostart           = view.autostart.Get();
	game.inversesig          = view.inversesig.Get();
}

void GameCtrl::GetValues() {
	Game& game = GetGame();
	
	view.fmlevel			.SetData(game.free_margin_level);
	view.maxsym				.SetData(game.max_symbols);
	view.tp					.SetData(game.tplimit);
	view.sl					.SetData(game.sllimit);
	view.timelimit			.SetData(game.timelimit);
	view.spreadlimit		.SetData(game.spread_limit);
	view.autostart			.Set(game.autostart);
	view.inversesig			.Set(game.inversesig);
}

void GameCtrl::Data() {
	Game& game = GetGame();
	System& sys = GetSystem();
	
	game.Refresh();
	
	int cursor = opplist.GetCursor();
	int sym = -1;
	if (cursor >= 0 && cursor < opplist.GetCount())
		sym = opplist.Get(cursor, 0);
	
	for(int i = 0; i < game.opps.GetCount(); i++) {
		GameOpportunity& o = game.opps[i];
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
		opplist.Set(i, 9, o.slow_signal ? "Short" : "Long");
		opplist.Set(i, 10, o.is_active ? "Is active" : "");
		opplist.Set(i, 11, o.profit);
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

void GameCtrl::SetView() {
	Game& game = GetGame();
	
	int cursor = opplist.GetCursor();
	if (cursor >= 0 && cursor < opplist.GetCount()) {
		int sym = opplist.Get(cursor, 0);
		int sys_sym = GetSystem().System::used_symbols_id[sym];
		GetOverlook().LoadDefaultProfile(sys_sym);
		
		GameOpportunity& o = game.opps[sym];
		view.profit.SetLabel(DblStr(o.profit));
	}
}

void GameCtrl::Start() {
	Game& game = GetGame();
	System& sys = GetSystem();
	int cursor = opplist.GetCursor();
	if (cursor < 0) return;
	int sym = opplist.Get(cursor, 0);
	GameOpportunity& o = game.opps[sym];
	
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

void GameCtrl::Stop() {
	Game& game = GetGame();
	System& sys = GetSystem();
	int cursor = opplist.GetCursor();
	if (cursor < 0) return;
	int sym = opplist.Get(cursor, 0);
	GameOpportunity& o = game.opps[sym];
	
	game.signal[sym] = 0;
	
	sys.RefreshReal();
}

}
