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




















WdayHourForecastErrors::WdayHourForecastErrors() {
	var_period = 10;
}

void WdayHourForecastErrors::Init() {
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
	SetCoreLevelCount(2);
	SetCoreLevel(0, 1.0);
	SetCoreLevel(1, 1.2);
	SetCoreLevelsColor(Color(192, 192, 192));
	SetCoreLevelsStyle(STYLE_DOT);
	SetCoreMaximum(1.5);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	
	int count = force_d0 ? 1 : 5*24*60 / period;
	total.SetCount(count);
	hour.SetCount(count);
	wday.SetCount(count);
	wdayhour.SetCount(count);
	
	AddSubCore<WdayHourStats>();
	
	VAR_PERIOD_VEC(total);
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(wday);
	VAR_PERIOD_VEC(hour);
}

void WdayHourForecastErrors::Start() {
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
	
	Core& wdayhourstats = At(0);
	
	ConstBuffer& stat_t_pre_mean		= wdayhourstats.GetBuffer(0);
	ConstBuffer& stat_t_pre_stddev		= wdayhourstats.GetBuffer(1);
	ConstBuffer& stat_h_pre_mean		= wdayhourstats.GetBuffer(2);
	ConstBuffer& stat_h_pre_stddev		= wdayhourstats.GetBuffer(3);
	ConstBuffer& stat_d_pre_mean		= wdayhourstats.GetBuffer(4);
	ConstBuffer& stat_d_pre_stddev		= wdayhourstats.GetBuffer(5);
	ConstBuffer& stat_dh_pre_mean		= wdayhourstats.GetBuffer(6);
	ConstBuffer& stat_dh_pre_stddev		= wdayhourstats.GetBuffer(7);
	
	//bars--;
	if (!counted)
		counted++;
	
	for ( int i = counted; i < bars; i++) {
		Time time = GetBaseSystem().GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		MovingOnlineVariance& t_var  = total[dh];
		MovingOnlineVariance& h_var  = hour[dh];
		MovingOnlineVariance& d_var  = wday[dh];
		MovingOnlineVariance& dh_var = wdayhour[dh];
		
		double change = open.Get(i) - open.Get(i-1);
		
		if (change != 0.0) {
			t_var.AddResult(SafeDiv((change - stat_t_pre_mean.Get(i)), stat_t_pre_stddev.Get(i), 2.0, -2.0 ));
			h_var.AddResult(SafeDiv((change - stat_h_pre_mean.Get(i)), stat_h_pre_stddev.Get(i), 2.0, -2.0 ));
			d_var.AddResult(SafeDiv((change - stat_d_pre_mean.Get(i)), stat_d_pre_stddev.Get(i), 2.0, -2.0 ));
			dh_var.AddResult(SafeDiv((change - stat_dh_pre_mean.Get(i)), stat_dh_pre_stddev.Get(i), 2.0, -2.0));
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



























WdayHourErrorAdjusted::WdayHourErrorAdjusted() {
	
}

void WdayHourErrorAdjusted::Init() {
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
	AddSubCore<WdayHourStats>();
	AddSubCore<WdayHourForecastErrors>();
}

void WdayHourErrorAdjusted::Start() {
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
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	Core& wdayhourstats = At(0);
	Core& wdayhourforecasterrors = At(1);
	
	ConstBuffer& stat_t_pre_mean		= wdayhourstats.GetBuffer(0);
	ConstBuffer& stat_t_pre_stddev		= wdayhourstats.GetBuffer(1);
	ConstBuffer& stat_h_pre_mean		= wdayhourstats.GetBuffer(2);
	ConstBuffer& stat_h_pre_stddev		= wdayhourstats.GetBuffer(3);
	ConstBuffer& stat_d_pre_mean		= wdayhourstats.GetBuffer(4);
	ConstBuffer& stat_d_pre_stddev		= wdayhourstats.GetBuffer(5);
	ConstBuffer& stat_dh_pre_mean		= wdayhourstats.GetBuffer(6);
	ConstBuffer& stat_dh_pre_stddev		= wdayhourstats.GetBuffer(7);
	
	ConstBuffer& err_t_pre_mean			= wdayhourforecasterrors.GetBuffer(0);
	ConstBuffer& err_t_pre_stddev		= wdayhourforecasterrors.GetBuffer(1);
	ConstBuffer& err_h_pre_mean			= wdayhourforecasterrors.GetBuffer(2);
	ConstBuffer& err_h_pre_stddev		= wdayhourforecasterrors.GetBuffer(3);
	ConstBuffer& err_d_pre_mean			= wdayhourforecasterrors.GetBuffer(4);
	ConstBuffer& err_d_pre_stddev		= wdayhourforecasterrors.GetBuffer(5);
	ConstBuffer& err_dh_pre_mean		= wdayhourforecasterrors.GetBuffer(6);
	ConstBuffer& err_dh_pre_stddev		= wdayhourforecasterrors.GetBuffer(7);
	
	for (int i = counted; i < bars; i++) {
		
		t_pre_mean.Set(i,    stat_t_pre_mean.Get(i) + stat_t_pre_stddev.Get(i) * err_t_pre_mean.Get(i));
		h_pre_mean.Set(i,    stat_h_pre_mean.Get(i) + stat_h_pre_stddev.Get(i) * err_h_pre_mean.Get(i));
		d_pre_mean.Set(i,    stat_d_pre_mean.Get(i) + stat_d_pre_stddev.Get(i) * err_d_pre_mean.Get(i));
		dh_pre_mean.Set(i,   stat_dh_pre_mean.Get(i) + stat_dh_pre_stddev.Get(i) * err_dh_pre_mean.Get(i));
		
		t_pre_stddev.Set(i,  stat_t_pre_stddev.Get(i) * err_t_pre_stddev.Get(i));
		h_pre_stddev.Set(i,  stat_h_pre_stddev.Get(i) * err_h_pre_stddev.Get(i));
		d_pre_stddev.Set(i,  stat_d_pre_stddev.Get(i) * err_d_pre_stddev.Get(i));
		dh_pre_stddev.Set(i, stat_dh_pre_stddev.Get(i) * err_dh_pre_stddev.Get(i));
		
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
		throw DataExc();
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	for (int i = counted; i < bars; i++) {
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
		
		double change = open.Get(i+1) - open.Get(i);
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





































WeekStatsForecastErrors::WeekStatsForecastErrors() {
	var_period = 10;
}

void WeekStatsForecastErrors::Init() {
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
	
	SetCoreLevelCount(2);
	SetCoreLevel(0, 1.0);
	SetCoreLevel(1, 1.2);
	SetCoreLevelsColor(Color(192, 192, 192));
	SetCoreLevelsStyle(STYLE_DOT);
	SetCoreMaximum(1.5);
	
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	int count = force_d0 ? 1 : 54;
	week.SetCount(count);
	month.SetCount(count);
	quarter.SetCount(count);
	year.SetCount(count);
	
	AddSubCore<WeekStats>();
	
	VAR_PERIOD_VEC(week);
	VAR_PERIOD_VEC(month);
	VAR_PERIOD_VEC(quarter);
	VAR_PERIOD_VEC(year);
}

void WeekStatsForecastErrors::Start() {
	Buffer& w_pre_mean		= GetBuffer(0);
	Buffer& w_pre_stddev	= GetBuffer(1);
	Buffer& m_pre_mean		= GetBuffer(2);
	Buffer& m_pre_stddev	= GetBuffer(3);
	Buffer& q_pre_mean		= GetBuffer(4);
	Buffer& q_pre_stddev	= GetBuffer(5);
	Buffer& y_pre_mean		= GetBuffer(6);
	Buffer& y_pre_stddev	= GetBuffer(7);
	
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		throw DataExc();
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	Core& weekstats = At(0);
	
	ConstBuffer& stat_w_pre_mean		= weekstats.GetBuffer(0);
	ConstBuffer& stat_w_pre_stddev		= weekstats.GetBuffer(1);
	ConstBuffer& stat_m_pre_mean		= weekstats.GetBuffer(2);
	ConstBuffer& stat_m_pre_stddev		= weekstats.GetBuffer(3);
	ConstBuffer& stat_q_pre_mean		= weekstats.GetBuffer(4);
	ConstBuffer& stat_q_pre_stddev		= weekstats.GetBuffer(5);
	ConstBuffer& stat_y_pre_mean		= weekstats.GetBuffer(6);
	ConstBuffer& stat_y_pre_stddev		= weekstats.GetBuffer(7);
	
	//bars--;
	if (!counted)
		counted++;
	
	for ( int i = counted; i < bars; i++) {
		Time time = GetBaseSystem().GetTime(GetPeriod(), i);
		
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
		
		MovingOnlineVariance& w_var  = week[y];
		MovingOnlineVariance& m_var  = month[y];
		MovingOnlineVariance& q_var  = quarter[y];
		MovingOnlineVariance& y_var  = year[y];
		
		double change = open.Get(i) - open.Get(i-1);
		if (change != 0.0) {
			w_var.AddResult( Upp::min(2.0, Upp::max(-2.0, (change - stat_w_pre_mean.Get(i)) / stat_w_pre_stddev.Get(i) )) );
			m_var.AddResult( Upp::min(2.0, Upp::max(-2.0, (change - stat_m_pre_mean.Get(i)) / stat_m_pre_stddev.Get(i) )) );
			q_var.AddResult( Upp::min(2.0, Upp::max(-2.0, (change - stat_q_pre_mean.Get(i)) / stat_q_pre_stddev.Get(i) )) );
			y_var.AddResult( Upp::min(2.0, Upp::max(-2.0, (change - stat_y_pre_mean.Get(i)) / stat_y_pre_stddev.Get(i) )) );
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



























WeekStatsErrorAdjusted::WeekStatsErrorAdjusted() {
	skip = false;
}

void WeekStatsErrorAdjusted::Init() {
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
	
	AddSubCore<WeekStats>();
	AddSubCore<WeekStatsForecastErrors>();
	
	if (GetBaseSystem().GetEndTS() - GetBaseSystem().GetBeginTS() < 365*24*60*60) {
		SetSkipAllocate();
		skip = true;
	}
}

void WeekStatsErrorAdjusted::Start() {
	if (skip)
		return;
	
	Buffer& w_pre_mean		= GetBuffer(0);
	Buffer& w_pre_stddev	= GetBuffer(1);
	Buffer& m_pre_mean		= GetBuffer(2);
	Buffer& m_pre_stddev	= GetBuffer(3);
	Buffer& q_pre_mean		= GetBuffer(4);
	Buffer& q_pre_stddev	= GetBuffer(5);
	Buffer& y_pre_mean		= GetBuffer(6);
	Buffer& y_pre_stddev	= GetBuffer(7);
	
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		throw DataExc();
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	Core& weekstats = At(0);
	Core& weekstatsforecasterrors = At(1);
	
	ConstBuffer& stat_w_pre_mean	= weekstats.GetBuffer(0);
	ConstBuffer& stat_w_pre_stddev	= weekstats.GetBuffer(1);
	ConstBuffer& stat_m_pre_mean	= weekstats.GetBuffer(2);
	ConstBuffer& stat_m_pre_stddev	= weekstats.GetBuffer(3);
	ConstBuffer& stat_q_pre_mean	= weekstats.GetBuffer(4);
	ConstBuffer& stat_q_pre_stddev	= weekstats.GetBuffer(5);
	ConstBuffer& stat_y_pre_mean	= weekstats.GetBuffer(6);
	ConstBuffer& stat_y_pre_stddev	= weekstats.GetBuffer(7);
	
	ConstBuffer& err_w_pre_mean		= weekstatsforecasterrors.GetBuffer(0);
	ConstBuffer& err_w_pre_stddev	= weekstatsforecasterrors.GetBuffer(1);
	ConstBuffer& err_m_pre_mean		= weekstatsforecasterrors.GetBuffer(2);
	ConstBuffer& err_m_pre_stddev	= weekstatsforecasterrors.GetBuffer(3);
	ConstBuffer& err_q_pre_mean		= weekstatsforecasterrors.GetBuffer(4);
	ConstBuffer& err_q_pre_stddev	= weekstatsforecasterrors.GetBuffer(5);
	ConstBuffer& err_y_pre_mean		= weekstatsforecasterrors.GetBuffer(6);
	ConstBuffer& err_y_pre_stddev	= weekstatsforecasterrors.GetBuffer(7);
	
	for ( int i = counted; i < bars; i++) {
		w_pre_mean.Set(i,    stat_w_pre_mean.Get(i) + stat_w_pre_stddev.Get(i) * err_w_pre_mean.Get(i));
		m_pre_mean.Set(i,    stat_m_pre_mean.Get(i) + stat_m_pre_stddev.Get(i) * err_m_pre_mean.Get(i));
		q_pre_mean.Set(i,    stat_q_pre_mean.Get(i) + stat_q_pre_stddev.Get(i) * err_q_pre_mean.Get(i));
		y_pre_mean.Set(i,    stat_y_pre_mean.Get(i) + stat_y_pre_stddev.Get(i) * err_y_pre_mean.Get(i));
		
		w_pre_stddev.Set(i,    stat_w_pre_stddev.Get(i) * err_w_pre_stddev.Get(i));
		m_pre_stddev.Set(i,    stat_m_pre_stddev.Get(i) * err_m_pre_stddev.Get(i));
		q_pre_stddev.Set(i,    stat_q_pre_stddev.Get(i) * err_q_pre_stddev.Get(i));
		y_pre_stddev.Set(i,    stat_y_pre_stddev.Get(i) * err_y_pre_stddev.Get(i));
	}
}






















WdayHourDiffWeek::WdayHourDiffWeek() {
	
}

void WdayHourDiffWeek::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(0,127,0));
	SetBufferLineWidth(1, 2);
	
	SetCoreLevelCount(2);
	SetCoreLevel(0,  0.5);
	SetCoreLevel(1, -0.5);
	SetCoreLevelsColor(Color(192, 192, 192));
	SetCoreLevelsStyle(STYLE_DOT);
	
	AddSubCore<WdayHourErrorAdjusted>();
	
	SetCoreMaximum(1.0);
	SetCoreMinimum(-1.0);
}

void WdayHourDiffWeek::Start() {
	Buffer& mean = GetBuffer(0);
	Buffer& cdf  = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	
	if (period >= 7*24*60*60) return;
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	Core& wdayhourerradj = At(0);
	Core& weekstatserradj = *Get<WeekStatsErrorAdjusted>();
	
	LOG("WdayHourDiffWeek::Start");
	int a = wdayhourerradj.GetPeriod();
	int b = GetPeriod();
	ASSERT(a == b);
	
	ConstBuffer& wd_t_pre_mean		= wdayhourerradj.GetBuffer(0);
	ConstBuffer& wd_t_pre_stddev	= wdayhourerradj.GetBuffer(1);
	ConstBuffer& wd_h_pre_mean		= wdayhourerradj.GetBuffer(2);
	ConstBuffer& wd_h_pre_stddev	= wdayhourerradj.GetBuffer(3);
	ConstBuffer& wd_d_pre_mean		= wdayhourerradj.GetBuffer(4);
	ConstBuffer& wd_d_pre_stddev	= wdayhourerradj.GetBuffer(5);
	ConstBuffer& wd_dh_pre_mean		= wdayhourerradj.GetBuffer(6);
	ConstBuffer& wd_dh_pre_stddev	= wdayhourerradj.GetBuffer(7);
	
	ConstBuffer& week_w_pre_mean	= weekstatserradj.GetBuffer(0);
	ConstBuffer& week_w_pre_stddev	= weekstatserradj.GetBuffer(1);
	ConstBuffer& week_m_pre_mean	= weekstatserradj.GetBuffer(2);
	ConstBuffer& week_m_pre_stddev	= weekstatserradj.GetBuffer(3);
	ConstBuffer& week_q_pre_mean	= weekstatserradj.GetBuffer(4);
	ConstBuffer& week_q_pre_stddev	= weekstatserradj.GetBuffer(5);
	ConstBuffer& week_y_pre_mean	= weekstatserradj.GetBuffer(6);
	ConstBuffer& week_y_pre_stddev	= weekstatserradj.GetBuffer(7);
	
	int this_period = GetPeriod();
	int w1_period = weekstatserradj.GetPeriod();
	int w1_count = GetBaseSystem().GetCount(w1_period);
	int week_w_pre_mean_count = week_w_pre_mean.GetCount();
	if (week_w_pre_mean_count != w1_count)
		return;
	
	if (counted) counted--;
	
	double div = (5*24*60 / period);
	
	for (int i = counted; i < bars; i++) {
		int w1_shift = GetBaseSystem().GetShift(this_period, w1_period, i);
		if (w1_shift >= week_w_pre_mean.GetCount()) break;
		
		// Make averages
		double w1_mean_av = (
			week_w_pre_mean.Get(w1_shift) +
			week_m_pre_mean.Get(w1_shift) +
			week_q_pre_mean.Get(w1_shift) +
			week_y_pre_mean.Get(w1_shift) ) / 4.0;
		
		double w1_stddev_av = (
			week_w_pre_stddev.Get(w1_shift) +
			week_m_pre_stddev.Get(w1_shift) +
			week_q_pre_stddev.Get(w1_shift) +
			week_y_pre_stddev.Get(w1_shift) ) / 4.0;
		
		double w1_tf_mean_av = w1_mean_av / div;
		
		double tf_mean_av = (
			wd_t_pre_mean.Get(i) +
			wd_h_pre_mean.Get(i) +
			wd_d_pre_mean.Get(i) +
			wd_dh_pre_mean.Get(i) ) / 4.0;
		
		double tf_stddev_av = (
			wd_t_pre_stddev.Get(i) +
			wd_h_pre_stddev.Get(i) +
			wd_d_pre_stddev.Get(i) +
			wd_dh_pre_stddev.Get(i) ) / 4.0;
		
		double mean_value = (tf_mean_av - w1_tf_mean_av) / fabs(w1_tf_mean_av) / 100.0;
		mean.Set(i, mean_value);
		
		if (w1_stddev_av != 0.0) {
			i = i;
		}
		double w1val = 1 - NormalCDF(0, w1_mean_av, w1_stddev_av) - 0.5;
		double tfval = 1 - NormalCDF(0, tf_mean_av, tf_stddev_av) - 0.5;
		double cdf_value = tfval - w1val;
		cdf.Set(i, cdf_value);
	}
}






















WdayHourWeekAdjusted::WdayHourWeekAdjusted() {
	
}

void WdayHourWeekAdjusted::Init() {
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
	
	AddSubCore<WdayHourErrorAdjusted>();
}

void WdayHourWeekAdjusted::Start() {
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
	
	if (period >= 7*24*60*60) throw DataExc();
	
	ConstBuffer& other_open = GetInputBuffer(0, 0);
	BaseSystem& bs = GetBaseSystem();
	
	Core& wdayhourerradj = At(0);
	Core& weekstatserradj = *Get<WeekStatsErrorAdjusted>();
	
	ConstBuffer& wd_t_pre_mean		= wdayhourerradj.GetBuffer(0);
	ConstBuffer& wd_t_pre_stddev		= wdayhourerradj.GetBuffer(1);
	ConstBuffer& wd_h_pre_mean		= wdayhourerradj.GetBuffer(2);
	ConstBuffer& wd_h_pre_stddev		= wdayhourerradj.GetBuffer(3);
	ConstBuffer& wd_d_pre_mean		= wdayhourerradj.GetBuffer(4);
	ConstBuffer& wd_d_pre_stddev		= wdayhourerradj.GetBuffer(5);
	ConstBuffer& wd_dh_pre_mean		= wdayhourerradj.GetBuffer(6);
	ConstBuffer& wd_dh_pre_stddev	= wdayhourerradj.GetBuffer(7);
	
	ConstBuffer& week_w_pre_mean		= weekstatserradj.GetBuffer(0);
	ConstBuffer& week_w_pre_stddev	= weekstatserradj.GetBuffer(1);
	ConstBuffer& week_m_pre_mean		= weekstatserradj.GetBuffer(2);
	ConstBuffer& week_m_pre_stddev	= weekstatserradj.GetBuffer(3);
	ConstBuffer& week_q_pre_mean		= weekstatserradj.GetBuffer(4);
	ConstBuffer& week_q_pre_stddev	= weekstatserradj.GetBuffer(5);
	ConstBuffer& week_y_pre_mean		= weekstatserradj.GetBuffer(6);
	ConstBuffer& week_y_pre_stddev	= weekstatserradj.GetBuffer(7);
	
	int this_period = GetPeriod();
	int w1_period = weekstatserradj.GetPeriod();
	
	double div = (5*24*60 / period);
	
	for ( int i = counted; i < bars; i++) {
		int w1_shift = bs.GetShift(this_period, w1_period, i);
		
		if (w1_shift >= week_w_pre_mean.GetCount())
			break;
		
		// Make averages
		double w1_mean_av = (
			week_w_pre_mean.Get(w1_shift) +
			week_m_pre_mean.Get(w1_shift) +
			week_q_pre_mean.Get(w1_shift) +
			week_y_pre_mean.Get(w1_shift) ) / 4.0;
		
		double w1_stddev_av = (
			week_w_pre_stddev.Get(w1_shift) +
			week_m_pre_stddev.Get(w1_shift) +
			week_q_pre_stddev.Get(w1_shift) +
			week_y_pre_stddev.Get(w1_shift) ) / 4.0;
		
		double w1_tf_mean_av = w1_mean_av / div;
		double w1_tf_stddev_av = w1_stddev_av / div;
		
		t_pre_mean.Set(i,    wd_t_pre_mean.Get(i) + w1_tf_mean_av);
		h_pre_mean.Set(i,    wd_h_pre_mean.Get(i) + w1_tf_mean_av);
		d_pre_mean.Set(i,    wd_d_pre_mean.Get(i) + w1_tf_mean_av);
		dh_pre_mean.Set(i,   wd_dh_pre_mean.Get(i) + w1_tf_mean_av);
		
		t_pre_stddev.Set(i,  wd_t_pre_stddev.Get(i) + w1_tf_stddev_av);
		h_pre_stddev.Set(i,  wd_h_pre_stddev.Get(i) + w1_tf_stddev_av);
		d_pre_stddev.Set(i,  wd_d_pre_stddev.Get(i) + w1_tf_stddev_av);
		dh_pre_stddev.Set(i, wd_dh_pre_stddev.Get(i) + w1_tf_stddev_av);
	}
}




















SubTfChanges::SubTfChanges() {
	other_counted = 0;
	var_period = 10;
}

void SubTfChanges::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,0,127));
	SetBufferColor(3, Color(0,0,127));
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(3, 2);
	
	// TODO get tf-id from periods
	BaseSystem& bs = GetBaseSystem();
	if (GetTimeframe() == bs.GetPeriodCount()-1) return;
	int tf_period = GetBaseSystem().GetPeriod(GetTimeframe()+1);
	bool force_d0 = tf_period >= 7*24*60;
	wdayhour   .SetCount(force_d0 ? 1 : 5*24*60 / tf_period);
	abswdayhour.SetCount(force_d0 ? 1 : 5*24*60 / tf_period);
	
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(abswdayhour);
}

void SubTfChanges::Start() {
	Buffer& mean = GetBuffer(0);
	Buffer& stddev = GetBuffer(1);
	Buffer& absmean = GetBuffer(2);
	Buffer& absstddev = GetBuffer(3);
	BaseSystem& bs = GetBaseSystem();
	
	if (GetTimeframe() == bs.GetPeriodCount()-1) return;
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	if (period >= 7*24*60) return;
	
	DUMPM(inputs[0].sources);
	int other_tf = GetTimeframe() + 1;
	ConstBuffer& other_open = GetInputBuffer(0, GetSymbol(), other_tf, 0);
	
	int this_period = GetPeriod();
	int other_period = bs.GetPeriod(GetTimeframe()+1);
	
	// If already processed
	int other_bars = bs.GetCount(other_period);
	if (other_counted >= other_bars)
		return;
	
	//other_bars--;
	
	double prev_open = 0;
	int prev_shift = bs.GetShift(other_period, this_period, other_counted); // TODO: check this, pos maybe should be -1
	prev_open = other_open.Get(prev_shift);
	
	for (int i = other_counted; i < other_bars; i++) {
		Time time = bs.GetTime(other_period, i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		if (!time.IsValid() || d < 0 || h >= h_count) continue; //TODO: solve the reason for these errors
		int dh = h + d * h_count;
		if (force_d0) {
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		MovingOnlineVariance& var     = wdayhour[dh];
		MovingOnlineVariance& absvar  = abswdayhour[dh];
		
		int shift = bs.GetShift(other_period, this_period, i);
		if (shift >= mean.GetCount()) // The counter may be the exact size of buffer but not more
			continue;
		
		double change = other_open.Get(i) - prev_open;
		if (change != 0.0) {
			var.AddResult(change);
			absvar.AddResult(fabs(change));
		}
		
		if (prev_shift != shift) {
			
			ASSERT(prev_shift < shift);
			prev_open = other_open.Get(i);
			
			mean.Set(shift, var.GetMean());
			stddev.Set(shift, var.GetDeviation());
			absmean.Set(shift, absvar.GetMean());
			absstddev.Set(shift, absvar.GetDeviation());
			
			var.Next();
			absvar.Next();
		}
		prev_shift = shift;
		
	}
	other_counted = other_bars;
}





















MetaTfChanges::MetaTfChanges() {
	offset = 0;
	prev_open_time = Time(1970,1,1);
	prev_open = 0;
	var_period = 10;
}

void MetaTfChanges::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,0,127));
	SetBufferColor(3, Color(0,0,127));
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(3, 2);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	abswdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(abswdayhour);
}

void MetaTfChanges::Start() {
	Buffer& stddev		= GetBuffer(0);
	Buffer& mean		= GetBuffer(1);
	Buffer& absstddev	= GetBuffer(2);
	Buffer& absmean		= GetBuffer(3);
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	if (period >= 24*60)
		return;
	
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	// If already processed
	if (counted >= bars)
		return;
	
	//bars--;
	
	BaseSystem& bs = GetBaseSystem();
	
	for ( int i = counted; i < bars; i++) {
		Time time = bs.GetTime(GetPeriod(), i);
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		if (force_d0) {
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		MovingOnlineVariance& var  = wdayhour[dh];
		MovingOnlineVariance& absvar  = abswdayhour[dh];
		
		if (period < 60) {
			if (time.minute == offset || time - 60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else if (period < 24*60) {
			if (time.hour == offset || time - 24*60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else if (period < 7*24*60) {
			if (DayOfWeek(time) == offset || time - 7*24*60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else Panic("Period error");
		
		double change = open.Get(i+1) - prev_open;
		if (change != 0.0) {
			var.AddResult(change);
			absvar.AddResult(fabs(change));
		}
		
		mean.Set(i, var.GetMean());
		stddev.Set(i, var.GetDeviation());
		absmean.Set(i, absvar.GetMean());
		absstddev.Set(i, absvar.GetDeviation());
		
		if (prev_open == 0)
			continue;
		
		var.Next();
		absvar.Next();
	}
}













HourHeat::HourHeat() {
	
}

void HourHeat::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(127,127,0));
	SetBufferColor(1, Color(0,127,127));
	SetBufferColor(2, Color(127,0,127));
	SetBufferLineWidth(2, 2);
}

void HourHeat::Start() {
	BaseSystem& bs = GetBaseSystem();
	Buffer& sub = GetBuffer(0);
	Buffer& day = GetBuffer(1);
	Buffer& sum = GetBuffer(2);
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	
	if (period >= 7*24*60*60) throw DataExc();
	
	// If already processed
	if (counted >= sub.GetCount())
		throw DataExc();
	
	Core& subtf = At(0);
	Core& metatf = At(1);
	
	ConstBuffer& submean		= GetInputBuffer(1, 2);
	ConstBuffer& substddev		= GetInputBuffer(1, 3);
	ConstBuffer& metamean		= GetInputBuffer(2, 2);
	ConstBuffer& metastddev		= GetInputBuffer(2, 3);
	
	for (int i = counted; i < bars; i++) {
		Time t = bs.GetTime(GetPeriod(), i);
		
		double sm = submean.Get(i);
		double ss = substddev.Get(i);
		double ssum = sm + ss;
		sub.Set(i, ssum);
		
		double diff = 0;
		if (t.hour != 0 && i > 0) {
			double mm = metamean.Get(i);
			double ms = metastddev.Get(i);
			double msum = mm + ms;
			
			double mm2 = metamean.Get(i-1);
			double ms2 = metastddev.Get(i-1);
			double msum2 = mm2 + ms2;
			
			diff = msum - msum2;
			day.Set(i, diff);
		}
		
		sum.Set(i, ssum + diff);
	}
}



























MetaTfCDF::MetaTfCDF() {
	offset = 0;
	prev_open_time = Time(1970,1,1);
	prev_open = 0;
	var_period = 10;
}

void MetaTfCDF::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Color(0,127,0));
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(wdayhour);
}

void MetaTfCDF::Start() {
	BaseSystem& bs = GetBaseSystem();
	Buffer& value = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	if (period >= 24*60)
		throw DataExc();
	
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	// If already processed
	if (counted >= bars)
		throw DataExc();
	
	if (counted) counted--;
	//bars--;
	
	for ( int i = counted; i < bars; i++) {
		Time time = bs.GetTime(GetPeriod(), i);
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time) - 1;
		int dh = h + d * h_count;
		if (force_d0) {
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		MovingOnlineVariance& var  = wdayhour[dh];
		
		if (period < 60) {
			if (time.minute == offset || time - 60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else if (period < 24*60) {
			if (time.hour == offset || time - 24*60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else if (period < 7*24*60) {
			if (DayOfWeek(time) == offset || time - 7*24*60*60 >= prev_open_time) {
				prev_open = open.Get(i);
				prev_open_time = time;
			}
		}
		else Panic("Period error");
		
		double change = open.Get(i+1) - prev_open;
		double cdf = var.GetCDF(change, 0);
		double v = fabs(cdf-0.5)*2;
		
		var.AddResult(change);
		
		value.Set(i, v);
		
		if (prev_open == 0)
			continue;
		
		var.Next();
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
	
	for ( int i = counted; i < bars; i++) {
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
		
		double change = open.Get(i+1) - open.Get(i);
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
	
	Core& whtrend = At(0);
	
	ConstBuffer& shift1_mean = GetInputBuffer(1, 0);
	ConstBuffer& shift2_mean = GetInputBuffer(1, 2);
	ConstBuffer& shift3_mean = GetInputBuffer(1, 4);
	
	ConstBuffer& shift1_stddev = GetInputBuffer(1, 1);
	ConstBuffer& shift2_stddev = GetInputBuffer(1, 3);
	ConstBuffer& shift3_stddev = GetInputBuffer(1, 5);
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	//bars--;
	
	for ( int i = counted; i < bars; i++) {
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
		double change = open.Get(i+1) - open.Get(i);
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
















EventOsc::EventOsc() {
	mul = 0.8;
	emgr = NULL;
	counted_events = 0;
}

void EventOsc::Init() {
	if (mul <= 0 || mul > 0.9) throw DataExc();
	
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	
	
	BridgeCore* bbd = link->link.Get<BridgeCore>();
	if (bbd) {
		keys.Add(bbd->GetKey0());
		keys.Add(bbd->GetKey1());
	}
	
	BridgeMeshConverter* bmc = link->link.Get<BridgeMeshConverter>();
	if (bmc) {
		keys.Add(bmc->GetKey());
	}
	
	if (keys.IsEmpty()) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
}

void EventOsc::Start() {
	Buffer& high	= GetBuffer(0);
	Buffer& med		= GetBuffer(1);
	Buffer& low		= GetBuffer(2);
	Buffer& info	= GetBuffer(3);
	int bars = GetBars();
	
	int count = emgr->GetCount();
	
	if (!count) {
		emgr->Refresh();
		count = emgr->GetCount();
	}
	
	for(int i = counted_events; i < count; i++) {
		Event& e = emgr->GetEvent(i);
		if (keys.Find(e.currency) != -1) {
			Vector<double>& buf = (e.impact == 0 ? info : (e.impact == 1 ? low : (e.impact == 2 ? med : high)));
			int shift = GetTime().GetShiftFromTime(e.timestamp, GetPeriod());
			if (shift < 0 || shift >= bars) continue;
			double value = 1.0;
			int j = 0;
			while (value >= 0.01) {
				int a = shift - j;
				int b = shift + j;
				if (a >= 0) buf.Inc(a, value);
				if (j && b < bars) buf.Inc(b, value);
				value *= mul;
				j++;
			}
		}
	}
	counted_events = count;
}












GroupEventOsc::GroupEventOsc() {
	mul = 0.8;
	feat_count = 0;
	ownmul = 10;
}

void GroupEventOsc::Init() {
	if (mul <= 0 || mul > 0.9) throw DataExc();
	if (ownmul < 0.01) throw DataExc();
	
	int period = GetPeriod();
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	PathResolver& res = GetResolver();
	PathLink* link;
	
	// Nodes only
	link = res.FindLinkPath("/nodes");
	if (!link) throw DataExc();
	
	feat_count = link->keys.GetCount();
	for(int i = 0; i < feat_count; i++) {
		int id = link->keys[i].keys[0].link->GetSymbol();
		if (id == -1) throw DataExc();
		if (RequireIdIndicator(id, period, "eosc", "mul", mul)) throw DataExc();
	}
	
	if (feat_count <= 1) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
}

void GroupEventOsc::Start() {
	Buffer& high	= GetBuffer(0);
	Buffer& med		= GetBuffer(1);
	Buffer& low		= GetBuffer(2);
	Buffer& info	= GetBuffer(3);
	
	int counted = GetCounted();
	int bars = GetBars();
	int id = GetSymbol();
	double mul = 1.0 / (feat_count - 1.0);
	double ownmul = mul * this->ownmul;
	for (int i = counted; i < bars; i++) {
		
		for(int j = 0; j < feat_count; j++) {
			Core& cont = At(j);
			if (j == id) {
				info.Inc(i, ownmul * cont.GetBuffer(3).Get(i));
				low.Inc(i,  ownmul * cont.GetBuffer(2).Get(i));
				med.Inc(i,  ownmul * cont.GetBuffer(1).Get(i));
				high.Inc(i, ownmul * cont.GetBuffer(0).Get(i));
			} else {
				info.Inc(i, mul * cont.GetBuffer(3).Get(i));
				low.Inc(i,  mul * cont.GetBuffer(2).Get(i));
				med.Inc(i,  mul * cont.GetBuffer(1).Get(i));
				high.Inc(i, mul * cont.GetBuffer(0).Get(i));
			}
		}
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
	
	if (RequireIndicator("feat")) throw DataExc();
	
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
	
	FeatureDetector& feat = dynamic_cast<FeatureDetector&>(At(0));
	
	int count = feat.GetKeypointCount();
	for(int i = counted_features; i < count; i++) {
		const FeatureKeypoint& kp = feat.GetKeypoint(i);
		int shift = kp.GetShift();
		int impact = kp.GetImpact();
		Vector<double>& buf = (impact == 0 ? info : (impact == 1 ? low : (impact == 2 ? med : high)));
		if (shift < 0 || shift >= bars) continue;
		double value = 1.0;
		int j = 0;
		while (value >= 0.01) {
			int a = shift - j;
			int b = shift + j;
			if (a >= 0) buf.Inc(a, value);
			if (j && b < bars) buf.Inc(b, value);
			value *= mul;
			j++;
		}
	}
	counted_features = count;
}






















GroupFeatureOsc::GroupFeatureOsc() {
	mul = 0.8;
	feat_count = 0;
	ownmul = 10;
}

void GroupFeatureOsc::Init() {
	if (mul <= 0 || mul > 0.9) throw DataExc();
	if (ownmul < 0.01) throw DataExc();
	
	int period = GetPeriod();
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	PathResolver& res = GetResolver();
	PathLink* link = res.FindLinkPath("/id");
	if (!link) throw DataExc();
	
	feat_count = link->keys.GetCount();
	for(int i = 0; i < feat_count; i++) {
		if (RequireIdIndicator(i, period, "fosc", "mul", mul)) throw DataExc();
	}
	
	if (feat_count <= 1) throw DataExc();
	
	SetCoreSeparateWindow();
	//SetBufferCount(4, 4);
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
}

void GroupFeatureOsc::Start() {
	Buffer& high	= GetBuffer(0);
	Buffer& med		= GetBuffer(1);
	Buffer& low		= GetBuffer(2);
	Buffer& info	= GetBuffer(3);
	int counted = GetCounted();
	int bars = GetBars();
	int id = GetSymbol();
	double mul = 1.0 / (feat_count - 1.0);
	double ownmul = mul * this->ownmul;
	for (int i = counted; i < bars; i++) {
		
		for(int j = 0; j < feat_count; j++) {
			Core& cont = At(j);
			if (j == id) {
				info.Inc(i, ownmul * cont.GetBuffer(3).Get(i));
				low.Inc(i,  ownmul * cont.GetBuffer(2).Get(i));
				med.Inc(i,  ownmul * cont.GetBuffer(1).Get(i));
				high.Inc(i, ownmul * cont.GetBuffer(0).Get(i));
			} else {
				info.Inc(i, mul * cont.GetBuffer(3).Get(i));
				low.Inc(i,  mul * cont.GetBuffer(2).Get(i));
				med.Inc(i,  mul * cont.GetBuffer(1).Get(i));
				high.Inc(i, mul * cont.GetBuffer(0).Get(i));
			}
		}
	}
}







ChannelPredicter::ChannelPredicter() {
	length = 4;
	
	//AddValue<double>("Hour Min");
	//AddValue<double>("Hour Max");
	//AddValue<double>("Day Min");
	//AddValue<double>("Day Max");
	//AddValue<double>("DayHour Min");
	//AddValue<double>("DayHour Max");
	//AddValue<double>("Average range");
}

String ChannelPredicter::GetStyle() const {
	return "";
}

void ChannelPredicter::Init() {
	//AddDependency("/whstat_slow", 0, 0);
}

void ChannelPredicter::Start() {
	/*const Core& whstat = GetDependency(0);
	double diff = 0.0;
	for(int j = 0; j < 6; j+=2) {
		double min_sum = 0;
		double max_sum = 0;
		for(int i = 0; i < length; i++) {
			double min	= *whstat.GetValue<double>(j+0, -1-i, attr);
			double max	= *whstat.GetValue<double>(j+1, -1-i, attr);
			/*ASSERT(max > min);
			ASSERT(max > 0);
			ASSERT(0 > min);*/
			/*min_sum += min;
			max_sum += max;
			diff += max - min;
		}
		*GetValue<double>(j+0, attr) = min_sum;
		*GetValue<double>(j+1, attr) = max_sum;
	}
	*GetValue<double>(6, attr) = diff / 3;
	return true;*/
	Panic("TODO");
}








	
}
