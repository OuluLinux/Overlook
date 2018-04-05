#include "Overlook.h"

namespace Overlook {

void GameMatchCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	
	w.DrawImage(0, 0, id);
}

void GameOrderCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	
	w.DrawImage(0, 0, id);
}

GameCtrl::GameCtrl() {
	Add(split.SizePos());
	split << opplist << view;
	split.Horz();
	split.SetPos(3333);
	CtrlLayout(view);
	
	view.split << match << order;
	view.split.Horz();
	
	view.start << THISBACK(Start);
	view.stop << THISBACK(Stop);
	view.signal.Add("Suggested");
	view.signal.Add("Long");
	view.signal.Add("Short");
	view.fmlevel.SetData(0.6);
	view.maxsym.SetData(1);
	view.signal.SetIndex(0);
	
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
	
	opplist << THISBACK(SetView);
}

void GameCtrl::Data() {
	Game& game = GetGame();
	System& sys = GetSystem();
	
	game.Refresh();
	
	int cursor = opplist.GetCursor();
	
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
		opplist.Set(i, 9, o.is_active ? "Is active" : "");
	}
	opplist.SetSortColumn(5, false);
	
	
	opplist.WhenAction.Clear();
	if (cursor >= 0 && cursor < opplist.GetCount())
		opplist.SetCursor(cursor);
	opplist << THISBACK(SetView);
	
	
	// Gather order profit data
	
	
	// Check limits
	
}

void GameCtrl::SetView() {
	
	int cursor = opplist.GetCursor();
	if (cursor >= 0 && cursor < opplist.GetCount()) {
		int sym = opplist.Get(cursor, 0);
		int sys_sym = GetSystem().System::used_symbols_id[sym];
		GetOverlook().LoadDefaultProfile(sys_sym);
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
