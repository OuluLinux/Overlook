#include "Overlook.h"

namespace Overlook {

AdvisorBase::AdvisorBase(int main_count, int main_visible) :
	main_count(main_count), main_visible(main_visible) {
	ASSERT(main_count > 0 && main_visible > 0 && main_visible <= main_count);
}

void AdvisorBase::BaseInit() {
	for(int i = 0; i < ADVISOR_PERIOD; i++) {
		int j = main_count + i;
		SetBufferColor(j, RainbowColor((double)i/ADVISOR_PERIOD));
		SetBufferLineWidth(j, 1);
	}
}

void AdvisorBase::EnableJobs() {
	#if ENABLE_TRAINING
	SetJobCount(1);
	
	open_buf = &GetInputBuffer(0, 0);
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJob(0, tf_str + "DQN Training")
		.SetBegin		(THISBACK(TrainingDQNBegin))
		.SetIterator	(THISBACK(TrainingDQNIterator))
		.SetEnd			(THISBACK(TrainingDQNEnd))
		.SetInspect		(THISBACK(TrainingDQNInspect))
		.SetCtrl		<TrainingDQNCtrl>();
	#endif
}

void AdvisorBase::LoadState(DQN::MatType& state, int cursor) {
	ConstBuffer& change_buf = GetBuffer(main_count - 1);
	SetSafetyLimit(cursor);
	
	for(int j = 0; j < INPUT_PERIOD; j++) {
		int pos = Upp::max(0, cursor - j);
		double change = change_buf.Get(pos);
		change = Upp::max(-1.0, Upp::min(+1.0, change));
		if (change >= 0.0) {
			state.Set(j * 2 + 0, change);
			state.Set(j * 2 + 1, 0.0);
		} else {
			state.Set(j * 2 + 0, 0.0);
			state.Set(j * 2 + 1, -change);
		}
	}
}

bool AdvisorBase::TrainingDQNBegin() {
	System& sys = GetSystem();
	
	data_count = sys.GetCountTf(GetTf());
	
	int bars = GetBars();
	ASSERT(bars > 0);
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	
	
	dqntraining_pts.SetCount(bars, 0);
	
	if (dqn_round < 0) {
		dqn_round = 0;
	}
	
	return true;
}

bool AdvisorBase::TrainingDQNIterator() {
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
				int pos = Random(Upp::min(data.GetCount(), dqn_round) - 1);
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

void AdvisorBase::RefreshAction(int cursor) {
	DQN::DQVector& before		= data[cursor];
	
	LoadState(tmp_before_state, cursor);
	dqn_trainer.Evaluate(tmp_before_state, before);
	
	for(int i = 0; i < ADVISOR_PERIOD; i++)
		GetBuffer(main_count + i).Set(cursor, before.weight[i]);
	
	GetOutput(0).label.Set(cursor, before.weight[0] < 0.0);
}

void AdvisorBase::RefreshReward(int cursor) {
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

void AdvisorBase::RunMain() {
	VectorBool& label		= GetOutput(0).label;
	
	int bars = GetBars();
	dqntraining_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	bool prev_signal = 0;
	
	label.SetCount(bars);
	
	for(int i = 0; i < bars; i++) {
		bool signal = label.Get(i);
		
		double curr		= open_buf->GetUnsafe(i);
		double next		= open_buf->GetUnsafe(i + 1);
		ASSERT(curr > 0.0);
		
		double change;
		if (!signal)	change = next / curr - 1.0;
		else			change = 1.0 - next / curr;
		
		change_total	*= 1.0 + change;
		
		dqntraining_pts[i] = change_total;
		
		prev_signal		= signal;
	}
	
	dqntraining_pts[bars] = change_total;
}

void AdvisorBase::RefreshAll() {
	#if ENABLE_TRAINING
	TimeStop ts;
	
	RefreshMain();
	LOG("AdvisorBase::Start ... RefreshMain " << ts.ToString());
	
	prev_counted = GetBars();
	#endif
}

void AdvisorBase::RefreshMain() {
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

bool AdvisorBase::TrainingDQNEnd() {
	ForceSetCounted(0);
	RefreshAll();
	return true;
}

bool AdvisorBase::TrainingDQNInspect() {
	bool succ = !dqntraining_pts.IsEmpty() && dqntraining_pts.Top() > 0.0;
	
	INSPECT(succ, "warning: negative result");
	
	return true;
}

void AdvisorBase::TrainingDQNCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	AdvisorBase* src = dynamic_cast<AdvisorBase*>(&*job->core);
	ASSERT(src);
	DrawVectorPolyline(id, sz, src->dqntraining_pts, polyline);
	
	w.DrawImage(0, 0, id);
}

}
