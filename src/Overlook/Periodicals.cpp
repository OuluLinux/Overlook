#include "Overlook.h"


namespace Overlook {


#define VAR_PERIOD(x) x.Clear(); x.SetPeriod(var_period);
#define VAR_PERIOD_VEC(x) for(int i = 0; i < x.GetCount(); i++) {x[i].Clear(); x[i].SetPeriod(var_period);}


inline double SafeDiff(double a, double b) {
	if (a == 0.0 || b == 0.0) return 0;
	double value = a - b;
	if (IsFin(value)) return value;
	return 0;
}

inline double SafeDiv(double a, double b, double max, double min) {
	if (b == 0.0) return 0.0;
	double value = a / b;
	if (value < min) return min;
	if (value > max) return max;
	return value;
}












WdayHourChanges::WdayHourChanges() {
	
}

void WdayHourChanges::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(0,127,0));
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	
	wdayhour.SetCount(force_d0 ? 1 : 7*24*60 / period);
}

void WdayHourChanges::Start() {
	Buffer& mean = GetBuffer(0);
	Buffer& stddev = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	BaseSystem& bs = GetBaseSystem();
	
	//bars--;
	if (!counted)
		counted = 1;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time time = bs.GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time);
		int dh = h + d * h_count;
		
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		
		OnlineVariance& var = wdayhour[dh];
		
		double diff = SafeDiff(open.Get(i), open.Get(i-1));
		if (diff != 0.0) {
			var.AddResult(diff);
		}
		
		mean.Set(i,    var.GetMean());
		stddev.Set(i,  var.GetDeviation());
	}
}

const OnlineVariance& WdayHourChanges::GetOnlineVariance(int shift) {
	BaseSystem& bs = GetBaseSystem();
	int period = GetMinutePeriod();
	Time time = bs.GetTime(GetPeriod(), shift);
	
	bool force_d0 = period >= 7*24*60;
	if (force_d0)
		return wdayhour[0];
	
	int h_count = 24 * 60 / period;
	int h = (time.minute + time.hour * 60) / period;
	int d = DayOfWeek(time);
	int dh = h + d * h_count;
	
	return wdayhour[dh];
}














WdayHourStats::WdayHourStats() {
	var_period = 10;
}

void WdayHourStats::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	
	hour.SetCount(force_d0 ? 1 : 24*60 / period);
	wday.SetCount(5);
	wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	
	VAR_PERIOD(total);
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(wday);
	VAR_PERIOD_VEC(hour);
}

void WdayHourStats::Start() {
	Buffer& t_pre_mean		= GetBuffer(0);
	Buffer& t_pre_stddev	= GetBuffer(1);
	Buffer& h_pre_mean		= GetBuffer(2);
	Buffer& h_pre_stddev	= GetBuffer(3);
	Buffer& d_pre_mean		= GetBuffer(4);
	Buffer& d_pre_stddev	= GetBuffer(5);
	Buffer& dh_pre_mean		= GetBuffer(6);
	Buffer& dh_pre_stddev	= GetBuffer(7);
	
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	BaseSystem& bs = GetBaseSystem();
	
	//bars--;
	if (!counted)
		counted = 1;
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time time = bs.GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		MovingOnlineVariance& t_var  = total;
		MovingOnlineVariance& h_var  = hour[h];
		MovingOnlineVariance& d_var  = wday[d];
		MovingOnlineVariance& dh_var = wdayhour[dh];
		
		double change = SafeDiff(open.Get(i), open.Get(i-1));
		if (change != 0.0) {
			t_var.AddResult(change);
			h_var.AddResult(change);
			d_var.AddResult(change);
			dh_var.AddResult(change);
		}
		
		t_pre_mean.Set(i,    t_var.GetMean());
		h_pre_mean.Set(i,    h_var.GetMean());
		d_pre_mean.Set(i,    d_var.GetMean());
		dh_pre_mean.Set(i,   dh_var.GetMean());
		
		t_pre_stddev.Set(i,  t_var.GetDeviation());
		h_pre_stddev.Set(i,  h_var.GetDeviation());
		d_pre_stddev.Set(i,  d_var.GetDeviation());
		dh_pre_stddev.Set(i, dh_var.GetDeviation());
		
		
		
		t_var.Next();
		h_var.Next();
		d_var.Next();
		dh_var.Next();
	}
}























