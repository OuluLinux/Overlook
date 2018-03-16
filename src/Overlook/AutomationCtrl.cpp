#include "Overlook.h"


namespace Overlook {
using namespace Upp;

void BooleansDraw::Paint(Draw& w) {
	System& sys = GetSystem();
	Automation& a = GetAutomation();
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	if (cursor >= 0 && cursor < a.processbits_cursor) {
		int ts = a.time_buf[cursor];
		Time t = Time(1970,1,1) + ts;
		LOG("BooleansDraw::Paint cursor time " + Format("%", t));
		
		int w = a.processbits_row_size;
		int h = a.sym_count;
		
		double xstep = (double)sz.cx / w;
		double ystep = (double)sz.cy / h;
		
		for(int i = 0; i < h; i++) {
			
			//auto& main_booleans = a.bits_buf[i][tf].main_booleans;
			
			int count = a.processbits_cursor;
			if (count == 0) continue;
			
			int y0 = i * ystep;
			int y1 = (i + 1) * ystep;
			for(int j = 0; j < w; j++) {
				int x0 = j * xstep;
				int x1 = (j + 1) * xstep;
				
				bool b = a.GetBit(cursor, i, j);
				if (b)
					id.DrawRect(x0, y0, x1-x0, y1-y0, Black());
			}
		}
	}
	
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
	tabs.Add(strands, "Strands");
	tabs.Add(strands);
	
	slider.MinMax(0,1);
	slider << THISBACK(Data);
	
	boolctrl.Add(bools.HSizePos(0, 300).VSizePos(0,30));
	boolctrl.Add(cursor_stats.RightPos(0, 300).VSizePos(0,30));
	boolctrl.Add(slider.HSizePos().BottomPos(0,30));
	
	cursor_stats.AddColumn("Key");
	cursor_stats.AddColumn("Value");
	
	strands.AddColumn("Index");
	strands.AddColumn("Signal bit");
	strands.AddColumn("Bit list");
	strands.AddColumn("Enable result");
	strands.AddColumn("Trigger result");
	strands.AddColumn("Weight result");
	
	PostCallback(THISBACK(Start));
}

void AutomationCtrl::Data() {
	System& sys = GetSystem();
	Automation& a = GetAutomation();
	
	int row = 0;
	for(int i = 0; i < a.sym_count; i++) {
		int sym = sys.used_symbols_id[i];
		int tf = a.tf;
		symbols.Set(i, 0, sym);
		symbols.Set(i, 1, tf);
		symbols.Set(i, 2, sys.GetSymbol(sym));
		symbols.Set(i, 3, sys.GetPeriod(tf));
		
		int job_id = a.GetSymGroupJobId(i);
		int prog = job_id * 1000 / Automation::GROUP_COUNT;
		String phase_str;
		switch (job_id) {
			case Automation::GROUP_SOURCE:	phase_str = "Source"; break;
			case Automation::GROUP_BITS:	phase_str = "Bits"; break;
			case Automation::GROUP_ENABLE:	phase_str = "Enable"; break;
			case Automation::GROUP_TRIGGER:	phase_str = "Trigger"; break;
			case Automation::GROUP_WEIGHT:	phase_str = "Weight"; break;
			case Automation::GROUP_COUNT:	phase_str = "Finished"; break;
		}
		
		symbols.Set(i, 4, phase_str);
		symbols.Set(i, 5, prog);
		symbols.SetDisplay(i, 3, ProgressDisplay());
	}
	
	
	int cursor = symbols.GetCursor();
	if (cursor == -1) return;
	
	
	int sym = symbols.Get(cursor, 0);
	int tf  = symbols.Get(cursor, 1);
	
	
	int tab = tabs.Get();
	if (tab == 0) {
		int last = a.processbits_cursor - 1;
		slider.MinMax(0, last);
		if (last < 0) return;
		
		bools.tf = tf;
		bools.cursor = slider.GetData();
		bools.Refresh();
		
		int cursor = slider.GetData();
		if (cursor < 0) cursor = 0;
		if (cursor > last) cursor = last;
		
		int row = 0;
		cursor_stats.Set(row,   0, "Time");
		cursor_stats.Set(row++, 1, Format("%", Time(1970,1,1) + a.time_buf[cursor]));
		
		for(int i = 0; i < sys.used_symbols_id.GetCount(); i++) {
			int sym = sys.used_symbols_id[i];
			String symstr = sys.symbols[sym];
			
			cursor_stats.Set(row,   0, symstr + " signal");
			bool signal  = a.GetBitOutput(cursor, i, 4);
			bool enabled = a.GetBitOutput(cursor, i, 5);
			int sig = enabled ? (signal ? -1 : +1) : 0;
			cursor_stats.Set(row++, 1, sig);
		}
	}
	else if (tab == 1) {
		for(int i = 0; i < a.strands[cursor].GetCount(); i++) {
			strands.Set(i, 0, i);
			strands.Set(i, 1, a.strands[cursor][i].sig_bit);
			strands.Set(i, 2, a.strands[cursor][i].BitString());
			for(int j = 0; j < GROUP_RESULTS; j++) {
				strands.Set(i, 3 + j, (double)a.strands[cursor][i].result[j]);
			}
		}
		strands.SetCount(a.strands[cursor].GetCount());
	}
}


}

