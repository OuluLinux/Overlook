#include "Overlook.h"


namespace Overlook {
using namespace Upp;


SystemCtrl::SystemCtrl() {
	prog.Set(0, 1);
	
	Add(prog.BottomPos(2,26).HSizePos(2,2));
	Add(load_sources.TopPos(2, 26).LeftPos(2,96));
	Add(load_booleans.TopPos(32, 26).LeftPos(2,96));
	
	load_sources.SetLabel("Load sources");
	load_sources <<= THISBACK(StartLoadSources);
	load_booleans.SetLabel("Load booleans");
	load_booleans <<= THISBACK(StartLoadBooleans);
	
	
}

void SystemCtrl::LoadSources() {
	System& sys = GetSystem();
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	stopped = true;
}

void SystemCtrl::LoadBooleans() {
	System& sys = GetSystem();
	ChartImage ci;
	
	const int period_count = 6;
	const int volat_div = 6;
	const int row_size = period_count * (9 + volat_div);
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		
		// Get reference values
		System& sys = GetSystem();
		double spread_point				= ci.GetPoint();
		Vector<Snap>& data_in			= sys.main_booleans[sym];
		const Vector<double>& open_buf	= si.db.open;
		const Vector<double>& low_buf	= si.db.low;
		const Vector<double>& high_buf	= si.db.high;
		
		
		// Prepare maind data
		int begin						= data_in.GetCount();
		int end							= open_buf.GetCount();
		data_in.SetCount(end);
		
		
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
		
		
		// Run main data filler
		for(int cursor = begin; cursor < end; cursor++) {
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
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	stopped = true;
}

void SystemCtrl::LoadStats() {
	System& sys = GetSystem();
	ChartImage image;
	
	sys.main_booleans.Clear();
	
	int sym = 0;
	while (running) {
		PostCallback(THISBACK2(SetProg, sym + 1, sys.GetSymbolCount() + 1));
		
		SourceImage& si = sys.data[sym][0];
		si.db.Start();
		
		
		for (int f = 0; f < FACTORY_COUNT && running; f++) {
			double prog = (double)f / (double)FACTORY_COUNT;
			PostCallback(THISBACK2(SetProg, (sym + 1.0 + prog) * 100.0, (sys.GetSymbolCount() + 1) * 100.0));
			
			FactoryDeclaration decl;
			decl.factory = f;
			
			int bars = si.db.open.GetCount();
			
			const int screen_count = bars;
			const int useful = bars;
			bool once = true;
			
			image.begin = 0;
			image.end = bars;
			image.symbol = sym;
			image.tf = 0;
			image.period = si.db.GetPeriod();
			image.cursor = 0;
			image.point = si.db.GetPoint();
			
			
			ImageCompiler comp;
			
			comp.SetMain(decl);
			
			comp.Compile(si, image);
			
			
			
			/*int bool_count = image.graphs[0].booleans.GetCount();
			
			sys.main_booleans.SetCount(prev_booleans_count + bool_count);
			
			for(int k = 0; k < bool_count; k++) {
				sys.main_booleans[prev_booleans_count + k] = image.graphs[0].booleans[k];
			}
			
			prev_booleans_count = sys.main_booleans.GetCount();*/
		}
		
		
		if (++sym >= sys.GetSymbolCount())
			break;
	}
	
	PostCallback(THISBACK2(SetProg, 0, 1));
	stopped = true;
}

}

