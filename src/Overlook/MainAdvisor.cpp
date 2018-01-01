#include "Overlook.h"


namespace Overlook {
using namespace Upp;

MainAdvisor::MainAdvisor() {
	
}

void MainAdvisor::Init() {
	System& sys = GetSystem();
	
	if (GetSymbol() != sys.GetAccountSymbol()) return;
	
	SetCoreSeparateWindow();
	
	SetBufferColor(0, RainbowColor(Randomf()));
	
	int tf = GetTf();
	for (int i = 0; i < SYM_COUNT; i++) {
		DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, sys.GetPrioritySymbol(i), tf));
		spread_point[i] = db->GetPoint();
		ASSERT(spread_point[i] > 0.0);
	}
	for (int i = 0; i < SYM_COUNT+1; i++) {
		int sym = i < SYM_COUNT ? sys.GetPrioritySymbol(i) : sys.GetStrongSymbol();
		open_buf[i] = &GetInputBuffer(0, sym, tf, 0);
		ASSERT(open_buf[i] != NULL);
	}
	
	String tf_str = GetSystem().GetPeriodString(tf) + " ";
	
	SetJobCount(1);
	
	SetJob(0, tf_str + "DQN Training")
		.SetBegin		(THISBACK(TrainingDQNBegin))
		.SetIterator	(THISBACK(TrainingDQNIterator))
		.SetEnd			(THISBACK(TrainingDQNEnd))
		.SetInspect		(THISBACK(TrainingDQNInspect))
		.SetCtrl		<TrainingDQNCtrl>();
}

void MainAdvisor::Start() {
	
	if (GetSymbol() != GetSystem().GetAccountSymbol()) return;
	
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
			LOG("MainAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

void MainAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	
	TimeStop ts;
	
	RefreshOutputBuffers();
	LOG("MainAdvisor::Start ... RefreshOutputBuffers " << ts.ToString());
	ts.Reset();
	
	RefreshMain();
	LOG("MainAdvisor::Start ... RefreshMain " << ts.ToString());
	
	prev_counted = GetBars();
}

void MainAdvisor::RefreshAction(int cursor) {
	DQN::DQVector& before		= data[cursor];
	
	LoadState(tmp_before_state, cursor);
	dqn_trainer.Evaluate(tmp_before_state, before);
}

void MainAdvisor::RefreshReward(int cursor) {
	DQN::DQVector& current	= data[cursor];
	
	
	int last = GetBars() - 2;
	
	int col = 0;
	for(int i = 0; i < SYM_COUNT+1; i++) {
		for(int j = 0; j < ADVISOR_PERIOD; j++) {
			int pos = cursor + j;
			if (pos > last)
				current.correct[col++] = 0;
			else
				current.correct[col++] = (open_buf[i]->GetUnsafe(pos+1) / open_buf[i]->GetUnsafe(pos) - 1.0) * 100.0;
		}
	}
	ASSERT(col == OUTPUT_SIZE);
	
	
	double sum_sq = 0.0;
	for(int i = 0; i < OUTPUT_SIZE; i++) {
		double err = current.weight[i] - current.correct[i];
		sum_sq += err * err;
	}
	double mse = (double)sum_sq / OUTPUT_SIZE;
	GetBuffer(0).Set(cursor, mse);
}

bool MainAdvisor::TrainingDQNBegin() {
	int bars = GetBars();
	ASSERT(bars > 0);
	data.SetCount(bars);
	
	RefreshOutputBuffers();
	
	dqntraining_pts.SetCount(bars, 0);
	
	if (dqn_round < 0) {
		dqn_round = 0;
	}
	
	for(int i = 0; i < inputs.GetCount(); i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				if (core && !core->IsJobsFinished())
					return false;
			}
		}
	}
	
	return true;
}

