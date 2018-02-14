#include "Overlook.h"

namespace Overlook {




RuleAnalyzer::RuleAnalyzer() {
	data_ctrl.ra = this;
	agentctrl.ra = this;
	data_slider << THISBACK(SetCursor);
	data_slider.MinMax(0, 1);
	
	CtrlLayout(*this, "Rule Analyzer");
	prog.Set(0, 1);
	analyze << THISBACK(PostStartProcess);
	LoadThis();
	
	
	data_tab.Add(data_ctrl.HSizePos(0,30).VSizePos());
	data_tab.Add(data_slider.BottomPos(0,30).HSizePos());
	agent_tab.Add(agentctrl.SizePos());
	
	enable.Set(is_enabled);
	enable.WhenAction << THISBACK(SetEnable);
	
	Data();
}

RuleAnalyzer::~RuleAnalyzer() {
	StopProcess();
	StoreThis();
}

String RuleAnalyzer::GetSignalString(int i) {
	switch (i) {
		case OPEN_LONG: return "Open long";
		case OPEN_SHORT: return "Open short";
		case CLOSE_LONG: return "Close long";
		case CLOSE_SHORT: return "Close short";
		case SUSTAIN: return "Sustain";
		case BLOCK: return "Block";
		case ATTENTION1: return "1 min attention";
		case ATTENTION5: return "5 mins attention";
		case ATTENTION15: return "15 mins attention";
		default: return "Invalid";
	}
}

void RuleAnalyzer::StartProcess() {
	StopProcess();
	
	analyze.SetLabel("Stop");
	analyze.WhenAction = THISBACK(PostStopProcess);
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Process));
}

void RuleAnalyzer::StopProcess() {
	analyze.SetLabel("Analyze");
	analyze.WhenAction = THISBACK(PostStartProcess);
	
	running = false;
	while (!stopped) Sleep(100);
}

void RuleAnalyzer::Prepare() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetSymbolCount(); i++) sym_ids.Add(i);
	tf_ids.Add(0);
	
	FactoryDeclaration tmp;
	tmp.arg_count = 0;
	tmp.factory = System::Find<DataBridge>();							indi_ids.Add(tmp);
    /*tmp.factory = System::Find<MovingAverage>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<MovingAverageConvergenceDivergence>();	indi_ids.Add(tmp);
    tmp.factory = System::Find<BollingerBands>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<ParabolicSAR>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<StandardDeviation>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AverageTrueRange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<BearsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<BullsPower>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<CommodityChannelIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<DeMarker>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<Momentum>();								indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeStrengthIndex>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<RelativeVigorIndex>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<StochasticOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<AcceleratorOscillator>();				indi_ids.Add(tmp);
    tmp.factory = System::Find<AwesomeOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<PeriodicalChange>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilityAverage>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<VolatilitySlots>();						indi_ids.Add(tmp);
    tmp.factory = System::Find<VolumeSlots>();							indi_ids.Add(tmp);
    tmp.factory = System::Find<ChannelOscillator>();					indi_ids.Add(tmp);
    tmp.factory = System::Find<ScissorChannelOscillator>();				indi_ids.Add(tmp);*/
    
    sys.System::GetCoreQueue(ci_queue, sym_ids, tf_ids, indi_ids);
    
    this->data_in.SetCount(sys.GetSymbolCount());
    this->data_out.SetCount(sys.GetSymbolCount());
	this->av_wins.SetCount(sys.GetSymbolCount());
	this->median_maps.SetCount(sys.GetSymbolCount());
	this->volat_divs.SetCount(sys.GetSymbolCount());
    
	opt.Init(signal_count, 1, opt_row_size, StrategyBest1Exp);
}

