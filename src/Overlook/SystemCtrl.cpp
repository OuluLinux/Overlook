#include "Overlook.h"


namespace Overlook {
using namespace Upp;

void BooleansDraw::Paint(Draw& w) {
	System& sys = GetSystem();
	
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	if (cursor >= 0) {
		
		int w = SystemCtrl::row_size;
		int h = sys.GetSymbolCount();
		
		double xstep = (double)sz.cx / w;
		double ystep = (double)sz.cy / h;
		
		for(int i = 0; i < h; i++) {
			int count = i < sys.main_booleans.GetCount() ? sys.main_booleans[i].GetCount() : 0;
			if (count == 0) continue;
			const Snap& snap = sys.main_booleans[i][min(count-1, cursor)];
			
			int y0 = i * ystep;
			int y1 = (i + 1) * ystep;
			for(int j = 0; j < w; j++) {
				int x0 = j * xstep;
				int x1 = (j + 1) * xstep;
				
				bool b = snap.Get(j);
				if (b)
					id.DrawRect(x0, y0, x1-x0, y1-y0, Black());
				
			}
		}
	}
	
	w.DrawImage(0, 0, id);
}

SystemCtrl::SystemCtrl() {
	prog.Set(0, 1);
	
	Add(prog.BottomPos(2,26).HSizePos(2,2));
	Add(load_sources.TopPos(2, 26).LeftPos(2,96));
	Add(load_booleans.TopPos(32, 26).LeftPos(2,96));
	Add(load_stats.TopPos(62, 26).LeftPos(2,96));
	Add(slider.HSizePos(100).BottomPos(30,30));
	Add(hsplit.HSizePos(100).VSizePos(0,60));
	
	slider.MinMax(0,1);
	slider << THISBACK(Data);
	
	hsplit << stats << bools;
	
	load_sources.SetLabel("Load sources");
	load_sources <<= THISBACK(StartLoadSources);
	load_booleans.SetLabel("Load booleans");
	load_booleans <<= THISBACK(StartLoadBooleans);
	load_stats.SetLabel("Load stats");
	load_stats <<= THISBACK(StartLoadStats);
	
	stats.AddColumn("Bit combination");
	stats.AddColumn("Percent");
	stats.AddColumn("Index");
	
}

void SystemCtrl::Data() {
	System& sys = GetSystem();
	
	const int sym = 0;
	
	slider.MinMax(0, sys.main_booleans.GetCount() ? sys.main_booleans[0].GetCount() : 1);
	bools.cursor = slider.GetData();
	bools.Refresh();
	
	
	if (sys.main_stats.IsEmpty()) return;
	
	if (running)
		stats.Clear();
	
	if (stats.GetCount() == 0 && stopped) {
			
		auto& stat_in = sys.main_stats[sym];
		
		
		int id = 0, row = 0;
		for (int i = 0; i < row_size; i++) {
			String key;
			key << i << " ";
			
			for(int i = 0; i < 2; i++) {
				SnapStats& ss = stat_in.data[id][i];
				
				stats.Set(row, 0, key + IntStr(i));
				if (ss.total > 0) {
					stats.Set(row, 1, IntStr(ss.actual * 100 / ss.total) + "%");
					stats.Set(row, 2, abs(ss.actual * 200 / ss.total - 100));
				} else {
					stats.Set(row, 1, "");
					stats.Set(row, 2, "");
				}
				row++;
			}
			
			id++;
		}
		
		stats.SetSortColumn(2, true);
	}
}

void SystemCtrl::LoadSources() {
	System& sys = GetSystem();
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		sys.StoreThis();
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	running = false;
	stopped = true;
	Enable();
}

void SystemCtrl::LoadBooleans() {
	System& sys = GetSystem();
	ChartImage ci;
	
	
	sys.main_booleans.SetCount(sys.GetSymbolCount());
	sys.main_signal.SetCount(sys.GetSymbolCount());
	sys.main_stats.SetCount(sys.GetSymbolCount());
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		
		// Get reference values
		System& sys = GetSystem();
		double spread_point				= si.db.GetPoint();
		ASSERT(spread_point != 0.0);
		Vector<Snap>& data_in			= sys.main_booleans[sym];
		VectorBool& main_in				= sys.main_signal[sym];
		const Vector<double>& open_buf	= si.db.open;
		const Vector<double>& low_buf	= si.db.low;
		const Vector<double>& high_buf	= si.db.high;
		
		
		// Prepare maind data
		int begin						= data_in.GetCount();
		int end							= open_buf.GetCount();
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
			Vector<OnlineAverageWindow1> av_wins;
			av_wins.SetCount(period_count);
			for(int i = 0; i < av_wins.GetCount(); i++)
				av_wins[i].SetPeriod(1 << (1+i));
			
			
			// Prepare VolatilityContext
			Vector<Vector<double> > volat_divs;
			Vector<VectorMap<int,int> > median_maps;
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
			
			DUMPCC(volat_divs);
			
			
			// Run main data filler
			for(int cursor = begin; cursor < end; cursor++) {
				#ifdef flagDEBUG
				if (cursor == 100000) break;
				#endif
				
				Snap& snap = data_in[cursor];
				int bit_pos = 0;
				double open1 = open_buf[cursor];
				
				
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
					if (label) {
						LOG(cursor);
					}
				
					
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
					
				
					// MovingAverage
					OnlineAverageWindow1& av_win = av_wins[k];
					double prev = av_win.GetMean();
					av_win.Add(open1);
					double curr = av_win.GetMean();
					label = open1 < prev;
					snap.Set(bit_pos++, label);
					
					
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
				}
				
				ASSERT(bit_pos == row_size);
			}
			
			sys.StoreThis();
		}
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	running = false;
	stopped = true;
	Enable();
}

void SystemCtrl::LoadStats() {
	System& sys = GetSystem();
	ChartImage image;
	
	if (sys.main_signal.IsEmpty()) {Enable(); running = false; stopped = true; return;}
	sys.main_stats.SetCount(sys.GetSymbolCount());
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		Vector<Snap>& data_in = sys.main_booleans[sym];
		VectorBool& main_in = sys.main_signal[sym];
		auto& stat_in = sys.main_stats[sym];
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		
		int bars = si.db.open.GetCount();
		
		
		for(; stat_in.cursor < bars; stat_in.cursor++) {
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
		
		
		sys.StoreThis();
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	running = false;
	stopped = true;
	Enable();
}

}

