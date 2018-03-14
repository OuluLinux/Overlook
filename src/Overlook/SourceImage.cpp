#if 0
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
		
		for (int i = 0; i < row_size; i++) {
			int value = snap.Get(i);
			
			SnapStats& ss = stat_in.data[i][value];
			ss.total++;
			if (signal) ss.actual++;
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
	
	if (phase > PHASE_SOURCE || sym == -1) {
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







}
#endif