void RuleAnalyzer::ProcessData() {
	
	// Get reference values
	System& sys = GetSystem();
	VectorSnap& data_in = this->data_in[data_cursor];
	DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[data_cursor]->core);
	ASSERT(db.GetSymbol() == data_cursor);
	double spread_point		= db.GetPoint();
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int begin = data_in.GetCount();
	data_in.SetCount(data_count);
	
	// Prepare Moving average
	Vector<OnlineAverageWindow1>& av_wins = this->av_wins[data_cursor];
	av_wins.SetCount(period_count);
	for(int i = 0; i < av_wins.GetCount(); i++)
		av_wins[i].SetPeriod(1 << (1+i));
	
	
	// Prepare VolatilityContext
	Vector<Vector<double> >& volat_divs = this->volat_divs[data_cursor];
	Vector<VectorMap<int,int> >& median_maps = this->median_maps[data_cursor];
	volat_divs.SetCount(period_count);
	median_maps.SetCount(period_count);
	for(int j = 0; j < period_count; j++) {
		int period = 1 << (1+j);
		VectorMap<int,int>& median_map = median_maps[j];
		for(int cursor = max(period, begin); cursor < data_count; cursor++) {
			double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
			int step = (int)((diff + spread_point * 0.5) / spread_point);
			median_map.GetAdd(step, 0)++;
		}
		SortByKey(median_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_map.GetCount(); i++)
			total += median_map[i];
		int64 count_div = total / volat_div;
		total = 0;
		int64 next_div = count_div;
		volat_divs[j].SetCount(0);
		volat_divs[j].Add(median_map.GetKey(0) * spread_point);
		for(int i = 0; i < median_map.GetCount(); i++) {
			total += median_map[i];
			if (total >= next_div) {
				next_div += count_div;
				volat_divs[j].Add(median_map.GetKey(i) * spread_point);
			}
		}
		if (volat_divs[j].GetCount() < volat_div)
			volat_divs[j].Add(median_map.TopKey() * spread_point);
	}
	
	
	// Run main data filler
	for(int cursor = begin; cursor < data_count; cursor++) {
		#ifdef flagDEBUG
		if (cursor >= 100000)
			break;
		#endif
		Snap& snap = data_in[cursor];
		int bit_pos = 0;
		double open1 = open_buf.Get(cursor);
		
		
		for(int k = 0; k < period_count; k++) {
			
			// OnlineMinimalLabel
			double cost	 = spread_point * (1 + k);
			const int count = 1;
			bool sigbuf[count];
			int begin = Upp::max(0, cursor - 200);
			int end = cursor + 1;
			OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
			bool label = sigbuf[count - 1];
			snap.Set(bit_pos++, label);
		
			
			// TrendIndex
			bool bit_value;
			int period = 1 << (1 + k);
			double err, av_change, buf_value;
			TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
			snap.Set(bit_pos++, buf_value > 0.0);
			
			
			#ifndef flagDEBUG
			
			// VolatilityContext
			int lvl = -1;
			if (cursor >= period) {
				double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
				for(int i = 0; i < volat_divs[k].GetCount(); i++) {
					if (diff < volat_divs[k][i]) {
						lvl = i - 1;
						break;
					}
				}
			}
			for(int i = 0; i < volat_div; i++)
				snap.Set(bit_pos++,  lvl == i);
			
		
			// MovingAverage
			OnlineAverageWindow1& av_win = av_wins[k];
			double prev = av_win.GetMean();
			av_win.Add(open1);
			double curr = av_win.GetMean();
			label = open1 < prev;
			snap.Set(bit_pos++, label);
			
			
			// Momentum
			begin = Upp::max(0, cursor - period);
			double open2 = open_buf.Get(begin);
			double value = open1 / open2 - 1.0;
			label = value < 0.0;
			snap.Set(bit_pos++, label);
			
			
			// Open/Close trend
			period = 1 << k;
			int dir = 0;
			int len = 0;
			if (cursor >= period * 3) {
				for (int i = cursor-period; i >= 0; i -= period) {
					int idir = open_buf.Get(i+period) > open_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != dir) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
		
		
			// High break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double hi = high_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = hi > high_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
			
			
			// Low break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double lo = low_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = lo < low_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
			
			
			// Trend reversal
			int t0 = +1;
			int t1 = +1;
			int t2 = -1;
			if (cursor >= 4*period) {
				double t0_diff		= open_buf.Get(cursor-0*period) - open_buf.Get(cursor-1*period);
				double t1_diff		= open_buf.Get(cursor-1*period) - open_buf.Get(cursor-2*period);
				double t2_diff		= open_buf.Get(cursor-2*period) - open_buf.Get(cursor-3*period);
				t0 = t0_diff > 0 ? +1 : -1;
				t1 = t1_diff > 0 ? +1 : -1;
				t2 = t2_diff > 0 ? +1 : -1;
			}
			if (t0 * t1 == -1 && t1 * t2 == +1) {
				snap.Set(bit_pos++, t0 == +1);
				snap.Set(bit_pos++, t0 != +1);
			} else {
				snap.Set(bit_pos++, false);
				snap.Set(bit_pos++, false);
			}
			
			#endif
		}
		
		ASSERT(bit_pos == row_size);
	}
}

void RuleAnalyzer::ProcessSignalPrediction() {
	RunAgent(false);
}

