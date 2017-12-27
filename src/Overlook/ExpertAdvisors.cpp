#include "Overlook.h"


namespace Overlook {
using namespace Upp;

DqnAdvisor::DqnAdvisor() {
	
}

void DqnAdvisor::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1.0);  // normalized
	SetCoreMaximum(+1.0);   // normalized
	
	SetBufferColor(0, Color(28, 42, 255));
	SetBufferColor(1, Color(255, 42, 0));
	SetBufferColor(2, Color(28, 255, 42));
	
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	open_buf = &GetInputBuffer(0, 0);
	spread_point = db->GetPoint();
	ASSERT(spread_point > 0.0);
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	SetJob(0, tf_str + "DQN Training")
		.SetBegin		(THISBACK(TrainingDQNBegin))
		.SetIterator	(THISBACK(TrainingDQNIterator))
		.SetEnd			(THISBACK(TrainingDQNEnd))
		.SetInspect		(THISBACK(TrainingDQNInspect))
		.SetCtrl		<TrainingDQNCtrl>();
	
}

void DqnAdvisor::Start() {
	
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
			LOG("DqnAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

void DqnAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	
	TimeStop ts;
	
	RefreshOutputBuffers();
	LOG("DqnAdvisor::Start ... RefreshOutputBuffers " << ts.ToString());
	ts.Reset();
	
	RefreshMain();
	LOG("DqnAdvisor::Start ... RefreshMain " << ts.ToString());
	
	prev_counted = GetBars();
}

void DqnAdvisor::RefreshAction(int cursor) {
	DQN::DQItem& before		= data[cursor];
	Buffer& sig0_dqnprob	= GetBuffer(0);
	Buffer& sig1_dqnprob	= GetBuffer(1);
	Buffer& sig2_dqnprob	= GetBuffer(2);
	
	LoadState(tmp_before_state, cursor);
	before.action = dqn_trainer.Act(tmp_before_state);
	
	sig0_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(0));
	sig1_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(1));
	sig2_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(2));
}

int DqnAdvisor::GetAction(DQN::DQItem& before, int cursor) {
	return before.action;
}

void DqnAdvisor::RefreshReward(int cursor) {
	DQN::DQItem& current	= data[cursor];
	VectorBool& label		= GetOutput(0).label;
	VectorBool& enabled		= GetOutput(1).label;
	double curr				= open_buf->GetUnsafe(cursor);
	double next				= cursor + 1 >= open_buf->GetCount() ? curr : open_buf->GetUnsafe(cursor + 1);
	int action				= GetAction(current, cursor);
	int prev_action			= cursor > 0 ? GetAction(data[cursor-1], cursor-1) : ACTION_IDLE;
	
	
	double change;
	if (action != ACTION_IDLE) {
		if (prev_action != action) {
			if (!action)		change = next / (curr + spread_point) - 1.0;
			else				change = 1.0 - next / (curr - spread_point);
		} else {
			if (!action)		change = next / curr - 1.0;
			else				change = 1.0 - next / curr;
		}
	} else {
		change = 0;
	}
	
	
	current.reward = change * 10000;
	
	
	label.Set(cursor, action);
	enabled.Set(cursor, action != ACTION_IDLE);
}

bool DqnAdvisor::TrainingDQNBegin() {
	
	SetRealArea();
	
	int bars = GetBars();
	ASSERT(bars > 0);
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	GetOutput(1).label.SetCount(bars);
	
	RefreshOutputBuffers();
	
	dqntraining_pts.SetCount(bars, 0);
	
	if (dqn_round < 0) {
		dqn_round = 0;
	}
	
	return true;
}

