#include "Overlook.h"


namespace Overlook {
using namespace Upp;


AccountAdvisor::AccountAdvisor() {
	
}

void AccountAdvisor::Init() {
	System& sys = GetSystem();
	
	if (GetSymbol() != sys.GetAccountSymbol()) return;
	if (GetMinutePeriod() != 15) return;
	
	SetCoreSeparateWindow();
	
	SetBufferColor(0, RainbowColor(Randomf()));
	
	tf_ids[0] = sys.FindPeriod(15);
	tf_ids[1] = sys.FindPeriod(60);
	tf_ids[2] = sys.FindPeriod(240);
	ASSERT(GetTf() == tf_ids[0]);
	
	
	tf_step[0] = 60 / 15;
	tf_step[1] = 240 / 60;
	tf_step[2] = 1;
	
	tf_div[0] = 1;
	tf_div[1] = tf_step[0];
	tf_div[2] = tf_step[0] * tf_step[1];
	
	int buf = 0;
	for(int j = 0; j < tf_count; j++) {
		int tf = tf_ids[j];
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = i < SYM_COUNT ? sys.GetPrioritySymbol(i) : sys.GetStrongSymbol();
			open_buf[buf] = &GetInputBuffer(0, sym, tf, 0);
			ASSERT(open_buf[buf] != NULL);
			buf++;
		}
	}
	
	int tf = GetTf();
	for (int i = 0; i < SYM_COUNT; i++) {
		DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, sys.GetPrioritySymbol(i), tf));
		spread_point[i] = db->GetPoint();
		ASSERT(spread_point[i] > 0.0);
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

void AccountAdvisor::Start() {
	System& sys = GetSystem();
	
	if (GetSymbol() != sys.GetAccountSymbol()) return;
	if (GetMinutePeriod() != 15) return;
	
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		RefreshSourcesOnlyDeep();
	}
	
	
	int bars					= GetBars();
	int tf						= GetTf();
	CoreIO* core				= CoreIO::GetInputCore(CORE_COUNT, GetSymbol(), tf);
	MainAdvisor* ma				= dynamic_cast<MainAdvisor*>(core);
	ASSERT(ma);
	bool sources_finished		= IsJobsFinished() && ma->IsJobsFinished() && ma->prev_counted == sys.GetCountTf(tf);
	
	if (sources_finished && prev_counted < bars) {
		RefreshSourcesOnlyDeep();
		
		LOG("AccountAdvisor::Start Refresh");
		RefreshAll();
	}
}

void AccountAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	
	TimeStop ts;
	
	RefreshOutputBuffers();
	LOG("AccountAdvisor::Start ... RefreshOutputBuffers " << ts.ToString());
	ts.Reset();
	
	RefreshMain();
	LOG("AccountAdvisor::Start ... RefreshMain " << ts.ToString());
	
	RunSimBroker();
	MainReal();
	
	prev_counted = GetBars();
}