void RuleAnalyzer::ProcessSignalInitial() {
	const double idx_limit_f = 0.75;
	
	// Get reference values
	System& sys = GetSystem();
	VectorSnap& data_out = this->data_out[data_cursor];
	DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[data_cursor]->core);
	ASSERT(db.GetSymbol() == data_cursor);
	double spread_point		= db.GetPoint();
	double cost				= spread_point * 3;
	int tf = 0;
	ASSERT(spread_point > 0.0);
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int begin = data_in.GetCount();
	data_out.SetCount(data_count);
	VectorMap<int, Order> orders;
	data_count--;
	
	if (begin != 0) return;
	
	for(int i = 0; i < data_count; i++) {
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < data_count; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		for(int k = i; k < j; k++) {
			data_out[k].Set(RT_SIGNAL, label);
		}
		
		i = j - 1;
	}
	
	bool prev_label = data_out[0].Get(RT_SIGNAL);
	int prev_switch = 0;
	for(int i = 1; i < data_count; i++) {
		bool label = data_out[i].Get(RT_SIGNAL);
		
		int len = i - prev_switch;
		
		if (label != prev_label) {
			Order& o = orders.Add(i);
			o.label = prev_label;
			o.start = prev_switch;
			o.stop = i;
			o.len = o.stop - o.start;
			
			double open = open_buf.GetUnsafe(o.start);
			double close = open_buf.GetUnsafe(o.stop);
			o.av_change = fabs(close - open) / len;
			
			double err = 0;
			for(int k = o.start; k < o.stop; k++) {
				double diff = open_buf.GetUnsafe(k+1) - open_buf.GetUnsafe(k);
				err += fabs(diff);
			}
			o.err = err / len;
			
			prev_switch = i;
			prev_label = label;
		}
	}
	
	
	
	struct ChangeSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.av_change < b.av_change;
		}
	};
	Sort(orders, ChangeSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.av_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	struct LengthSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.len < b.len;
		}
	};
	Sort(orders, LengthSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.len_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	
	struct ErrorSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.err > b.err;
		}
	};
	Sort(orders, ErrorSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.err_idx = (double)i / (double)(orders.GetCount() - 1);
		o.idx = o.av_idx + o.err_idx + 2 * o.len_idx;
	}
	
	
	struct IndexSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.idx < b.idx;
		}
	};
	Sort(orders, IndexSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.idx_norm = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.idx_norm < idx_limit_f) continue;
		
		ASSERT(o.start <= o.stop);
		
		for(int k = o.start; k < o.stop; k++) {
			data_out[k].Set(RT_ENABLED, true);
		}
	}
}

void RuleAnalyzer::Data() {
	if (this->data_in.IsEmpty()) return;
	
	int begin_id = 0;
	
	auto& data_in = this->data_in[0];
	int data_count = data_in.GetCount() / row_size;
	if (data_slider.GetMax() != data_count-1) {
		data_slider.MinMax(0, data_count-1);
		data_slider.SetData(data_count-1);
	}
	
	int seconds = total / prev_speed / GetUsedCpuCores();
	if (perc < 100) {
		int hours = seconds / 3600;		seconds = seconds % 3600;
		int minutes = seconds / 60;		seconds = seconds % 60;
		Title(Format("Rule Analyzer :: %d%% :: Time remaining %d hours %d minutes %d seconds", perc, hours, minutes, seconds));
		prog.Set(actual, total);
	}
	else {
		Title("Rule Analyzer :: Ready");
		prog.Set(0, 1);
	}
	
	agentctrl.Refresh();
}

void RuleAnalyzer::Process() {
	System& sys = GetSystem();
	
	while (running && !is_prepared) {
		Prepare();
		is_prepared = true;
	}
	
	while (running && (ci_queue.IsEmpty() || processed_cursor < ci_queue.GetCount())) {
		sys.System::Process(*ci_queue[processed_cursor++], true);
	}
	
	while (running && data_cursor < sys.GetSymbolCount()) {
		ProcessData();
		ProcessSignalInitial();
		data_cursor++;
	}
	while (running) {
		ProcessIteration();
	}
	
	stopped = true;
}

void RuleAnalyzer::ProcessIteration() {
	System& sys = GetSystem();
	TimeStop ts;
	int phase_total = opt.GetMaxRounds();
	
	int current_in_loop = 0;
	while (ts.Elapsed() < 100 && phase < FINISHED) {
		if (current < phase_total && phase >= 0 && phase < FINISHED) {
			switch (phase) {
				case TRAIN:				IterateTrain(); break;
				case TEST:				IterateTest(); break;
			}
		}
		
		current++;
		
		
		if (current >= phase_total) {
			current = 0;
			phase++;
			
			if (phase >= FINISHED) {
				phase = 0;
				PostCallback(THISBACK(StopProcess));
			}
		}
	}
	
	actual = opt.GetRound();
	total = phase_total;
	prev_speed = ((double)current_in_loop / (double)ts.Elapsed()) * 1000.0;
	if (prev_speed < 1.0) prev_speed = 1.0;
	perc = actual * 100 / total;
	
	PostData();
	
	bool ready = phase == FINISHED;
	
}

