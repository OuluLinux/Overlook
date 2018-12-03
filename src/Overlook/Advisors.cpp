#include "Overlook.h"

#if 0
namespace Overlook {

DqnAdvisor::DqnAdvisor() {
	

}

void DqnAdvisor::Init() {
	SetBufferColor(0, Red());
	SetCoreSeparateWindow();
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	point = GetDataBridge()->GetPoint();
	
	SetJob(0, tf_str + " Training")
		.SetBegin		(THISBACK(TrainingBegin))
		.SetIterator	(THISBACK(TrainingIterator))
		.SetEnd			(THISBACK(TrainingEnd))
		.SetInspect		(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void DqnAdvisor::Start() {
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		//RefreshGrid(true);
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("DqnAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool DqnAdvisor::TrainingBegin() {
	#ifdef flagDEBUG
	max_rounds = 10000;
	#else
	max_rounds = 20000000;
	#endif
	training_pts.SetCount(max_rounds, 0);
	
	
	// Allow iterating
	return true;
}

void DqnAdvisor::LoadInput(int pos) {
	int matpos = 0;
	
	ConstBuffer& input_buf = GetInputBuffer(1, 0);
	for(int i = 0; i < input_length; i++) {
		int j = pos - i;
		double d = input_buf.Get(j);
		d -= 0.5;
		d *= 2 * 50;
		tmp_mat.Set(matpos++, d);
	}
	
	ASSERT(matpos == tmp_mat.GetLength());
}

bool DqnAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	int size = input_length;
	int range = GetBars() - 1;
	range -= size;
	
	int pos = size + Random(range);
	
	LoadInput(pos);
	
	dqn.SetGamma(0);
	
	double error;
	int action = dqn.Act(tmp_mat);
	double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
	if (action) change *= -1;
	double reward = change >= 0 ? +10 : -10;
	error = dqn.Learn(tmp_mat, action, reward, NULL);
	
	
	training_pts[round] = error;
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

bool DqnAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
	RefreshAll();
	return true;
}

bool DqnAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void DqnAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	ConstBuffer& timebuf = GetInputBuffer(0, 4);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	/*VectorBool& signal  = GetOutput(0).label;
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
	
	bool initial = prev_counted == 0;
	if (prev_counted < input_length) prev_counted = input_length;
	
	if (have_othersyms) {
		for(int i = 0; i < inputs[1].GetCount(); i++) {
			bars = min(bars, inputs[1][i].core->GetBuffer(0).GetCount());
		}
	}
	
	for(int i = prev_counted; i < bars; i++) {
		
		LoadInput(i);
		
		int sig, mult = 1;
		
		if (have_actionmode == ACTIONMODE_SIGN) {
			int action = dqn.Act(tmp_mat);
			sig = action ? -1 : +1;
		}
		else if (have_actionmode == ACTIONMODE_TREND) {
			int action = dqn.Act(tmp_mat);
			double change = open_buf.Get(i) - open_buf.Get(i - 1);
			bool trend_action = change < 0.0;
			if (action) trend_action = !trend_action;
			sig = trend_action ? -1 : +1;
		}
		else if (have_actionmode == ACTIONMODE_WEIGHTED) {
			int action = dqn.Act(tmp_mat);
			bool sign_action = action % 2;
			sig = sign_action ? -1 : +1;
			mult  = 1 + action / 2;
		}
		else if (have_actionmode == ACTIONMODE_HACK) {
			double output[2];
			dqn.Evaluate(tmp_mat, output, 2);
			
			//sig = output[0] < 0.5 || output[1] < 0.5 ? (output[0] > output[1] ? -1 : +1) : 0;
			sig = output[0] > output[1] ? -1 : +1;
		}
		else Panic("Action not implemented");
		
		signal. Set(i, sig <  0);
		enabled.Set(i, sig != 0);
		
		
		if (i < bars-1) {
			if (sig > 0) {
				total += (+(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
			else if (sig < 0) {
				total += (-(open_buf.Get(i+1) - open_buf.Get(i)) - spread) * mult;
			}
		}
		out.Set(i, total);
	}
	
	#ifndef flagDEBUG
	if (do_test && initial) {
		DumpTest();
	}
	#endif
	
	//int sig = enabled.Get(bars-1) ? (signal.Get(bars-1) ? -1 : +1) : 0;
	//GetSystem().SetSignal(GetSymbol(), sig);
	//ReleaseLog("DqnAdvisor::RefreshAll symbol " + IntStr(GetSymbol()) + " sig " + IntStr(sig));
	
	
	// Keep counted manually
	prev_counted = bars;
	*/
	
	//DUMP(main_interval);
	//DUMP(grid_interval);
}

void DqnAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	DqnAdvisor* ea = dynamic_cast<DqnAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

void DqnAdvisor::DumpTest() {
	int bars = GetBars() - 1;
	int begin = bars * 0.66;
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	double spread = GetDataBridge()->GetSpread();
	if (spread == 0)
		spread = GetDataBridge()->GetPoint() * 3;
	
	int signs_correct = 0, signs_total = 0;
	double pos_total = 0, neg_total = 0;
	double total = 0;
	
	for(int i = begin; i < bars; i++) {
		
		LoadInput(i);
		
		int sig, mult = 1;
		
		int action = dqn.Act(tmp_mat);
		sig = action ? -1 : +1;
		
		double diff;
		if (sig > 0) {
			diff = +(open_buf.Get(i+1) - open_buf.Get(i));
		}
		else if (sig < 0) {
			diff = -(open_buf.Get(i+1) - open_buf.Get(i));
		}
		double change = (diff - spread) * mult;
		total += change;
		
		if (diff > 0)
			signs_correct++;
		signs_total++;
		
		if (diff >= 0) pos_total += diff;
		else neg_total -= diff;
	}
	
	/*
	String symstr = GetSystem().GetSymbol(GetSymbol());
	String tfstr = GetSystem().GetPeriodString(GetTf());
	
	String dir = ConfigFile("test_results");
	RealizeDirectory(dir);
	String filename = Format("%d-%d-%d-%d-%d-%d-%d-%s-%s.txt",
		have_normaldata,
		have_normalma,
		have_hurst,
		have_anomaly,
		have_othersyms,
		have_actionmode,
		input_length,
		symstr,
		tfstr);
	String filepath = AppendFileName(dir, filename);
	FileOut fout(filepath);
	
	fout << "have_normaldata = " << have_normaldata << "\r\n";
	fout << "have_normalma   = " << have_normalma << "\r\n";
	fout << "have_hurst      = " << have_hurst << "\r\n";
	fout << "have_anomaly    = " << have_anomaly << "\r\n";
	fout << "have_othersyms  = " << have_othersyms << "\r\n";
	fout << "have_actionmode = " << have_actionmode << "\r\n";
	fout << "input_length    = " << input_length << "\r\n";
	fout << "symbol          = " << symstr << "\r\n";
	fout << "timeframe       = " << tfstr << "\r\n";
	fout << "\r\n";
	fout << "signs_correct   = " << DblStr((double)signs_correct / (double)signs_total) << "\r\n";
	
	double drawdown = (neg_total / (neg_total + pos_total));
	fout << "drawdown        = " << DblStr(drawdown) << "\r\n";
	fout << "total, spreads  = " << DblStr(total) << "\r\n";
	*/
}

}
#endif