bool DqnAdvisor::TrainingDQNIterator() {
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
		
		if (dqn_round > 100) {
			for(int j = 0; j < 5; j++) {
				int pos = Random(Upp::min(data.GetCount(), dqn_round) - 1);
				DQN::DQItem& before = data[pos];
				LoadState(tmp_before_state, pos);
				LoadState(tmp_after_state, pos+1);
				dqn_trainer.LearnAny(tmp_before_state, before.action, before.reward, tmp_after_state);
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

bool DqnAdvisor::TrainingDQNEnd() {
	ForceSetCounted(0);
	RefreshAll();
	return true;
}

bool DqnAdvisor::TrainingDQNInspect() {
	bool succ = !dqntraining_pts.IsEmpty() && dqntraining_pts.Top() > 0.0;
	
	INSPECT(succ, "warning: negative result");
	
	return true;
}








void DqnAdvisor::RunMain() {
	ConstVectorBool& label		= GetOutput(0).label;
	ConstVectorBool& enabled	= GetOutput(1).label;
	
	int bars = GetBars();
	dqntraining_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	
	bool prev_signal = 0, prev_is_enabled = 0;
	for(int i = 0; i < bars; i++) {
		bool signal		= label.Get(i);
		bool is_enabled	= enabled.Get(i);
		
		if (is_enabled) {
			double curr		= open_buf->GetUnsafe(i);
			double next		= open_buf->GetUnsafe(i + 1);
			ASSERT(curr > 0.0);
			double change;
			
			if (!prev_is_enabled || prev_signal != signal) {
				if (!signal)	change = next / (curr + spread_point) - 1.0;
				else			change = 1.0 - next / (curr - spread_point);
			} else {
				if (!signal)	change = next / curr - 1.0;
				else			change = 1.0 - next / curr;
			}
			
			change_total	*= 1.0 + change;
		}
		
		dqntraining_pts[i] = change_total;
		
		prev_signal		= signal;
		prev_is_enabled	= is_enabled;
	}
	dqntraining_pts[bars] = change_total;
}

void DqnAdvisor::SetRealArea() {
	int week				= 1*5*24*60 / MAIN_PERIOD_MINUTES;
	int data_count			= open_buf->GetCount();
	area.train_begin		= week;
	area.train_end			= data_count - 2;
	area.test0_begin		= data_count - 2;
	area.test0_end			= data_count - 1;
	area.test1_begin		= data_count - 1;
	area.test1_end			= data_count;
}

void DqnAdvisor::RefreshOutputBuffers() {
	int bars = GetBars();
	
	if (prev_counted < 0) prev_counted = 0; // hotfix. probably useless in future
	
	if (full_mask.GetCount() != bars)
		full_mask.SetCount(bars).One();
	
	SetSafetyLimit(bars);
	
	data.SetCount(bars);
	
	bufs.SetCount(0);
	bufs.Reserve(4*4);
	for(int i = 0; i < SRC_COUNT*PERIOD_COUNT; i++) {
		int sym = (i % SRC_COUNT) < 2 ? GetSymbol() : GetSystem().GetStrongSymbol();
		for(int j = 1; j < 3; j++)
			bufs.Add(&GetInputBuffer(1 + i, sym, GetTf(), -1 - j));
	}
	ASSERT(INPUT_COUNT == bufs.GetCount());
}

void DqnAdvisor::LoadState(DQN::MatType& state, int cursor) {
	SetSafetyLimit(cursor);
	
	int col = 0;
	for(int j = 0; j < bufs.GetCount(); j++) {
		ConstBuffer& buf = *bufs[j];
		for(int k = 0; k < INPUT_PERIOD; k++) {
			double value = buf.Get(Upp::max(0, cursor - k));
			if (!IsFin(value)) {
				LOG("warning: not finite " << j << " " << (cursor - k));
				value = 0.0;
			}
			state.Set(col++, value);
		}
	}
	ASSERT(col == INPUT_COUNT * INPUT_PERIOD);
}

void DqnAdvisor::RefreshMain() {
	int bars					= GetBars();
	int cursor					= Upp::max(0, prev_counted-1);
	
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	GetOutput(1).label.SetCount(bars);
	
	for(; cursor < bars; cursor++) {
		SetSafetyLimit(cursor);
		DQN::DQItem& before = data[cursor];
		
		RefreshAction(cursor);
		RefreshReward(cursor);
	}
}

void DqnAdvisor::TrainingDQNCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	DqnAdvisor* rfa = dynamic_cast<DqnAdvisor*>(&*job->core);
	ASSERT(rfa);
	DrawVectorPolyline(id, sz, rfa->dqntraining_pts, polyline);
	
	w.DrawImage(0, 0, id);
}


}