void RuleAnalyzer::IterateTrain() {
	if (opt.IsEnd()) return;
	
	System& sys = GetSystem();
	
	
	opt.Start();
	
	double result = RunAgent(true);
	
	//if (opt.GetRound() % 100 == 0)
	{
		training_pts.Add(result);
	}
	
	opt.Stop(result);
}

void RuleAnalyzer::IterateTest() {
	
}

double RuleAnalyzer::RunAgent(bool test) {
	ASSERT(row_size <= 128);
	
	const Vector<bool>& trial = opt.GetTrialSolution();
	
	
	// signals open, close, sustain, attention, block
	bool cur_signals[signal_count];
	int bit = 0;
	Snap sig_poles[signal_count*2];
	double sig_dist_mult[signal_count];
	for(int i = 0; i < signal_count; i++) {
		
		// two per signal... on and off poles
		for(int j = 0; j < 2; j++) {
			Snap& s = sig_poles[i * 2 + j];
			double& d = sig_dist_mult[i * 2 + j];
			for(int k = 0; k < row_size; k++) {
				s.Set(k, trial[bit++]);
			}
			d = 1.0;
			for(int k = 0; k < trsh_bits; k++)
				if (trial[bit++]) d += 1.0;
			d /= trsh_bits + 2.0;
		}
	}
	ASSERT(bit == trial.GetCount());
	
	
	int sym_count = data_in.GetCount();
	double av_accuracy = 0.0;
	
	for(int sym_id = 0; sym_id < sym_count; sym_id++) {
		const VectorSnap& data_in = this->data_in[sym_id];
		VectorSnap& data_out = this->data_out[sym_id];
		ASSERT(data_in.GetCount() == data_out.GetCount());
		int data_count = data_in.GetCount();
		
		int correct_testbits = 0;
		int total_signaltestbits = 0;
		double total_change = 1.0;
		bool is_enabled = false, prev_signal;
		int step = 1;
		for(int i = 0; i < data_count; i += step) {
			
			const Snap& sym_in = data_in[i];
			
			Snap* pole = sig_poles;
			double* mult = sig_dist_mult;
			bool signal[signal_count];
			for(int j = 0; j < signal_count; j++) {
				
				// two per signal... on and off poles
				double dist[2];
				for(int k = 0; k < 2; k++) {
					dist[k] = sym_in.GetDistance(*pole) * *mult;
					pole++;
					mult++;
				}
				
				signal[j] = dist[1] > dist[0];
			}
			
			bool& is_openl			= signal[OPEN_LONG];
			bool& is_opens			= signal[OPEN_SHORT];
			bool& is_closel			= signal[CLOSE_LONG];
			bool& is_closes			= signal[CLOSE_SHORT];
			bool& is_sustain		= signal[SUSTAIN];
			bool& is_block			= signal[BLOCK];
			bool& is_attention1		= signal[ATTENTION1];
			bool& is_attention5		= signal[ATTENTION5];
			bool& is_attention15	= signal[ATTENTION15];
			
			
			
			
			if (is_enabled) {
				if (is_block) {
					is_openl = false;
					is_opens = false;
					is_closel = true;
					is_closes = true;
				}
				else if (is_sustain) {
					is_openl = false;
					is_opens = false;
					is_closel = false;
					is_closes = false;
				}

				if ((is_closel && prev_signal == 0) || (is_closes && prev_signal == 1))
					is_enabled = false;
			}
			
			if (is_openl || is_opens) {
				bool was_enable = is_enabled;
				is_enabled = true;
				
				if (was_enable) {
					if (is_openl && prev_signal == 0)
						; // keep going
					else if (is_opens && prev_signal == 1)
						; // keep going
					else if (is_openl && is_opens)
						is_enabled = false; // conflict, don't open
					else if (is_openl)
						prev_signal = 0;
					else if (is_opens)
						prev_signal = 1;
				}
			}
			
			if (is_attention1)
				step = 1;
			else if (is_attention5)
				step = 5;
			else if (is_attention15)
				step = 15;
			
			
			
			
			if (test) {
				const auto& out = data_out[i];
				bool test_enabled = out.Get(RT_ENABLED);
				bool test_signal = out.Get(RT_SIGNAL);
				
				if (test_enabled == is_enabled) {
					correct_testbits++;
					if (test_enabled) {
						total_signaltestbits++;
						if (test_signal == prev_signal)
							correct_testbits++;
					}
				}
			} else {
				
				for(int j = i; j < i+step; j++) {
					auto& out = data_out[j];
					
					out.Set(RT_ENABLED, is_enabled);
					out.Set(RT_SIGNAL, prev_signal);
				}
			}
		}
		
		av_accuracy += (double)correct_testbits / (double)(data_count + total_signaltestbits);
	}
	
	av_accuracy /= sym_count;
	
	return av_accuracy;
}

