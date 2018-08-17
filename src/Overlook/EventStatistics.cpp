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
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(5).AddArg(0).AddArg(5);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(10).AddArg(0).AddArg(10);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(15).AddArg(0).AddArg(15);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(20).AddArg(0).AddArg(20);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(30).AddArg(0).AddArg(20);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(40).AddArg(0).AddArg(20);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(50).AddArg(0).AddArg(10);
	indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(60).AddArg(0).AddArg(10);
	indi_ids.Add().Set(sys.Find<NewsNow>()).AddArg(1);
	ASSERT(indi_ids.GetCount() == SRC_COUNT);

	for(int i = 0; i < sys.GetNetCount(); i++)
		symbols.Add(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	//symbols.Add("NewsNet");
	//symbols.Add("AfterNewsNet");
	for(int i = 0; i < symbols.GetCount(); i++)
		sym_ids.Add(sys.FindSymbol(symbols[i]));
	
	tf_ids.Add(VTF);
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	stats.SetCount(sym_ids.GetCount());
	for(int j = 0; j < stats.GetCount(); j++) {
		stats[j].SetCount(SRC_COUNT);
		for(int i = 0; i < stats[j].GetCount(); i++)
			stats[j][i].SetCount(width);
	}
	
	bufs.SetCount(sym_ids.GetCount());
	lbls.SetCount(sym_ids.GetCount());
	db.SetCount(sym_ids.GetCount(), NULL);
	db_m1.SetCount(sym_ids.GetCount(), NULL);
	for(int i = 0; i < bufs.GetCount(); i++) {
		bufs[i].SetCount(indi_ids.GetCount());
		lbls[i].SetCount(indi_ids.GetCount(), NULL);
	}
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		sys.Process(ci, true);
		
		Core& c = *ci.core;
		
		int faci = -1;
		for(int i = 0; i < indi_ids.GetCount(); i++) {
			const FactoryDeclaration& fd = indi_ids[i];
			if (fd.factory == c.GetFactory()) {
				bool match = true;
				for(int j = 0; j < fd.arg_count && match; j++)
					if (fd.args[j] != ci.args[j])
						match = false;
				if (match) {
					faci = i;
					break;
				}
			}
		}
		int symi = sym_ids.Find(c.GetSymbol());
		int tfi = tf_ids.Find(c.GetTf());
		
		if (symi == -1) continue;
		if (c.GetTf() == 0 && c.GetFactory() == 0)
			db_m1[symi] = dynamic_cast<DataBridge*>(&c);
		if (tfi == -1) continue;
		
		auto& v = bufs[symi][faci];
		v.SetCount(c.GetBufferCount());
		for(int j = 0; j < c.GetBufferCount(); j++) {
			v[j] = &c.GetBuffer(j);
		}
		
		if (faci == 0) {
			db[symi] = &dynamic_cast<DataBridge&>(c);
		}
		else if (c.GetLabelCount() && c.GetLabelBufferCount(0)) {
			lbls[symi][faci] = &c.GetLabelBuffer(0, 0);
		}
	}
	ASSERT(db_m1[0]);
	
	RefreshData();
	
	for(int i = 0; i < sym_ids.GetCount(); i++)
		UpdateEvents(i);
	
	ReleaseLog("EventStatistics work queue init took " + ts.ToString());
	
	
	prev_bars = bufs[0][OPEN][0]->GetCount();
}

void EventStatistics::RefreshData() {
	System& sys = GetSystem();
	for(int i = 0; i < work_queue.GetCount(); i++)
		sys.Process(*work_queue[i], true);
}

void EventStatistics::Start() {
	System& sys = GetSystem();
	
	RefreshData();
	
	// Play alarm when vtf has new bars
	int bars = bufs[0][OPEN][0]->GetCount();
	if (bars != prev_bars) {
		PlayAlarm(0);
		prev_bars = bars;
	}
	
	// Play alarm when 1.5pip*net_size change happens fast
	DataBridge& d = *db_m1[0];
	bars = d.GetBuffer(0).GetCount();
	if (bars - last_alarm1_bars >= 5) {
		int count = 5;
		int pips = 15;
		int max_pips = 0;
		double o0 = d.GetBuffer(0).Get(bars - 1);
		for (int i = bars-2; i >= bars-count-1; i--) {
			double o1 = d.GetBuffer(0).Get(i);
			int pips = fabs((o0 - o1) / d.GetPoint());
			max_pips = max(max_pips, pips);
		}
		if (max_pips >= pips) {
			PlayAlarm(1);
			last_alarm1_bars = bars;
		}
	}
}


void EventStatistics::UpdateEvents(int sym) {
	System& sys = GetSystem();
	
	int width = sys.GetVtfWeekbars();
	int height = SRC_COUNT;
	
	ConstBuffer& open_buf = *bufs[sym][OPEN][0];
	
	for(int i = 0; i < SRC_COUNT; i++) {
		ConstLabelSignal& lbl = *lbls[sym][i];
		Vector<StatSlot>& stats = this->stats[sym][i];
		
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
		case BB5:		return "Bollinger bands 5";
		case BB10:		return "Bollinger bands 10";
		case BB15:		return "Bollinger bands 15";
		case BB20:		return "Bollinger bands 20";
		case BB30:		return "Bollinger bands 30";
		case BB40:		return "Bollinger bands 40";
		case BB50:		return "Bollinger bands 50";
		case BB60:		return "Bollinger bands 60";
		case NEWSNOW:	return "News Now";
	}
	return "Unknown";
}

