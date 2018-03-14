#include "Overlook.h"


namespace Overlook {
using namespace Upp;

void BooleansDraw::Paint(Draw& w) {
	System& sys = GetSystem();
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	/*if (cursor >= 0) {
		
		int w = Snap::src_row_size;
		int h = sys.GetSymbolCount();
		
		double xstep = (double)sz.cx / w;
		double ystep = (double)sz.cy / h;
		
		for(int i = 0; i < h; i++) {
			if (i >= sys.data.GetCount()) continue;
			
			auto& main_booleans = sys.data[i][tf].main_booleans;
			
			int count = main_booleans.GetCount();
			if (count == 0) continue;
			const Snap& snap = main_booleans[min(count-1, cursor)];
			
			int y0 = i * ystep;
			int y1 = (i + 1) * ystep;
			for(int j = 0; j < w; j++) {
				int x0 = j * xstep;
				int x1 = (j + 1) * xstep;
				
				bool b = snap.Get(j);
				if (b)
					id.DrawRect(x0, y0, x1-x0, y1-y0, Black());
				
			}
		}
	}*/
	
	w.DrawImage(0, 0, id);
}

AutomationCtrl::AutomationCtrl() {
	
	Add(toggle.TopPos(2, 26).LeftPos(2,96));
	Add(hsplit.HSizePos(100).VSizePos());
	hsplit.SetPos(2000);
	hsplit << symbols << symctrl_place;
	
	toggle.SetLabel("Start");
	toggle <<= THISBACK(Start);
	
	symbols.AddIndex();
	symbols.AddIndex();
	symbols.AddColumn("Symbol");
	symbols.AddColumn("Tf");
	symbols.AddColumn("Phase");
	symbols.AddColumn("Processed");
	symbols <<= THISBACK(Data);
	
	symctrl_place.Add(tabs.SizePos());
	
	tabs.WhenSet << THISBACK(Data);
	tabs.Add(boolctrl, "Bits");
	tabs.Add(boolctrl);
	tabs.Add(stats, "Stats");
	tabs.Add(stats);
	tabs.Add(try_strands, "Try strands");
	tabs.Add(try_strands);
	tabs.Add(catch_strands, "Catch strands");
	tabs.Add(catch_strands);
	
	slider.MinMax(0,1);
	slider << THISBACK(Data);
	
	boolctrl.Add(bools.HSizePos().VSizePos(0,30));
	boolctrl.Add(slider.HSizePos().BottomPos(0,30));
	
	stats.AddColumn("Bit combination");
	stats.AddColumn("Percent");
	stats.AddColumn("Index");
	
	try_strands.AddColumn("Index");
	try_strands.AddColumn("Signal bit");
	try_strands.AddColumn("Bit list");
	try_strands.AddColumn("Result");
	
	catch_strands.AddColumn("Index");
	catch_strands.AddColumn("Signal bit");
	catch_strands.AddColumn("Bit list");
	catch_strands.AddColumn("Result");
	
	
	PostCallback(THISBACK(Start));
}

void AutomationCtrl::Data() {
	System& sys = GetSystem();
	
	/*
	int row = 0;
	for(int i = 0; i < sys.jobs.GetCount(); i++) {
		Job& job = *sys.jobs[i];
		int sym = job.GetSymbol();
		int tf = job.GetTf();
		symbols.Set(i, 0, sym);
		symbols.Set(i, 1, tf);
		symbols.Set(i, 2, sym >= 0 ? sys.GetSymbol(sym) : "Account");
		symbols.Set(i, 3, sys.GetPeriod(tf));
		symbols.Set(i, 4, job.GetPhaseString());
		symbols.Set(i, 5, job.GetProgress());
		symbols.SetDisplay(i, 3, ProgressDisplay());
	}
	
	
	int cursor = symbols.GetCursor();
	if (cursor == -1) return;
	
	int sym = symbols.Get(cursor, 0);
	int tf  = symbols.Get(cursor, 1);
	
	
	Job& sym_data = sym >= 0 ? (Job&)sys.data[sym][tf] : (Job&)sys.account[tf];
	auto& main_booleans = sym_data.main_booleans;
	
	int tab = tabs.Get();
	if (tab == 0) {
		slider.MinMax(0, main_booleans.GetCount() - 1);
		bools.tf = tf;
		bools.cursor = slider.GetData();
		bools.Refresh();
	}
	else if (tab == 1) {
		auto& stat_in = sym_data.main_stats;
		
		
		int id = 0, row = 0;
		for (int i = 0; i < SourceImage::row_size; i++) {
			String key;
			key << i << " ";
			
			for(int i = 0; i < 2; i++) {
				SnapStats& ss = stat_in.data[id][i];
				
				stats.Set(row, 0, key + IntStr(i));
				if (ss.total > 0) {
					stats.Set(row, 1, IntStr(ss.actual * 100 / ss.total) + "%");
					stats.Set(row, 2, abs(ss.actual * 200 / ss.total - 100));
				} else {
					stats.Set(row, 1, "");
					stats.Set(row, 2, "");
				}
				row++;
			}
			
			id++;
		}
		
		stats.SetSortColumn(2, true);
		
	}
	else if (tab == 2) {
		for(int i = 0; i < sym_data.try_strands.GetCount(); i++) {
			try_strands.Set(i, 0, i);
			try_strands.Set(i, 1, sym_data.try_strands[i].sig_bit);
			try_strands.Set(i, 2, sym_data.try_strands[i].BitString());
			try_strands.Set(i, 3, (double)sym_data.try_strands[i].result);
		}
		try_strands.SetCount(sym_data.try_strands.GetCount());
	}
	else if (tab == 3) {
		for(int i = 0; i < sym_data.catch_strands.GetCount(); i++) {
			catch_strands.Set(i, 0, i);
			catch_strands.Set(i, 1, sym_data.catch_strands[i].sig_bit);
			catch_strands.Set(i, 2, sym_data.catch_strands[i].BitString());
			catch_strands.Set(i, 3, (double)sym_data.catch_strands[i].result);
		}
		catch_strands.SetCount(sym_data.catch_strands.GetCount());
	}*/
}


}