WdayHourDiff::WdayHourDiff() {
	var_period_fast = 10;
	var_period_diff = 10;
}

void WdayHourDiff::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	AddSubCore<WdayHourStats>().Set("period", var_period_fast);
	AddSubCore<WdayHourStats>().Set("period", var_period_fast + var_period_diff);
}

void WdayHourDiff::Start() {
	Buffer& t_pre_mean		= GetBuffer(0);
	Buffer& t_pre_stddev	= GetBuffer(1);
	Buffer& h_pre_mean		= GetBuffer(2);
	Buffer& h_pre_stddev	= GetBuffer(3);
	Buffer& d_pre_mean		= GetBuffer(4);
	Buffer& d_pre_stddev	= GetBuffer(5);
	Buffer& dh_pre_mean		= GetBuffer(6);
	Buffer& dh_pre_stddev	= GetBuffer(7);
	// TODO: copy whstat logic in this, don't use other class
	
	int counted = GetCounted();
	
	if (counted > 0) counted--;
	
	Core& a = At(0);
	ConstBuffer& t_pre_stddev_fast		= a.GetBuffer(0);
	ConstBuffer& t_pre_mean_fast		= a.GetBuffer(1);
	ConstBuffer& h_pre_stddev_fast		= a.GetBuffer(2);
	ConstBuffer& h_pre_mean_fast		= a.GetBuffer(3);
	ConstBuffer& d_pre_stddev_fast		= a.GetBuffer(4);
	ConstBuffer& d_pre_mean_fast		= a.GetBuffer(5);
	ConstBuffer& dh_pre_stddev_fast		= a.GetBuffer(6);
	ConstBuffer& dh_pre_mean_fast		= a.GetBuffer(7);
	
	Core& b = At(1);
	ConstBuffer& t_pre_stddev_slow		= b.GetBuffer(0);
	ConstBuffer& t_pre_mean_slow		= b.GetBuffer(1);
	ConstBuffer& h_pre_stddev_slow		= b.GetBuffer(2);
	ConstBuffer& h_pre_mean_slow		= b.GetBuffer(3);
	ConstBuffer& d_pre_stddev_slow		= b.GetBuffer(4);
	ConstBuffer& d_pre_mean_slow		= b.GetBuffer(5);
	ConstBuffer& dh_pre_stddev_slow		= b.GetBuffer(6);
	ConstBuffer& dh_pre_mean_slow		= b.GetBuffer(7);
	
	int count = Upp::min(Upp::min(GetBars(), t_pre_stddev_fast.GetCount()), t_pre_stddev_slow.GetCount());
	//ASSERT( count >= bars - 1 ); // If fails and needs to be skipped, remember to reduce GetCounted value
	
	for ( int i = counted; i < count; i++) {
		SetSafetyLimit(i);
		t_pre_mean.Set(i,    t_pre_mean_fast.Get(i)    - t_pre_mean_slow.Get(i));
		h_pre_mean.Set(i,    h_pre_mean_fast.Get(i)    - h_pre_mean_slow.Get(i));
		d_pre_mean.Set(i,    d_pre_mean_fast.Get(i)    - d_pre_mean_slow.Get(i));
		dh_pre_mean.Set(i,   dh_pre_mean_fast.Get(i)   - dh_pre_mean_slow.Get(i));
		
		t_pre_stddev.Set(i,  t_pre_stddev_fast.Get(i)  - t_pre_stddev_slow.Get(i));
		h_pre_stddev.Set(i,  h_pre_stddev_fast.Get(i)  - h_pre_stddev_slow.Get(i));
		d_pre_stddev.Set(i,  d_pre_stddev_fast.Get(i)  - d_pre_stddev_slow.Get(i));
		dh_pre_stddev.Set(i, dh_pre_stddev_fast.Get(i) - dh_pre_stddev_slow.Get(i));
	}
}