void AccountAdvisor::RefreshMain() {
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

void AccountAdvisor::RefreshAction(int cursor) {
	DQN::DQVector& before		= data[cursor];
	
	LoadState(tmp_before_state, cursor);
	dqn_trainer.Evaluate(tmp_before_state, before);
}

void AccountAdvisor::RefreshReward(int cursor) {
	System& sys = GetSystem();
	DQN::DQVector& current	= data[cursor];
	
	int best_tf[SYM_COUNT];
	double best_value[SYM_COUNT];
	for(int i = 0; i < SYM_COUNT; i++) {
		best_tf[i] = -1;
		best_value[i] = 0.0;
	}
	
	int buf = 0;
	for(int i = 0; i < tf_count; i++) {
		int pos = sys.GetShiftTf(tf_ids[0], tf_ids[i], cursor);
		int last = sys.GetCountTf(tf_ids[i]) - 2;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			double av_change = 0.0;
			
			for (int k = 0; k < tf_step[i]; k++) {
				int pos2 = pos + k;
				if (pos2 <= last) {
					double next = open_buf[buf]->GetUnsafe(pos2+1);
					double curr = open_buf[buf]->GetUnsafe(pos2);
					bool action = next < curr;
					if (!action)	av_change += next / (curr + spread_point[j]) - 1.0;
					else			av_change += 1.0 - next / (curr - spread_point[j]);
				}
			}
			
			av_change /= tf_div[i];
			av_change -= 0.0002;
			
			if (av_change > best_value[j]) {
				best_tf[j] = i;
				best_value[j] = av_change;
			}
			
			buf++;
		}
	}
	
	
	int col = 0;
	for(int j = 0; j < SYM_COUNT; j++) {
		for(int i = 0; i < tf_count; i++) {
			current.correct[col++] = best_tf[j] == i ? 0.0 : 1.0;
		}
		current.correct[col++] = best_tf[j] == -1 ? 0.0 : 1.0;
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

bool AccountAdvisor::TrainingDQNBegin() {
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

bool AccountAdvisor::TrainingDQNIterator() {
	GetCurrentJob().SetProgress(dqn_round, dqn_max_rounds);
	
	ASSERT(!data.IsEmpty());
	
	const int min_pos = 100;
	
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
			int count = Upp::min(data.GetCount() - 1, dqn_round) - min_pos - 1;
			if (count < 1) break;
			int pos = min_pos + Random(count);
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

bool AccountAdvisor::TrainingDQNEnd() {
	RefreshSourcesOnlyDeep();
	ForceSetCounted(0);
	RefreshAll();
	return true;
}

bool AccountAdvisor::TrainingDQNInspect() {
	bool succ = !dqntraining_pts.IsEmpty() && dqntraining_pts.Top() > 0.0;
	
	INSPECT(succ, "warning: negative result");
	
	return true;
}

void AccountAdvisor::RunMain() {
	System& sys = GetSystem();
	int bars = Upp::min(data.GetCount(), GetBars());
	dqntraining_pts.SetCount(bars, 0.0);
	bars -= tf_step[tf_count-1];
	
	double change_total = 1.0;
	
	bool prev_enabled[SYM_COUNT];
	bool prev_signal[SYM_COUNT];
	for(int i = 0; i < SYM_COUNT; i++) prev_signal[i] = 0;
	
	for(int i = 0; i < bars; i++) {
		const DQN::DQVector& current = data[i];
		double change_sum = 0.0;
		
		int col = 0;
		for (int j = 0; j < SYM_COUNT; j++) {
			
			double least_value = current.weight[col++];
			int least_tf = 0;
			for(int k = 1; k <= tf_count; k++) {
				double w = current.weight[col++];
				if (w < least_value) {
					least_value = w;
					least_tf = k;
				}
			}
			
			bool is_enabled, signal;
			double change;
			if (least_tf == tf_count) {
				is_enabled = false;
				change = 0.0;
			}
			else {
				int pos = sys.GetShiftTf(tf_ids[0], tf_ids[least_tf], i);
				const MainAdvisor::DQN::DQVector& adv_current = ma[least_tf]->data[pos];
				
				is_enabled = true;
				signal = adv_current.weight[j * 2 + 0] > adv_current.weight[j * 2 + 1];
				double curr		= open_buf[j]->GetUnsafe(i);
				double next		= open_buf[j]->GetUnsafe(i + 1);
				ASSERT(curr > 0.0);
				
				if (prev_signal[j] != signal || !prev_enabled[j]) {
					if (!signal)	change = next / (curr + spread_point[j]) - 1.0;
					else			change = 1.0 - next / (curr - spread_point[j]);
				}
				else {
					if (!signal)	change = next / curr - 1.0;
					else			change = 1.0 - next / curr;
				}
			}
			
			
			prev_signal[j] = signal;
			prev_enabled[j] = is_enabled;
			change_sum += change;
		}
		change_total	*= 1.0 + change_sum;
		dqntraining_pts[i] = change_total;
	}
	dqntraining_pts[bars] = change_total;
}

void AccountAdvisor::RefreshOutputBuffers() {
	int bars = GetBars();
	
	if (prev_counted < 0) prev_counted = 0; // hotfix. probably useless in future
	
	SetSafetyLimit(bars);
	
	data.SetCount(bars);
	
	cores.SetCount(0);
	cores.Reserve((SYM_COUNT+1) * CORE_COUNT * tf_count);
	for (int l = 0; l < tf_count; l++) {
		int tf = tf_ids[l];
		for(int i = 0; i < SYM_COUNT+1; i++) {
			int sym = i < SYM_COUNT ? GetSystem().GetPrioritySymbol(i) : GetSystem().GetStrongSymbol();
			for(int j = 0; j < CORE_COUNT; j++) {
				CoreIO* c = GetInputCore(j, sym, tf);
				ASSERT(c);
				cores.Add(c);
			}
		}
		
		ma[l] = dynamic_cast<MainAdvisor*>(GetInputCore(CORE_COUNT, GetSymbol(), tf));
		ASSERT(ma[l] != NULL);
	}
}

void AccountAdvisor::LoadState(DQN::MatType& state, int cursor) {
	SetSafetyLimit(cursor);
	
	int col = 0;
	
	
	// Time bits
	for(int i = 0; i < 5+24+12; i++)
		state.Set(col + i, 1.0);
	
	Time t = GetSystem().GetTimeTf(GetTf(), cursor);
	
	int wday = Upp::max(0, Upp::min(5, DayOfWeek(t) - 1));
	state.Set(col + wday, 0.0);
	col += 5;
	
	state.Set(col + t.hour, 0.0);
	col += 24;
	
	state.Set(col + t.minute / 5, 0.0);
	col += 12;
	
	
	tmp_assist.SetCount(ASSIST_COUNT);
	
	int c = 0;
	for (int l = 0; l < tf_count; l++) {
		int pos = GetSystem().GetShiftTf(tf_ids[0], tf_ids[l], cursor);
		for(int i = 0; i < SYM_COUNT+1; i++) {
			tmp_assist.Zero();
			for(int j = 0; j < CORE_COUNT; j++) {
				CoreIO& cio = *cores[c++];
				cio.Assist(pos, tmp_assist);
			}
			for(int k = 0; k < ASSIST_COUNT; k++) {
				double value = tmp_assist.Get(k) ? 0.0 : 1.0;
				state.Set(col++, value);
			}
		}
	}
	ASSERT(col == INPUT_SIZE);
}




void AccountAdvisor::MainReal() {
	System&	sys				= GetSystem();
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_hour == 6) {
		sys.WhenInfo("Closing all orders before market break");

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}

		mt.SignalOrders(true);
		return;
	}
	
	
	
	sys.WhenInfo("Updating MetaTrader");
	sys.WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		mt.RefreshLimits();
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = sys.GetPrioritySymbol(i);
			
			int sig = sb.GetSignal(sym);
			
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
			LOG("Real symbol " << sym << " signal " << sig);
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale((MULT_MAXSCALES - 1)*MULT_MAXSCALE_MUL * SYM_COUNT);
		mt.SignalOrders(true);
	}
	catch (...) {
		
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
}


void AccountAdvisor::RunSimBroker() {
	System& sys			= GetSystem();
	DataBridge* db		= dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	CoreIO* core		= CoreIO::GetInputCore(CORE_COUNT, GetSymbol(), GetTf());
	MainAdvisor* ma		= dynamic_cast<MainAdvisor*>(core);
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	int tf = GetTf();
	int bars = Upp::min(GetBars(), ma->data.GetCount());
	db->ForceCount(bars);
	
	
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale(SYM_COUNT);
	
	
	for(int i = 0; i < bars; i++) {
		const auto& current = ma->data[i];
		
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *this->open_buf[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		
		int col = 0;
		for(int j = 0; j < SYM_COUNT; j++) {
			int sym	= sys.GetPrioritySymbol(j);
			
			
			double least_value = current.weight[col++];
			int least_tf = 0;
			for(int k = 1; k <= tf_count; k++) {
				double w = current.weight[col++];
				if (w < least_value) {
					least_value = w;
					least_tf = k;
				}
			}
			
			bool is_enabled, signal;
			double change;
			if (least_tf == tf_count) {
				is_enabled = false;
				change = 0.0;
			}
			else {
				int pos = sys.GetShiftTf(tf_ids[0], tf_ids[least_tf], i);
				MainAdvisor* m = this->ma[least_tf];
				const MainAdvisor::DQN::DQVector& adv_current = m->data[pos];
				
				is_enabled = true;
				signal = adv_current.weight[j * 2 + 0] > adv_current.weight[j * 2 + 1];
			}
			
			
			int sig = !is_enabled ? 0 : (signal == false ? +1 : -1);
			
			
			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}
		
		
		sb.SignalOrders(false);
		
		
		double value = sb.AccountEquity();
		if (value < 0.0) {
			sb.ZeroEquity();
			value = 0.0;
		}
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
		
		//LOG("SB " << i << " eq " << value);
	}
}


void AccountAdvisor::TrainingDQNCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	AccountAdvisor* rfa = dynamic_cast<AccountAdvisor*>(&*job->core);
	ASSERT(rfa);
	DrawVectorPolyline(id, sz, rfa->dqntraining_pts, polyline);
	
	w.DrawImage(0, 0, id);
}























WeekSlotAdvisor::WeekSlotAdvisor() {
	
}

void WeekSlotAdvisor::Init() {
	tfmins		= GetMinutePeriod();
	weekslots	= 5 * 24 * 60 / tfmins;
	cols		= weekslots * SYM_COUNT;
}

void WeekSlotAdvisor::Start() {
	System& sys = GetSystem();
	
	if (GetSymbol() != sys.GetAccountSymbol()) return;
	
	if (once) {
		once = false;
		//if (prev_counted) prev_counted--;
		prev_counted = false;
		RefreshSourcesOnlyDeep();
	}
	
	
	if (tf_ids.GetCount() == 0) {
		ASSERT(tf_ids.IsEmpty());
		int this_tfmins = GetMinutePeriod();
		for(int i = sys.GetPeriodCount()-1; i >= GetTf(); i--) {
			int tf_mins = sys.GetPeriod(i);
			if (IsTfUsed(tf_mins)) {
				tf_ids.Add(i);
			}
		}
		ASSERT(tf_ids.GetCount() > 0);
	}
	
	
	bool sources_finished = true;
	int bars = GetBars();
	int chk_count = 0;
	for(int j = 0; j < tf_ids.GetCount(); j++) {
		int tf = tf_ids[j];
		CoreIO* core				= CoreIO::GetInputCore(2, GetSymbol(), tf);
		MainAdvisor* adv			= dynamic_cast<MainAdvisor*>(core);
		ASSERT(adv);
		sources_finished			&= adv->IsJobsFinished()
									&& adv->prev_counted == sys.GetCountTf(tf);
		chk_count++;
	}
	
	
	if (sources_finished && prev_counted < bars && chk_count > 0) {
		RefreshSourcesOnlyDeep();
		
		if (spread_point.IsEmpty()) {
			spread_point	.SetCount(SYM_COUNT, 0);
			inputs			.SetCount(SYM_COUNT, NULL);
			for(int i = 0; i < SYM_COUNT; i++) {
				int symbol					= sys.GetPrioritySymbol(i);
				int tf						= GetTf();
				ConstBuffer& open_buf		= GetInputBuffer(0, symbol, tf, 0);
				inputs[i]					= &open_buf;
			}
			
			
			mains			.SetCount(tf_ids.GetCount());
			for(int j = 0; j < tf_ids.GetCount(); j++) {
				int tf = tf_ids[j];
				CoreIO* core				= CoreIO::GetInputCore(2, GetSymbol(), tf);
				ASSERT(core);
				MainAdvisor* ma				= dynamic_cast<MainAdvisor*>(core);
				ASSERT(ma);
				mains[j]					= ma;
				
				if (!j) {
					for(int i = 0; i < SYM_COUNT; i++) {
						int symbol			= sys.GetPrioritySymbol(i);
						spread_point[i]		= mains[j]->GetSpreadPoint(i);
					}
				}
			}
			
		}
		
		LOG("WeekSlotAdvisor::Start Refresh");
		RefreshMain();
		prev_counted = bars;
	}
}

void WeekSlotAdvisor::RefreshMain() {
	RunSimBroker();
	
	MainReal();
}


void WeekSlotAdvisor::MainReal() {
	System&	sys				= GetSystem();
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 2 * 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_hour == 6) {
		sys.WhenInfo("Closing all orders before market break");

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}

		mt.SignalOrders(true);
		return;
	}
	
	
	
	sys.WhenInfo("Updating MetaTrader");
	sys.WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		mt.RefreshLimits();
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = sys.GetPrioritySymbol(i);
			
			int sig = sb.GetSignal(sym);
			
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
			LOG("Real symbol " << sym << " signal " << sig);
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale((MULT_MAXSCALES - 1)*MULT_MAXSCALE_MUL * SYM_COUNT);
		mt.SignalOrders(true);
	}
	catch (...) {
		
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
}


