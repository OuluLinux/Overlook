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
	
	opplist.AddIndex();
	opplist.AddColumn("Symbol");
	opplist.AddColumn("Volatility");
	opplist.AddColumn("Spread");
	opplist.AddColumn("Trend");
	opplist.AddColumn("Average");
	opplist.AddColumn("Level");
	opplist.AddColumn("Signal");
}

void GameCtrl::Data() {
	Game& game = GetGame();
	System& sys = GetSystem();
	
	game.Refresh();
	
	int cursor = opplist.GetCursor();
	
	for(int i = 0; i < game.opps.GetCount(); i++) {
		GameOpportunity& o = game.opps[i];
		
		opplist.Set(i, 0, i);
		opplist.Set(i, 1, sys.used_symbols[i]);
		opplist.Set(i, 2, o.vol_idx);
		opplist.Set(i, 3, o.spread_idx);
		opplist.Set(i, 4, o.trend_idx);
		opplist.Set(i, 5, o.GetAverageIndex());
		opplist.Set(i, 6, o.level);
		opplist.Set(i, 7, o.signal ? "Short" : "Long");
	}
	opplist.SetSortColumn(4, false);
	
	if (cursor >= 0 && cursor < opplist.GetCount())
		opplist.SetCursor(cursor);
}

}
