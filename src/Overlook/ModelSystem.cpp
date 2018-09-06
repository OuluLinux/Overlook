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
		
		int mult = (is_signal ? -1 : +1) * (is_inverse ? -1 : +1);
		
		double o0 = open_buf.Get(cursor);
		double o1 = open_buf.Get(cursor-1);
		double gain = (o0 / o1 - 1.0) * mult - spread_factor;
		account_gain *= 1.0 + gain;
		
		LOG(id << " " << cursor << " " << account_gain);
	}
	
	
	temp.net = -1;
	temp.mean = 0;
	
	
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
			
			
			
			
			// Check for open pattern
			for(int j = primary_pattern_id; j >= 0; j--) {
				int pattern_begin = cursor - j;
				if (pattern_begin < 1)
					continue;
				bool enabled = lbl.enabled.Get(pattern_begin);
				bool prev_enabled = lbl.enabled.Get(pattern_begin-1);
				bool signal = lbl.signal.Get(pattern_begin);
				if (signal != is_signal)
					continue;
				
				if (enabled && !prev_enabled) {
					bool is_pattern = true;
					for (int check_cursor = pattern_begin+1; check_cursor <= cursor; check_cursor++) {
						int pos0 = check_cursor;
						int pos1 = pos0 - 1;
						if (pos1 < 0) {
							is_pattern = false;
							break;
						}
						double o0 = open_buf.Get(pos0);
						double o1 = open_buf.Get(pos1);
						double diff = o0 - o1;
						if (signal)		diff *= -1;
						if (is_inverse) diff *= -1;
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
						}
						
						break;
					}
				}
			}
		}
	}
	
	
	/*if (temp.net >= 0) {
		LOG(id << " " << cursor << " " << temp.net << " " << temp.src << " " << temp.mean);
	}*/
	
	cursor++;
	return true;
}










Model::Model() {
	
}

void Model::Init() {
	cores.Init();
	
	int id = 0;
	for (int event_count = 3; event_count < 4; event_count++) {
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
	bool tick_used = false;
	for(int i = 0; i < settings.GetCount(); i++) {
		tick_used |= settings[i].Tick(msys, *this);
	}
	return tick_used;
}






ModelSystem::ModelSystem() {
	
}

void ModelSystem::Init() {
	System& sys = GetSystem();
	
	
	// Model: Bollinger Bands
	Model& bb = models.Add();
	bb.cores.AddIndi(sys.Find<DataBridge>());
	#ifdef flagDEBUG
	for (int period = 20; period <= 90; period += 10)
		for (int dev = 10; dev <= 20; dev+=10)
			bb.cores.AddIndi(sys.Find<BollingerBands>()).AddArg(period).AddArg(0).AddArg(dev);
	#else
	for (int period = 10; period <= 100; period += 10)
		for (int dev = 5; dev <= 20; dev++)
			bb.cores.AddIndi(sys.Find<BollingerBands>()).AddArg(period).AddArg(0).AddArg(dev);
	#endif
	for(int i = 0; i < sys.GetNetCount(); i++)
		bb.cores.AddSymbol(sys.GetSymbol(sys.GetNormalSymbolCount() + sys.GetCurrencyCount() + i));
	bb.cores.AddTf(4); // H1
	bb.Init();
	
	
}

void ModelSystem::Start() {
	while (true) {
		bool tick_used = false;
		for(int i = 0; i < models.GetCount(); i++)
			tick_used |= models[i].Tick(*this);
		if (!tick_used) break;
	}
}




ModelSystemCtrl::ModelSystemCtrl() {
	
}
	
void ModelSystemCtrl::Data() {
	
}

}