void WeekSlotAdvisor::RunSimBroker() {
	System& sys = GetSystem();
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));
	
	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);
	
	int tf = GetTf();
	int bars = GetBars();
	db->ForceCount(bars);
	
	
	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);
	sb.SetFreeMarginScale(SYM_COUNT);
	
	
	for(int i = 0; i < bars; i++) {
		int weekslot = i % weekslots;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *inputs[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		Time time = sys.GetTimeTf(tf, i);
		int t = time.hour * 100 + time.minute;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int sig = 0;
			
			int read_tf;
			switch (j) {
				case 0: // EURUSD
					if      (t >=  745 && t < 1630) read_tf = 0;
					else if (t >= 1630 && t < 2000) read_tf = 1;
					else read_tf = 2;
					break;
					
				case 1: // EURJPY
					if      (t >=  330 && t <  800) read_tf = 0;
					else if (t >= 1330 && t < 1800) read_tf = 0;
					else if (t >=  800 && t < 1330) read_tf = 1;
					else read_tf = 2;
					break;
					
				case 2: // USDCHF
					if      (t >=  645 && t < 1045) read_tf = 0;
					else if (t >= 1300 && t < 1630) read_tf = 0;
					else if (t >= 1045 && t < 1300) read_tf = 1;
					else read_tf = 2;
					break;
				
				case 3: // USDJPY
					if      (t >=  515 && t < 1200) read_tf = 0;
					else if (t >= 1600 && t < 1630) read_tf = 0;
					else if (t >= 1200 && t < 1600) read_tf = 1;
					else read_tf = 2;
					break;
			}
			if (read_tf != -1) {
				int read_pos = sys.GetShiftTf(tf, tf_ids[read_tf], i);
				const auto& current = mains[read_tf]->data[read_pos];
				bool signal = current.weight[j * 2 + 0] > current.weight[j * 2 + 1];
				
				sig		= signal ? -1 : +1;
			}
			
			int sym		= sys.GetPrioritySymbol(j);
			
			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}
		
		
		sb.SignalOrders(false);
		
		
		double value = sb.AccountEquity();
		if (value < 0.0) {
			sb.ZeroEquity();
			value = 0.0;
		}
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
		
		//LOG("SB " << i << " eq " << value);
	}
}

}


