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
	
	#ifdef flagDEBUG
	max_rounds = 10000;
	#else
	max_rounds = 1000000;
	#endif
	
	training_pts.SetCount(max_rounds, 0);
	
	
	// Allow iterating
	return true;
}

bool RapierishAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	/*int size = ses.GetGraphCount()-1;
	int range = GetBars();
	if (do_test) range *= TRAININGAREA_FACTOR;
	range -= size;
	
	int pos = size + Random(range);
	
		
	sequence.SetCount(size);
	for(int i = 0; i < size; i++) {
		int j = pos - size + i;
		double change = open_buf.Get(j+1) - open_buf.Get(j);
		sequence[i] = ChangeToChar(change);
		Panic("TODO use normalized data and solve reversion function with CAS");
	}
	
	ses.Learn(sequence);
	
	training_pts[round] = ses.GetPerplexity();*/
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

bool RapierishAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
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
	RefreshSourcesOnlyDeep();
	ConstBuffer& timebuf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	VectorBool& signal  = GetOutput(0).label;
	VectorBool& enabled = GetOutput(1).label;
	Buffer& out = GetBuffer(0);
	
	// ---- Do your final result work here ----
	//RefreshBits();
	//SortByValue(results, GridResult());
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 2.0;
	
	int bars = GetBars();
	signal.SetCount(bars);
	enabled.SetCount(bars);
	for(int i = prev_counted; i < bars; i++) {
		/*
		sequence.SetCount(size - 1);
		for(int j = 0; j < size - 1; j++) {
			int k = i - size + j + 1;
			double change = open_buf.Get(k+1) - open_buf.Get(k);
			sequence[j] = ChangeToChar(change);
		}
		ses.Predict(sequence, true, 1.0, true);
		
		int sig = CharToChange(sequence.Top()) >= 0 ? +1 : -1;
		*/
		int sig;
		
		signal. Set(i, sig == -1);
		enabled.Set(i, sig !=  0);
		
		
		if (i < bars-1) {
			if (sig > 0) {
				total += +(open_buf.Get(i+1) - open_buf.Get(i)) - spread;
			}
			else if (sig < 0) {
				total += -(open_buf.Get(i+1) - open_buf.Get(i)) - spread;
			}
		}
		out.Set(i, total);
		
		if (i == bars-1)
			GetSystem().SetSignal(GetSymbol(), sig);
	}
	
	
	
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
