#include "DataCore.h"

namespace DataCore {

SpreadStats::SpreadStats() {
	AddValue<double>("Average Spread");
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":3,"
			"}"
		"}"
	);
}

void SpreadStats::SetArguments(const VectorMap<String, Value>& args) {
	
}

void SpreadStats::Init() {
	TimeVector& tv = GetTimeVector();
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int total = sym_count * tf_count;
	data.SetCount(total);
	
	for(int i = 0; i < total; i++) {
		int sym = i / tf_count;
		int tf = i % tf_count;
		SymTf& s = data[i];
		
		int period = tv.GetPeriod(tf) * tv.GetBasePeriod() / 60; // minutes, not seconds
		bool force_d0 = period >= 7*24*60;
		s.stats.SetCount(force_d0 ? 1 : 5*24*60 / period);
		s.has_stats = false;
	}
}

bool SpreadStats::Process(const SlotProcessAttributes& attr) {
	TimeVector& tv = GetTimeVector();
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	SymTf& s = data[attr.sym_id * tf_count + attr.tf_id];
	
	int period = tv.GetPeriod(attr.tf_id) * tv.GetBasePeriod() / 60; // minutes, not seconds
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
		
	if (!s.has_stats) {
		
		// Open askbid-file
		String local_askbid_file = ConfigFile("askbid.bin");
		FileIn src(local_askbid_file);
		ASSERTEXC_(src.IsOpen() && src.GetSize(), "DataBridge should have downloaded askbid.bin");
		int data_size = src.GetSize();
		int cursor = 0;
		
		src.Seek(cursor);
		
		int struct_size = 4 + 4 + 8 + 8;
		
		while ((cursor + struct_size) <= data_size) {
			
			int timestamp, askbid_id;
			double ask, bid;
			src.Get(&timestamp, 4);
			src.Get(&askbid_id, 4);
			src.Get(&ask, 8);
			src.Get(&bid, 8);
			cursor += struct_size;
			
			if (attr.sym_id != askbid_id) continue;
			
			Time time = TimeFromTimestamp(timestamp);
			int h = (time.minute + time.hour * 60) / period;
			int d = DayOfWeek(time) - 1;
			int dh = h + d * h_count;
			
			if (force_d0) {
				h = 0;
				d = 0;
				dh = 0;
			}
			// Skip weekend
			else if (d == -1 || d == 5) {
				continue;
			}
			
			OnlineVariance& var = s.stats[dh];
			
			double spread = ask - bid;
			if (spread > 0.0) // must be technically realistic
				var.AddResult(spread);
		}
		
		s.has_stats = true;
	}
	
	// Get value
	Time time = tv.GetTime(attr.GetPeriod(), attr.GetCounted());
	int h = (time.minute + time.hour * 60) / period;
	int d = DayOfWeek(time) - 1;
	int dh = h + d * h_count;
	if (force_d0 || d == -1 || d == 5) {
		h = 0;
		d = 0;
		dh = 0;
	}
	OnlineVariance& var = s.stats[dh];
	double mean_spread = var.GetMean();
	
	// Write value
	double* out = GetValue<double>(0, attr);
	*out = mean_spread;
	
	return true;
}










ValueChange::ValueChange() {
	AddValue<double>("Difference");
	AddValue<double>("Increase");
	AddValue<double>("Decrease");
	AddValue<double>("Best Increase");
	AddValue<double>("Best Decrease");
	AddValue<double>("Worst Increase");
	AddValue<double>("Worst Decrease");
	AddValue<double>("Value Change");
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"128,128,128\","
				"\"line_width\":4,"
			"},"
			"\"value1\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":3,"
			"},"
			"\"value2\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":3,"
			"},"
			"\"value3\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value4\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":2,"
			"},"
			"\"value5\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":2,"
			"},"
			"\"value6\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value7\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value8\":{"
				"\"color\":\"255,255,255\","
				"\"line_width\":1,"
			"},"
		"}"
	);
	
}

void ValueChange::SetArguments(const VectorMap<String, Value>& args) {
	
}

