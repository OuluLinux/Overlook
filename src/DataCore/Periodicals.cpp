#include "DataCore.h"


namespace DataCore {

#define VAR_PERIOD(x) x.Clear(); x.SetPeriod(var_period);
#define VAR_PERIOD_VEC(x) for(int i = 0; i < x.GetCount(); i++) {x[i].Clear(); x[i].SetPeriod(var_period);}


WdayHourStats::WdayHourStats() {
	var_period = 10;
	
	AddValue<double>("Total Mean");
	AddValue<double>("Total Min");
	AddValue<double>("Total Max");
	AddValue<double>("Hour Mean");
	AddValue<double>("Hour Min");
	AddValue<double>("Hour Max");
	AddValue<double>("Day Mean");
	AddValue<double>("Day Min");
	AddValue<double>("Day Max");
	AddValue<double>("DayHour Mean");
	AddValue<double>("DayHour Min");
	AddValue<double>("DayHour Max");
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value1\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value2\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value3\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value4\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value5\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value6\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value7\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value8\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value9\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"},"
			"\"value10\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"},"
			"\"value11\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"}"
		"}"
	);
}

void WdayHourStats::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void WdayHourStats::Init() {
	AddDependency("/change");
	
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
		s.hour.SetCount(force_d0 ? 1 : 24*60 / period);
		s.wday.SetCount(5);
		s.wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
		
		VAR_PERIOD(s.total);
		VAR_PERIOD_VEC(s.wdayhour);
		VAR_PERIOD_VEC(s.wday);
		VAR_PERIOD_VEC(s.hour);
	}
}

bool WdayHourStats::Process(const SlotProcessAttributes& attr) {
	const Slot& change_slot = GetDependency(0);
	
	double* t_mean		= GetValue<double>(0, attr);
	double* t_min		= GetValue<double>(1, attr);
	double* t_max		= GetValue<double>(2, attr);
	double* h_mean		= GetValue<double>(3, attr);
	double* h_min		= GetValue<double>(4, attr);
	double* h_max		= GetValue<double>(5, attr);
	double* d_mean		= GetValue<double>(6, attr);
	double* d_min		= GetValue<double>(7, attr);
	double* d_max		= GetValue<double>(8, attr);
	double* dh_mean		= GetValue<double>(9, attr);
	double* dh_min		= GetValue<double>(10, attr);
	double* dh_max		= GetValue<double>(11, attr);
	
	double* change		= change_slot.GetValue<double>(0, 0, attr);
	
	TimeVector& tv = GetTimeVector();
	int tf_count = tv.GetPeriodCount();
	SymTf& s = data[attr.sym_id * tf_count + attr.tf_id];
	
	Time time = tv.GetTime(attr.GetPeriod(), attr.GetCounted());
	int period = tv.GetPeriod(attr.tf_id) * tv.GetBasePeriod() / 60; // minutes, not seconds
	int h = (time.minute + time.hour * 60) / period;
	int d = DayOfWeek(time) - 1;
	int h_count = 24 * 60 / period;
	int dh = h + d * h_count;
	bool force_d0 = period >= 7*24*60;
	
	// If long periods or weekend
	if (force_d0 || d == -1 || d == 5) {
		h = 0;
		d = 0;
		dh = 0;
	}
	
	MovingStepDistribution& t_var  = s.total;
	MovingStepDistribution& h_var  = s.hour[h];
	MovingStepDistribution& d_var  = s.wday[d];
	MovingStepDistribution& dh_var = s.wdayhour[dh];
	
	if (change && *change != 0.0) {
		t_var.AddResult(*change);
		h_var.AddResult(*change);
		d_var.AddResult(*change);
		dh_var.AddResult(*change);
	}
	
	*t_mean			= t_var.GetMean();
	*h_mean			= h_var.GetMean();
	*d_mean			= d_var.GetMean();
	*dh_mean		= dh_var.GetMean();
	
	*t_min			= t_var.Get(0.05);
	*h_min			= h_var.Get(0.05);
	*d_min			= d_var.Get(0.05);
	*dh_min			= dh_var.Get(0.05);
	
	*t_max			= t_var.Get(0.95);
	*h_max			= h_var.Get(0.95);
	*d_max			= d_var.Get(0.95);
	*dh_max			= dh_var.Get(0.95);
	
	t_var.Next();
	h_var.Next();
	d_var.Next();
	dh_var.Next();
	
	return true;
}


