int EventStatistics::GetSignal(int sym, int i, int src) {
	if (src == 0) {
		ConstBuffer& open_buf = *bufs[sym][OPEN][0];
		if (!i) return 0;
		double o0 = open_buf.Get(i);
		double o1 = open_buf.Get(i-1);
		return o0 < o1 ? -1 : +1;
	} else {
		ConstLabelSignal& lbl = *lbls[sym][src];
		bool signal = lbl.signal.Get(i);
		bool enabled = lbl.enabled.Get(i);
		LOG("GetSignal " << i << " sig " << (int)signal << " enabled " << (int)enabled << " " << (int)&lbl);
		return enabled ? (signal ? -1 : +1) : 0;
	}
}

int EventStatistics::GetPreferredNet() {
	ConstBuffer& nn_buf = *bufs[0][NEWSNOW][0];
	if (nn_buf.IsEmpty()) return 0;
	double d = nn_buf.Top();
	return d < 0;
}

int EventStatistics::GetLatestSlotId() {
	System& sys = GetSystem();
	ConstBuffer& nn_buf = *bufs[0][NEWSNOW][0];
	int slot_id = (nn_buf.GetCount() - 1) % sys.GetVtfWeekbars();
	return slot_id;
}
	
const StatSlot& EventStatistics::GetLatestSlot(int net, int i) {
	System& sys = GetSystem();
	ConstBuffer& nn_buf = *bufs[0][NEWSNOW][0];
	int slot_id = (nn_buf.GetCount() - 1) % sys.GetVtfWeekbars();
	return this->stats[net][i][slot_id];
}
	
const StatSlot& EventStatistics::GetSlot(int net, int i, int wb) {
	return this->stats[net][i][wb];
}




















EventStatisticsCtrl::EventStatisticsCtrl() {
	Add(symlist.TopPos(0,30).LeftPos(0,100));
	Add(list.VSizePos().HSizePos(100));
	Add(activelbl.TopPos(30,60).LeftPos(0,100));
	
	
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetNetCount(); i++)
		symlist.Add(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	symlist.SetIndex(0);
	symlist <<= THISBACK(Data);
	
	
	String colw;
	
	list.AddColumn("Latest time");
	colw += "3 ";
	for(int i = 0; i < 3; i++) {
		list.AddColumn(IntStr(i + 1) + ". description");
		list.AddColumn(IntStr(i + 1) + ". average");
		list.AddColumn(IntStr(i + 1) + ". class");
		list.AddColumn(IntStr(i + 1) + ". signal");
		colw += "3 2 1 1 ";
	}
	list.ColumnWidths(colw);
	
	
}
	
void EventStatisticsCtrl::Data() {
	System& sys = GetSystem();
	EventStatistics& es = GetEventStatistics();
	int width = sys.GetVtfWeekbars();
	int height = EventStatistics::SRC_COUNT;
	int sym = symlist.GetIndex();
	
	if (es.stats.IsEmpty())
		return;
	
	String albl = es.GetPreferredNet() ? "AfterNewsNet\nis preferred" : "NewsNet\nis preferred";
	activelbl.SetLabel(albl);
	
	ConstBuffer& time_buf = es.db[sym]->GetBuffer(4);
	int limit = time_buf.GetCount() % width;
	
	VectorMap<int, double> stats;
	for(int i = 0; i < width; i++) {
		int time_pos = i < limit ? time_buf.GetCount() - limit + i : time_buf.GetCount() - width - limit + i;
		Time t = Time(1970,1,1) + time_buf.Get(time_pos);
		
		stats.Clear();
		for(int j = 0; j < height; j++) {
			OnlineVariance& av = es.stats[sym][j][i].av;
			double mean = av.GetMean();
			double cdf = av.GetCDF(0, 0);
			if (cdf < 0.5) cdf = 1.0 - cdf;
			int grade = (1.0 - cdf) / 0.05;
			if (grade < EventOptimization::grade_count) {
				stats.Add(j, +mean);
				stats.Add(-j-1, -mean);
			}
		}
		SortByValue(stats, StdGreater<double>());
		
		int col = 0;
		list.Set(i, col++, t);
		
		for(int j = 0; j < 3 && j < stats.GetCount(); j++) {
			int k = stats.GetKey(j);
			int l = k >= 0 ? k : -k-1;
			String desc = es.GetDescription(l);
			int sig = es.GetSignal(sym, time_pos, l);
			if (k < 0) {
				desc += " inverse";
				sig *= -1;
			}
			double v = stats[j];
			double cdf = es.stats[sym][l][i].av.GetCDF(0, k >= 0);
			int grade = 'A' + (1.0 - cdf) / 0.05;
			String grade_str;
			grade_str.Cat(grade);
			list.Set(i, col++, desc);
			list.Set(i, col++, v);
			list.Set(i, col++, grade_str);
			list.Set(i, col++, sig == 0 ? "" : (sig > 0 ? "Long" : "Short"));
		}
		
	}
	
}

}