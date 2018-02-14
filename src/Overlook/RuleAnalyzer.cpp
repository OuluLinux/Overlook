#include "Overlook.h"

namespace Overlook {




RuleAnalyzer::RuleAnalyzer() {
	not_stopped = 0;
	waiting = 0;
	
	data_ctrl.ra = this;
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
		case OPEN: return "Open";
		case CLOSE: return "Close";
		case SUSTAIN: return "Sustain";
		case BLOCK: return "Block";
		case ATTENTION: return "Attention level";
		default: return "Invalid";
	}
}

void RuleAnalyzer::StartProcess() {
	StopProcess();
	
	analyze.SetLabel("Stop");
	analyze.WhenAction = THISBACK(PostStopProcess);
	
	running = true;
	ASSERT(not_stopped == 0);
	thrds.Clear();
	int cpus = GetUsedCpuCores();
	for(int i = 0; i < cpus; i++) {
		not_stopped++;
		thrds.Add().Start(THISBACK1(Process, i));
	}
}

void RuleAnalyzer::StopProcess() {
	analyze.SetLabel("Analyze");
	analyze.WhenAction = THISBACK(PostStartProcess);
	
	running = false;
	while (not_stopped > 0) Sleep(100);
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
    
    this->data.SetCount(sys.GetSymbolCount());
    this->rtdata.SetCount(sys.GetSymbolCount());
	this->av_wins.SetCount(sys.GetSymbolCount());
	this->median_maps.SetCount(sys.GetSymbolCount());
	this->volat_divs.SetCount(sys.GetSymbolCount());
    
    agent.Init();
}

void RuleAnalyzer::ProcessData() {
	
	// Get reference values
	System& sys = GetSystem();
	VectorBool& data = this->data[data_cursor];
	DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[data_cursor]->core);
	ASSERT(db.GetSymbol() == data_cursor);
	double spread_point		= db.GetPoint();
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int bit_count = row_size * data_count;
	int begin = data.GetCount() / row_size;
	data.SetCount(bit_count);
	
	
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
		int bit_pos = cursor * row_size;
		double open1 = open_buf.Get(cursor);
		
		int bit_begin = bit_pos;
		
		for(int k = 0; k < period_count; k++) {
			
			// OnlineMinimalLabel
			double cost	 = spread_point * (1 + k);
			const int count = 1;
			bool sigbuf[count];
			int begin = Upp::max(0, cursor - 200);
			int end = cursor + 1;
			OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
			bool label = sigbuf[count - 1];
			data.Set(bit_pos++, label);
		
			
			// TrendIndex
			bool bit_value;
			int period = 1 << (1 + k);
			double err, av_change, buf_value;
			TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
			data.Set(bit_pos++, buf_value > 0.0);
			
			
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
				data.Set(bit_pos++,  lvl == i);
			
		
			// MovingAverage
			OnlineAverageWindow1& av_win = av_wins[k];
			double prev = av_win.GetMean();
			av_win.Add(open1);
			double curr = av_win.GetMean();
			label = open1 < prev;
			data.Set(bit_pos++, label);
			
			
			// Momentum
			begin = Upp::max(0, cursor - period);
			double open2 = open_buf.Get(begin);
			double value = open1 / open2 - 1.0;
			label = value < 0.0;
			data.Set(bit_pos++, label);
			
			
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
			data.Set(bit_pos++, len > 2);
		
		
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
			data.Set(bit_pos++, len > 2);
			
			
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
			data.Set(bit_pos++, len > 2);
			
			
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
				data.Set(bit_pos++, t0 == +1);
				data.Set(bit_pos++, t0 != +1);
			} else {
				data.Set(bit_pos++, false);
				data.Set(bit_pos++, false);
			}
			
			#endif
		}
		
		ASSERT(bit_pos - bit_begin == row_size);
	}
}

void RuleAnalyzer::ProcessSignalPrediction() {
	
}

void RuleAnalyzer::ProcessSignalInitial() {
	const double idx_limit_f = 0.75;
	
	// Get reference values
	System& sys = GetSystem();
	VectorBool& data = this->rtdata[data_cursor];
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
	int bit_count = rt_row_size * data_count;
	int begin = data.GetCount() / rt_row_size;
	data.SetCount(bit_count);
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
			data.Set(k * rt_row_size + RT_SIGNAL, label);
		}
		
		i = j - 1;
	}
	
	bool prev_label = data.Get(0 * rt_row_size + RT_SIGNAL);
	int prev_switch = 0;
	for(int i = 1; i < data_count; i++) {
		bool label = data.Get(i * rt_row_size + RT_SIGNAL);
		
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
			data.Set(k * rt_row_size + RT_ENABLED, true);
		}
	}
}

void RuleAnalyzer::Data() {
	if (this->data.IsEmpty()) return;
	
	int begin_id = 0;
	
	
	
	auto& data = this->data[0];
	int data_count = data.GetCount() / row_size;
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
}

