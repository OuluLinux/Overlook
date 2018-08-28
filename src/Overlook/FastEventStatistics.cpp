#include "Overlook.h"

namespace Overlook {

FastEventStatistics::FastEventStatistics() {
	
	
}

FastEventStatistics::~FastEventStatistics() {
	running = false;
	while (!stopped) Sleep(100);
}
	
void FastEventStatistics::Init() {
	TimeStop ts;
	
	System& sys = GetSystem();
	
	int sym_count = sys.GetNormalSymbolCount();
	int cur_count = sys.GetCurrencyCount();
	int net_count = sys.GetNetCount();
	int width = FAST_WIDTH;
	
	indi_ids.Add().Set(sys.Find<DataBridge>());
	indi_lbls.Add("Lbl");
	for (int period = 10; period <= 100; period += 10) {
		for (int dev = 5; dev <= 20; dev++) {
			indi_ids.Add().Set(sys.Find<BollingerBands>()).AddArg(period).AddArg(0).AddArg(dev);
			indi_lbls.Add("Bollinger Bands " + IntStr(period) + "/" + IntStr(dev));
		}
	}
	

	for(int i = 0; i < sys.GetNetCount(); i++)
		symbols.Add(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	//symbols.Add("NewsNet");
	//symbols.Add("AfterNewsNet");
	for(int i = 0; i < symbols.GetCount(); i++)
		sym_ids.Add(sys.FindSymbol(symbols[i]));
	
	tf_ids.Add(FAST_TF);
	
	sys.GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
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
		sys.Process(ci, false, true);
		
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
	
	
	
	
	LoadThis();
	if (stats.IsEmpty()) {
		stats.SetCount(sym_ids.GetCount());
		for(int j = 0; j < stats.GetCount(); j++) {
			stats[j].SetCount(SRC_COUNT);
			for(int i = 0; i < stats[j].GetCount(); i++)
				stats[j][i].SetCount(width);
		}
		
		for(int i = 0; i < sym_ids.GetCount(); i++)
			UpdateEvents(i);
		
		StoreThis();
	}
	
	ReleaseLog("FastEventStatistics work queue init took " + ts.ToString());
	
	
	prev_bars = bufs[0][0][0]->GetCount();
}

void FastEventStatistics::RefreshData() {
	System& sys = GetSystem();
	for(int i = 0; i < work_queue.GetCount(); i++)
		sys.Process(*work_queue[i], false);
}

void FastEventStatistics::Start() {
	RefreshData();
}


void FastEventStatistics::UpdateEvents(int sym) {
	System& sys = GetSystem();
	
	int width = FAST_WIDTH;
	int height = SRC_COUNT;
	
	ConstBuffer& open_buf = *bufs[sym][0][0];
	ConstBuffer& time_buf = *bufs[sym][0][4];
	
	for(int i = 1; i < SRC_COUNT; i++) {
		ConstLabelSignal& lbl = *lbls[sym][i];
		Vector<FastStatSlot>& stats = this->stats[sym][i];
		
		for(int j = 1; j < lbl.signal.GetCount() - 1; j++) {
			bool enabled = lbl.enabled.Get(j);
			bool prev_enabled = lbl.enabled.Get(j-1);
			bool signal = lbl.signal.Get(j);
			
			
			if (enabled && !prev_enabled) {
				Time t = Time(1970,1,1) + time_buf.Get(j);
				int wday = DayOfWeek(t);
				int slot = wday * 24 + t.hour;
				FastStatSlot& stat = stats[slot];
				double o = open_buf.Get(j);
				int neg_count = 0, inv_neg_count = 0;
				for(int k = 0; k < MAX_FAST_LEN; k++) {
					int pos0 = j+1+k;
					int pos1 = j+k;
					if (pos0 >= open_buf.GetCount()) break;
					double o0 = open_buf.Get(pos0);
					double o1 = open_buf.Get(pos1);
					double diff = o0 - o1;
					double diff_from_begin = o0 - o;
					#if NEGCOUNT_ENABLED
					if (signal) {
						diff *= -1;
						diff_from_begin *= -1;
					}
					if (diff < 0) {
						stat.AddResult(neg_count, diff_from_begin, o0 / o - 1.0);
						neg_count++;
					}
					else if (diff > 0) {
						stat.AddInvResult(inv_neg_count, -diff_from_begin, o0 / o - 1.0);
						inv_neg_count++;
					}
					#else
					stat.AddResult(k, diff_from_begin, o0 / o - 1.0);
					stat.AddInvResult(k, -diff_from_begin, o0 / o - 1.0);
					#endif
				}
			}
		}
	}
	
	
}

String FastEventStatistics::GetDescription(int i) {
	if (i < 0 || i >= indi_lbls.GetCount())
		return "Unknown";
	return indi_lbls[i];
}

int FastEventStatistics::GetSignal(int sym, int i, int src) {
	if (src == 0) {
		return 0;
	} else {
		ConstLabelSignal& lbl = *lbls[sym][src];
		bool signal = lbl.signal.Get(i);
		bool enabled = lbl.enabled.Get(i);
		return enabled ? (signal ? -1 : +1) : 0;
	}
}

int FastEventStatistics::GetOpenSignal(int sym, int i, int src) {
	if (src == 0 || !i) {
		return 0;
	} else {
		ConstLabelSignal& lbl = *lbls[sym][src];
		bool prev_enabled = lbl.enabled.Get(i-1);
		if (prev_enabled)
			return 0;
		bool signal = lbl.signal.Get(i);
		bool enabled = lbl.enabled.Get(i);
		return enabled ? (signal ? -1 : +1) : 0;
	}
}

int FastEventStatistics::GetLatestSlotId() {
	System& sys = GetSystem();
	ConstBuffer& time_buf = *bufs[0][0][4];
	Time t = Time(1970,1,1) + time_buf.Top();
	int wday = DayOfWeek(t);
	int slot_id = wday * 24 + t.hour;
	return slot_id;
}
	
const FastStatSlot& FastEventStatistics::GetLatestSlot(int net, int i) {
	System& sys = GetSystem();
	ConstBuffer& time_buf = *bufs[0][0][4];
	Time t = Time(1970,1,1) + time_buf.Top();
	int wday = DayOfWeek(t);
	int slot_id = wday * 24 + t.hour;
	return this->stats[net][i][slot_id];
}
	
const FastStatSlot& FastEventStatistics::GetSlot(int net, int i, int wb) {
	return this->stats[net][i][wb];
}




















FastEventStatisticsCtrl::FastEventStatisticsCtrl() {
	Add(symlist.TopPos(0,30).LeftPos(0,100));
	Add(steplist.TopPos(30,30).LeftPos(0,100));
	Add(list.VSizePos().HSizePos(100));
	
	
	System& sys = GetSystem();
	for(int i = 0; i < sys.GetNetCount(); i++)
		symlist.Add(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	symlist.SetIndex(0);
	symlist <<= THISBACK(Data);
	
	for(int i = 0; i < MAX_FAST_LEN; i++)
		steplist.Add(i);
	steplist.SetIndex(0);
	steplist <<= THISBACK(Data);
	
	String colw;
	
	list.AddColumn("Latest time");
	colw += "3 ";
	for(int i = 0; i < 3; i++) {
		list.AddColumn(IntStr(i + 1) + ". description");
		list.AddColumn(IntStr(i + 1) + ". average");
		list.AddColumn(IntStr(i + 1) + ". class");
		colw += "3 2 1 ";
	}
	list.ColumnWidths(colw);
	
	
}
	
void FastEventStatisticsCtrl::Data() {
	System& sys = GetSystem();
	FastEventStatistics& es = GetFastEventStatistics();
	int width = FAST_WIDTH;
	int height = FastEventStatistics::SRC_COUNT;
	int sym = symlist.GetIndex();
	int step = steplist.GetIndex();
	
	if (es.stats.IsEmpty())
		return;
	
	
	ConstBuffer& time_buf = es.db[sym]->GetBuffer(4);
	int limit = time_buf.GetCount() % width;
	
	VectorMap<int, double> stats;
	for(int i = 0; i < width; i++) {
		
		stats.Clear();
		for(int j = 1; j < height; j++) {
			{
				OnlineVariance& av = es.stats[sym][j][i].av[step];
				double mean = av.GetMean();
				double cdf = av.GetCDF(0, 0);
				if (cdf < 0.5) cdf = 1.0 - cdf;
				int grade = (1.0 - cdf) / 0.05;
				if (grade < FastEventOptimization::grade_count && mean > 0)
					stats.Add(j, mean);
			}
			{
				OnlineVariance& av = es.stats[sym][j][i].inv_av[step];
				double mean = av.GetMean();
				double cdf = av.GetCDF(0, 0);
				if (cdf < 0.5) cdf = 1.0 - cdf;
				int grade = (1.0 - cdf) / 0.05;
				if (grade < FastEventOptimization::grade_count && mean > 0)
					stats.Add(-j-1, mean);
			}
		}
		SortByValue(stats, StdGreater<double>());
		
		int col = 0;
		list.Set(i, col++, i);
		
		for(int j = 0; j < 3 && j < stats.GetCount(); j++) {
			int k = stats.GetKey(j);
			int l = k >= 0 ? k : -k-1;
			String desc = es.GetDescription(l);
			double v = stats[j];
			double cdf;
			if (k >= 0) {
				cdf = es.stats[sym][l][i].av[step].GetCDF(0, true);
			} else {
				cdf = es.stats[sym][l][i].inv_av[step].GetCDF(0, true);
				desc += " inverse";
			}
			int grade = 'A' + (1.0 - cdf) / 0.05;
			String grade_str;
			grade_str.Cat(grade);
			list.Set(i, col++, desc);
			list.Set(i, col++, v);
			list.Set(i, col++, grade_str);
		}
		
		for(int j = stats.GetCount(); j < 3; j++) {
			list.Set(i, col++, "");
			list.Set(i, col++, "");
			list.Set(i, col++, "");
		}
	}
	
}

}