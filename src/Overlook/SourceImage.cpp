#include "Overlook.h"

namespace Overlook {

bool SourceImage::LoadSources() {
	if (GetTf())
		GetSystem().GetSource(GetSymbol(), 0).db.Start();
	db.Start();
	return true;
}

void Job::LoadBooleans() {
	System& sys = GetSystem();
		
	// Get reference values
	double spread_point				= GetPoint();
	ASSERT(spread_point != 0.0);
	Vector<Snap>& data_in			= main_booleans;
	VectorBool& main_in				= main_signal;
	const Vector<double>& open_buf	= GetOpen();
	const Vector<double>& low_buf	= GetLow();
	const Vector<double>& high_buf	= GetHigh();
	
	
	// Prepare maind data
	int begin						= data_in.GetCount();
	end								= open_buf.GetCount();
	data_in.SetCount(end);
	main_in.SetCount(end);
	
	if (begin < end) {
		
		// Main signal (minimal label)
		{
			double cost	 = spread_point * 10;
			const int count = end - begin;
			Vector<bool> sigbuf;
			sigbuf.SetCount(count);
			int main_begin = max(0, begin - 200);
			int main_end = end;
			OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, main_begin, main_end, sigbuf.Begin(), count);
			for(int i = begin, j = 0; i < end; i++, j++) {
				bool label = sigbuf[j];
				main_in.Set(i, label);
			}
		}
		
		// Prepare Moving average
		av_wins.SetCount(period_count);
		for(int i = 0; i < av_wins.GetCount(); i++)
			av_wins[i].SetPeriod(1 << (1+i));
		
		
		// Prepare VolatilityContext
		volat_divs.SetCount(period_count);
		median_maps.SetCount(period_count);
		for(int j = 0; j < period_count; j++) {
			int period = 1 << (1+j);
			VectorMap<int,int>& median_map = median_maps[j];
			for(int cursor = max(period, begin); cursor < end; cursor++) {
				double diff = fabs(open_buf[cursor] - open_buf[cursor - period]);
				int step = (int)((diff + spread_point * 0.5) / spread_point);
				median_map.GetAdd(step, 0)++;
			}
			if (median_map.IsEmpty())
				return;
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
		
		
		// Prepare ChannelOscillator
		ec.SetCount(period_count);
		for(int i = 0; i < ec.GetCount(); i++) {
			ec[i].SetSize(1 << (1+i));
			ec[i].pos = begin - 1;
		}
		
		
		// Prepare BollingerBands
		bbma.SetCount(period_count);
		for(int i = 0; i < bbma.GetCount(); i++) {
			bbma[i].SetPeriod(1 << (1+i));
		}
		
		
		// Run main data filler
		for(int cursor = begin; cursor < end; cursor++) {
			#ifdef flagDEBUG
			if (cursor == 100000) break;
			#endif
			
			Snap& snap = data_in[cursor];
			int bit_pos = 0;
			double open1 = open_buf[cursor];
			
			
			for(int k = 0; k < period_count; k++) {
				
				// MovingAverage
				OnlineAverageWindow1& av_win = av_wins[k];
				double prev = av_win.GetMean();
				av_win.Add(open1);
				double curr = av_win.GetMean();
				snap.Set(bit_pos++, curr < prev);
				snap.Set(bit_pos++, open1 < prev);
				
				
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
				
				
				// VolatilityContext
				int lvl = -1;
				if (cursor >= period) {
					double diff = fabs(open_buf[cursor] - open_buf[cursor - period]);
					for(int i = 0; i < volat_divs[k].GetCount(); i++) {
						if (diff < volat_divs[k][i]) {
							lvl = i - 1;
							break;
						}
					}
				}
				for(int i = 0; i < volat_div; i++)
					snap.Set(bit_pos++,  lvl == i);
			
				
				// Momentum
				begin = Upp::max(0, cursor - period);
				double open2 = open_buf[begin];
				double value = open1 / open2 - 1.0;
				label = value < 0.0;
				snap.Set(bit_pos++, label);
				
				
				// Open/Close trend
				period = 1 << k;
				int dir = 0;
				int len = 0;
				if (cursor >= period * 3) {
					for (int i = cursor-period; i >= 0; i -= period) {
						int idir = open_buf[i+period] > open_buf[i] ? +1 : -1;
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
					double hi = high_buf[cursor-period];
					for (int i = cursor-1-period; i >= 0; i -= period) {
						int idir = hi > high_buf[i] ? +1 : -1;
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
					double lo = low_buf[cursor-period];
					for (int i = cursor-1-period; i >= 0; i -= period) {
						int idir = lo < low_buf[i] ? +1 : -1;
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
					double t0_diff		= open_buf[cursor-0*period] - open_buf[cursor-1*period];
					double t1_diff		= open_buf[cursor-1*period] - open_buf[cursor-2*period];
					double t2_diff		= open_buf[cursor-2*period] - open_buf[cursor-3*period];
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
				
				
				// ChannelOscillator
				period = 1 << (1 + k);
				ExtremumCache& ec = this->ec[k];
				double low  = cursor > 0 ? low_buf[cursor-1] : open1;
				double high = cursor > 0 ? high_buf[cursor-1] : open1;
				double max = Upp::max(open1, high);
				double min = Upp::min(open1, low);
				ec.Add(max, min);
				double ch_high = high_buf[ec.GetHighest()];
				double ch_low = low_buf[ec.GetLowest()];
				double ch_diff = ch_high - ch_low;
				value = (open1 - ch_low) / ch_diff * 2.0 - 1.0;
				snap.Set(bit_pos++, value < 0.0);
				snap.Set(bit_pos++, value < 0.0 ? (value < -0.5) : (value > +0.5));
				
				
				// BollingerBands
				OnlineAverageWindow1& bbma = this->bbma[k];
				bbma.Add(open1);
				double ma = bbma.GetMean();
				double tmp = 0.0;
				for (int j = 0; j < period; j++) {
					double value = open_buf[Upp::max(0, cursor - j)];
					tmp += pow ( value - ma, 2 );
				}
				tmp = sqrt( tmp / period );
				double bb_top = ma + tmp;
				double bb_bot = ma - tmp;
				bool bb_a = open1 < ma;
				bool bb_b = open1 >= bb_top;
				bool bb_c = open1 <= bb_bot;
				snap.Set(bit_pos++, open1 < ma);
				snap.Set(bit_pos++, open1 < ma ? open1 <= bb_bot : open1 >= bb_top);
				
				
				// Descriptor bits
				period = 1 << k;
				for(int i = 0; i < descriptor_count; i++) {
					int pos = Upp::max(0, cursor - period * (i + 1));
					double open2 = open_buf[pos];
					bool value = open1 < open2;
					snap.Set(bit_pos++, value);
				}
			}
			
			ASSERT(bit_pos == row_size - extra_row);
		}
	}
}

void Job::LoadStats() {
	System& sys = GetSystem();
	
	Vector<Snap>& data_in = main_booleans;
	VectorBool& main_in = main_signal;
	auto& stat_in = main_stats;
	
	int stat_osc_cursor = stat_in.cursor;
	
	for(; stat_in.cursor < end; stat_in.cursor++) {
		#ifdef flagDEBUG
		if (stat_in.cursor == 100000) break;
		#endif
		
		bool signal = main_in.Get(stat_in.cursor);
		Snap& snap = data_in[stat_in.cursor];
		
		int id = 0;
		for (int i = 0; i < row_size; i++) {
			int value = snap.Get(i);
			
			SnapStats& ss = stat_in.data[id][value];
			ss.total++;
			if (signal) ss.actual++;
			
			id++;
		}
	}
	
	
	const int ma_period = 15;
	stat_osc_ma.SetPeriod(ma_period);
	if (!stat_osc_cursor <= 100)
		stat_osc_ma.Clear();
	for(; stat_osc_cursor < end; stat_osc_cursor++) {
		Snap& snap = data_in[stat_osc_cursor];
		#ifdef flagDEBUG
		if (stat_osc_cursor == 100000) break;
		#endif
		
		int idxsum = 0, trueidxsum = 0;
		for(int j = 0; j < SNAP_BITS; j++) {
			bool value = snap.Get(j);
			SnapStats& ss = stat_in.data[j][(int)value];
			if (!ss.total) continue;
			int idx = abs(ss.actual * 200 / ss.total - 100);
			if (idx < 10) continue;
			idxsum += idx;
			if (ss.actual >= ss.total / 2)
				trueidxsum += idx;
		}
		
		double d = idxsum > 0 ? (double)trueidxsum / (double)idxsum * -2.0 + 1.0 : 0.0;
		stat_osc_ma.Add(d);
		double ma = stat_osc_ma.GetMean();
		
		bool signal = d < 0.0;
		bool enable = d > 0 ? ma > 0.5 : ma < -0.5;
		snap.Set(row_size - extra_row + 0, signal);
		snap.Set(row_size - extra_row + 1, enable);
	}
}













bool StrandList::Has(Strand& s) {
	for(int i = 0; i < strand_count; i++) {
		if (s.result == strands[i].result)
			return true;
	}
	return false;
}

void StrandList::Sort() {
	for(int i = 0; i < strand_count; i++) {
		int pos = i;
		double max = strands[i].result;
		for(int j = i+1; j < strand_count; j++) {
			Strand& s = strands[j];
			if (s.result > max) {
				max = s.result;
				pos = j;
			}
		}
		Swap(strands[i], strands[pos]);
	}
}

void StrandList::Dump() {
	DUMP(strand_count);
	DUMP(cursor);
	for(int i = 0; i < strand_count; i++) {
		LOG("   " << i << ": " << strands[i].ToString());
	}
}

bool StrandItem::Evolve(int bit, StrandItem& dst) {
	dst = *this;
	bool fail = false;
	for(int i = 0; i < dst.count; i++) {
		if (dst.bits[i] == bit) {
			fail = true;
			break;
		}
	}
	dst.Add(bit);
	return fail;
}

String Strand::ToString() const {
	String out;
	out << "result=" << (double)result << ", " << BitString();
	return out;
}

String Strand::BitString() const {
	String out;
	for(int i = 0; i < enabled.count; i++)
		out << "e" << i << "=" << enabled.bits[i] << ", ";
	for(int i = 0; i < signal_true.count; i++)
		out << "s+" << i << "=" << signal_true.bits[i] << ", ";
	for(int i = 0; i < signal_false.count; i++)
		out << "s-" << i << "=" << signal_false.bits[i] << ", ";
	for(int i = 0; i < trigger_true.count; i++)
		out << "t+" << i << "=" << trigger_true.bits[i] << ", ";
	for(int i = 0; i < trigger_false.count; i++)
		out << "t-" << i << "=" << trigger_false.bits[i] << ", ";
	return out;
}



String Job::GetPhaseString() const {
	switch (phase){
		case PHASE_SOURCE: return "Source";
		case PHASE_BOOLEANS: return "Booleans";
		case PHASE_STATS: return "Statistics";
		case PHASE_TRYSTRANDS: return "Try strands";
		case PHASE_CATCHSTRANDS: return "Catch strands";
	}
	return "";
}

double Job::GetProgress() const {
	return phase * 1000 / PHASE_COUNT;
}

bool Job::IsFinished() const {
	return is_finished;
}

void Job::Process() {
	System& sys = GetSystem();
	
	int sym = GetSymbol();
	int tf = GetTf();
	ReleaseLog(Format("Job::Process sym=%d tf=%d", sym, tf)); // keep this to avoid optimization bugs
	
	if (phase > PHASE_SOURCE) {
		if (tf < MIN_REAL_TFID || tf > MAX_REAL_TFID)
			return;
	}
	
	if (!lock.TryEnter()) return;
	
	if (phase == PHASE_SOURCE) {
		bool r = LoadSources();
		if (sys.running && r) phase++;
	}
	else if (phase == PHASE_BOOLEANS) {
		LoadBooleans();
		if (sys.running) phase++;
	}
	else if (phase == PHASE_STATS) {
		LoadStats();
		if (sys.running) phase++;
	}
	else if (phase == PHASE_TRYSTRANDS) {
		LoadTryStrands();
		if (sys.running) phase++;
	}
	else if (phase == PHASE_CATCHSTRANDS) {
		LoadCatchStrands();
		
		
		if (sys.running) {phase = PHASE_SOURCE; is_finished = true;}
		Sleep(1000);
	}
	
	lock.Leave();
}








void System::StartJobs() {
	if (running || not_stopped > 0) return;
	
	AddJournal("Starting system jobs");
	
	running = true;
	int thrd_count = GetUsedCpuCores();
	not_stopped = 0;
	for(int i = 0; i < thrd_count; i++) {
		not_stopped++;
		Thread::Start(THISBACK1(JobWorker, i));
	}
}

void System::StopJobs() {
	AddJournal("Stopping system jobs");
	
	running = false;
}

void System::JobWorker(int i) {
	
	while (running) {
		
		workitem_lock.Enter();
		bool found = false;
		int job_id;
		for(int i = 0; i < jobs.GetCount(); i++) {
			job_id = worker_cursor++;
			if (worker_cursor >= jobs.GetCount())
				worker_cursor = 0;
			Job& job = *jobs[job_id];
			if (job.IsFinished())
				continue;
			found = true;
			break;
		}
		if (!found) running = false;
		workitem_lock.Leave();
		if (!found) break;
		
		Job& job = *jobs[job_id];
		job.Process();
	}
	
	
	
	not_stopped--;
}

}