void RuleAnalyzer::Refresh() {
	System& sys				= GetSystem();
	if (phase < FINISHED)
		return;
	for(data_cursor = 0; data_cursor < sys.GetSymbolCount(); data_cursor++) {
		sys.System::Process(*ci_queue[data_cursor], true);
		ProcessData();
		ProcessSignalPrediction();
	}
	if (is_enabled)
		RefreshReal();
}


bool RuleAnalyzer::RefreshReal() {
	System& sys				= GetSystem();
	Time now				= GetUtcTime();
	int wday				= DayOfWeek(now);
	Time after_3hours		= now + 3 * 60 * 60;
	int wday_after_3hours	= DayOfWeek(after_3hours);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return true;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_3hours == 6) {
		sys.WhenInfo("Closing all orders before market break");
		
		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}
		
		mt.SignalOrders(true);
		return true;
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
		const int MAX_SYMOPEN = 4;
		const double FMLEVEL = 0.6;
		
		for (int sym_id = 0; sym_id < sys.GetSymbolCount(); sym_id++) {
			VectorSnap& data_out = this->data_out[sym_id];
			bool enabled = data_out.Top().Get(RT_ENABLED);
			bool signal  = data_out.Top().Get(RT_SIGNAL);
			int sig = enabled ? (signal ? -1 : +1) : 0;
			int prev_sig = mt.GetSignal(sym_id);
			
			if (sig == prev_sig && sig != 0)
				mt.SetSignalFreeze(sym_id, true);
			else {
				if ((!prev_sig && sig) || (prev_sig && sig != prev_sig)) {
					if (open_count >= MAX_SYMOPEN)
						sig = 0;
					else
						open_count++;
				}
				
				mt.SetSignal(sym_id, sig);
				mt.SetSignalFreeze(sym_id, false);
			}
			LOG("Real symbol " << sym_id << " signal " << sig);
		}
		
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale(MAX_SYMOPEN);
		mt.SignalOrders(true);
	}
	catch (UserExc e) {
		LOG(e);
		return false;
	}
	catch (...) {
		return false;
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
	
	return true;
}






















void RuleAnalyzer::RADataCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int sym_count = ra->data_in.GetCount();
	
	for(int sym_id = 0; sym_id < sym_count; sym_id++) {
		const VectorSnap& data_in = ra->data_in[sym_id];
		int sym_pos = min(data_in.GetCount() - 1, cursor);
		double xstep = (double)sz.cx / (ra->period_count * ra->data_in.GetCount());
		double ystep = (double)sz.cy / ra->INPUT_COUNT;
		int w = xstep + 0.5;
		int h = ystep + 0.5;
		if (sym_pos < 0 || sym_pos + 1 > data_in.GetCount()) continue;
		int bit_pos = 0;
		for(int i = 0; i < ra->period_count; i++) {
			int x = xstep * (i + sym_id * ra->period_count);
			for(int j = 0; j < ra->INPUT_COUNT; j++) {
				int y = ystep * j;
				
				bool bit = data_in[sym_pos].Get(bit_pos++);
				if (bit) id.DrawRect(x, y, w, h, Black());
			}
		}
	}
	
	
	d.DrawImage(0,0,id);
}

void RuleAnalyzer::OrganizationAgentCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int data_count = ra->training_pts.GetCount();
	if (data_count >= 2) {
		
		pts.SetCount(0);
		
		double minv = DBL_MAX, maxv = -DBL_MAX;
		for(int i = 0; i < data_count; i++) {
			double d = ra->training_pts[i];
			if (d < minv) minv = d;
			if (d > maxv) maxv = d;
		}
		double diff = maxv - minv;
		
		double xstep = (double)sz.cx / (data_count - 1);
		for(int i = 0; i < data_count; i++) {
			double d = ra->training_pts[i];
			Point& pt = pts.Add();
			pt.x = i * xstep;
			pt.y = sz.cy * (1.0 - ((d - minv) / diff));
		}
		id.DrawPolyline(pts, 1, Color(56, 85, 150));
	}
	
	d.DrawImage(0,0,id);
}

}