WeekStats::WeekStats() {
	var_period = 10;
}

void WeekStats::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	month.SetCount(force_d0 ? 1 : 5);
	quarter.SetCount(force_d0 ? 1 : 14);
	year.SetCount(force_d0 ? 1 : 54);
	
	VAR_PERIOD(week);
	VAR_PERIOD_VEC(month);
	VAR_PERIOD_VEC(quarter);
	VAR_PERIOD_VEC(year);
}

void WeekStats::Start() {
	Buffer& w_pre_mean		= GetBuffer(0);
	Buffer& w_pre_stddev	= GetBuffer(1);
	Buffer& m_pre_mean		= GetBuffer(2);
	Buffer& m_pre_stddev	= GetBuffer(3);
	Buffer& q_pre_mean		= GetBuffer(4);
	Buffer& q_pre_stddev	= GetBuffer(5);
	Buffer& y_pre_mean		= GetBuffer(6);
	Buffer& y_pre_stddev	= GetBuffer(7);
	
	int bars = GetBars() - 1;
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		return;
	
	if (!counted) counted++;
	else counted--;
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time time = GetBaseSystem().GetTime(GetPeriod(), i);
		
		int m = 0;
		{
			Time t(time.year, time.month, time.day);
			int wd = DayOfWeek(t);
			int wdmod = 7 - wd;
			t += wdmod*24*60*60;
			while (time >= t) {
				m++;
				t += 7*24*60*60;
			}
		}
		
		int q = 0;
		{
			Time t(time.year, time.month - (time.month-1) % 3, 1);
			int wd = DayOfWeek(t);
			int wdmod = 7 - wd;
			t += wdmod*24*60*60;
			while (time >= t) {
				q++;
				t += 7*24*60*60;
			}
		}
		
		int y = 0;
		{
			Time t(time.year, 1, 1);
			int wd = DayOfWeek(t);
			int wdmod = 7 - wd;
			t += wdmod*24*60*60;
			while (time >= t) {
				y++;
				t += 7*24*60*60;
			}
		}
		
		MovingOnlineVariance& w_var  = week;
		MovingOnlineVariance& m_var  = month[m];
		MovingOnlineVariance& q_var  = quarter[q];
		MovingOnlineVariance& y_var  = year[y];
		
		double change = open.Get(i) - open.Get(i-1);
		if (change != 0.0) {
			w_var.AddResult(change);
			m_var.AddResult(change);
			q_var.AddResult(change);
			y_var.AddResult(change);
		}
		
		w_pre_mean.Set(i,    w_var.GetMean());
		m_pre_mean.Set(i,    m_var.GetMean());
		q_pre_mean.Set(i,    q_var.GetMean());
		y_pre_mean.Set(i,    y_var.GetMean());
		
		w_pre_stddev.Set(i,  w_var.GetDeviation());
		m_pre_stddev.Set(i,  m_var.GetDeviation());
		q_pre_stddev.Set(i,  q_var.GetDeviation());
		y_pre_stddev.Set(i,  y_var.GetDeviation());
		
		w_var.Next();
		m_var.Next();
		q_var.Next();
		y_var.Next();
	}
}










WdayHourTrending::WdayHourTrending() {
	var_period = 10;
}

void WdayHourTrending::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(6, 2);
	SetBufferLineWidth(7, 2);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	s1.SetCount(force_d0 ? 54 : 5*24*60 / period);
	s2.SetCount(force_d0 ? 54 : 5*24*60 / period);
	s3.SetCount(force_d0 ? 54 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(s1);
	VAR_PERIOD_VEC(s2);
	VAR_PERIOD_VEC(s3);
}

