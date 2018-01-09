#include "Overlook.h"


namespace Overlook {
using namespace Upp;

MainAdvisor::MainAdvisor() {
	
}

void MainAdvisor::Init() {
	System& sys = GetSystem();
	
	if (GetSymbol() != sys.GetAccountSymbol()) return;
	if (GetMinutePeriod() != 15) return;
	
	SetCoreSeparateWindow();
	
	SetBufferColor(0, RainbowColor(Randomf()));
	
	for(int i = 0; i < SYM_COUNT; i++) {
		for(int j = 0; j < week_bits; j++) {
			int hour = j % (24 * 4);
			int minutes = (j % 4) * 15;
			int t = hour * 100 + minutes;
			
			bool& b = priority_bits[i * week_bits + j];
			b = false;
			
			switch (i) {
				case 0: // EURUSD
					if      (t >=  800 && t < 1630) b = true;
					break;
					
				case 1: // EURJPY
					if      (t >=  330 && t <  800) b = true;
					else if (t >= 1300 && t < 1630) b = true;
					break;
					
				case 2: // USDCHF
					if      (t >=  645 && t < 1045) b = true;
					else if (t >= 1300 && t < 1630) b = true;
					break;
				
				case 3: // USDJPY
					if      (t >=  515 && t < 1200) b = true;
					else if (t >= 1600 && t < 1630) b = true;
					break;
			}
		}
	}
	
	
	
	int buf = 0;
	for(int j = 0; j < tf_count; j++) {
		
		switch (j) {
			case 0:	tf_ids[0] = sys.FindPeriod(15);		tf_step[0] = 60 / 15;	tf_div[0] = 1;							break;
			case 1:	tf_ids[1] = sys.FindPeriod(60);		tf_step[1] = 240 / 60;	tf_div[1] = tf_step[0];					break;
			case 2:	tf_ids[2] = sys.FindPeriod(240);	tf_step[2] = 1;			tf_div[2] = tf_step[0] * tf_step[1];	break;
			default: Panic("Tf count too high");
		}
		
		
		int tf = tf_ids[j];
		for (int i = 0; i < SYM_COUNT+1; i++) {
			int sym = i < SYM_COUNT ? sys.GetPrioritySymbol(i) : sys.GetStrongSymbol();
			open_buf[buf] = &GetInputBuffer(0, sym, tf, 0);
			ASSERT(open_buf[buf] != NULL);
			buf++;
		}
	}
	ASSERT(buf == (SYM_COUNT+1) * tf_count);
	
	int tf = GetTf();
	for (int i = 0; i < SYM_COUNT; i++) {
		DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, sys.GetPrioritySymbol(i), tf));
		spread_point[i] = db->GetPoint();
		ASSERT(spread_point[i] > 0.0);
	}
	spread_point[SYM_COUNT] = 0.0002;
	
	
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
	if (GetMinutePeriod() != 15) return;
	
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		RefreshSourcesOnlyDeep();
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
	
	RunSimBroker();
	MainReal();
	
	prev_counted = GetBars();
}

void MainAdvisor::RefreshAction(int cursor) {
	DQN::DQVector& before		= data[cursor];
	
	LoadState(tmp_before_state, cursor);
	dqn_trainer.Evaluate(tmp_before_state, before);
}