WdayHourDiff::WdayHourDiff() {
	AddValue<double>("Total Mean");
	AddValue<double>("Total Min");
	AddValue<double>("Total Max");
	AddValue<double>("Hour Mean");
	AddValue<double>("Hour Min");
	AddValue<double>("Hour Max");
	AddValue<double>("Day Mean");
	AddValue<double>("Day Min");
	AddValue<double>("Day Max");
	AddValue<double>("DayHour Mean");
	AddValue<double>("DayHour Min");
	AddValue<double>("DayHour Max");
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value1\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value2\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value3\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value4\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value5\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value6\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value7\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value8\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value9\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"},"
			"\"value10\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"},"
			"\"value11\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":1,"
			"}"
		"}"
	);
	
	var_period_fast = 10;
	var_period_diff = 10;
}

void WdayHourDiff::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period_fast");
	if (i != -1)
		var_period_fast = args[i];
	i = args.Find("period_diff");
	if (i != -1)
		var_period_diff = args[i];
}

void WdayHourDiff::Init() {
	AddDependency("/whstat_fast");
	AddDependency("/whstat_slow");
}

bool WdayHourDiff::Process(const SlotProcessAttributes& attr) {
	const Slot& whstat_fast = GetDependency(0);
	const Slot& whstat_slow = GetDependency(1);
	
	double* t_mean_fast		= whstat_fast.GetValue<double>(0, attr);
	double* t_min_fast		= whstat_fast.GetValue<double>(1, attr);
	double* t_max_fast		= whstat_fast.GetValue<double>(2, attr);
	double* h_mean_fast		= whstat_fast.GetValue<double>(3, attr);
	double* h_min_fast		= whstat_fast.GetValue<double>(4, attr);
	double* h_max_fast		= whstat_fast.GetValue<double>(5, attr);
	double* d_mean_fast		= whstat_fast.GetValue<double>(6, attr);
	double* d_min_fast		= whstat_fast.GetValue<double>(7, attr);
	double* d_max_fast		= whstat_fast.GetValue<double>(8, attr);
	double* dh_mean_fast	= whstat_fast.GetValue<double>(9, attr);
	double* dh_min_fast		= whstat_fast.GetValue<double>(10, attr);
	double* dh_max_fast		= whstat_fast.GetValue<double>(11, attr);
	
	double* t_mean_slow		= whstat_slow.GetValue<double>(0, attr);
	double* t_min_slow		= whstat_slow.GetValue<double>(1, attr);
	double* t_max_slow		= whstat_slow.GetValue<double>(2, attr);
	double* h_mean_slow		= whstat_slow.GetValue<double>(3, attr);
	double* h_min_slow		= whstat_slow.GetValue<double>(4, attr);
	double* h_max_slow		= whstat_slow.GetValue<double>(5, attr);
	double* d_mean_slow		= whstat_slow.GetValue<double>(6, attr);
	double* d_min_slow		= whstat_slow.GetValue<double>(7, attr);
	double* d_max_slow		= whstat_slow.GetValue<double>(8, attr);
	double* dh_mean_slow	= whstat_slow.GetValue<double>(9, attr);
	double* dh_min_slow		= whstat_slow.GetValue<double>(10, attr);
	double* dh_max_slow		= whstat_slow.GetValue<double>(11, attr);
	
	double* t_mean			= GetValue<double>(0, attr);
	double* t_min			= GetValue<double>(1, attr);
	double* t_max			= GetValue<double>(2, attr);
	double* h_mean			= GetValue<double>(3, attr);
	double* h_min			= GetValue<double>(4, attr);
	double* h_max			= GetValue<double>(5, attr);
	double* d_mean			= GetValue<double>(6, attr);
	double* d_min			= GetValue<double>(7, attr);
	double* d_max			= GetValue<double>(8, attr);
	double* dh_mean			= GetValue<double>(9, attr);
	double* dh_min			= GetValue<double>(10, attr);
	double* dh_max			= GetValue<double>(11, attr);
	
	*t_mean					= *t_mean_fast    - *t_mean_slow;
	*h_mean					= *h_mean_fast    - *h_mean_slow;
	*d_mean					= *d_mean_fast    - *d_mean_slow;
	*dh_mean				= *dh_mean_fast   - *dh_mean_slow;
	
	*t_min					= *t_min_fast     - *t_min_slow;
	*h_min					= *h_min_fast     - *h_min_slow;
	*d_min					= *d_min_fast     - *d_min_slow;
	*dh_min					= *dh_min_fast    - *dh_min_slow;
	
	*t_max					= *t_max_fast     - *t_max_slow;
	*h_max					= *h_max_fast     - *h_max_slow;
	*d_max					= *d_max_fast     - *d_max_slow;
	*dh_max					= *dh_max_fast    - *dh_max_slow;
	
	return true;
}



