bool MainAdvisor::TrainingDQNIterator() {
	GetCurrentJob().SetProgress(dqn_round, dqn_max_rounds);
	
	ASSERT(!data.IsEmpty());
	
	double max_epsilon = 0.20;
	double min_epsilon = 0.00;
	double epsilon = (max_epsilon - min_epsilon) * (dqn_max_rounds - dqn_round) / dqn_max_rounds + min_epsilon;
	dqn_trainer.SetEpsilon(epsilon);
	dqn_trainer.SetGamma(0.01);
	
	for(int i = 0; i < 10; i++) {
		int cursor = dqn_round % (data.GetCount() - 1);
		RefreshAction(cursor);
		RefreshReward(cursor);
		
		for(int j = 0; j < 5; j++) {
			int count = Upp::min(data.GetCount() - ADVISOR_PERIOD*3, dqn_round-ADVISOR_PERIOD*2) - 1;
			if (count < 1) break;
			int pos = 2*ADVISOR_PERIOD + Random(count);
			if (pos < 0 || pos >= data.GetCount()) continue;
			DQN::DQVector& before = data[pos];
			LoadState(tmp_before_state, pos);
			LoadState(tmp_after_state, pos+1);
			dqn_trainer.Learn(tmp_before_state, before, tmp_after_state);
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

bool MainAdvisor::TrainingDQNEnd() {
	ForceSetCounted(0);
	RefreshAll();
	return true;
}

bool MainAdvisor::TrainingDQNInspect() {
	bool succ = !dqntraining_pts.IsEmpty() && dqntraining_pts.Top() > 0.0;
	
	INSPECT(succ, "warning: negative result");
	
	return true;
}








void MainAdvisor::RunMain() {
	int bars = Upp::min(data.GetCount(), GetBars());
	dqntraining_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	
	const int peek_period = Upp::min(ADVISOR_PERIOD-1, 15);
	
	bool prev_signal[SYM_COUNT];
	for(int i = 0; i < SYM_COUNT; i++) prev_signal[i] = 0;
	
	for(int i = 0; i < bars; i++) {
		DQN::DQVector& current = data[i];
		double change_sum = 0.0;
		for (int j = 0; j < SYM_COUNT; j++) {
			int begin = j * ADVISOR_PERIOD;
			int end = begin + peek_period;
			double weight_sum = 0.0;
			for(int k = begin; k < end; k++)
				weight_sum += current.weight[k];
			bool signal = weight_sum < 0.0;
			double curr		= open_buf[j]->GetUnsafe(i);
			double next		= open_buf[j]->GetUnsafe(i + 1);
			ASSERT(curr > 0.0);
			double change;
			
			if (prev_signal[j] != signal) {
				if (!signal)	change = next / (curr + spread_point[j]) - 1.0;
				else			change = 1.0 - next / (curr - spread_point[j]);
			} else {
				if (!signal)	change = next / curr - 1.0;
				else			change = 1.0 - next / curr;
			}
			
			prev_signal[j] = signal;
			change_sum += change;
		}
		change_total	*= 1.0 + change_sum;
		dqntraining_pts[i] = change_total;
	}
	dqntraining_pts[bars] = change_total;
}

void MainAdvisor::RefreshOutputBuffers() {
	int bars = GetBars();
	
	if (prev_counted < 0) prev_counted = 0; // hotfix. probably useless in future
	
	SetSafetyLimit(bars);
	
	data.SetCount(bars);
	
	bufs.SetCount(0);
	bufs.Reserve(INPUTBUF_COUNT + 2);
	int tf = GetTf();
	for(int i = 0; i < SYM_COUNT+1; i++) {
		int sym = i < SYM_COUNT ? GetSystem().GetPrioritySymbol(i) : GetSystem().GetStrongSymbol();
		for(int j = 0; j < SYMBOLBUF_COUNT; j++) {
			bufs.Add(&GetInputBuffer(1 + j, sym, tf, -1));
		}
	}
	ASSERT(INPUTBUF_COUNT == bufs.GetCount());
}

void MainAdvisor::LoadState(DQN::MatType& state, int cursor) {
	SetSafetyLimit(cursor);
	
	int col = 0;
	
	// Time of week
	Time t = GetSystem().GetTimeTf(GetTf(), cursor);
	int wday = DayOfWeek(t) - 1;
	double hour = (t.hour * 60 + t.minute) / (24.0 * 60.0);
	double week = ((wday * 24 + t.hour) * 60 + t.minute) / (5.0 * 24.0 * 60.0);
	ASSERT(week >= 0.0 && week < 1.0);
	state.Set(col++, hour);
	state.Set(col++, week);
	
	for(int j = 0; j < bufs.GetCount(); j++) {
		ConstBuffer& buf = *bufs[j];
		for(int k = 0; k < INPUT_PERIOD; k++) {
			double value = buf.GetUnsafe(Upp::max(0, cursor - k));
			if (!IsFin(value)) {
				LOG("warning: not finite " << j << " " << (cursor - k));
				value = 0.0;
			}
			state.Set(col++, value);
		}
	}
	ASSERT(col == INPUT_SIZE);
}

void MainAdvisor::RefreshMain() {
	int bars					= GetBars();
	int cursor					= Upp::max(0, prev_counted-1);
	
	data.SetCount(bars);
	
	dqn_trainer.SetEpsilon(0); // no random actions
	
	for(; cursor < bars; cursor++) {
		SetSafetyLimit(cursor);
		DQN::DQVector& before = data[cursor];
		
		RefreshAction(cursor);
		RefreshReward(cursor);
	}
}

void MainAdvisor::TrainingDQNCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	MainAdvisor* rfa = dynamic_cast<MainAdvisor*>(&*job->core);
	ASSERT(rfa);
	DrawVectorPolyline(id, sz, rfa->dqntraining_pts, polyline);
	
	w.DrawImage(0, 0, id);
}


}
