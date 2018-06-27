#include "Overlook.h"

namespace Overlook {

DqnAdvisor::DqnAdvisor() {
	
}

void DqnAdvisor::Init() {
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
	max_rounds = 1000000;
	#endif
	training_pts.SetCount(max_rounds, 0);
	
	
	if (round == 0) {
		dqn.Init(1, input_size, output_size);
		dqn.LoadInitJSON(
			"{\n"
			"\t\"update\":\"qlearn\",\n"
			"\t\"gamma\":0.9,\n"
			"\t\"epsilon\":0.2,\n"
			"\t\"alpha\":0.005,\n"
			"\t\"experience_add_every\":5,\n"
			"\t\"experience_size\":10000,\n"
			"\t\"learning_steps_per_iteration\":5,\n"
			"\t\"tderror_clamp\":1.0,\n"
			"\t\"num_hidden_units\":100,\n"
			"}\n");
		dqn.Reset();
		reward_sum = 0.0;
	}
	
	// Allow iterating
	return true;
}

void DqnAdvisor::LoadInput(int pos) {
	int matpos = 0;
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	double open = open_buf.Get(pos);
	
	tmp_mat.SetCount(input_size);
	level_dist.SetCount(level_count);
	level_len.SetCount(level_count);
	level_dir.SetCount(level_count);
	
	int sym_count = have_othersyms ? this->sym_count : 1;
	
	for(int s = 0; s < sym_count; s++) {
		ConstBuffer& input_buf = have_othersyms ? inputs[0][s].core->GetBuffer(0) : open_buf;
		
		for(int i = 0; i < window_count; i++) {
			int window_size = 1 << (5 + i*2);
			double begin = open - level_side * point;
			
			for(int j = 0; j < level_count; j++) {
				level_dist[j] = window_size;
				level_len[j] = 0;
				level_dir[j] = 0;
			}
			
			double prev;
			for(int j = 0; j < window_size; j++) {
				int dist = window_size - 1 - j;
				if (dist > pos)
					continue;
				double o = input_buf.Get(pos - dist);
				int k = (o - state_est) / point + level_side;
				if (k < 0 || k >= level_count)
					continue;
				
				level_dist[k] = dist;
				level_len[k]++;
				
				if (j != 0) {
					if (prev != o) {
						level_dir[k] = o > prev ? +1 : -1;
					}
				}
				prev = o;
			}
			
			for(int i = 0; i < level_count; i++) {
				double dist_sens = (double)level_dist[i] / (double)window_size;
				double len_sens = 1.0 - (double)level_len[i] / (double)window_size;
				double dir_sens = level_dir[i];
				
				tmp_mat.Set(matpos++, dist_sens);
				tmp_mat.Set(matpos++, len_sens);
				tmp_mat.Set(matpos++, dir_sens);
			}
		}
	}
	
	double speed_sens = state_speed / (level_side*point);
	double opendist_sens = (state_est - open) / (level_side*point);
	tmp_mat.Set(matpos++, speed_sens);
	tmp_mat.Set(matpos++, opendist_sens);
	
	if (state_orderisopen) {
		double orderest_sens = (state_est - state_orderopen) / (level_side*point);
		double orderprofit_sens = (open - state_orderopen) / (level_side*point);
		
		if (state_ordertype) {
			orderest_sens *= -1.0;
			orderprofit_sens *= -1.0;
		}
		
		tmp_mat.Set(matpos++, orderest_sens);
		tmp_mat.Set(matpos++, orderprofit_sens);
		tmp_mat.Set(matpos++, 0.0);
	} else {
		tmp_mat.Set(matpos++, 0);
		tmp_mat.Set(matpos++, 0);
		tmp_mat.Set(matpos++, 1.0);
	}
	
	ASSERT(matpos == input_size);
}

void DqnAdvisor::ResetState(int pos) {
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	double open = open_buf.Get(pos);
	state_speed = 0;
	state_est = open;
	state_orderopen = open;
	state_orderisopen = false;
	state_ordertype = false;
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
	
	pos = size + (round / acts_per_step) % range;
	if (round % (acts_per_step * range) == 0)
		ResetState(pos);
	
	LoadInput(pos);
	
	double prog = (double)round / (double)max_rounds;
	double epsilon = (1.0 - prog) * 0.2;
	dqn.SetEpsilon(epsilon);
	
	int action = dqn.Act(tmp_mat);
	
	double open = open_buf.Get(pos);
	if (action == ACTION_UP)
		state_speed += point * 0.005;
	else if (action == ACTION_DOWN)
		state_speed -= point * 0.005;
	if (state_est < open)
		state_speed += point * 0.001;
	else
		state_speed -= point * 0.001;
	state_est += state_speed;
	
	double max = open + level_side * point;
	double min = open - level_side * point;
	if (state_est > max) {
		state_est = max;
		state_speed = 0;
	}
	if (state_est < min) {
		state_est = min;
		state_speed = 0;
	}
	
	double reward = 0.0;
	double est = state_est - open;
	int estpips = (est + point * 0.5) / point;
	bool force_experience = false;
	if (!state_orderisopen) {
		if (estpips >= +4) {
			state_orderisopen = true;
			state_ordertype = false;
			state_orderopen = open + point*3; // spread
		}
		else if (estpips <= -4) {
			state_orderisopen = true;
			state_ordertype = true;
			state_orderopen = open - point*3;
		}
	} else {
		// If buy
		if (state_ordertype == false) {
			// If price is estimated to go even lower
			if (estpips < +4) {
				state_orderisopen = false;
				reward = +(open - state_orderopen) / point * 10;
				if (reward >= 0) force_experience = true;
				state_est = open;
				state_speed = 0;
			}
			else {
				reward = open >= state_orderopen ? 0.1 : -0.1;
			}
		} else {
			if (estpips > -4) {
				state_orderisopen = false;
				reward = -(open - state_orderopen) / point * 10;
				if (reward >= 0) force_experience = true;
				state_est = open;
				state_speed = 0;
			}
			/*else {
				reward = open <= state_orderopen ? 0.1 : -0.1;
			}*/
		}
	}
	
	dqn.Learn(reward, force_experience);
	
	
	reward_sum += reward;
	training_pts[round] = reward_sum;
	
	
	
	// Keep count of iterations
	round++;
	ReleaseLog(IntStr(round) + DblStr(reward_sum));
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	if (save_elapsed.Elapsed() > 60*1000) {
		save_elapsed.Reset();
		StoreCache();
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
	/*
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
		
		int action = dqn.Act(tmp_mat);
		sig = action ? -1 : +1;
		
		
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
	*/
}

void DqnAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	DqnAdvisor* ea = dynamic_cast<DqnAdvisor*>(&*job->core);
	ASSERT(ea);
	
	Size graphsz(400, sz.cy);
	DrawVectorPolyline(id, graphsz, ea->training_pts, polyline, ea->round);
	
	
	int sensor_width = 3;
	int x = sz.cx - sensor_width;
	for(int i = 0; i < ea->tmp_mat.GetCount(); i++) {
		double sens = ea->tmp_mat[i];
		int h = (sens + 1) / 2.0 * sz.cy;
		int y = sz.cy - h;
		id.DrawRect(x, y, sensor_width, h, Black);
		x -= sensor_width;
	}
	
	Rect picrect(graphsz.cx, 0, x + sensor_width, sz.cy);
	
	ConstBuffer& input_buf = ea->GetInputBuffer(0, 0);
	int pos = ea->pos;
	int window_size = 1 << 7;
	
	double min = ea->state_est;
	double max = ea->state_est;
	if (ea->state_orderisopen) {
		min = Upp::min(ea->state_orderopen, min);
		max = Upp::max(ea->state_orderopen, max);
	}
	for(int i = 0; i < window_size; i++) {
		int dist = window_size - 1 - i;
		if (dist > pos)
			continue;
		double o = input_buf.Get(pos - dist);
		if (o < min) min = o;
		if (o > max) max = o;
	}
	double diff = max - min;
	
	polyline.SetCount(window_size);
	double x_step = (double)(picrect.Width() - 1) / window_size;
	int h = picrect.Height();
	for(int i = 0; i < window_size; i++) {
		int dist = window_size - 1 - i;
		if (dist > pos)
			continue;
		double o = input_buf.Get(pos - dist);
		Point& pt = polyline[i];
		pt.x = picrect.left + i * x_step;
		pt.y = h - (o - min) / diff * h;
	}
	id.DrawPolyline(polyline, 2, Green);
	
	int box_h = 5;
	int box_h2 = box_h / 2;
	int box_w = 5;
	int y;
	if (ea->state_orderisopen) {
		x = picrect.right - box_w;
		y = h - (ea->state_orderopen - min) / diff * h - box_h2;
		id.DrawRect(x, y, box_w, box_h, Color(255, 42, 0));
	}
	y = h - (ea->state_est - min) / diff * h - box_h2;
	id.DrawRect(x, y, box_w, box_h, Color(28, 127, 255));
	
	
	w.DrawImage(0, 0, id);
}

}
