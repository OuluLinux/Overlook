#include "Overlook.h"

namespace Overlook {

ModelSetting::ModelSetting() {
	
}

void ModelSetting::Init(Model& m) {
	int width = 24*7;
	
	stats.SetCount(m.cores.GetSymbolCount());
	for(int j = 0; j < stats.GetCount(); j++) {
		stats[j].SetCount(m.cores.GetIndiCount());
		for(int i = 0; i < stats[j].GetCount(); i++) {
			stats[j][i].SetCount(width);
			for(int k = 0; k < stats[j][i].GetCount(); k++) {
				stats[j][i][k].SetPeriod(event_count);
			}
		}
	}
	
	switch (secondary_pattern_id) {
		case 0: secondary_pattern = "0"; break;
		case 1: secondary_pattern = "01"; break;
		case 2: secondary_pattern = "001"; break;
		case 3: secondary_pattern = "011"; break;
	}
}

bool ModelSetting::Tick(ModelSystem& msys, Model& m) {
	
	bool all_usable = true;
	for(int i = 0; i < m.cores.GetSymbolCount() && all_usable; i++) {
		ConstBuffer& open_buf = m.cores.GetBuffer(i, 0, 0);
		if (cursor >= open_buf.GetCount())
			all_usable = false;
	}
	
	if (!all_usable)
		return false;
	
	if (cursor <= 0) {
		cursor++;
		return true;
	}
	
	
	if (temp.net >= 0) {
		ConstBuffer& open_buf = m.cores.GetBuffer(temp.net, 0, 0);
		ConstBuffer& time_buf = m.cores.GetBuffer(temp.net, 0, 4);
		
		int mult = (is_signal ? -1 : +1) * (is_inverse ? -1 : +1) * (temp.is_secondary_inverse ? -1 : +1);
		
		double o0 = open_buf.Get(cursor);
		double o1 = open_buf.Get(cursor-1);
		double gain = (o0 / o1 - 1.0) * mult - spread_factor;
		account_gain *= 1.0 + gain;
		
		//LOG(id << " " << cursor << " " << account_gain);
		
		temp.len++;
	}
	
	
	for (int sym = 0; sym < m.cores.GetSymbolCount(); sym++) {
		ConstBuffer& open_buf = m.cores.GetBuffer(sym, 0, 0);
		ConstBuffer& time_buf = m.cores.GetBuffer(sym, 0, 4);
		
		for(int i = 1; i < m.cores.GetIndiCount(); i++) {
			ConstLabelSignal& lbl = m.cores.GetLabelSignal(sym, i);
			
			// Check for closing pattern for statistics gathering
			{
				int pattern_begin = cursor - 1 - primary_pattern_id - 1;
				if (pattern_begin < 1)
					continue;
				bool enabled = lbl.enabled.Get(pattern_begin);
				bool prev_enabled = lbl.enabled.Get(pattern_begin-1);
				bool signal = lbl.signal.Get(pattern_begin);
				if (signal != is_signal)
					continue;
				
				if (enabled && !prev_enabled) {
					
					double o0 = open_buf.Get(cursor);
					double o1 = open_buf.Get(cursor-1);
					double diff = o0 - o1;
					if (signal) diff *= -1;
					
					bool check_pattern = false;
					if (!is_inverse && diff < 0)
						check_pattern = true;
					else if (is_inverse && diff > 0)
						check_pattern = true;
					
					
					if (check_pattern) {
						
						bool is_pattern = true;
						for(int j = 0; j <= primary_pattern_id; j++) {
							int pos0 = cursor - 1 - j;
							int pos1 = pos0 - 1;
							if (pos1 < 0) {
								is_pattern = false;
								break;
							}
							double o0 = open_buf.Get(pos0);
							double o1 = open_buf.Get(pos1);
							double diff = o0 - o1;
							if (signal)		diff *= -1;
							if (is_inverse)	diff *= -1;
							if (diff < 0) {
								is_pattern = false;
								break;
							}
						}
						
						if (is_pattern) {
							Time t = Time(1970,1,1) + time_buf.Get(pattern_begin);
							int wday = DayOfWeek(t);
							int slot = wday * 24 + t.hour;
							StatSlot& stat = stats[sym][i][slot];
							
							
							double o = open_buf.Get(pattern_begin);
							double gain = o0 / o - 1;
							if (signal)		gain *= -1;
							if (is_inverse)	gain *= -1;
							
							stat.av.Add(gain);
							stat.abs_av.Add(fabs(gain));
						}
					}
				}
			}
		}
	}
	
	
	if (temp.net >= 0 && temp.len < secondary_pattern.GetCount()) {
		temp.is_secondary_inverse = secondary_pattern[temp.len] == '1';
	}
	else {
		temp.net = -1;
		temp.mean = 0;
	}
	
	for (int sym = 0; sym < m.cores.GetSymbolCount(); sym++) {
		ConstBuffer& open_buf = m.cores.GetBuffer(sym, 0, 0);
		ConstBuffer& time_buf = m.cores.GetBuffer(sym, 0, 4);
		
		for(int i = 1; i < m.cores.GetIndiCount(); i++) {
			ConstLabelSignal& lbl = m.cores.GetLabelSignal(sym, i);
			
			// Check for open pattern
			bool enabled = lbl.enabled.Get(cursor);
			bool prev_enabled = lbl.enabled.Get(cursor-1);
			bool signal = lbl.signal.Get(cursor);
			if (signal == is_signal && enabled && !prev_enabled) {
				Time t = Time(1970,1,1) + time_buf.Get(cursor);
				int wday = DayOfWeek(t);
				int slot = wday * 24 + t.hour;
				StatSlot& stat = stats[sym][i][slot];
				
				int event_count = stat.av.Get().GetEventCount();
				if (event_count < this->event_count)
					continue;
				
				double cdf = stat.av.Get().GetCDF(0.0, true);
				int grade = (1.0 - cdf) / grade_div;
				if (grade >= grade_count)
					continue;
				
				double abs_cdf = stat.abs_av.Get().GetCDF(spread_factor, true);
				int abs_grade = (1.0 - abs_cdf) / grade_div;
				if (abs_grade >= grade_count)
					continue;
				
				double mean = stat.av.Get().GetMean();
				
				if (mean > temp.mean) {
					temp.mean = mean;
					temp.net = sym;
					temp.src = i;
					temp.grade = grade;
					temp.abs_grade = abs_grade;
					temp.cdf = cdf;
					temp.len = 0;
					temp.is_secondary_inverse = secondary_pattern[0] == '1';
				}
			}
		}
	}
	
	
	history.Add(account_gain);
	
	cursor++;
	return true;
}










Model::Model() {
	
}

void Model::Init() {
	cores.Init();
	
	int id = 0;
	for (int event_count = 3; event_count < 6; event_count+=2) {
		for(int i = 0; i < ModelSetting::primary_pattern_count; i++) {
			for(int j = 0; j < ModelSetting::secondary_pattern_count; j++) {
				for (int is_inverse = 0; is_inverse < 2; is_inverse++) {
					for (int is_signal = 0; is_signal < 2; is_signal++) {
						ModelSetting& set = settings.Add();
						
						set.event_count = event_count;
						set.primary_pattern_id = i;
						set.secondary_pattern_id = j;
						set.is_inverse = is_inverse;
						set.is_signal = is_signal;
						set.id = id++;
						set.Init(*this);
					}
				}
			}
		}
	}
}

bool Model::Tick(ModelSystem& msys) {
	cores.Refresh();
	
	double sig_gain = 1.0;
	int sig_net = -1, sig = 0;
	
	bool tick_used = false;
	for(int i = 0; i < settings.GetCount(); i++) {
		ModelSetting& ms = settings[i];
		bool used = ms.Tick(msys, *this);
		tick_used |= used;
		
		int net = ms.GetSignalNet();
		if (net >= 0) {
			double gain = ms.GetAccountGain();
			if (gain > sig_gain) {
				sig_net = net;
				sig = ms.GetSignal();
				sig_gain = gain;
			}
		}
	}
	
	cur_sig = sig;
	cur_sig_net = sig_net;
	cur_gain = sig_gain;
	
	return tick_used;
}






ModelSystem::ModelSystem() {
	
}

void ModelSystem::Init() {
	System& sys = GetSystem();
	
	
	// Model: Bollinger Bands
	Model& bb = models.Add();
	bb.cores.AddIndi(sys.Find<DataBridge>());
	for (int period = 20; period <= 100; period += 10)
		for (int dev = 10; dev <= 20; dev+=5)
			bb.cores.AddIndi(sys.Find<BollingerBands>()).AddArg(period).AddArg(0).AddArg(dev);
	for(int i = 0; i < sys.GetNetCount(); i++)
		bb.cores.AddSymbol(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	bb.cores.AddTf(4); // H1
	bb.Init();
	
	
	// Model: Anomaly
	Model& anomaly = models.Add();
	anomaly.cores.AddIndi(sys.Find<DataBridge>());
	anomaly.cores.AddIndi(sys.Find<Anomaly>()).AddArg(0);
	anomaly.cores.AddIndi(sys.Find<Anomaly>()).AddArg(1);
	for(int i = 0; i < sys.GetNetCount(); i++)
		anomaly.cores.AddSymbol(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	anomaly.cores.AddTf(4); // H1
	anomaly.Init();
	
}

void ModelSystem::Start() {
	System& sys = GetSystem();
	
	int ticks = 0;
	
	while (true) {
		bool tick_used = false;
		for(int i = 0; i < models.GetCount(); i++) {
			bool used = models[i].Tick(*this);
			tick_used |= used;
			if (used) ticks++;
		}
		if (!tick_used) break;
	}
	
	if (ticks) {
		
		// Automation
		double max_gain = 0;
		int max_gain_i = -1, max_gain_sig = 0, max_gain_net = -1;
		for(int i = 0; i < models.GetCount(); i++) {
			Model& m = models[i];
			if (m.cur_gain > max_gain && m.cur_sig) {
				max_gain = m.cur_gain;
				max_gain_sig = m.cur_sig;
				max_gain_net = m.cur_sig_net;
				max_gain_i = i;
			}
		}
		
		Sentiment& sent = GetSentiment();
		SentimentSnapshot& ss = sent.AddSentiment();
		ss.added = GetUtcTime();
		ss.comment = "Clear";
		ss.cur_pres.SetCount(sys.GetCurrencyCount(), 0);
		ss.pair_pres.SetCount(sys.GetSymbolCount(), 0);
		ss.fmlevel = 1.0;
		if (max_gain_sig) {
			ss.comment = Format("Active %d %d %d", max_gain_i, max_gain_net, max_gain_sig);
			ss.fmlevel = 0.6;
			ss.tplimit = 0.1;
			
			if (Config::fixed_tplimit)	ss.tplimit = TPLIMIT;
			
			System::NetSetting& net = sys.GetNet(max_gain_net);
			
			for(int j = 0; j < sent.GetSymbolCount(); j++) {
				String sym = sent.GetSymbol(j);
				int k = net.symbols.Find(sym);
				if (k == -1) continue;
				ss.pair_pres[j] = max_gain_sig * net.symbols[k];
			}
		}
		sent.StoreThis();
	}
}




ModelSystemCtrl::ModelSystemCtrl() {
	System& sys = GetSystem();
	ModelSystem& ms = GetModelSystem();
	
	Add(modellist.TopPos(0,30).LeftPos(0,100));
	Add(mslist.TopPos(30,30).LeftPos(0,100));
	Add(symlist.TopPos(60,30).LeftPos(0,100));
	Add(valuelist.VSizePos().LeftPos(100, 200));
	Add(list.VSizePos().HSizePos(300, 300));
	Add(drawer.VSizePos().RightPos(0, 300));
	
	for(int i = 0; i < ms.GetModelCount(); i++)
		modellist.Add("Model #" + IntStr(i));
	modellist.SetIndex(0);
	modellist <<= THISBACK(Data);
	
	const Model& m = ms.GetModel(0);
	for(int i = 0; i < m.GetSettingCount(); i++)
		mslist.Add(IntStr(i));
	mslist.SetIndex(0);
	mslist <<= THISBACK(Data);
	
	for(int i = 0; i < sys.GetNetCount(); i++)
		symlist.Add(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	symlist.SetIndex(0);
	symlist <<= THISBACK(Data);
	
	String colw;
	
	valuelist.AddColumn("Key");
	valuelist.AddColumn("Value");
	
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
	
void ModelSystemCtrl::Data() {
	System& sys = GetSystem();
	ModelSystem& msys = GetModelSystem();
	
	int width = 24 * 7;
	
	int sym = symlist.GetIndex();
	int msi = mslist.GetIndex();
	int model = modellist.GetIndex();
	
	drawer.msi = msi;
	drawer.model = model;
	drawer.Refresh();
	
	const Model& m = msys.GetModel(model);
	const ModelSetting& ms = m.GetSetting(msi);
	
	valuelist.Set(0, 0, "is_inverse");
	valuelist.Set(0, 1, ms.is_inverse);
	valuelist.Set(1, 0, "is_signal");
	valuelist.Set(1, 1, ms.is_signal);
	valuelist.Set(2, 0, "event_count");
	valuelist.Set(2, 1, ms.event_count);
	valuelist.Set(3, 0, "primary_pattern_id");
	valuelist.Set(3, 1, ms.primary_pattern_id);
	valuelist.Set(4, 0, "secondary_pattern");
	valuelist.Set(4, 1, ms.secondary_pattern);
	valuelist.Set(5, 0, "account_gain");
	valuelist.Set(5, 1, ms.account_gain);
	valuelist.Set(6, 0, "signal");
	valuelist.Set(6, 1, ms.GetSignal());
	valuelist.Set(7, 0, "signal net");
	valuelist.Set(7, 1, ms.GetSignalNet());
	valuelist.Set(8, 0, "cursor");
	valuelist.Set(8, 1, ms.cursor);
	valuelist.Set(9, 0, "cur_gain");
	valuelist.Set(9, 1, m.cur_gain);
	valuelist.Set(10, 0, "cur_sig");
	valuelist.Set(10, 1, m.cur_sig);
	valuelist.Set(11, 0, "cur_sig_net");
	valuelist.Set(11, 1, m.cur_sig_net);
	
	
	VectorMap<int, double> stats;
	for(int i = 0; i < width; i++) {
		
		stats.Clear();
		for(int j = 1; j < m.cores.GetIndiCount(); j++) {
			const OnlineVariance& av = ms.stats[sym][j][i].av.Get();
			double mean = av.GetMean();
			double cdf = av.GetCDF(0, 0);
			if (cdf < 0.5) cdf = 1.0 - cdf;
			int grade = (1.0 - cdf) / ms.grade_div;
			if (grade < ms.grade_count && mean > 0)
				stats.Add(j, mean);
		}
		SortByValue(stats, StdGreater<double>());
		
		int col = 0;
		list.Set(i, col++, i);
		
		for(int j = 0; j < 3 && j < stats.GetCount(); j++) {
			int k = stats.GetKey(j);
			int l = k >= 0 ? k : -k-1;
			String desc = m.cores.GetIndiDescription(l);
			if (ms.is_inverse)
				desc += " inverse";
			double v = stats[j];
			double cdf = ms.stats[sym][l][i].av.Get().GetCDF(0, true);
			int grade = 'A' + (1.0 - cdf) / ms.grade_div;
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
