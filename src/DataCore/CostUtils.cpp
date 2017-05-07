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

void SpreadStats::SerializeCache(Stream& s, int sym_id, int tf_id) {
	TimeVector& tv = GetTimeVector();
	int tf_count = tv.GetPeriodCount();
	int i = sym_id * tf_count + tf_id;
	SymTf& symtf = data[i];
	s % symtf;
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
			
			double spread = ask / bid - 1.0;
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
	AddValue<double>("Change");
	AddValue<double>("Change With Proxy");
	AddValue<double>("Low Change With Proxy");
	AddValue<double>("High Change With Proxy");
	AddValue<double>("Spread Change");
	
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
	AddDependency("/open", 1, 0);
	AddDependency("/spread", 1, 0);
	
	TimeVector& tv = GetTimeVector();
	SlotPtr src = tv.FindLinkSlot("/open");
	db = dynamic_cast<DataBridge*>(&*src);
	ASSERTEXC(db);
	
	int sym_count = tv.GetSymbolCount();
	data.SetCount(sym_count);
	
	for(int i = 0; i < sym_count; i++) {
		Sym& s = data[i];
		if (i < db->GetSymbolCount()) {
			const Symbol& sym = db->GetSymbol(i);
			s.has_proxy = sym.proxy_id != -1;
			s.proxy_id = sym.proxy_id;
			s.proxy_factor = sym.proxy_factor;
			ASSERTEXC(!s.has_proxy || s.proxy_factor != 0);
		} else {
			s.has_proxy = false;
			s.proxy_id = -1;
			s.proxy_factor = 0;
		}
	}
}

bool ValueChange::Process(const SlotProcessAttributes& attr) {
	const Slot& src = GetDependency(0);
	const Slot& spreadsrc = GetDependency(1);
	double* change_			= GetValue<double>(0, attr);
	double* value_change	= GetValue<double>(1, attr);
	double* low_change		= GetValue<double>(2, attr);
	double* high_change		= GetValue<double>(3, attr);
	double* spread_change	= GetValue<double>(4, attr);
	double* open			= src.GetValue<double>(0, 1, attr);
	double* close			= src.GetValue<double>(0, 0, attr);
	double* low				= src.GetValue<double>(1, 1, attr);
	double* high			= src.GetValue<double>(2, 1, attr);
	double* spread			= spreadsrc.GetValue<double>(0, 0, attr);
	
	if (!open || *open == 0.0) return false;
	
	int id = attr.sym_id;
	
	Sym& s = data[id];
	
	if (!s.has_proxy) {
		double open_value = *open;
		double change = *close / open_value - 1.0;
		*change_ = change; // same without proxy
		*low_change  = *low / open_value - 1.0;
		*high_change = *high / open_value - 1.0;
		*spread_change = *spread;
		*value_change = change;
		ASSERT(*high_change >= *low_change);
	} else {
		double* proxy_spread = spreadsrc.GetValue<double>(0, s.proxy_id, attr.tf_id, 0, attr);
		
		// Get buffers
		double* proxy_open	= src.GetValue<double>(0, s.proxy_id, attr.tf_id, 1, attr);
		double* proxy_close	= src.GetValue<double>(0, s.proxy_id, attr.tf_id, 0, attr);
		double* proxy_low	= src.GetValue<double>(1, s.proxy_id, attr.tf_id, 1, attr);
		double* proxy_high	= src.GetValue<double>(2, s.proxy_id, attr.tf_id, 1, attr);
		
		// Proxy and normal.
		double proxy_open_value = *proxy_open;
		double open_value = *open;
		double proxy_change, proxy_high_change, proxy_low_change;
		ASSERTEXC(s.proxy_factor != 0);
		if (s.proxy_factor == 1) {
			proxy_change		= *proxy_close / proxy_open_value - 1.0;
			proxy_high_change	= *proxy_high / proxy_open_value - 1.0;
			proxy_low_change	= *proxy_low / proxy_open_value - 1.0;
		} else {
			proxy_change		= proxy_open_value / *proxy_close - 1.0;
			proxy_low_change	= proxy_open_value / *proxy_high - 1.0;
			proxy_high_change	= proxy_open_value / *proxy_low - 1.0;
		}
		double change = *close / open_value - 1.0;
		double low_change_	= *low / open_value - 1.0;
		double high_change_	= *high / open_value - 1.0;
		*change_ = change; // different with proxy
		*low_change = proxy_low_change + low_change_;		// Note: unrealistic, probably not simultaneously
		*high_change = proxy_high_change + high_change_;	// Note: unrealistic, probably not simultaneously
		*spread_change = *spread + *proxy_spread;
		*value_change	= change + proxy_change;
		ASSERT(*high_change >= *low_change);
	}
	
	return true;
}






























IdealOrders::IdealOrders() {
	
}

void IdealOrders::SetArguments(const VectorMap<String, Value>& args) {
	
}

void IdealOrders::Init() {
	AddDependency("/open", 1, 0);
	
}

bool IdealOrders::Process(const SlotProcessAttributes& attr) {
	
	// Search ideal order sequence with A* search in SimBroker
	Panic("TODO");
	
	return false;
}


}
