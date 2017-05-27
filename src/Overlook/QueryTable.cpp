#include "Overlook.h"

namespace Overlook {

QueryTable::QueryTable() {
	bytes = 0;
	bits = 0;
	target_count = 0;
}

int QueryTable::GetLargestInfoGainPredictor(int target_id) {
	gains.Clear();
	
	ASSERT(!columns.IsEmpty());
	ASSERT(!data.IsEmpty());
	
	VectorMap<int, int> target_count;
	for(int i = 0; i < data.GetCount(); i++) {
		int t = Get(i, target_id);
		int j = target_count.Find(t);
		if (j == -1) target_count.Add(t, 1);
		else target_count[j]++;
	}
	
	double total = 0;
	for(int i = 0; i < target_count.GetCount(); i++)
		total += target_count[i];
	
	double entropy = 0;
	for(int i = 0; i < target_count.GetCount(); i++)  {
		double part = target_count[i] * 1.0 / total;
		entropy -= part * log2((double)part);
	}
	
	for(int i = this->target_count; i < columns.GetCount(); i++) {
		
		VectorMap<int, VectorMap<int, int> > pred_counts;
		
		double pred_total = 0;
		for(int j = 0; j < data.GetCount(); j++) {
			int c = Get(j, i);
			int k = pred_counts.Find(c);
			if (k == -1) {k = pred_counts.GetCount(); pred_counts.Add(c);}
			VectorMap<int, int>& pred_target_count = pred_counts[k];
			int t = Get(j, target_id);
			k = pred_target_count.Find(t);
			if (k == -1) pred_target_count.Add(t, 1);
			else pred_target_count[k]++;
			pred_total += 1;
		}
		
		double pred_entropy = 0;
		for(int j = 0; j < pred_counts.GetCount(); j++) {
			VectorMap<int, int>& pred_target_count = pred_counts[j];
			int pred_cat_total = 0;
			for(int k = 0; k < pred_target_count.GetCount(); k++)
				pred_cat_total += pred_target_count[k];
			double pred_cat_entropy = 0;
			for(int k = 0; k < pred_target_count.GetCount(); k++) {
				double part = pred_target_count[k] * 1.0 / pred_cat_total;
				pred_cat_entropy -= part * log2((double)part);
			}
			
			pred_entropy += (double)pred_cat_total / pred_total * pred_cat_entropy;
			
		}
		
		double gain = entropy - pred_entropy;
		gains.Add(gain);
	}
	
	double max_gain = -DBL_MAX;
	int max_id = -1;
	for(int i = 0; i < gains.GetCount(); i++) {
		if (gains[i] > max_gain) {
			max_gain = gains[i];
			max_id = i;
		}
	}
	
	return max_id;
}

inline int MaxBits(uint32 max_value) {
	for(int i = 0; i < 32; i++) {
		uint32 lim = pow(2, i);
		if (lim >= max_value)
			return i;
	}
	Panic("Invalid value");
}















QueryTableForecaster::QueryTableForecaster() {
	
}

void QueryTableForecaster::Init() {
	
	// Add targets
	for(int i = 0; i < 6; i++) {
		int len = pow(2, i);
		String change = IntStr(len) + "-Change ";
		for(int j = 0; j < 6; j++) {
			int bits = pow(2, j);
			String desc = change + IntStr(bits) + "-Step";
			qt.AddColumn(desc, bits);
		}
	}
	qt.EndTargets();
	
	// Add constants columns
	qt.AddColumn("Month",		MaxBits(12));
	qt.AddColumn("Day",			MaxBits(31));
	qt.AddColumn("Wday",		MaxBits(7));
	qt.AddColumn("WdayHour",	MaxBits(7*24));
	qt.AddColumn("Hour",		MaxBits(24));
	qt.AddColumn("5-min",		MaxBits(12));
	
	// Add columns from inputs
	for(int i = 0; i < optional_inputs.GetCount(); i++) {
		Input& indi_input = optional_inputs[i];
		if (indi_input.sources.IsEmpty()) continue;
		ASSERT(indi_input.sources.GetCount() == 1);
		
		Panic("TODO");
	}
}

void QueryTableForecaster::Start() {
	int bars = GetBars();
	int counted = GetCounted();
	int tf = GetTf();
	BaseSystem& bs = GetBaseSystem();
	ValueChange& bc = *Get<ValueChange>();
	double max_change = bc.GetMax();
	double min_change = bc.GetMin();
	double diff = max_change - min_change;
	
	bars -= 32;
	
	qt.SetCount(bars);
	
	TimeStop ts;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+32);
		
		Time t = bs.GetTimeTf(tf, i);
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		double open = Open(i);
		
		int pos = 0;
		for(int j = 0; j < 6; j++) {
			int len = pow(2, j);
			int read_pos = i + len;
			double close = Open(read_pos);
			double change = open != 0.0 ? close / open - 1.0 : 0.0;
			
			for(int k = 0; k < 6; k++) {
				int div = pow(2, k + 1);
				double step = diff / div;
				int v = (change - min_change) / step;
				if (v < 0) v = 0;
				
				qt.Set(i, pos++, v);
			}
		}
		
		qt.Set(i, pos++, t.month);
		qt.Set(i, pos++, t.day);
		qt.Set(i, pos++, dow);
		qt.Set(i, pos++, wdayhour);
		qt.Set(i, pos++, t.hour);
		qt.Set(i, pos++, t.minute / 5);
	}
	
	LOG(ts.ToString());
	
	int col = qt.GetLargestInfoGainPredictor(0);
	DUMP(col);
	DUMPC(qt.GetInfoGains());
}















QueryTableHugeForecaster::QueryTableHugeForecaster() {
	
}

void QueryTableHugeForecaster::Init() {
	
}

void QueryTableHugeForecaster::Start() {
	
	Panic("TODO");
}


QueryTableMetaForecaster::QueryTableMetaForecaster() {
	
}

void QueryTableMetaForecaster::Init() {
	
}

void QueryTableMetaForecaster::Start() {
	
	Panic("TODO");
}


QueryTableAgent::QueryTableAgent() {
	
}

void QueryTableAgent::Init() {
	
}

void QueryTableAgent::Start() {
	
	Panic("TODO");
}


QueryTableHugeAgent::QueryTableHugeAgent() {
	
}

void QueryTableHugeAgent::Init() {
	
}

void QueryTableHugeAgent::Start() {
	
	Panic("TODO");
}


QueryTableMetaAgent::QueryTableMetaAgent() {
	
}

void QueryTableMetaAgent::Init() {
	
}

void QueryTableMetaAgent::Start() {
	
	Panic("TODO");
}


QueryTableDoubleAgent::QueryTableDoubleAgent() {
	
}

void QueryTableDoubleAgent::Init() {
	
}

void QueryTableDoubleAgent::Start() {
	
	Panic("TODO");
}

}
