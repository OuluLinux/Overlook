#include "Overlook.h"

namespace Overlook {


RecurrentAdvisor::RecurrentAdvisor() {
	String model_str = "{\n"

		// model parameters
		"\t\"generator\":\"lstm\",\n" // can be 'rnn' or 'lstm' or 'highway'
		"\t\"hidden_sizes\":[20,20],\n" // list of sizes of hidden layers
		"\t\"letter_size\":5,\n" // size of letter embeddings
		
		// optimization
		"\t\"regc\":0.000001,\n" // L2 regularization strength
		"\t\"learning_rate\":0.01,\n" // learning rate
		"\t\"clipval\":5.0\n" // clip gradients at this value
		"}";
	
	ValueMap js = ParseJSON(model_str);
	ses.Load(js);
	// vocabulary = {START, 0, 1}
	ses.SetInputSize(input_size+1);
	ses.SetOutputSize(input_size+1);
	ses.Init();

}

void RecurrentAdvisor::Init() {
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

void RecurrentAdvisor::Start() {
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		//RefreshGrid(true);
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("RecurrentAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

int RecurrentAdvisor::ChangeToChar(double change) {
	int chr = change / point;
	chr += sign_max;
	if (chr < 0) chr = 0;
	if (chr >= input_size) chr = input_size -1;
	chr++; // char 0 is BEGIN
	return chr;
}

double RecurrentAdvisor::CharToChange(int chr) {
	chr--;
	chr -= sign_max;
	double change = chr * point;
	return change;
}

bool RecurrentAdvisor::TrainingBegin() {
	
	#ifdef flagDEBUG
	max_rounds = 10000;
	#else
	max_rounds = 1000000;
	#endif
	
	training_pts.SetCount(max_rounds, 0);
	
	
	// Allow iterating
	return true;
}

bool RecurrentAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	int size = ses.GetGraphCount()-1;
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
	
	training_pts[round] = ses.GetPerplexity();
	
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

bool RecurrentAdvisor::TrainingEnd() {
	double max_d = -DBL_MAX;
	int max_i = 0;
	
	
	RefreshAll();
	return true;
}

bool RecurrentAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void RecurrentAdvisor::RefreshAll() {
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
	
	int size = ses.GetGraphCount()-1;
	int bars = GetBars();
	signal.SetCount(bars);
	enabled.SetCount(bars);
	if (prev_counted < size) prev_counted = size;
	for(int i = prev_counted; i < bars; i++) {
		
		sequence.SetCount(size - 1);
		for(int j = 0; j < size - 1; j++) {
			int k = i - size + j + 1;
			double change = open_buf.Get(k+1) - open_buf.Get(k);
			sequence[j] = ChangeToChar(change);
		}
		ses.Predict(sequence, true, 1.0, true);
		
		int sig = CharToChange(sequence.Top()) >= 0 ? +1 : -1;
		
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

void RecurrentAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	RecurrentAdvisor* ea = dynamic_cast<RecurrentAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}






















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
	
	if (!have_othersyms) {
		if (have_normaldata) {
			ConstBuffer& input_buf = GetInputBuffer(1, 0);
			for(int i = 0; i < input_length; i++) {
				int j = pos - i;
				double d = input_buf.Get(j);
				d -= 0.5;
				d *= 2 * 50;
				tmp_mat.Set(matpos++, d);
			}
		}
		
		if (have_normalma) {
			ConstBuffer& input_buf = GetInputBuffer(1, 1);
			for(int i = 0; i < input_length; i++) {
				int j = pos - i;
				double d = input_buf.Get(j);
				d -= 0.5;
				d *= 2 * 50;
				tmp_mat.Set(matpos++, d);
			}
		}
		
		if (have_hurst) {
			ConstBuffer& input_buf = GetInputBuffer(2, 0);
			for(int i = 0; i < input_length; i++) {
				int j = pos - i;
				double d = input_buf.Get(j);
				d -= 0.5;
				d *= 2 * 50;
				tmp_mat.Set(matpos++, d);
			}
		}
		
		if (have_anomaly) {
			ConstBuffer& input_buf = GetInputBuffer(3, 0);
			for(int i = 0; i < input_length; i++) {
				int j = pos - i;
				double d = input_buf.Get(j);
				d -= 0.5;
				d *= 2 * 50;
				tmp_mat.Set(matpos++, d);
			}
		}
	} else {
		if (have_normaldata) {
			for(int s = 0; s < other_syms; s++) {
				ConstBuffer& input_buf = inputs[1][s].core->GetBuffer(0);
				for(int i = 0; i < input_length; i++) {
					int j = pos - i;
					double d = input_buf.Get(j);
					d -= 0.5;
					d *= 2 * 50;
					tmp_mat.Set(matpos++, d);
				}
			}
		}
		if (have_normalma) {
			for(int s = 0; s < other_syms; s++) {
				ConstBuffer& input_buf = inputs[1][s].core->GetBuffer(1);
				for(int i = 0; i < input_length; i++) {
					int j = pos - i;
					double d = input_buf.Get(j);
					d -= 0.5;
					d *= 2 * 50;
					tmp_mat.Set(matpos++, d);
				}
			}
		}
		if (have_hurst) {
			for(int s = 0; s < other_syms; s++) {
				ConstBuffer& input_buf = inputs[2][s].core->GetBuffer(0);
				for(int i = 0; i < input_length; i++) {
					int j = pos - i;
					double d = input_buf.Get(j);
					d -= 0.5;
					d *= 2 * 50;
					tmp_mat.Set(matpos++, d);
				}
			}
		}
		if (have_anomaly) {
			for(int s = 0; s < other_syms; s++) {
				ConstBuffer& input_buf = inputs[3][s].core->GetBuffer(0);
				for(int i = 0; i < input_length; i++) {
					int j = pos - i;
					double d = input_buf.Get(j);
					d -= 0.5;
					d *= 2 * 50;
					tmp_mat.Set(matpos++, d);
				}
			}
		}
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
	if (do_test) range *= TRAININGAREA_FACTOR;
	range -= size;
	
	int pos = size + Random(range);
	
	LoadInput(pos);
	
	dqn.SetGamma(0);
	
	double error;
	if (have_actionmode == ACTIONMODE_SIGN) {
		int action = dqn.Act(tmp_mat);
		double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
		if (action) change *= -1;
		double reward = change >= 0 ? +10 : -10;
		error = dqn.Learn(tmp_mat, action, reward, NULL);
	}
	else if (have_actionmode == ACTIONMODE_TREND) {
		int action = dqn.Act(tmp_mat);
		double change1 = open_buf.Get(pos + 1) - open_buf.Get(pos);
		double change2 = open_buf.Get(pos) - open_buf.Get(pos - 1);
		bool trend_action = change2 < 0.0;
		if (action) trend_action = !trend_action;
		if (trend_action) change1 *= -1;
		double reward = change1 >= 0 ? +10 : -10;
		error = dqn.Learn(tmp_mat, action, reward, NULL);
	}
	else if (have_actionmode == ACTIONMODE_WEIGHTED) {
		int action = dqn.Act(tmp_mat);
		double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
		bool sign_action = action % 2;
		int sign_mul = 1 + action / 2;
		if (sign_action) change *= -1;
		double reward = change >= 0 ? +10 : -10;
		reward *= sign_mul;
		error = dqn.Learn(tmp_mat, action, reward, NULL);
	}
	else if (have_actionmode == ACTIONMODE_HACK) {
		double output[2];
		
		double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
		if (change >= 0) {
			output[0] = 0;
			output[1] = 1;
		} else {
			output[0] = 1;
			output[1] = 0;
		}
		
		error = dqn.Learn(tmp_mat, output);
	}
	else Panic("Action not implemented");
	
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
	int begin = bars * TRAININGAREA_FACTOR;
	
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
	
	fout << "have_normaldata = " << (const int)have_normaldata << "\r\n";
	fout << "have_normalma   = " << (const int)have_normalma << "\r\n";
	fout << "have_hurst      = " << (const int)have_hurst << "\r\n";
	fout << "have_anomaly    = " << (const int)have_anomaly << "\r\n";
	fout << "have_othersyms  = " << (const int)have_othersyms << "\r\n";
	fout << "have_actionmode = " << (const int)have_actionmode << "\r\n";
	fout << "input_length    = " << (const int)input_length << "\r\n";
	fout << "symbol          = " << symstr << "\r\n";
	fout << "timeframe       = " << tfstr << "\r\n";
	fout << "\r\n";
	fout << "signs_correct   = " << DblStr((double)signs_correct / (double)signs_total) << "\r\n";
	
	double drawdown = (neg_total / (neg_total + pos_total));
	fout << "drawdown        = " << DblStr(drawdown) << "\r\n";
	fout << "total, spreads  = " << DblStr(total) << "\r\n";
	
}
















MultiDqnAdvisor::MultiDqnAdvisor() {
	
}

void MultiDqnAdvisor::Init() {
	SetBufferColor(0, Red());
	SetCoreSeparateWindow();
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0);
}

void MultiDqnAdvisor::Start() {
	
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
