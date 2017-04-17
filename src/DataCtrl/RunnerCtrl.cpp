#include "DataCtrl.h"

namespace DataCtrl {

RunnerDraw::RunnerDraw() {
	
}

void RunnerDraw::Init(int mode, RunnerCtrl* r) {
	this->mode = mode;
	runner = r;
}

void RunnerDraw::Paint(Draw& w) {
	if (!IsVisible()) return;
	
	if (tmp_draw) {
		Size sz(GetSize());
		w.DrawImage(0, 0, sz.cx, sz.cy, *tmp_draw);
	} else {
		w.DrawRect(GetSize(), White());
	}
}

void RunnerDraw::RefreshDraw() {
	if (!Thread::IsShutdownThreads()) return;
	
	Size sz(GetSize());
	sz.cx -= sz.cx % 4;
	sz.cy -= sz.cy % 4;
	
	tmp_draw.Clear();
	tmp_draw = new ImageDraw(sz);
	ImageDraw& id = *tmp_draw;
	id.DrawRect(sz, White());
	
	// Time-duration plotter
	if (mode == 0) {
		int max = 0;
		for(int i = 0; i < runner->attrs.GetCount(); i++) {
			const SlotProcessAttributes& a = runner->attrs[i];
			if (a.duration > max) max = a.duration;
		}
		
		if (max > 0) {
			pts.SetCount(0);
			double xstep = (double)sz.cx / (runner->attrs.GetCount()-1);
			for(int i = 0; i < runner->attrs.GetCount(); i++) {
				const SlotProcessAttributes& a = runner->attrs[i];
				int x = i * xstep;
				int y = sz.cy - (int64)a.duration * (int64)sz.cy / (int64)max;
				pts << Point(x, y);
			}
			id.DrawPolyline(pts, 1, Green());
		}
	}
	else {
		TimeVector& tv = GetTimeVector();
		int tf_count = tv.GetPeriodCount();
		int sym_count = tv.GetSymbolCount();
		double ystep = (double)sz.cy / tf_count;
		int ybar = ystep + 0.5;
		xsteps.SetCount(tf_count);
		for(int i = 0; i < tf_count; i++) {
			int bars = tv.GetCount(tv.GetPeriod(i));
			xsteps[i] = (double)sz.cx / bars;
		}
		for (int i = 0; i < runner->attrs.GetCount(); i++) {
			const SlotProcessAttributes& a = runner->attrs[i];
			int x = xsteps[a.tf_id] * a.pos[a.tf_id];
			int y = sz.cy - ystep * (a.tf_id + 1);
			int xbar = Upp::max(1, (int)(xsteps[a.tf_id] + 0.5));
			id.DrawRect(x, y, xbar, ybar, Green());
		}
	}
}





RunnerCtrl::RunnerCtrl() {
	CtrlLayout(*this);
	
	running = false;
	stopped = true;
	
	last_total_duration = 0;
	last_total_ready = 0;
	last_total = 0;
	
	timedraw.Init(0, this);
	slotdraw.Init(1, this);
	
	PostCallback(THISBACK(Refresher));
}

RunnerCtrl::~RunnerCtrl() {
	running = false;
	while (!stopped) {Sleep(100);}
}
	
void RunnerCtrl::Refresher() {
	timedraw.Refresh();
	slotdraw.Refresh();
	
	totaltime.SetLabel("Time total: " + IntStr(last_total_duration) + "ms");
	if (last_total) {
		totalready.SetLabel("Ready total: " +
			IntStr(last_total_ready * 100 / last_total) + "%" " (" +
			IntStr(last_total_ready) + "/" + IntStr(last_total) + ")");
	}
}

void RunnerCtrl::Init() {
	TimeVector& tv = GetTimeVector();
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int slot_count = tv.GetCustomSlotCount();
	
	symbols.AddColumn("Symbol");
	for(int i = 0; i < sym_count; i++) {
		symbols.Add(tv.GetSymbol(i));
	}
	
	tfs.AddColumn("Period");
	tfs.AddColumn("Minutes");
	tfs.AddColumn("Hours");
	tfs.AddColumn("Days");
	for(int i = 0; i < tf_count; i++) {
		int tf = tv.GetPeriod(i);
		int mins = tf * tv.GetBasePeriod() / 60;
		int hours = mins / 60;
		int days = mins / (60 * 24);
		tfs.Add(tf, mins, hours, days);
	}
	
	slots.AddColumn("Name");
	slots.AddColumn("Size");
	slots.AddColumn("Offset");
	slots.AddColumn("Path");
	slots.ColumnWidths("1 1 1 3");
	for(int i = 0; i < slot_count; i++) {
		const Slot& s = tv.GetCustomSlot(i);
		slots.Add(
			s.GetKey(),
			Format("0x%X", s.GetReservedBytes()),
			Format("0x%X", s.GetOffset()),
			s.GetPath());
	}
	
	
	// Draw existing data
	slotdraw.RefreshDraw();
	/*
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Run));
	*/
}

void RunnerCtrl::SetArguments(const VectorMap<String, Value>& args) {
	
}

void RunnerCtrl::Run() {
	TimeVector& tv = GetTimeVector();
	
	attrs.Clear();
	int time_step = 0;
	for (TimeVector::Iterator it = tv.Begin(); !it.IsEnd() && !Thread::IsShutdownThreads(); it++) {
		it.GetSlotProcessAttributes(attrs);
		if (!time_step) time_step = attrs.GetCount();
	}
	
	struct Sorter {
		bool operator() (const SlotProcessAttributes& a, const SlotProcessAttributes& b) const {
			if (a.time		< b.time)		return true;
			if (a.time		> b.time)		return false;
			if (a.slot_pos	< b.slot_pos)	return true;
			if (a.slot_pos	> b.slot_pos)	return false;
			if (a.tf_id		< b.tf_id)		return true;
			if (a.tf_id		> b.tf_id)		return false;
			if (a.sym_id	< b.sym_id)		return true;
			if (a.sym_id	> b.sym_id)		return false;
			Panic("Duplicate attribute");
			return false;
		}
	};
	
	Sort(attrs, Sorter());
	/*for(int i = 0; i < 1000; i++) {
		SlotProcessAttributes& a = attrs[i];
		Time time;
		time.Set(a.time);
		String str = Format("%", time);
		LOG(Format("time=%s slot=%d tf=%d sym=%d", str, a.slot_pos, a.tf_id, a.sym_id));
	}*/
	
	TimeStop ts, ts_total;
	
	while (!Thread::IsShutdownThreads() && running) {
		while (!Thread::IsShutdownThreads() && running) {
			bool all_processed = true;
			int64 skiptime = 0;
			
			Vector<SlotProcessAttributes>::Iterator cur = attrs.Begin();
			Vector<SlotProcessAttributes>::Iterator end = attrs.End();
			
			ts_total.Reset();
			int total = 0;
			int total_ready = 0;
			
			for(int i = 0; cur != end && !Thread::IsShutdownThreads(); cur++, i++) {
				SlotProcessAttributes& sa = *cur;
				
				total++;
				
				if (sa.time == skiptime) {
					sa.duration = 0;
					continue;
				}
				
				if (sa.slot->IsReady(sa)) {
					sa.duration = 0;
					total_ready++;
					continue;
				}
				
				ts.Reset();
				if (!sa.slot->Process(sa)) {
					all_processed = false;
					
					// skip all processing until next time-position
					skiptime = sa.time;
				}
				else {
					// Set sym/tf/pos/slot position as ready
					sa.slot->SetReady(sa);
					total_ready++;
				}
				sa.duration = ts.Elapsed();
				
			}
			
			tv.StoreChangedCache();
			if (!running) break;
			
			last_total_duration = ts_total.Elapsed();
			last_total = total;
			last_total_ready = total_ready;
			
			timedraw.RefreshDraw();
			slotdraw.RefreshDraw();
			PostCallback(THISBACK(Refresher));
			
			if (all_processed)
				break;
		}
		
		for(int i = 0; i < 60 && !Thread::IsShutdownThreads() && running; i++) {
			Sleep(1000);
		}
	}
	
	stopped = true;
}

}