void MainAdvisor::RefreshReward(int cursor) {
	System& sys = GetSystem();
	DQN::DQVector& current	= data[cursor];
	
	
	int col = 0;
	int buf = 0;
	for(int j = 0; j < tf_count; j++) {
		int pos = sys.GetShiftTf(tf_ids[0], tf_ids[j], cursor);
		int last = sys.GetCountTf(tf_ids[j]) - 2;
		for(int i = 0; i < SYM_COUNT+1; i++) {
			if (pos > last) {
				current.correct[col++] = 0.5;
				current.correct[col++] = 0.5;
				current.correct[col++] = 0.5;
				current.correct[col++] = 0.5;
			} else {
				double next  = open_buf[buf]->GetUnsafe(pos+1);
				double curr  = open_buf[buf]->GetUnsafe(pos);
				double point = spread_point[i];
				bool action  = next < curr;
				current.correct[col++] = !action ? 0.0 : 1.0;
				current.correct[col++] =  action ? 0.0 : 1.0;
				bool exceeds_spreads, exceptional_value;
				if (!action) {
					exceeds_spreads   = next >= curr + point;
					exceptional_value = next >= curr + point * 3;
				}
				else {
					exceeds_spreads   = next <= curr - point;
					exceptional_value = next <= curr - point * 3;
				}
				current.correct[col++] =  exceeds_spreads   ? 0.0 : 1.0;
				current.correct[col++] =  exceptional_value ? 0.0 : 1.0;
			}
			
			buf++;
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
	
	bool prev_signal[SYM_COUNT], prev_enabled[SYM_COUNT];
	for(int i = 0; i < SYM_COUNT; i++) {
		prev_signal[i] = 0;
		prev_enabled[i] = 0;
	}
	
	for(int i = 0; i < bars; i++) {
		DQN::DQVector& current = data[i];
		double change_sum = 0.0;
		int open_count = 0;
		for (int j = 0; j < SYM_COUNT; j++) {
			double long_proximity  = current.weight[j * SYM_BITS + 0];
			double short_proximity = current.weight[j * SYM_BITS + 1];
			bool signal = long_proximity > short_proximity;
			bool too_weak_signal = !signal ? long_proximity >= 0.5 : short_proximity >= 0.5;
			bool exceeds_spreads   = current.weight[j * SYM_BITS + 2] < 0.5;
			bool exceptional_value = current.weight[j * SYM_BITS + 3] < 0.5;
			bool try_continue = prev_enabled[j] && signal == prev_signal[j];
			bool is_priority = priority_bits[j * week_bits + (i % week_bits)];
			bool is_enabled = !too_weak_signal &&
				((is_priority && (exceeds_spreads || try_continue)) ||
				(!is_priority && (exceptional_value || try_continue)));
			
			if (is_enabled) {
				open_count++;
				double curr		= open_buf[j]->GetUnsafe(i);
				double next		= open_buf[j]->GetUnsafe(i + 1);
				ASSERT(curr > 0.0);
				double change;
				
				if (prev_signal[j] != signal || !prev_enabled[j]) {
					if (!signal)	change = next / (curr + spread_point[j]) - 1.0;
					else			change = 1.0 - next / (curr - spread_point[j]);
				} else {
					if (!signal)	change = next / curr - 1.0;
					else			change = 1.0 - next / curr;
				}
				
				change_sum += change;
			}
			else if (is_priority)
				open_count++;
			
			prev_signal[j] = signal;
			prev_enabled[j] = is_enabled;
		}
		if (open_count) {
			change_sum /= open_count;
			change_total	*= 1.0 + change_sum;
		}
		dqntraining_pts[i] = change_total;
	}
	dqntraining_pts[bars] = change_total;
}

void MainAdvisor::RefreshOutputBuffers() {
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
	}
}

void MainAdvisor::LoadState(DQN::MatType& state, int cursor) {
	SetSafetyLimit(cursor);
	
	int col = 0;
	
	
	// Time bits
	for(int i = 0; i < 5+24+4; i++)
		state.Set(col + i, 1.0);
	
	Time t = GetSystem().GetTimeTf(GetTf(), cursor);
	
	int wday = Upp::max(0, Upp::min(5, DayOfWeek(t) - 1));
	state.Set(col + wday, 0.0);
	col += 5;
	
	state.Set(col + t.hour, 0.0);
	col += 24;
	
	state.Set(col + t.minute / 15, 0.0);
	col += 4;
	
	
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

void MainAdvisor::MainReal() {
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
		int open_count = 0;
		for (int i = 0; i < SYM_COUNT; i++) {
			int sym = sys.GetPrioritySymbol(i);
			
			int sig = sb.GetSignal(sym);
			
			if (sig != 0) open_count++;
			
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
			LOG("Real symbol " << sym << " signal " << sig);
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale(open_count ? open_count : 1);
		mt.SignalOrders(true);
	}
	catch (...) {
		
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
}


void MainAdvisor::RunSimBroker() {
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
	
	
	for(int i = 0; i < bars; i++) {
		DQN::DQVector& current = data[i];
		
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& open_buf = *this->open_buf[j];
			double curr = open_buf.GetUnsafe(i);
			int symbol = sys.GetPrioritySymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();
		
		Time time = sys.GetTimeTf(tf, i);
		int t = time.hour * 100 + time.minute;
		int open_count = 0;
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int sym			= sys.GetPrioritySymbol(j);
			int prev_sig	= sb.GetSignal(sym);
			bool prev_signal  = prev_sig == +1 ? false : true;
			bool prev_enabled = prev_sig !=  0;
			
			double long_proximity  = current.weight[j * SYM_BITS + 0];
			double short_proximity = current.weight[j * SYM_BITS + 1];
			bool signal = long_proximity > short_proximity;
			bool too_weak_signal = !signal ? long_proximity >= 0.5 : short_proximity >= 0.5;
			bool exceeds_spreads   = current.weight[j * SYM_BITS + 2] < 0.5;
			bool exceptional_value = current.weight[j * SYM_BITS + 3] < 0.5;
			bool try_continue = prev_enabled && signal == prev_signal;
			bool is_priority = priority_bits[j * week_bits + (i % week_bits)];
			bool is_enabled = !too_weak_signal &&
				((is_priority && (exceeds_spreads || try_continue)) ||
				(!is_priority && (exceptional_value || try_continue)));
			
			int sig = 0;
			
			if (is_enabled) {
				sig = signal ? -1 : +1;
				open_count++;
			}
			else if (is_priority)
				open_count++;
			
			
			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}
		
		sb.SetFreeMarginScale(open_count ? open_count : 1);
		
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
