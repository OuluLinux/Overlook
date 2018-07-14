#include "Overlook.h"


namespace Overlook {


AutoChartistSymbol::AutoChartistSymbol() {
	
}

void AutoChartistSymbol::Data() {
	System& sys = GetSystem();
	
	ASSERT(sym != -1 && tf != -1);
	int period = sys.GetPeriod(tf);
	
	//sym = sys.System::FindSymbol("EURUSD"); LOG("REMOVE");
	
	if (work_queue.IsEmpty()) {
		Index<int> sym_ids, tf_ids;
		Vector<FactoryDeclaration> indi_ids;
		sym_ids.Add(sym);
		tf_ids.Add(tf);
		indi_ids.Add().factory = sys.Find<DataBridge>();
		sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
		ASSERT(!work_queue.IsEmpty());
	}
	
	
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
	}
	
	Core& c = *work_queue.Top()->core;
	DataBridge& db = dynamic_cast<DataBridge&>(c);
	ConstBuffer& open = c.GetBuffer(0);
	ConstBuffer& low = c.GetBuffer(1);
	ConstBuffer& high = c.GetBuffer(2);
	point = db.GetPoint();
	
	data.SetCount(open.GetCount());
	
	VectorMap<int, int> low_lines, high_lines;
	for(int cursor = max(1, counted); cursor < open.GetCount(); cursor++) {
		DataPoint& pt = data[cursor];
		
		pt.high = high.Get(cursor);
		pt.low = low.Get(cursor);
		
		
		// High break
		{
			int dir = 0;
			int len = 0;
			double hi = high.Get(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = hi > high.Get(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			
			pt.high_len = len;
		}
		
		// Low break
		{
			int dir = 0;
			int len = 0;
			double lo = low.Get(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = lo < low.Get(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			
			pt.low_len = len;
		}
		
		
		// Collect count of angles. The highest count means strong line.
		low_lines.Clear();
		high_lines.Clear();
		double l0 = low.Get(cursor);
		double h0 = high.Get(cursor);
		
		for (int i = cursor - 1, j = 1; i >= 0 && j < 100; i--, j++) {
			DataPoint& pt1 = data[i];
			double l1 = low.Get(i);
			double h1 = high.Get(i);
			
			double ldiff = (l0 - l1) / point;
			double hdiff = (h0 - h1) / point;
			
			double langle = ldiff / (j * period);
			double hangle = hdiff / (j * period);
			
			int liangle = langle * 1000;
			int hiangle = hangle * 1000;
			
			low_lines.GetAdd(liangle, 0) += pt1.low_len;
			high_lines.GetAdd(hiangle, 0) += pt1.high_len;
		}
		
		
		// Find the strongest line. Store it to data-vector.
		pt.low_weight = INT_MIN;
		pt.high_weight = INT_MIN;
		for(int i = 0; i < low_lines.GetCount(); i++) {
			int amount = low_lines[i];
			if (amount > pt.low_weight) {
				pt.low_angle = low_lines.GetKey(i);
				pt.low_weight = amount;
			}
		}
		for(int i = 0; i < high_lines.GetCount(); i++) {
			int amount = high_lines[i];
			if (amount > pt.high_weight) {
				pt.high_angle = high_lines.GetKey(i);
				pt.high_weight = amount;
			}
		}
		
		
		// Find local strongest line. Use it as triangle.
		int highest_weight = INT_MIN, lowest_weight = INT_MIN;
		for (int i = cursor - 1, j = 1; i >= 0 && j < 20; i--, j++) {
			DataPoint& pt1 = data[i];
			
			if (pt1.high_weight >= highest_weight) {
				pt.highest_pos = i;
				highest_weight = pt1.high_weight;
			}
			
			if (pt1.low_weight >= lowest_weight) {
				pt.lowest_pos = i;
				lowest_weight = pt1.low_weight;
			}
		}
		
		
		
		
	}
	counted = open.GetCount() - 1;
	
	
	
	
	
	
}






AutoChartist::AutoChartist() {
	
	LoadThis();
	
}

void AutoChartist::StartData() {
	if (running) return;
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Data));
}

void AutoChartist::Data() {
	System& sys = GetSystem();
	
	actual = 0;
	total = sys.GetSymbolCount() * sys.GetPeriodCount();
	symbols.SetCount(total);
	TimeStop ts;
	
	int s = 0;
	for(int i = 0; i < sys.GetSymbolCount() && IsRunning(); i++) {
		for(int j = 0; j < sys.GetPeriodCount() && IsRunning(); j++) {
			AutoChartistSymbol& sym = symbols[s++];
			sym.sym = i;
			sym.tf = j;
			
			if (j >= 2)
				sym.Data();
			
			actual++;
		}
		
		if (ts.Elapsed() > 60*1000) {
			StoreThis();
			ts.Reset();
		}
	}
	
	if (IsRunning())
		StoreThis();
	
	running = false;
	stopped = true;
}







AutoChartistCtrl::AutoChartistCtrl() {
	Add(par.SizePos());
	par.Add(prog.TopPos(0,20).HSizePos());
	par.Add(split.VSizePos(20).HSizePos());
	prog.Set(0, 1);
	
	
	#if AUTOCHARTIST_DEBUG
	split << tflist << symlist << timelist << featlist;
	
	tflist.AddColumn("Tf");
	symlist.AddColumn("Symbol");
	timelist.AddColumn("Time");
	featlist.AddColumn("Key");
	featlist.AddColumn("Value");
	
	tflist << THISBACK(Data);
	symlist << THISBACK(Data);
	timelist << THISBACK(Data);
	
	
	#endif
	
	
}

void AutoChartistCtrl::Data() {
	AutoChartist& ac = GetAutoChartist();
	System& sys = GetSystem();
	
	prog.Set(ac.actual, ac.total);
	
	ac.StartData();
	
	for(int i = 0; i < sys.GetPeriodCount(); i++) {
		tflist.Set(i, 0, sys.GetPeriod(i));
	}
	
	for(int i = 0; i < sys.GetSymbolCount(); i++) {
		symlist.Set(i, 0, sys.GetSymbol(i));
	}
	
	int tfcursor = tflist.GetCursor();
	int symcursor = symlist.GetCursor();
	
	if (tfcursor >= 0 && tfcursor < tflist.GetCount() && symcursor >= 0 && symcursor < symlist.GetCount()) {
		AutoChartistSymbol& sym = ac.symbols[symcursor * tflist.GetCount() + tfcursor];
		
		int period = sys.GetPeriod(tfcursor);
		
		int row = 0;
		for(int i = max(0, sym.data.GetCount() - 100); i < sym.data.GetCount(); i++) {
			timelist.Set(row++, 0, i);
		}
		timelist.SetCount(row);
		
		
		int timecursor = timelist.GetCursor();
		timecursor = (timecursor >= 0 && timecursor < timelist.GetCount()) ? timelist.Get(timecursor, 0) : -1;
		
		if (timecursor >= 0 && timecursor < sym.data.GetCount()) {
			
			AutoChartistSymbol::DataPoint& pt = sym.data[timecursor];
			AutoChartistSymbol::DataPoint& highest_pt = sym.data[pt.highest_pos];
			AutoChartistSymbol::DataPoint& lowest_pt = sym.data[pt.lowest_pos];
			
			int row = 0;
			#define VAR(x) featlist.Set(row, 0, #x); featlist.Set(row++, 1, x);
			VAR(pt.high_len);
			VAR(pt.low_len);
			VAR(pt.high_angle);
			VAR(pt.low_angle);
			VAR(pt.high_weight);
			VAR(pt.low_weight);
			VAR(pt.highest_pos);
			VAR(pt.lowest_pos);
			VAR(highest_pt.high_len);
			VAR(highest_pt.low_len);
			VAR(highest_pt.high_angle);
			VAR(highest_pt.low_angle);
			VAR(highest_pt.high_weight);
			VAR(highest_pt.low_weight);
			VAR(highest_pt.highest_pos);
			VAR(highest_pt.lowest_pos);
			VAR(lowest_pt.high_len);
			VAR(lowest_pt.low_len);
			VAR(lowest_pt.high_angle);
			VAR(lowest_pt.low_angle);
			VAR(lowest_pt.high_weight);
			VAR(lowest_pt.low_weight);
			VAR(lowest_pt.highest_pos);
			VAR(lowest_pt.lowest_pos);
			
			if (symcursor != prev_symcursor || tfcursor != prev_tfcursor || timecursor != prev_timecursor) {
				Profile profile;
				ProfileGroup& pgroup = profile.charts.Add();
				pgroup.symbol = symcursor;
				pgroup.tf = tfcursor;
				pgroup.keep_at_end = true;
				pgroup.right_offset = true;
				pgroup.decl.factory = System::Find<DataBridge>();
				::Overlook::Overlook& ol = GetOverlook();
				ol.LoadProfile(profile);
				ol.TileWindow();
				
				Chart* c = ol.GetChart(0);
				if (c) {
					ChartObject& high_line = c->AddObject();
					high_line.type = OBJ_TREND;
					high_line.pos0 = pt.highest_pos;
					high_line.price0 = highest_pt.high;
					high_line.pos1 = pt.highest_pos - 10;
					high_line.price1 = highest_pt.high - pt.high_angle * 0.001 * period * sym.point * 10;
					high_line.clr = Blue();
					
					ChartObject& low_line = c->AddObject();
					low_line.type = OBJ_TREND;
					low_line.pos0 = pt.lowest_pos;
					low_line.price0 = lowest_pt.low;
					low_line.pos1 = pt.lowest_pos - 10;
					low_line.price1 = lowest_pt.low - pt.low_angle * 0.001 * period * sym.point * 10;
					low_line.clr = Red();
					
					ChartObject& vert_line = c->AddObject();
					vert_line.type == OBJ_VLINE;
					vert_line.pos0 = timecursor;
					vert_line.clr = Green();
				}
			}
			
			prev_symcursor = symcursor;
			prev_tfcursor = tfcursor;
			prev_timecursor = timecursor;
		}
	}
}

}
