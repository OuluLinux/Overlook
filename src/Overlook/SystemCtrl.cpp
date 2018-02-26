#include "Overlook.h"


namespace Overlook {
using namespace Upp;

void BooleansDraw::Paint(Draw& w) {
	System& sys = GetSystem();
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	if (cursor >= 0) {
		
		int w = SourceImage::row_size;
		int h = sys.GetSymbolCount();
		
		double xstep = (double)sz.cx / w;
		double ystep = (double)sz.cy / h;
		
		for(int i = 0; i < h; i++) {
			if (i >= sys.data.GetCount()) continue;
			
			auto& main_booleans = sys.data[i][0].main_booleans;
			
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
	}
	
	w.DrawImage(0, 0, id);
}

SystemCtrl::SystemCtrl() {
	prog.Set(0, 1);
	
	Add(prog.BottomPos(2,26).HSizePos(2,2));
	Add(load_all.TopPos(2, 26).LeftPos(2,96));
	Add(slider.HSizePos(100).BottomPos(30,30));
	Add(hsplit.HSizePos(100).VSizePos(0,60));
	
	slider.MinMax(0,1);
	slider << THISBACK(Data);
	
	hsplit << bools << stats << strands;
	
	load_all.SetLabel("Load all");
	load_all <<= THISBACK(StartLoadAll);
	
	stats.AddColumn("Bit combination");
	stats.AddColumn("Percent");
	stats.AddColumn("Index");
	
	strands.AddColumn("Index");
	strands.AddColumn("Bit list");
	strands.AddColumn("Result");
	
}

void SystemCtrl::Data() {
	ASSERT(SourceImage::row_size == SNAP_BITS);
	
	System& sys = GetSystem();
	
	const int sym = 0;
	
	auto& sym_data = sys.data[sym][0];
	auto& main_booleans = sym_data.main_booleans;
	
	slider.MinMax(0, main_booleans.GetCount() - 1);
	bools.cursor = slider.GetData();
	bools.Refresh();
	
	
	
	if (running)
		stats.Clear();
	
	if (stats.GetCount() == 0 && stopped) {
			
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
		
		
		for(int i = 0; i < sym_data.strands.GetCount(); i++) {
			strands.Set(i, 0, i);
			strands.Set(i, 1, sym_data.strands[i].BitString());
			strands.Set(i, 2, sym_data.strands[i].result);
		}
	}
}


void SystemCtrl::LoadAll() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetSymbolCount(); i++) {
		for(int j = 0; j < sys.GetPeriodCount(); j++) {
			sys.data[i][j].LoadAll();
		}
	}
	
	PostCallback(THISBACK(Enable)); running = false; stopped = true;
}

}

