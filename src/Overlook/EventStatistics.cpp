#include "Overlook.h"

namespace Overlook {

EventStatistics::EventStatistics() {
	
	
}

EventStatistics::~EventStatistics() {
	running = false;
	while (!stopped) Sleep(100);
}
	
void EventStatistics::Init() {
	TimeStop ts;
	
	System& sys = GetSystem();
	
	int sym_count = sys.GetNormalSymbolCount();
	int cur_count = sys.GetCurrencyCount();
	int net_count = sys.GetNetCount();
	int width = sys.GetVtfWeekbars();
	
	indi_ids.Add().Set(sys.Find<DataBridge>());
	indi_ids.Add().Set(sys.Find<SimpleHurstWindow>()).AddArg(4);
	indi_ids.Add().Set(sys.Find<SimpleHurstWindow>()).AddArg(8);
	indi_ids.Add().Set(sys.Find<SimpleHurstWindow>()).AddArg(16);
	indi_ids.Add().Set(sys.Find<MovingAverage>()).AddArg(3).AddArg(0).AddArg(1);
	indi_ids.Add().Set(sys.Find<MovingAverage>()).AddArg(9).AddArg(0).AddArg(1);
	indi_ids.Add().Set(sys.Find<MovingAverage>()).AddArg(27).AddArg(0).AddArg(1);
	indi_ids.Add().Set(sys.Find<BollingerBands>());
	indi_ids.Add().Set(sys.Find<ParabolicSAR>());
	indi_ids.Add().Set(sys.Find<PeriodicalChange>());
	indi_ids.Add().Set(sys.Find<TickBalanceOscillator>()).AddArg(4);
	indi_ids.Add().Set(sys.Find<TickBalanceOscillator>()).AddArg(8);
	indi_ids.Add().Set(sys.Find<TickBalanceOscillator>()).AddArg(16);
	indi_ids.Add().Set(sys.Find<PeekChange>()).AddArg(5);
	indi_ids.Add().Set(sys.Find<PeekChange>()).AddArg(15);
	indi_ids.Add().Set(sys.Find<PeekChange>()).AddArg(30);
	ASSERT(indi_ids.GetCount() == SRC_COUNT);
	stats.SetCount(SRC_COUNT);
	for(int i = 0; i < stats.GetCount(); i++)
		stats[i].SetCount(width);

	for(int i = 0; i < indi_ids.GetCount(); i++)
		fac_ids.Add(indi_ids[i].factory);
	
	symbols.Add("NewsNet");
	for(int i = 0; i < symbols.GetCount(); i++)
		sym_ids.Add(sys.FindSymbol(symbols[i]));
	
	tf_ids.Add(VTF);
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	
	bufs.SetCount(fac_ids.GetCount());
	lbls.SetCount(fac_ids.GetCount(), NULL);
	
	int faci = 0;
	for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
		//int faci = fac_ids.Find(c.GetFactory());
		int tfi = tf_ids.Find(c.GetTf());
		
		if (tfi == -1) continue;
		
		auto& v = bufs[faci];
		v.SetCount(c.GetBufferCount());
		for(int j = 0; j < c.GetBufferCount(); j++) {
			v[j] = &c.GetBuffer(j);
		}
		
		if (faci == 0) {
			db = &dynamic_cast<DataBridge&>(c);
		} else {
			lbls[faci] = &c.GetLabelBuffer(0, 0);
		}
		
		faci++;
	}
	ASSERT(faci == fac_ids.GetCount());
	
	
	ReleaseLog("EventStatistics work queue init took " + ts.ToString());
	
	
	prev_bars = bufs[OPEN][0]->GetCount();
}

void EventStatistics::Start() {
	System& sys = GetSystem();
	
	for(int i = 0; i < work_queue.GetCount() /*&& IsRunning()*/; i++)
		sys.Process(*work_queue[i], true);
	
	UpdateEvents();
	
	int bars = bufs[OPEN][0]->GetCount();
	if (bars != prev_bars) {
		PlayAlarm();
		prev_bars = bars;
	}
}


