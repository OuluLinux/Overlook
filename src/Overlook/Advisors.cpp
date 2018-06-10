#include "Overlook.h"

namespace Overlook {


RapierishAdvisor::RapierishAdvisor() {
	
}

void RapierishAdvisor::Init() {
	SetBufferColor(0, Red());
	SetCoreSeparateWindow();
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	point = GetDataBridge()->GetPoint();
	
	SetJobCount(1);
	
	SetJob(0, tf_str + " Training")
		.SetBegin		(THISBACK(TrainingBegin))
		.SetIterator	(THISBACK(TrainingIterator))
		.SetEnd			(THISBACK(TrainingEnd))
		.SetInspect		(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void RapierishAdvisor::Start() {
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		//RefreshGrid(true);
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("RapierishAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool RapierishAdvisor::TrainingBegin() {
	
	test_settings.SetCount(0);
	for(int break_period = 40; break_period <= 100; break_period += 5) {
		for(int reverse_pips = 1; reverse_pips < 10; reverse_pips++) {
			for(int open_steps = 1; open_steps < 10; open_steps++) {
				for (int stop_loss_pips = 1; stop_loss_pips < 10; stop_loss_pips++) {
					for (int take_profit_pips = 0; take_profit_pips < 13; take_profit_pips++) {
						Setting& s = test_settings.Add();
						s.a = break_period;
						s.b = reverse_pips;
						s.c = open_steps;
						s.d = stop_loss_pips;
						s.e = take_profit_pips;
					}
				}
			}
		}
	}
	
	max_rounds = test_settings.GetCount();
	training_pts.SetCount(max_rounds, 0);
	
	
	// Allow iterating
	return true;
}

bool RapierishAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	Setting& setting = test_settings[round];
	
	training_pts[round] = TestSetting(setting, false);
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

double RapierishAdvisor::TestSetting(Setting& setting, bool write_signal) {
	
	int break_period = setting.a;
	int reverse_pips = setting.b;
	int open_steps = setting.c;
	int stop_loss_pips = setting.d;
	int take_profit_pips = setting.e;
	
	ConstBuffer& timebuf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& low_buf  = GetInputBuffer(0, 1);
	ConstBuffer& high_buf = GetInputBuffer(0, 2);
	VectorBool& signal  = GetOutput(0).label;
	VectorBool& enabled = GetOutput(1).label;
	Buffer& out = GetBuffer(0);
	
	DataBridge& db = *GetDataBridge();
	double point = db.GetPoint();
	double spread = db.GetSpread();
	if (spread == 0 || point == 0)
		Panic("Spread == 0 for symbol " + GetSystem().GetSymbol(GetSymbol()));
	
	if (spread < point * 2) spread = point * 2;
	
	take_profit_pips += spread / point;
	
	int bars = GetBars();
	signal.SetCount(bars);
	enabled.SetCount(bars);
	
	double pips = 0;
	
	ExtremumCache ec;
	ec.SetSize(200);
	
	int begin = max(0, bars - 100000);
	ec.pos = begin-1;
	
	enum {IDLE, WAITING_REVERSE, WAITING_START, OPEN, CLOSED};
	
	int waiting_break = IDLE;
	bool waiting_type;
	double waiting_price;
	for(int cursor = begin; cursor < bars; cursor++) {
		
		double lo = low_buf.Get(max(0, cursor-1));
		double hi = high_buf.Get(max(0, cursor-1));
		ec.Add(lo, hi);
		if (cursor < 100) continue;
		
		int low_len = cursor - ec.GetLowest();
		int high_len = cursor - ec.GetHighest();
		
		bool break_type = low_len > high_len;
		int break_len = break_type ? low_len : high_len;
		
		if (break_len >= break_period && waiting_break != OPEN) {
			waiting_break = WAITING_REVERSE;
			waiting_type = break_type;
			waiting_price = open_buf.Get(cursor);
		}
		
		if (waiting_break == WAITING_REVERSE) {
			double price = open_buf.Get(cursor);
			double diff = price - waiting_price;
			if (waiting_type) diff *= -1;
			int diff_pt = diff / point;
			
			if (diff_pt <= -reverse_pips) {
				waiting_break = WAITING_START;
			}
			
		}
		
		if (waiting_break == WAITING_START) {
			double price = open_buf.Get(cursor);
			double prev = open_buf.Get(cursor - 1);
			double diff = price - prev;
			if (!waiting_type) {
				double change = open_steps * diff;
				double tgt = price + change;
				if (tgt >= waiting_price && change >= spread) {
					waiting_break = OPEN;
					waiting_price = price;
				}
			}
			else {
				double change = open_steps * diff;
				double tgt = price + change;
				if (tgt <= waiting_price && -change >= spread) {
					waiting_break = OPEN;
					waiting_price = price;
				}
			}
			
		}
		
		if (waiting_break == OPEN) {
			double price = open_buf.Get(cursor);
			double diff = price - waiting_price;
			int diff_pt = diff / point;
			
			if (!waiting_type) {
				if (diff_pt <= -stop_loss_pips || diff_pt >= take_profit_pips) {
					waiting_break = CLOSED;
					pips += diff - spread;
				}
			} else {
				if (-diff_pt <= -stop_loss_pips || -diff_pt >= take_profit_pips) {
					waiting_break = CLOSED;
					pips += -diff - spread;
				}
			}
			
			if (write_signal) {
				if (waiting_break == OPEN) {
					signal.Set(cursor, waiting_type);
					enabled.Set(cursor, true);
				} else {
					signal.Set(cursor, false);
					enabled.Set(cursor, false);
				}
			}
		}
		else if (write_signal) {
			signal.Set(cursor, false);
			enabled.Set(cursor, false);
		}
		
		
	}
	
	//ReleaseLog("pips " + DblStr(pips) + " break_period " + IntStr(break_period) + " reverse_pips " + IntStr(reverse_pips) + " open_steps " + IntStr(open_steps) + " stop_loss_pips " + IntStr(stop_loss_pips) + " take_profit_pips " + IntStr(take_profit_pips));
	return pips;
}

bool RapierishAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	
	for(int i = 0; i < training_pts.GetCount(); i++) {
		Setting& s = test_settings[i];
		double result = training_pts[i];
		if (result > max_d) {
			max_d = result;
			best_setting = s;
		}
	}
	
	RefreshAll();
	return true;
}

bool RapierishAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void RapierishAdvisor::RefreshAll() {
	
	TestSetting(best_setting, true);
	
	
	VectorBool& signal  = GetOutput(0).label;
	VectorBool& enabled = GetOutput(1).label;
	
	
	int bars = GetBars();
	bool signal_ = signal.Get(bars-1);
	bool enabled_ = enabled.Get(bars-1);
	int sig = enabled_ ? (signal_ ? -1 : +1) : 0;
	
	GetSystem().SetSignal(GetSymbol(), sig);
	
	
	
	// Keep counted manually
	prev_counted = bars;
	
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
}

void RapierishAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	RapierishAdvisor* ea = dynamic_cast<RapierishAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

















MultiTfAdvisor::MultiTfAdvisor() {
	
}

void MultiTfAdvisor::Init() {
	SetBufferColor(0, Red());
	SetCoreSeparateWindow();
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0);
}

void MultiTfAdvisor::Start() {
	
	for(int i = 0; i < inputs.GetCount(); i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				if (core && !core->IsJobsFinished())
					return;
			}
		}
	}
	
	bool signal, enabled = true;
	int total = 0;
	for(int i = 1; i < 3; i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				
				bool src_signal = core->GetOutput(0).label.Top();
				if (!total)
					signal = src_signal;
				else if (src_signal != signal)
					enabled = false;
				total++;
			}
		}
	}
	
	int sig = enabled ? (signal ? -1 : +1) : 0;
	GetSystem().SetSignal(GetSymbol(), sig);
}



}
