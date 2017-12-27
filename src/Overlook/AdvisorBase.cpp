#include "Overlook.h"

namespace Overlook {

AdvisorBase::AdvisorBase(int main_count, int main_visible) :
	main_count(main_count), main_visible(main_visible) {
	ASSERT(main_count > 0 && main_visible > 0 && main_visible <= main_count);
}

void AdvisorBase::BaseInit() {
	for(int i = 0; i < RF_COUNT; i++) {
		int j = main_count + i;
		SetBufferColor(j, Color(0,128,128));
		SetBufferLineWidth(j, 4);
	}
}

void AdvisorBase::EnableJobs() {
	SetJobCount(1);
	
	open_buf = &GetInputBuffer(0, 0);
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	/*
	SetJob(0, tf_str + "Random Forest Training")
		.SetBegin		(THISBACK(TrainingRFBegin))
		.SetIterator	(THISBACK(TrainingRFIterator))
		.SetEnd			(THISBACK(TrainingRFEnd))
		.SetInspect		(THISBACK(TrainingRFInspect))
		.SetCtrl		<TrainingRFCtrl>();
	*/
	
	SetJob(0, tf_str + "DQN Training")
		.SetBegin		(THISBACK(TrainingDQNBegin))
		.SetIterator	(THISBACK(TrainingDQNIterator))
		.SetEnd			(THISBACK(TrainingDQNEnd))
		.SetInspect		(THISBACK(TrainingDQNInspect))
		.SetCtrl		<TrainingDQNCtrl>();
}

bool AdvisorBase::TrainingRFBegin() {
	System& sys = GetSystem();
	
	data_count = sys.GetCountTf(GetTf());
	
	full_mask.SetCount(data_count).One();
	
	training_pts.SetCount(RF_COUNT, 0.0);
	rflist.SetCount(RF_COUNT);
	
	rf_trainer.options.tree_count		= 100;
	rf_trainer.options.max_depth		= 4;
	rf_trainer.options.tries_count		= 10;
	
	return true;
}

bool AdvisorBase::TrainingRFIterator() {
	ASSERT(full_mask.GetCount() > 0 && rflist_iter >= 0);
	
	int a = rflist_iter;
	int t = rflist.GetCount();
	GetCurrentJob().SetProgress(a, t);
	
	
	RF& rf = rflist[rflist_iter];
	
	VectorBool& real_mask = rf.b;
	real_mask.SetCount(data_count);
	ConstBuffer& change_buf = GetBuffer(main_count - 1);
	int pos = 0;
	if (rflist_iter == 0) {
		for(int i = 0; i < data_count-1; i++) {
			bool label = change_buf.Get(i+1) < 0.0;
			real_mask.Set(i, label);
			if (label) pos++;
		}
	}
	if (rflist_iter == 1) {
		for(int i = 0; i < data_count-2; i++) {
			bool label0 = change_buf.Get(i+1) < 0.0;
			bool label1 = change_buf.Get(i+2) < 0.0;
			bool label  = label0 == label1;
			real_mask.Set(i, label);
		}
	}
	
	rf_trainer.forest.memory.Attach(&rf.a);
	
	ConstBufferSource bufs;
	bufs.SetSerialDepth(INPUT_PERIOD);
	bufs.SetSource(0, change_buf);
	
	area.train_begin		= INPUT_PERIOD*2;
	area.train_end			= data_count - 2;
	area.test0_begin		= data_count - 2;
	area.test0_end			= data_count - 1;
	area.test1_begin		= data_count - 1;
	area.test1_end			= data_count;
	
	rf_trainer.Process(area, bufs, real_mask, full_mask);
	
	LOG(GetSymbol() << ": SOURCE " << rflist_iter << ": train_accuracy: " << rf_trainer.stat.train_accuracy << " test_accuracy: " << rf_trainer.stat.test0_accuracy);
	
	
	// This is only for progress drawer...
	if (rflist_iter == 0) {
		int cursor = 0;
		ConstBufferSourceIter iter(bufs, &cursor);
		Buffer& buf = GetBuffer(main_count + rflist_iter);
		for(; cursor < bars; cursor++) {
			SetSafetyLimit(cursor);
			double prob = rf_trainer.forest.PredictOne(iter);
			double d = prob * -2.0 + 1.0;
			buf.Set(cursor, d);
		}
		RunMainRF();
	}
	rf_trainer.forest.memory.Detach();
	
	
	rflist_iter++;
	if (rflist_iter >= rflist.GetCount())
		SetJobFinished();
	
	return true;
}

bool AdvisorBase::TrainingRFEnd() {
	ForceSetCounted(0);
	RefreshOutputBuffers();
	return true;
}

bool AdvisorBase::TrainingRFInspect() {
	
	// Whatever...
	
	return true;
}

void AdvisorBase::RefreshOutputBuffers() {
	
	ConstBufferSource bufs;
	
	for(int i = 0; i < rflist.GetCount(); i++) {
		RF& rf = rflist[i];
		
		VectorBool& real_mask = rf.b;
		
		rf_trainer.forest.memory.Attach(&rf.a);
		
		bufs.SetSerialDepth(INPUT_PERIOD);
		bufs.SetSource(0, GetBuffer(main_count - 1));
		
		int cursor = prev_counted;
		ConstBufferSourceIter iter(bufs, &cursor);
		
		Buffer& buf = GetBuffer(main_count + i);
		
		for(; cursor < bars; cursor++) {
			SetSafetyLimit(cursor);
			double prob = rf_trainer.forest.PredictOne(iter);
			double d = prob * -2.0 + 1.0;
			buf.Set(cursor, d);
		}
		rf_trainer.forest.memory.Detach();
	}
	
	data.SetCount(bars);
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
	
	area.train_begin		= INPUT_PERIOD*2;
	area.train_end			= data_count - 2;
	area.test0_begin		= data_count - 2;
	area.test0_end			= data_count - 1;
	area.test1_begin		= data_count - 1;
	area.test1_end			= data_count;
	
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
				DQN::DQItem& before = data[pos];
				LoadState(tmp_before_state, pos);
				LoadState(tmp_after_state, pos+1);
				dqn_trainer.LearnAny(tmp_before_state, before.action, before.reward, tmp_after_state);
			}
		}
		
		dqn_round++;
	}
	
	
	// This is only for progress drawer...
	RunMainDQN();
	
	
	// Stop eventually
	if (dqn_round >= dqn_max_rounds) {
		SetJobFinished();
	}
	
	
	return true;
}