void WdayHourTrending::Start() {
	Buffer& shift1_mean		= GetBuffer(0);
	Buffer& shift1_stddev	= GetBuffer(1);
	Buffer& shift2_mean		= GetBuffer(2);
	Buffer& shift2_stddev	= GetBuffer(3);
	Buffer& shift3_mean		= GetBuffer(4);
	Buffer& shift3_stddev	= GetBuffer(5);
	Buffer& mean_av			= GetBuffer(6);
	Buffer& stddev_av		= GetBuffer(7);
	
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	BaseSystem& bs = GetBaseSystem();
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	//bars--;
	if (!counted)
		counted++;
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time time = bs.GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			int y = 0;
			{
				Time t(time.year, 1, 1);
				int wd = DayOfWeek(t);
				int wdmod = 7 - wd;
				t += wdmod*24*60*60;
				while (time >= t) {
					y++;
					t += 7*24*60*60;
				}
			}
			dh = y;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		MovingOnlineVariance& s1_var  = s1[dh];
		MovingOnlineVariance& s2_var  = s2[dh];
		MovingOnlineVariance& s3_var  = s3[dh];
		
		double change = open.Get(i) - open.Get(i-1);
		if (change != 0.0) {
			double c1 = 0, c2 = 0, c3 = 0;
			if (i > 0) c1 = open.Get(i-1+1) >= open.Get(i-1) ? change : -change;
			if (i > 1) c2 = open.Get(i-2+1) >= open.Get(i-2) ? change : -change;
			if (i > 2) c3 = open.Get(i-3+1) >= open.Get(i-3) ? change : -change;
			s1_var.AddResult(c1);
			s2_var.AddResult(c2);
			s3_var.AddResult(c3);
		}
		
		double m1 = s1_var.GetMean();
		double m2 = s2_var.GetMean();
		double m3 = s3_var.GetMean();
		shift1_mean.Set(i, m1);
		shift2_mean.Set(i, m2);
		shift3_mean.Set(i, m3);
		mean_av.Set(i, (m1 + m2 + m3) / 3.0);
		
		double d1 = s1_var.GetDeviation();
		double d2 = s2_var.GetDeviation();
		double d3 = s3_var.GetDeviation();
		shift1_stddev.Set(i, d1);
		shift2_stddev.Set(i, d2);
		shift3_stddev.Set(i, d3);
		stddev_av.Set(i, (d1 + d2 + d3) / 3.0);
		
		s1_var.Next();
		s2_var.Next();
		s3_var.Next();
	}
}








WdayHourTrendSuccess::WdayHourTrendSuccess() {
	var_period = 10;
}

void WdayHourTrendSuccess::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,0,127));
	SetBufferColor(4, Color(127,0,127));
	SetBufferLineWidth(4, 2);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	vars.SetCount(force_d0 ? 54 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(vars);
}

void WdayHourTrendSuccess::Start() {
	BaseSystem& bs = GetBaseSystem();
	Buffer& mean		= GetBuffer(0);
	Buffer& stddev		= GetBuffer(1);
	Buffer& success		= GetBuffer(2);
	Buffer& erradj		= GetBuffer(3);
	Buffer& dir			= GetBuffer(4);
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	if (counted < 3) counted = 3;
	
	//WdayHourTrending& whtrend = *Get<WdayHourTrending>();
	
	ConstBuffer& shift1_mean = GetInputBuffer(1, 0);
	ConstBuffer& shift2_mean = GetInputBuffer(1, 2);
	ConstBuffer& shift3_mean = GetInputBuffer(1, 4);
	
	ConstBuffer& shift1_stddev = GetInputBuffer(1, 1);
	ConstBuffer& shift2_stddev = GetInputBuffer(1, 3);
	ConstBuffer& shift3_stddev = GetInputBuffer(1, 5);
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	//bars--;
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time time = bs.GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			int y = 0;
			{
				Time t(time.year, 1, 1);
				int wd = DayOfWeek(t);
				int wdmod = 7 - wd;
				t += wdmod*24*60*60;
				while (time >= t) {
					y++;
					t += 7*24*60*60;
				}
			}
			dh = y;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		MovingOnlineVariance& var  = vars[dh];
		
		double cdf1 = 1 - NormalCDF(0, shift1_mean.Get(i), shift1_stddev.Get(i));
		double cdf2 = 1 - NormalCDF(0, shift2_mean.Get(i), shift2_stddev.Get(i));
		double cdf3 = 1 - NormalCDF(0, shift3_mean.Get(i), shift3_stddev.Get(i));
		
		double change1 = open.Get(i-1+1) / open.Get(i-1) - 1;
		double change2 = open.Get(i-2+1) / open.Get(i-2) - 1;
		double change3 = open.Get(i-3+1) / open.Get(i-3) - 1;
		
		change1 *= (cdf1 - 0.5) * 2;
		change2 *= (cdf2 - 0.5) * 2;
		change3 *= (cdf3 - 0.5) * 2;
		
		double change_sum = change1 + change2 + change3;
		dir.Set(i, change_sum);
		
		//double change_sum = (change1 >= 0 ? 1 : -1) + (change2 >= 0 ? 1 : -1) + (change3 >= 0 ? 1 : -1);
		
		// Make decision
		bool type = change_sum < 0;
		
		
		
		// Wait for change
		double change = open.Get(i) - open.Get(i-1);
		double type_change = (type ? -1.0 : 1.0) * change; // selling changed direction
		
		if (change != 0.0)
			var.AddResult(type_change);
		
		double mean_av = var.GetMean();
		mean.Set(i, mean_av);
		stddev.Set(i, var.GetDeviation());
		success.Set(i, type_change);
		
		double erradj_change = (mean_av < 0 ? -1.0 : 1.0) * type_change;
		erradj.Set(i, erradj_change);
		
		
		var.Next();
	}
}











