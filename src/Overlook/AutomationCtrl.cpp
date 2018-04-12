#include "Overlook.h"

namespace Overlook {
using namespace Upp;


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
	
	tabs.Add(evolvectrl, "Evolve");
	tabs.Add(evolvectrl);
	tabs.WhenSet << THISBACK(Data);
	
	evolvectrl.Add(evolvesplit.SizePos());
	for(int i = 0; i < USEDSYMBOL_COUNT; i++)
		evolvesplit << evolveprog.Add();
	evolvesplit.Vert();
	
	PostCallback(THISBACK(Start));
}

void AutomationCtrl::Data() {
	System& sys = GetSystem();
	Automation& a = GetAutomation();
	
	int row = 0;
	for(int i = 0; i < a.sym_count; i++) {
		int sym = sys.used_symbols_id[i];
		int tf = a.slow[i].tf;
		symbols.Set(i, 0, sym);
		symbols.Set(i, 1, tf);
		symbols.Set(i, 2, sys.GetSymbol(sym));
		symbols.Set(i, 3, sys.GetPeriod(tf));
		
		int job_id = a.GetSymGroupJobId(i);
		int prog = job_id * 1000 / Automation::GROUP_COUNT;
		String phase_str;
		switch (job_id) {
			case Automation::GROUP_SOURCE:		phase_str = "Source"; break;
			case Automation::GROUP_EVOLVE:		phase_str = "Evolve"; break;
			case Automation::GROUP_COUNT:		phase_str = "Finished"; break;
		}
		
		symbols.Set(i, 4, phase_str);
		symbols.Set(i, 5, prog);
		symbols.SetDisplay(i, 3, ProgressDisplay());
	}
	
	
	int cursor = symbols.GetCursor();
	if (cursor == -1) {
		cursor = 0;
		symbols.SetCursor(0);
	}
	
	
	int sym = symbols.Get(cursor, 0);
	int tf  = symbols.Get(cursor, 1);
	
	
	int tab = tabs.Get();
	/*if (tab == 0) {
		int last = a.slow[0].processbits_cursor - 1;
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
		cursor_stats.Set(row++, 1, Format("%", Time(1970,1,1) + a.slow[0].time_buf[cursor]));
		
		for(int i = 0; i < sys.used_symbols_id.GetCount(); i++) {
			int sym = sys.used_symbols_id[i];
			String symstr = sys.symbols[sym];
			
			cursor_stats.Set(row,   0, symstr + " signal");
			bool signal  = a.slow[i].GetBitOutput(cursor, OUT_EVOLVE_SIG);
			bool enabled = a.slow[i].GetBitOutput(cursor, OUT_EVOLVE_ENA);
			int sig = enabled ? (signal ? -1 : +1) : 0;
			cursor_stats.Set(row++, 1, sig);
		}
	}
	else if (tab == 0) {*/
	if (tab == 0) {
		for(int i = 0; i < USEDSYMBOL_COUNT; i++) {
			evolveprog[i].Set(a.slow[i].brain_iters, a.slow[i].max_iters);
		}
	}
}


}