void AdvisorBase::RefreshAction(int cursor) {
	DQN::DQItem& before		= data[cursor];
	LoadState(tmp_before_state, cursor);
	before.action = dqn_trainer.Act(tmp_before_state);
	
	Buffer& sig0_dqnprob	= GetBuffer(main_count + RF_COUNT + 0);
	Buffer& sig1_dqnprob	= GetBuffer(main_count + RF_COUNT + 1);
	Buffer& sig2_dqnprob	= GetBuffer(main_count + RF_COUNT + 2);
	sig0_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(0));
	sig1_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(1));
	sig2_dqnprob.Set(cursor, dqn_trainer.data.add2.output.Get(2));
		
	GetOutput(0).label.Set(cursor, before.action);
}

int AdvisorBase::GetAction(DQN::DQItem& before, int cursor) {
	return before.action;
}

void AdvisorBase::RefreshReward(int cursor) {
	DQN::DQItem& current	= data[cursor];
	double curr				= open_buf->GetUnsafe(cursor);
	double next				= cursor + 1 >= open_buf->GetCount() ? curr : open_buf->GetUnsafe(cursor + 1);
	int action				= GetAction(current, cursor);
	int prev_action			= cursor > 0 ? GetAction(data[cursor-1], cursor-1) : ACTION_IDLE;
	
	double change;
	if (action != ACTION_IDLE) {
		if (!action)		change = next / curr - 1.0;
		else				change = 1.0 - next / curr;
	} else {
		change = 0;
	}
	
	current.reward = change * 10000;
}

void AdvisorBase::RunMainRF() {
	int bars = GetBars();
	training_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	bool prev_signal = 0;
	
	Buffer& buf = GetBuffer(main_count);
	
	for(int i = 0; i < bars; i++) {
		bool signal		= buf.Get(i) < 0.0;
		
		double curr		= open_buf->GetUnsafe(i);
		double next		= open_buf->GetUnsafe(i + 1);
		ASSERT(curr > 0.0);
		
		double change;
		if (!signal)	change = next / curr - 1.0;
		else			change = 1.0 - next / curr;
		
		change_total	*= 1.0 + change;
		
		training_pts[i] = change_total;
		
		prev_signal		= signal;
	}
	
	training_pts[bars] = change_total;
}

void AdvisorBase::RunMainDQN() {
	int bars = GetBars();
	dqntraining_pts.SetCount(bars, 0.0);
	bars--;
	
	double change_total = 1.0;
	bool prev_signal = 0, prev_is_enabled = 0;
	
	GetOutput(0).label.SetCount(bars);
	
	for(int i = 0; i < bars; i++) {
		int action		= data[i].action;
		bool signal		= action;
		bool is_enabled	= action != ACTION_IDLE;
		
		if (is_enabled) {
			double curr		= open_buf->GetUnsafe(i);
			double next		= open_buf->GetUnsafe(i + 1);
			ASSERT(curr > 0.0);
			
			double change;
			if (!signal)	change = next / curr - 1.0;
			else			change = 1.0 - next / curr;
			
			change_total	*= 1.0 + change;
		}
		
		GetOutput(0).label.Set(i, signal);
		//GetOutput(1).label.Set(cursor, is_enabled);
		
		dqntraining_pts[i] = change_total;
		
		prev_signal		= signal;
		prev_is_enabled	= is_enabled;
	}
	
	dqntraining_pts[bars] = change_total;
}

void AdvisorBase::RefreshAll() {
	TimeStop ts;
	
	RefreshOutputBuffers();
	LOG("AdvisorBase::Start ... RefreshOutputBuffers " << ts.ToString());
	ts.Reset();
	
	RefreshMain();
	LOG("AdvisorBase::Start ... RefreshMain " << ts.ToString());
	
	prev_counted = GetBars();
}

void AdvisorBase::RefreshMain() {
	int bars					= GetBars();
	int cursor					= Upp::max(0, prev_counted-1);
	
	data.SetCount(bars);
	GetOutput(0).label.SetCount(bars);
	
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

void AdvisorBase::TrainingRFCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	AdvisorBase* src = dynamic_cast<AdvisorBase*>(&*job->core);
	ASSERT(src);
	DrawVectorPolyline(id, sz, src->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
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