ChannelPredicter::ChannelPredicter() {
	length = 4;
	
	AddValue<double>("Hour Min");
	AddValue<double>("Hour Max");
	AddValue<double>("Day Min");
	AddValue<double>("Day Max");
	AddValue<double>("DayHour Min");
	AddValue<double>("DayHour Max");
	AddValue<double>("Average range");
}

void ChannelPredicter::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("length");
	if (i != -1)
		length = args[i];
}

void ChannelPredicter::Init() {
	AddDependency("/whstat_slow");
}

bool ChannelPredicter::Process(const SlotProcessAttributes& attr) {
	const Slot& whstat = GetDependency(0);
	double diff = 0.0;
	for(int j = 0; j < 6; j+=2) {
		double min_sum = 0;
		double max_sum = 0;
		for(int i = 0; i < length; i++) {
			double min	= *whstat.GetValue<double>(j+0, -1-i, attr);
			double max	= *whstat.GetValue<double>(j+1, -1-i, attr);
			ASSERT(max > min);
			ASSERT(max > 0);
			ASSERT(0 > min);
			min_sum += min;
			max_sum += max;
			diff += max - min;
		}
		*GetValue<double>(j+0, attr) = min_sum;
		*GetValue<double>(j+1, attr) = max_sum;
	}
	*GetValue<double>(6, attr) = diff / 3;
	return true;
}












EventOsc::EventOsc() {
	AddValue<double>("High Level");
	AddValue<double>("Medium Level");
	AddValue<double>("Low Level");
	AddValue<double>("Info Level");
	
	mul = 0.8;
	emgr = NULL;
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":4,"
			"},"
			"\"value1\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":3,"
			"},"
			"\"value2\":{"
				"\"color\":\"0,128,128\","
				"\"line_width\":2,"
			"},"
			"\"value3\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
		"}"
	);
	
}

void EventOsc::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("mul");
	if (i != -1)
		mul = args[i];
}

void EventOsc::Init() {
	ASSERTEXC(!(mul <= 0 || mul > 0.9));
	
	AddDependency("/open");
	
	TimeVector& tv = GetTimeVector();
	SlotPtr src = tv.FindLinkSlot("/open");
	db = dynamic_cast<DataBridge*>(&*src);
	ASSERTEXC(db);
	
	int tf_count = tv.GetPeriodCount();
	int sym_count = tv.GetSymbolCount();
	int mtsym_count = db->GetSymbolCount();
	int total = tf_count * sym_count;
	
	data.SetCount(total);
	for(int i = 0; i < total; i++) {
		int id = i / tf_count;
		Sym& s = data[i];
		s.counted_events = 0;
		
		if (id < mtsym_count) {
			const Symbol& sym = db->GetSymbol(id);
			
			if (sym.IsForex()) {
				s.keys.Add(sym.name.Left(3));
				s.keys.Add(sym.name.Right(3));
			}
			else {
				s.keys.Add(sym.currency_base);
			}
		} else {
			id -= mtsym_count;
			const Currency& cur = GetMetaTrader().GetCurrency(id);
			s.keys.Add(cur.name);
		}
	}
}

bool EventOsc::Process(const SlotProcessAttributes& attr) {
	EventManager& emgr = GetEventManager();
	TimeVector& tv = GetTimeVector();
	int tf_count = tv.GetPeriodCount();
	
	Sym& s = data[attr.sym_id * tf_count + attr.tf_id];
	
	int count = emgr.GetCount();
	int bars = attr.GetBars();
	
	if (!count) {
		emgr.Refresh();
		count = emgr.GetCount();
	}
	
	for (int i = s.counted_events; i < count; i++) {
		Event& e = emgr.GetEvent(i);
		if (s.keys.Find(e.currency) != -1) {
			int buf_id = 3 - e.impact;
			int shift = tv.GetShiftFromTime(e.timestamp, attr.GetPeriod());
			if (shift < 0 || shift >= bars) continue;
			double value = 1.0;
			int j = 0;
			while (value >= 0.01) {
				int a = shift - j;
				int b = shift + j;
				if (a >= 0) {
					double* d = GetValuePos<double>(buf_id, attr.sym_id, attr.tf_id, a, attr);
					if (d) *d = value;
				}
				if (j && b < bars) {
					double* d = GetValuePos<double>(buf_id, attr.sym_id, attr.tf_id, b, attr);
					if (d) *d = value;
				}
				value *= mul;
				j++;
			}
		}
	}
	
	s.counted_events = count;
	
	return true;
}


}
