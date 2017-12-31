#include "Overlook.h"


namespace Overlook {
using namespace Upp;

PatternAdvisor::PatternAdvisor() {
	
}

void PatternAdvisor::Init() {
	SetCoreSeparateWindow();
	
	for(int i = 0; i < ADVISOR_PERIOD; i++) {
		SetBufferColor(i, RainbowColor((double)i / ADVISOR_PERIOD));
	}
	
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	spread_point = db->GetPoint();
	ASSERT(spread_point > 0.0);
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	
	open_buf   = &GetInputBuffer(0, 0);
	low_buf    = &GetInputBuffer(0, 1);
	high_buf   = &GetInputBuffer(0, 2);
	volume_buf = &GetInputBuffer(0, 3);
	
	#if ENABLE_TRAINING
	SetJobCount(1);
	
	SetJob(0, tf_str + "DQN Training")
		.SetBegin		(THISBACK(TrainingDQNBegin))
		.SetIterator	(THISBACK(TrainingDQNIterator))
		.SetEnd			(THISBACK(TrainingDQNEnd))
		.SetInspect		(THISBACK(TrainingDQNInspect))
		.SetCtrl		<TrainingDQNCtrl>();
	#endif
}

void PatternAdvisor::Start() {
	
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
	}
	
	int bars = GetBars();
	Output& out = GetOutput(0);
	for(int i = 0; i < out.buffers.GetCount(); i++) {
		ASSERT(bars == out.buffers[i].GetCount());
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("PatternAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

void PatternAdvisor::RefreshAll() {
	#if ENABLE_TRAINING
	RefreshSourcesOnlyDeep();
	
	TimeStop ts;
	
	RefreshOutputBuffers();
	LOG("PatternAdvisor::Start ... RefreshOutputBuffers " << ts.ToString());
	ts.Reset();
	
	RefreshMain();
	LOG("PatternAdvisor::Start ... RefreshMain " << ts.ToString());
	
	prev_counted = GetBars();
	#endif
}

void PatternAdvisor::RefreshAction(int cursor) {
	DQN::DQVector& before = data[cursor];
	
	LoadState(tmp_before_state, cursor);
	dqn_trainer.Evaluate(tmp_before_state, before);
	
	for(int i = 0; i < ADVISOR_PERIOD; i++)
		GetBuffer(i).Set(cursor, before.weight[i]);
	
	GetOutput(0).label.Set(cursor, before.weight[0] < 0.0);
}

void PatternAdvisor::RefreshReward(int cursor) {
	DQN::DQVector& current	= data[cursor];
	
	int last = GetBars() - 2;
	for(int i = 0; i < ADVISOR_PERIOD; i++) {
		int pos = cursor + i;
		if (pos > last)
			current.correct[i] = 0;
		else
			current.correct[i] = (open_buf->GetUnsafe(pos+1) / open_buf->GetUnsafe(pos) - 1.0) * 100.0;
	}
}

bool PatternAdvisor::TrainingDQNBegin() {
	int bars = GetBars();
	ASSERT(bars > 0);
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	
	RefreshOutputBuffers();
	
	dqntraining_pts.SetCount(bars, 0);
	
	if (dqn_round < 0) {
		dqn_round = 0;
	}
	
	return true;
}

bool PatternAdvisor::TrainingDQNIterator() {
	GetCurrentJob().SetProgress(dqn_round, dqn_max_rounds);
	
	ASSERT(!data.IsEmpty());
	
	double max_epsilon = 0.20;
	double min_epsilon = 0.00;
	double epsilon = (max_epsilon - min_epsilon) * (dqn_max_rounds - dqn_round) / dqn_max_rounds + min_epsilon;
	dqn_trainer.SetEpsilon(epsilon);
	dqn_trainer.SetGamma(0.0);
	
	for(int i = 0; i < 10; i++) {
		int cursor = dqn_round % (data.GetCount() - 1);
		RefreshAction(cursor);
		RefreshReward(cursor);
		
		if (dqn_round > 100) {
			for(int j = 0; j < 5; j++) {
				int pos = 2*ADVISOR_PERIOD + Random(Upp::min(data.GetCount() - ADVISOR_PERIOD*3, dqn_round-ADVISOR_PERIOD*2) - 1);
				if (pos < 0) continue;
				DQN::DQVector& before = data[pos];
				LoadState(tmp_before_state, pos);
				LoadState(tmp_after_state, pos+1);
				dqn_trainer.Learn(tmp_before_state, before, tmp_after_state);
			}
		}
		
		dqn_round++;
	}
	
	
	// This is only for progress drawer...
	RunMain();
	
	
	// Stop eventually
	if (dqn_round >= dqn_max_rounds) {
		SetJobFinished();
	}
	
	
	return true;
}

bool PatternAdvisor::TrainingDQNEnd() {
	ForceSetCounted(0);
	RefreshAll();
	return true;
}

bool PatternAdvisor::TrainingDQNInspect() {
	bool succ = !dqntraining_pts.IsEmpty() && dqntraining_pts.Top() > 0.0;
	
	INSPECT(succ, "warning: negative result");
	
	return true;
}








void PatternAdvisor::RunMain() {
	ConstVectorBool& label		= GetOutput(0).label;
	
	int bars = GetBars();
	dqntraining_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	
	bool prev_signal = 0;
	for(int i = 0; i < bars; i++) {
		bool signal		= label.Get(i);
		
		double curr		= open_buf->GetUnsafe(i);
		double next		= open_buf->GetUnsafe(i + 1);
		ASSERT(curr > 0.0);
		double change;
		
		if (prev_signal != signal) {
			if (!signal)	change = next / (curr + spread_point) - 1.0;
			else			change = 1.0 - next / (curr - spread_point);
		} else {
			if (!signal)	change = next / curr - 1.0;
			else			change = 1.0 - next / curr;
		}
		
		change_total	*= 1.0 + change;
		
		dqntraining_pts[i] = change_total;
		
		prev_signal		= signal;
	}
	
	dqntraining_pts[bars] = change_total;
}

void PatternAdvisor::RefreshOutputBuffers() {
	int bars = GetBars();
	
	ConstBuffer& open   = *open_buf;
	ConstBuffer& low    = *low_buf;
	ConstBuffer& high   = *high_buf;
	ConstBuffer& volume = *volume_buf;
	
	if (prev_counted < 0) prev_counted = 0; // hotfix. probably useless in future
	
	input_data.SetCount(bars * row_size);
	SetSafetyLimit(bars);
	data.SetCount(bars);
	
	int begin = Upp::max(ADVISOR_PERIOD*2, prev_counted);
	int col = begin * row_size;
	for(int cursor = begin; cursor < bars; cursor++) {
		// Open/Close trend
		{
			int dir = 0;
			int len = 0;
			for (int i = cursor-1; i >= 0; i--) {
				int idir = open.GetUnsafe(i+1) > open.GetUnsafe(i) ? +1 : -1;
				if (dir != 0 && idir != dir) break;
				dir = idir;
				len++;
			}
			
			if (len > 1)
				input_data.Set(col + (dir == +1 ? 0 : trend_max) + Upp::min(trend_max-1, log2_64(len)-1), true);
			col += 2 * trend_max;
		}
		
		// High trend
		{
			int dir = 0;
			int len = 0;
			for (int i = cursor-2; i >= 0; i--) {
				int idir = high.GetUnsafe(i+1) > high.GetUnsafe(i) ? +1 : -1;
				if (dir != 0 && idir != dir) break;
				dir = idir;
				len++;
			}
			if (len > 1)
				input_data.Set(col + (dir == +1 ? 0 : trend_max) + Upp::min(trend_max-1, log2_64(len)-1), true);
			col += 2 * trend_max;
		}
		
		// Low trend
		{
			int dir = 0;
			int len = 0;
			for (int i = cursor-2; i >= 0; i--) {
				int idir = low.GetUnsafe(i+1) < low.GetUnsafe(i) ? -1 : +1;
				if (dir != 0 && idir != dir) break;
				dir = idir;
				len++;
			}
			if (len > 1)
				input_data.Set(col + (dir == +1 ? 0 : trend_max) + Upp::min(trend_max-1, log2_64(len)-1), true);
			col += 2 * trend_max;
		}
		
		// Sideways trend
		{
			int dir = 0;
			int len = 0;
			for (int i = cursor-1; i >= 0; i--) {
				double lowhigh_av = 0.0;
				for(int j = i; j < cursor; j++)
					lowhigh_av += high.GetUnsafe(j) - low.GetUnsafe(j);
				lowhigh_av /= cursor - i;
				
				double change = fabs(open.GetUnsafe(cursor) - open.GetUnsafe(i));
				bool is_less = change <= lowhigh_av;
				if (dir != 0 && !is_less) break;
				dir = 1;
				len++;
			}
			if (len > 1)
				input_data.Set(col + Upp::min(trend_max-1, log2_64(len)-1), true);
			col += trend_max;
		}
		
		// High break
		{
			int dir = 0;
			int len = 0;
			double hi = high.GetUnsafe(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = hi > high.GetUnsafe(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			if (len > 1)
				input_data.Set(col + Upp::min(break_max-1, log2_64(len)-1), true);
			col += break_max;
		}
		
		// Low break
		{
			int dir = 0;
			int len = 0;
			double lo = low.GetUnsafe(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = lo < low.GetUnsafe(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			if (len > 1)
				input_data.Set(col + Upp::min(break_max-1, log2_64(len)-1), true);
			col += break_max;
		}
		
		// Trend reversal
		if (cursor >= 4) {
			double t0_diff		= open.GetUnsafe(cursor-0) - open.GetUnsafe(cursor-1);
			double t1_diff		= open.GetUnsafe(cursor-1) - open.GetUnsafe(cursor-2);
			double t2_diff		= open.GetUnsafe(cursor-2) - open.GetUnsafe(cursor-3);
			int t0 = t0_diff > 0 ? +1 : -1;
			int t1 = t1_diff > 0 ? +1 : -1;
			int t2 = t2_diff > 0 ? +1 : -1;
			if (t0 * t1 == -1 && t1 * t2 == +1) {
				double t0_hilodiff	= high.GetUnsafe(cursor-1) - low.GetUnsafe(cursor-1);
				if (fabs(t0_hilodiff) >= fabs(t1_diff)) {
					if (t0 == +1)	input_data.Set(col + 0, true);
					else			input_data.Set(col + 1, true);
				} else {
					if (t0 == +1)	input_data.Set(col + 2, true);
					else			input_data.Set(col + 3, true);
				}
			}
		}
		col += 4;
	}
}

void PatternAdvisor::LoadState(DQN::MatType& state, int cursor) {
	for(int i = 0; i < num_states; i++)
		state.Set(i, 1.0);
	
	if (cursor < ADVISOR_PERIOD*2) return;
	
	int input_col = (cursor+1) * row_size - 1;
	int output_col = 0;
	for (int p = 0; p < ADVISOR_PERIOD; p++) {
		for(int i = 0; i < row_size; i++) {
			if (input_data.Get(input_col))
				state.Set(output_col, 0.0);
			input_col--;
			output_col++;
		}
		if (input_col < 0) break;
	}
	
	ASSERT(output_col == num_states);
}

void PatternAdvisor::RefreshMain() {
	int bars					= GetBars();
	int cursor					= Upp::max(0, prev_counted-1);
	
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	
	dqn_trainer.SetEpsilon(0); // no random actions
	
	for(; cursor < bars; cursor++) {
		SetSafetyLimit(cursor);
		RefreshAction(cursor);
		RefreshReward(cursor);
	}
}

void PatternAdvisor::TrainingDQNCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	PatternAdvisor* rfa = dynamic_cast<PatternAdvisor*>(&*job->core);
	ASSERT(rfa);
	DrawVectorPolyline(id, sz, rfa->dqntraining_pts, polyline);
	
	w.DrawImage(0, 0, id);
}


}