void RuleAnalyzer::Process(int thrd_id) {
	System& sys = GetSystem();
	
	while (running && !is_prepared) {
		if (thrd_id == 0) {
			Prepare();
			is_prepared = true;
		}
		else
			Sleep(100);
	}
	
	while (running && (ci_queue.IsEmpty() || processed_cursor < ci_queue.GetCount())) {
		if (thrd_id == 0)
			sys.System::Process(*ci_queue[processed_cursor++], true);
		else
			Sleep(100);
	}
	
	while (running && data_cursor < sys.GetSymbolCount()) {
		if (thrd_id == 0) {
			ProcessData();
			ProcessSignalInitial();
			data_cursor++;
		}
		else Sleep(100);
	}
	while (running) {
		ProcessIteration(thrd_id);
	}
	not_stopped--;
}

void RuleAnalyzer::ProcessIteration(int thrd_id) {
	System& sys = GetSystem();
	TimeStop ts;
	int phase_total = period_count * signal_count;
	
	int current_in_loop = 0;
	while (ts.Elapsed() < 100 && phase < FINISHED) {
		int current = thrd_current++;
		
		if (current < phase_total && phase >= 0 && phase < FINISHED) {
			switch (phase) {
				case TRAIN:				IterateTrain(current); break;
				case TEST:				IterateTest(current); break;
			}
		}
		
		current_in_loop++;
		
		
		if (thrd_current >= phase_total) {
			if (thrd_id == 0) {
				while (waiting != thrds.GetCount() - 1) Sleep(100); // wait before phase,joinlevel can be changed
				thrd_current = 0;
				phase++;
				
				if (phase >= FINISHED) {
					phase = 0;
					PostCallback(THISBACK(StopProcess));
				}
			}
			else {
				waiting++;
				while (thrd_current >= phase_total) Sleep(100);
				waiting--;
			}
		}
	}
	
	if (thrd_id == 0) {
		actual = phase_total * phase + thrd_current;
		total = phase_total * FINISHED;
		prev_speed = ((double)current_in_loop / (double)ts.Elapsed()) * 1000.0;
		if (prev_speed < 1.0) prev_speed = 1.0;
		perc = actual * 100 / total;
	}
	PostData();
	
	bool ready = phase == FINISHED;
	
}

void RuleAnalyzer::IterateTrain(int current) {
	System& sys = GetSystem();
	
	BitOptimizer opt;
	
	
	int opt_row_size = COUNT * 2 + trsh_bits;
	opt.SetDimensions(signal_count, period_count, opt_row_size);
	
	
	
	
	
	while (!optimizer.IsEnd()) {
		
		optimizer.Start();
		
		for(int i = 0; i < signal_count; i++) {
			for(int j = 0; j < period_count; j++) {
				VectorSnap& row = agent.Get(i, j);
				dword* opt_row = opt.Get(i, j);
				row.Copy(opt_row);
			}
		}
		
		double result = agent.Test();
		
		optimizer.Stop();
		
	}
}

void RuleAnalyzer::IterateTest(int current) {
	
}

void RuleAnalyzer::ProcessRealtime() {
	
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
	ProcessRealtime();
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
			VectorBool& data = this->rtdata[sym_id];
			int bit_begin = data.GetCount() - rt_row_size;
			bool enabled = data.Get(bit_begin + 0);
			bool signal  = data.Get(bit_begin + 1);
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
















OrganizationAgent::OrganizationAgent() {
	
}

void OrganizationAgent::Init() {
	
	
	
}











void RuleAnalyzer::RADataCtrl::Paint(Draw& d) {
	Size sz = GetSize();
	ImageDraw id(sz);
	
	id.DrawRect(sz, White());
	
	int mem_pos = cursor * ra->row_size;
	
	for(int sym_id = 0; sym_id < ra->data.GetCount(); sym_id++) {
		const VectorBool& data = ra->data[sym_id];
		int sym_mem_pos = min(data.GetCount() - ra->row_size, mem_pos);
		double xstep = (double)sz.cx / (ra->period_count * ra->data.GetCount());
		double ystep = (double)sz.cy / ra->COUNT;
		int w = xstep + 0.5;
		int h = ystep + 0.5;
		if (sym_mem_pos < 0 || sym_mem_pos + ra->row_size > data.GetCount()) continue;
		for(int i = 0; i < ra->period_count; i++) {
			int x = xstep * (i + sym_id * ra->period_count);
			for(int j = 0; j < ra->COUNT; j++) {
				int y = ystep * j;
				
				bool bit = data.Get(sym_mem_pos++);
				if (bit)
					id.DrawRect(x, y, w, h, Black());
			}
		}
	}
	
	
	d.DrawImage(0,0,id);
}

}



