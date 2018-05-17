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
	if (do_test) range *= 0.66;
	range -= size;
	
	int pos = size + Random(range);
	
		
	sequence.SetCount(size);
	for(int i = 0; i < size; i++) {
		int j = pos - size + i;
		double change = open_buf.Get(j+1) - open_buf.Get(j);
		sequence[i] = ChangeToChar(change);
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
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	for(int i = 0; i < input_length; i++) {
		int j = pos - input_length + i;
		double change = open_buf.Get(j+1) - open_buf.Get(j);
		if (change >= 0) {
			tmp_mat.Set(i * 2 + 0, 1.0 - +change / point / 5);
			tmp_mat.Set(i * 2 + 1, 1.0);
		} else {
			tmp_mat.Set(i * 2 + 0, 1.0);
			tmp_mat.Set(i * 2 + 1, 1.0 - -change / point / 5);
		}
	}
}

bool DqnAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	int size = input_length;
	int range = GetBars() - 1;
	if (do_test) range *= 0.66;
	range -= size;
	
	int pos = size + Random(range);
	
	LoadInput(pos);
	
	double output[2];
	
	double change = open_buf.Get(pos + 1) - open_buf.Get(pos);
	if (change >= 0) {
		output[0] = 0;
		output[1] = 1;
	} else {
		output[0] = 1;
		output[1] = 0;
	}
	
	dqn.SetGamma(0);
	double error = dqn.Learn(tmp_mat, output);
	
	
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
	if (prev_counted < input_length) prev_counted = input_length;
	for(int i = prev_counted; i < bars; i++) {
		
		LoadInput(i);
		double output[2];
		dqn.Evaluate(tmp_mat, output, 2);
		
		int sig = output[0] < 0.5 || output[1] < 0.5 ? (output[0] > output[1] ? -1 : +1) : 0;
		
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

void DqnAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	DqnAdvisor* ea = dynamic_cast<DqnAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}


}