void EventStatistics::UpdateEvents() {
	System& sys = GetSystem();
	
	int width = sys.GetVtfWeekbars();
	int height = SRC_COUNT;
	
	ConstBuffer& open_buf = *bufs[OPEN][0];
	
	for(int i = 0; i < SRC_COUNT; i++) {
		ConstLabelSignal& lbl = *lbls[i];
		Vector<StatSlot>& stats = this->stats[i];
		
		if (i == 0) {
			for(int j = 1; j < open_buf.GetCount() - 1; j++) {
				double o0 = open_buf.Get(j);
				double o1 = open_buf.Get(j-1);
				double diff = o0 - o1;
				bool signal = diff < 0;
				
				StatSlot& stat = stats[j % width];
				o1 = o0;
				o0 = open_buf.Get(j+ 1);
				diff = o0 - o1;
				if (signal) diff *= -1;
				stat.AddResult(diff);
			}
		} else {
			for(int j = 0; j < lbl.signal.GetCount() - 1; j++) {
				bool enabled = lbl.enabled.Get(j);
				bool signal = lbl.signal.Get(j);
				
				
				if (enabled) {
					StatSlot& stat = stats[j % width];
					double o0 = open_buf.Get(j+1);
					double o1 = open_buf.Get(j);
					double diff = o0 - o1;
					if (signal) diff *= -1;
					stat.AddResult(diff);
				}
			}
		}
	}
	
	
}

String EventStatistics::GetDescription(int i) {
	switch (i) {
		case OPEN:		return "Prev change dir";
		case HU4:		return "Hurst 4";
		case HU8:		return "Hurst 8";
		case HU16:		return "Hurst 16";
		case MA3:		return "Moving average 3";
		case MA9:		return "Moving average 9";
		case MA27:		return "Moving average27";
		case BB:		return "Bollinger bands";
		case PSAR:		return "Parabolic SAR";
		case PC:		return "Periodical Change";
		case TB4:		return "Tick Balance 4";
		case TB8:		return "Tick Balance 8";
		case TB16:		return "Tick Balance 16";
		case PEEKC5:	return "Peek change 5";
		case PEEKC15:	return "Peek change 15";
		case PEEKC30:	return "Peek change 30";
	}
	return "Unknown";
}

int EventStatistics::GetSignal(int i, int src) {
	if (src == 0) {
		ConstBuffer& open_buf = *bufs[OPEN][0];
		if (!i) return 0;
		double o0 = open_buf.Get(i);
		double o1 = open_buf.Get(i-1);
		return o0 < o1 ? -1 : +1;
	} else {
		ConstLabelSignal& lbl = *lbls[src];
		bool signal = lbl.signal.Get(i);
		bool enabled = lbl.enabled.Get(i);
		return enabled ? (signal ? -1 : +1) : 0;
	}
}





















EventStatisticsCtrl::EventStatisticsCtrl() {
	Add(list.SizePos());
	
	list.AddColumn("Latest time");
	
	for(int i = 0; i < 3; i++) {
		list.AddColumn(IntStr(i + 1) + ". description");
		list.AddColumn(IntStr(i + 1) + ". average");
		list.AddColumn(IntStr(i + 1) + ". signal");
	}
	
	
}
	
void EventStatisticsCtrl::Data() {
	System& sys = GetSystem();
	EventStatistics& es = GetEventStatistics();
	int width = sys.GetVtfWeekbars();
	int height = EventStatistics::SRC_COUNT;
	
	if (es.stats.IsEmpty())
		return;
	
	ConstBuffer& time_buf = es.db->GetBuffer(4);
	int limit = time_buf.GetCount() % width;
	
	VectorMap<int, double> stats;
	for(int i = 0; i < width; i++) {
		int time_pos = i < limit ? time_buf.GetCount() - limit + i : time_buf.GetCount() - width - limit + i;
		Time t = Time(1970,1,1) + time_buf.Get(time_pos);
		
		stats.Clear();
		for(int j = 0; j < height; j++) {
			double mean = es.stats[j][i].av.GetMean();
			stats.Add(j, +mean);
			stats.Add(-j-1, -mean);
		}
		SortByValue(stats, StdGreater<double>());
		
		int col = 0;
		list.Set(i, col++, t);
		
		for(int j = 0; j < 3; j++) {
			int k = stats.GetKey(j);
			int l = k >= 0 ? k : -k-1;
			String desc = es.GetDescription(l);
			int sig = es.GetSignal(time_pos, l);
			if (k < 0) {
				desc += " inverse";
				sig *= -1;
			}
			double v = stats[j];
			list.Set(i, col++, desc);
			list.Set(i, col++, v);
			list.Set(i, col++, sig == 0 ? "" : (sig > 0 ? "Long" : "Short"));
		}
		
	}
	
}

}