void ValueChange::Init() {
	TimeVector& tv = GetTimeVector();
	
	src = FindLinkSlot("/open");
	ASSERTEXC(src);
	
	spread = FindLinkSlot("/spread");
	ASSERTEXC(spread);
	
	db = dynamic_cast<DataBridge*>(&*src);
	ASSERTEXC(db);
	
	int sym_count = tv.GetSymbolCount();
	data.SetCount(sym_count);
	
	for(int i = 0; i < sym_count; i++) {
		Sym& s = data[i];
		const Symbol& sym = db->GetSymbol(i);
		
		s.has_proxy = sym.proxy_id != -1;
		s.proxy_id = sym.proxy_id;
		s.proxy_factor = sym.proxy_factor;
	}
}

bool ValueChange::Process(const SlotProcessAttributes& attr) {
	double* diff_		= GetValue<double>(0, attr);
	double* inc			= GetValue<double>(1, attr);
	double* dec			= GetValue<double>(2, attr);
	double* best_inc	= GetValue<double>(3, attr);
	double* best_dec	= GetValue<double>(4, attr);
	double* worst_inc	= GetValue<double>(5, attr);
	double* worst_dec	= GetValue<double>(6, attr);
	double* value_diff	= GetValue<double>(7, attr);
	double* open		= src->GetValue<double>(0, 0, attr);
	double* close		= src->GetValue<double>(0, -1, attr);
	double* low			= src->GetValue<double>(1, 0, attr);
	double* high		= src->GetValue<double>(2, 0, attr);
	double* spread		= this->spread->GetValue<double>(0, attr);

	int id = attr.sym_id;
	
	Sym& s = data[id];
	
	if (!s.has_proxy) {
		double cost = *spread;
		double open_value = *open;
		double diff = *close - open_value;
		*inc = diff - cost;
		*dec = -diff - cost;
		double high_diff = *high - open_value;
		double low_diff  = *low - open_value;
		ASSERT(high_diff >= low_diff);
		*best_inc = high_diff - cost;
		*best_dec = -low_diff - cost;
		*worst_inc = low_diff - cost;
		*worst_dec = -high_diff - cost;
		*value_diff = diff;
		*diff_ = diff; // same without proxy
	} else {
		double* proxy_spread = this->spread->GetValue<double>(0, s.proxy_id, attr.tf_id, 0, attr);
		
		// Get buffers
		double* proxy_open	= src->GetValue<double>(0, s.proxy_id, attr.tf_id, 0, attr);
		double* proxy_close	= src->GetValue<double>(0, s.proxy_id, attr.tf_id, 1, attr);
		double* proxy_low	= src->GetValue<double>(1, s.proxy_id, attr.tf_id, 0, attr);
		double* proxy_high	= src->GetValue<double>(2, s.proxy_id, attr.tf_id, 0, attr);
		
		double cost = *spread + *proxy_spread;
		
		// Proxy and normal.
		double proxy_open_value = *proxy_open;
		double open_value = *open;
		double proxy_diff, proxy_high_diff, proxy_low_diff;
		if (s.proxy_factor == 1) {
			proxy_diff		= *proxy_close - proxy_open_value;
			proxy_high_diff	= *proxy_high - proxy_open_value;
			proxy_low_diff	= *proxy_low - proxy_open_value;
		} else {
			proxy_diff		= proxy_open_value - *proxy_close;
			proxy_low_diff	= proxy_open_value - *proxy_high;
			proxy_high_diff	= proxy_open_value - *proxy_low;
		}
		double diff = *proxy_close - open_value;
		*inc	= diff + proxy_diff - cost;
		*dec	= -diff - proxy_diff - cost;
		double high_diff	= *high - open_value;
		double low_diff	= *low - open_value;
		ASSERT(high_diff >= low_diff);
		ASSERT(proxy_high_diff >= proxy_low_diff);
		*best_inc	= proxy_high_diff + high_diff - cost;
		*best_dec	= -proxy_low_diff  - low_diff  - cost;
		*worst_inc	= proxy_low_diff + low_diff - cost;
		*worst_dec	= -proxy_high_diff - high_diff - cost;
		*value_diff	= diff + proxy_diff;
		*diff_ = diff; // different with proxy
	}
	
	return true;
}

}
