#include "Overlook.h"

namespace Overlook {

RuleAnalyzer::RuleAnalyzer() {
	not_stopped = 0;
	waiting = 0;
	
	data_check.ra = this;
	datactrl_cursor << THISBACK(SetCursor);
	datactrl_cursor.MinMax(0, 1);
	
	CtrlLayout(*this, "Rule Analyzer");
	prog.Set(0, 1);
	analyze << THISBACK(PostStartProcess);
	LoadThis();
	
	begin.AddColumn("Bit period #");
	begin.AddColumn("Bit type");
	begin.AddColumn("Prob. Av.");
	begin.AddColumn("Succ. idx");
	begin.ColumnWidths("1 3 2 2");
	
	sustain.AddColumn("Bit period #");
	sustain.AddColumn("Bit type");
	sustain.AddColumn("Prob. Av.");
	sustain.AddColumn("Succ. idx");
	sustain.ColumnWidths("1 3 2 2");
	
	end.AddColumn("Bit period #");
	end.AddColumn("Bit type");
	end.AddColumn("Prob. Av.");
	end.AddColumn("Succ. idx");
	end.ColumnWidths("1 3 2 2");
	
	enable.Set(is_enabled);
	enable.WhenAction << THISBACK(SetEnable);
	
	Data();
}

RuleAnalyzer::~RuleAnalyzer() {
	StopProcess();
	StoreThis();
}

void RuleAnalyzer::StartProcess() {
	RealizeStats();
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
}

void RADataCtrl::Paint(Draw& d) {
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
	Vector<OnlineAverageWindow1> av_wins;
	av_wins.SetCount(period_count);
	for(int i = 0; i < av_wins.GetCount(); i++)
		av_wins[i].SetPeriod(1 << (1+i));
	
	
	// Prepare VolatilityContext
	Vector<Vector<double> > volat_divs;
	for(int j = 0; j < period_count; j++) {
		int period = 1 << (1+j);
		volat_divs.Add();
		VectorMap<int,int> median_map;
		for(int cursor = period; cursor < data_count; cursor++) {
			double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
			int step = (int)((diff + spread_point * 0.5) / spread_point);
			median_map.GetAdd(step, 0)++;
		}
		SortByKey(median_map, StdLess<int>());
		volat_divs[j].SetCount(0);
		int64 total = 0;
		for(int i = 0; i < median_map.GetCount(); i++)
			total += median_map[i];
		int64 count_div = total / volat_div;
		total = 0;
		int64 next_div = count_div;
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

void RuleAnalyzer::Data() {
	if (this->data.IsEmpty()) return;
	RealizeStats();
	
	int begin_id = 0;
	
	if (begin_id >= 0 && begin_id < row_size) {
		auto& stats = this->stats[Upp::min(JOINLEVEL_COUNT-1, joinlevel)][begin_id];
		if (stats.begins.IsEmpty()) return;
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.begins[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			begin.Set(i, 0, period_id);
			begin.Set(i, 1, GetTypeString(type_id));
			begin.Set(i, 2, stat.prob_av);
			begin.Set(i, 3, stat.succ_idx);
		}
		begin.SetSortColumn(3, true);
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.sustains[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			sustain.Set(i, 0, period_id);
			sustain.Set(i, 1, GetTypeString(type_id));
			sustain.Set(i, 2, stat.prob_av);
			sustain.Set(i, 3, stat.succ_idx);
		}
		sustain.SetSortColumn(3, true);
		
		for(int i = 0; i < row_size; i++) {
			BitStats& stat = stats.ends[i];
			int period_id = i / COUNT;
			int type_id = i % COUNT;
			end.Set(i, 0, period_id);
			end.Set(i, 1, GetTypeString(type_id));
			end.Set(i, 2, stat.prob_av);
			end.Set(i, 3, stat.succ_idx);
		}
		end.SetSortColumn(3, true);
	}
	
	
	auto& data = this->data[0];
	int data_count = data.GetCount() / row_size;
	if (datactrl_cursor.GetMax() != data_count-1) {
		datactrl_cursor.MinMax(0, data_count-1);
		datactrl_cursor.SetData(data_count-1);
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
	int phase_total = row_size * row_size * LOOP_COUNT;
	
	int current_in_loop = 0;
	while (ts.Elapsed() < 100 && phase < FINISHED && joinlevel < JOINLEVEL_COUNT) {
		int current = thrd_current++;
		
		if (current < phase_total && phase >= BEGIN && phase <= END) {
			Iterate(current, phase - BEGIN);
		}
		
		current_in_loop++;
		
		
		if (thrd_current >= phase_total) {
			if (thrd_id == 0) {
				while (waiting != thrds.GetCount() - 1) Sleep(100); // wait before phase,joinlevel can be changed
				thrd_current = 0;
				phase++;
				
				if (phase >= FINISHED) {
					phase = 0;
					joinlevel++;
					
					if (joinlevel >= JOINLEVEL_COUNT)
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
		actual = phase_total * (phase + FINISHED * joinlevel) + thrd_current;
		total = phase_total * FINISHED * JOINLEVEL_COUNT;
		prev_speed = ((double)current_in_loop / (double)ts.Elapsed()) * 1000.0;
		if (prev_speed < 1.0) prev_speed = 1.0;
		perc = actual * 100 / total;
	}
	PostData();
	
	bool ready = phase == FINISHED && joinlevel == JOINLEVEL_COUNT;
	
}

void RuleAnalyzer::RealizeStats() {
	if (!stats.IsEmpty()) return;
	
	System& sys = GetSystem();
	data.SetCount(sys.GetSymbolCount());
	stats.SetCount(JOINLEVEL_COUNT);
	for(int i = 0; i < stats.GetCount(); i++) {
		stats[i].SetCount(row_size);
		for(int j = 0; j < row_size; j++) {
			BeginStats& bs = stats[i][j];
			bs.begins.SetCount(row_size);
			bs.sustains.SetCount(row_size);
			bs.ends.SetCount(row_size);
			if (i == 0) bs.begin_bits.Add(j);
		}
	}
}

void RuleAnalyzer::Iterate(int current, int type) {
	System& sys = GetSystem();
	RealizeStats();
	
	int bit_id = current / (row_size * row_size);
	int bit_opt = current % (row_size * row_size);
	int begin_id = bit_opt / row_size;
	int sub_id = bit_opt % row_size;
	
	// Getvalues for bits
	auto& stats = this->stats[joinlevel][begin_id];
	
	if (!type && !joinlevel) {
		BitStats& stat = stats.initial;
		stat.prob_av = 0.0;
		for(int i = 0; i < sys.GetSymbolCount(); i++) {
			stat.prob_av += GetBitProbTest(i, begin_id, type, sub_id, 0);
		}
		stat.prob_av /= sys.GetSymbolCount();
		stat.succ_idx = fabs(stat.prob_av * 200.0 - 100.0);
		stats.type = stat.prob_av < 0.5; // Determine type!
		return;
	}
	
	auto& sub_stats = type == 0 ? stats.begins : (type == 1 ? stats.sustains : stats.ends);
	BitStats& stat = sub_stats[sub_id];
	stat.prob_av = 0.0;
	for(int i = 0; i < sys.GetSymbolCount(); i++) {
		stat.prob_av += GetBitProbTest(i, begin_id, type, sub_id, stats.type);
	}
	stat.prob_av /= sys.GetSymbolCount();
	stat.succ_idx = fabs(stat.prob_av * 200.0 - 100.0);
	
	// When bit is added
	if (bit_opt+1 == row_size * row_size && current > 0) {
		for(int begin_id = 0; begin_id < row_size; begin_id++) {
			auto& stats = this->stats[joinlevel][begin_id];
			auto& sub_stats = type == 0 ? stats.begins : (type == 1 ? stats.sustains : stats.ends);
			auto& bits = (type == 0 ? stats.begin_bits : (type == 1 ? stats.sust_bits : stats.end_bits));
			double max_succ_idx = -DBL_MAX;
			int max_pos = -1;
			for(int i = 0; i < sub_stats.GetCount(); i++) {
				double d = sub_stats[i].succ_idx;
				if (d > max_succ_idx) {
					if (bits.Find(i)) continue;
					max_succ_idx = d;
					max_pos = i;
				}
			}
			// Append best bit
			if (max_pos != -1) {
				ASSERT(max_pos >= 0 && max_pos < row_size);
				bits.Add(max_pos);
			}
		}
	}
}

double RuleAnalyzer::GetBitProbBegin(int symbol, int begin_id) {
	VectorBool& data = this->data[symbol];
	ConstBuffer& open_buf = ci_queue[symbol]->core->GetBuffer(0);
	
	int bars = data.GetCount() / row_size - BEGIN_PEEK;
	
	int true_count = 0;
	int pos = begin_id;
	for(int i = 0; i < bars; i++) {
		bool pred_value = data.Get(pos);
		bool actual_value = open_buf.Get(i + BEGIN_PEEK) < open_buf.Get(i);
		pos += row_size;
		if (pred_value == actual_value) true_count++;
	}
	
	return (double)true_count / (double)bars;
}

double RuleAnalyzer::GetBitProbTest(int symbol, int begin_id, int type, int sub_id, bool inv_action) {
	VectorBool& data = this->data[symbol];
	ConstBuffer& open_buf = ci_queue[symbol]->core->GetBuffer(0);
	BeginStats& begin = this->stats[joinlevel][begin_id];
	
	int peek;
	bool test_begin = false, test_sust = false, test_end = false;
	switch (type) {
		case 0:
			test_begin = true;
			peek = BEGIN_PEEK;
			break;
		case 1:
			test_sust = true;
			peek = SUSTAIN_PEEK;
			break;
		case 2:
			test_end = true;
			peek = END_PEEK;
			break;
		default: Panic("Invalid caller");
	}
	
	int bars = data.GetCount() / row_size - peek;
	
	int open_left = 0;
	bool open_dir = false;
	
	int true_count = 0;
	int begin_pos = 0;
	int total = 0;
	for(int i = 0; i < bars; i++) {
		
		// Existing values
		bool begin_value = true;
		for(int j = 0; j < begin.begin_bits.GetCount(); j++)
			begin_value &= data.Get(begin_pos + begin.begin_bits[j]); // AND
		if (begin_value && open_left < BEGIN_PEEK)
			open_left = BEGIN_PEEK;
		
		bool sust_value = false;
		for(int j = 0; j < begin.sust_bits.GetCount(); j++)
			sust_value |= data.Get(begin_pos + begin.sust_bits[j]); // OR
		if (sust_value && open_left == 1)
			open_left++;
		
		bool end_value = false;
		for(int j = 0; j < begin.end_bits.GetCount(); j++)
			end_value |= data.Get(begin_pos + begin.end_bits[j]); // OR
		if (end_value)
			open_left = 0;
		
		// Test values
		if (test_begin && begin_value) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value == actual_value) true_count++;
			total++;
		}
		if (test_sust && open_left > 0) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value == actual_value) true_count++;
			total++;
		}
		if (test_end && open_left > 0) {
			bool pred_value = data.Get(begin_pos + sub_id);
			bool actual_value = open_buf.Get(i + peek) < open_buf.Get(i);
			if (inv_action) actual_value = !actual_value;
			if (pred_value != actual_value) true_count++;
			total++;
		}
		
		if (open_left > 0)
			open_left--;
		begin_pos += row_size;
	}
	
	return total > 0 ? (double)true_count / (double)total : 0.50;
}


void RuleAnalyzer::ProcessRealtime() {
	
	// Get reference values
	System& sys = GetSystem();
	
	for(int i = 0; i < sys.GetSymbolCount(); i++) {
		VectorBool& data = this->data[i];
		DataBridge& db = dynamic_cast<DataBridge&>(*ci_queue[i]->core);
		ASSERT(db.GetSymbol() == i);
		double spread_point		= db.GetPoint();
		
		
		// Prepare maind data
		ConstBuffer& open_buf = db.GetBuffer(0);
		ConstBuffer& low_buf  = db.GetBuffer(1);
		ConstBuffer& high_buf = db.GetBuffer(2);
		int data_count = open_buf.GetCount();
		int bit_count = rt_row_size * data_count;
		int begin = data.GetCount() / rt_row_size;
		int bit_begin = data.GetCount();
		data.SetCount(bit_count);
		
		
		// Continue previous signal
		bool enabled = false;
		bool signal = false;
		if (begin) {
			enabled = data.Get(bit_begin - rt_row_size + 0);
			signal  = data.Get(bit_begin - rt_row_size + 1);
		}
		// Search for signal open
		double start_value = 0;
		int open_left = 0;
		if (enabled) {
			int begin_cursor = bit_begin - rt_row_size;
			for (; begin_cursor >= 0; begin_cursor -= rt_row_size) {
				bool cursor_enabled = data.Get(begin_cursor + 0);
				if (!cursor_enabled) {
					begin_cursor += rt_row_size;
					break;
				}
			}
			if (begin_cursor < 0) begin_cursor = 0;
			int cursor = begin_cursor / rt_row_size;
			start_value = open_buf.Get(cursor);
			open_left = 1;
		}
		
		// Run main data filler
		double total_change = 1.0;
		int begin_pos = begin * row_size;
		for(int cursor = begin; cursor < data_count; cursor++) {
			int bit_pos = cursor * rt_row_size;
			double open1 = open_buf.Get(cursor);
			
			for(int i = 0; i < stats.Top().GetCount(); i++) {
				BeginStats& begin = stats.Top()[i];
				
				// Existing values
				bool begin_value = true;
				for(int j = 0; j < begin.begin_bits.GetCount(); j++)
					begin_value &= data.Get(begin_pos + begin.begin_bits[j]); // AND
				if (begin_value && open_left < BEGIN_PEEK)
					open_left = BEGIN_PEEK;
				
				bool sust_value = false;
				for(int j = 0; j < begin.sust_bits.GetCount(); j++)
					sust_value |= data.Get(begin_pos + begin.sust_bits[j]); // OR
				if (sust_value && open_left == 1)
					open_left++;
				
				bool end_value = false;
				for(int j = 0; j < begin.end_bits.GetCount(); j++)
					end_value |= data.Get(begin_pos + begin.end_bits[j]); // OR
				if (end_value)
					open_left = 0;
				
				bool do_close = false, do_open = false;
				if (open_left > 0) {
					// Change signal
					if (enabled && signal != begin.type) {
						do_close = true;
						do_open = true;
					}
					// Clean start
					else if (!enabled) {
						do_open = true;
					}
				}
				else if (enabled) {
					do_close = true;
				}
				
				if (do_close) {
					double stop_value = open_buf.Get(cursor);
					if (!signal)	total_change *= stop_value / start_value;
					else			total_change *= 1.0 - (stop_value / start_value - 1.0);
					enabled = false;
				}
				
				if (do_open) {
					start_value = open_buf.Get(cursor);
					enabled = true;
					signal = begin.type;
				}
				
				// Use first which leaves enabled signal on
				if (enabled)
					break;
			}
			
			begin_pos += row_size;
		}
	}
}

void RuleAnalyzer::Refresh() {
	if (joinlevel < JOINLEVEL_COUNT)
		return;
	ProcessData();
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
			VectorBool& data = this->data[sym_id];
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

}