FeatureOsc::FeatureOsc() {
	mul = 0.8;
	counted_features = 0;
}

void FeatureOsc::Init() {
	if (mul <= 0 || mul > 0.9) throw DataExc();
	
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
}

void FeatureOsc::Start() {
	Buffer& high	= GetBuffer(0);
	Buffer& med		= GetBuffer(1);
	Buffer& low		= GetBuffer(2);
	Buffer& info	= GetBuffer(3);
	
	int bars = GetBars();
	SetSafetyLimit(bars);
	FeatureDetector& feat = *Get<FeatureDetector>();
	
	int count = feat.GetKeypointCount();
	for(int i = counted_features; i < count; i++) {
		const FeatureKeypoint& kp = feat.GetKeypoint(i);
		int shift = kp.GetShift();
		int impact = kp.GetImpact();
		Buffer& buf = (impact == 0 ? info : (impact == 1 ? low : (impact == 2 ? med : high)));
		if (shift < 0 || shift >= bars) continue;
		double value = 1.0;
		int j = 0;
		while (value >= 0.01) {
			//int a = shift - j;
			int b = shift + j;
			//if (a >= 0) buf.Inc(a, value); // this actually gives harmful peek to future
			if (j && b < bars) buf.Inc(b, value);
			value *= mul;
			j++;
		}
	}
	counted_features = count;
}












ChannelPredicter::ChannelPredicter() {
	
}

void ChannelPredicter::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(64, 0, 0));
	SetBufferColor(2, Color(128, 0, 0));
	SetBufferColor(4, Color(196, 0, 0));
	SetBufferColor(1, Color(0, 64, 0));
	SetBufferColor(3, Color(0, 128, 0));
	SetBufferColor(5, Color(0, 196, 0));
	SetBufferColor(6, Color(0, 0, 64));
}

void ChannelPredicter::Start() {
	const WdayHourStats& whstat = *Get<WdayHourStats>();
	
	int bars = GetBars();
	int counted = GetCounted();
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double diff = 0.0;
		for(int j = 0; j < 6; j+=2) {
			double mean = whstat.GetBuffer(j+0).Get(i);
			double stddev = whstat.GetBuffer(j+1).Get(i);
			// TODO: correct distribution calculations... this is incorrect in many levels
			double min = mean - stddev;
			double max = mean + stddev;
			ASSERT(max >= min);
			diff += max - min;
			GetBuffer(j+0).Set(i, min);
			GetBuffer(j+1).Set(i, max);
		}
		GetBuffer(6).Set(i, diff / 3);
	}
}








	
}
