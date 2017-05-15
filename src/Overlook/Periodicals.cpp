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

void WdayHourChanges::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WdayHourChanges::Init() {
	SetCoreSeparateWindow();
	SetBufferCount(2);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(0,127,0));
	
	SetIndexCount(2);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, stddev);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	
	wdayhour.SetCount(force_d0 ? 1 : 7*24*60 / period);
	
}

void WdayHourChanges::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
		int h = (time.minute + time.hour * 60) / period;
		int d = DayOfWeek(time);
		int dh = h + d * h_count;
		
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		
		OnlineVariance& var = wdayhour[dh];
		
		double diff = SafeDiff(open.Get(i+1), open.Get(i));
		if (diff != 0.0) {
			var.AddResult(diff);
		}
		
		mean.Set(i,    var.GetMean());
		stddev.Set(i,  var.GetDeviation());
	}
	*/
}

const OnlineVariance& WdayHourChanges::GetOnlineVariance(int shift) {
	/*int period = GetMinutePeriod();
	Time time = GetTime().GetTime(GetPeriod(), shift);
	
	bool force_d0 = period >= 7*24*60;
	if (force_d0)
		return wdayhour[0];
	
	int h_count = 24 * 60 / period;
	int h = (time.minute + time.hour * 60) / period;
	int d = DayOfWeek(time);
	int dh = h + d * h_count;
	
	return wdayhour[dh];*/
}














WdayHourStats::WdayHourStats() {
	var_period = 10;
}

void WdayHourStats::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
	
}

void WdayHourStats::Init() {
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, t_pre_mean);
	SetIndexBuffer ( 1, t_pre_stddev);
	SetIndexBuffer ( 2, h_pre_mean);
	SetIndexBuffer ( 3, h_pre_stddev);
	SetIndexBuffer ( 4, d_pre_mean);
	SetIndexBuffer ( 5, d_pre_stddev);
	SetIndexBuffer ( 6, dh_pre_mean);
	SetIndexBuffer ( 7, dh_pre_stddev);
	
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
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
		
		double change = SafeDiff(open.Get(i+1), open.Get(i));
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
	*/
}























WdayHourDiff::WdayHourDiff() {
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
	/*
	Core::Init();
	
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, t_pre_mean);
	SetIndexBuffer ( 1, t_pre_stddev);
	SetIndexBuffer ( 2, h_pre_mean);
	SetIndexBuffer ( 3, h_pre_stddev);
	SetIndexBuffer ( 4, d_pre_mean);
	SetIndexBuffer ( 5, d_pre_stddev);
	SetIndexBuffer ( 6, dh_pre_mean);
	SetIndexBuffer ( 7, dh_pre_stddev);
	
	if (RequireIndicator("whstat", "period", var_period_fast)) throw DataExc();
	if (RequireIndicator("whstat", "period", var_period_fast + var_period_diff)) throw DataExc();
	*/
}

void WdayHourDiff::Start() {
	/*
	int counted = GetCounted();
	
	if (counted > 0) counted--;
	
	Core& a = At(0);
	const FloatVector& t_pre_stddev_fast	= a.GetIndex(0);
	const FloatVector& t_pre_mean_fast		= a.GetIndex(1);
	const FloatVector& h_pre_stddev_fast	= a.GetIndex(2);
	const FloatVector& h_pre_mean_fast		= a.GetIndex(3);
	const FloatVector& d_pre_stddev_fast	= a.GetIndex(4);
	const FloatVector& d_pre_mean_fast		= a.GetIndex(5);
	const FloatVector& dh_pre_stddev_fast	= a.GetIndex(6);
	const FloatVector& dh_pre_mean_fast		= a.GetIndex(7);
	
	Core& b = At(1);
	const FloatVector& t_pre_stddev_slow	= b.GetIndex(0);
	const FloatVector& t_pre_mean_slow		= b.GetIndex(1);
	const FloatVector& h_pre_stddev_slow	= b.GetIndex(2);
	const FloatVector& h_pre_mean_slow		= b.GetIndex(3);
	const FloatVector& d_pre_stddev_slow	= b.GetIndex(4);
	const FloatVector& d_pre_mean_slow		= b.GetIndex(5);
	const FloatVector& dh_pre_stddev_slow	= b.GetIndex(6);
	const FloatVector& dh_pre_mean_slow		= b.GetIndex(7);
	
	int count = min(min(GetBars(), t_pre_stddev_fast.GetCount()), t_pre_stddev_slow.GetCount());
	//ASSERT( count >= bars - 1 ); // If fails and needs to be skipped, remember to reduce GetCounted value
	
	for ( int i = counted; i < count; i++ ) {
		
		t_pre_mean.Set(i,    t_pre_mean_fast.Get(i)    - t_pre_mean_slow.Get(i));
		h_pre_mean.Set(i,    h_pre_mean_fast.Get(i)    - h_pre_mean_slow.Get(i));
		d_pre_mean.Set(i,    d_pre_mean_fast.Get(i)    - d_pre_mean_slow.Get(i));
		dh_pre_mean.Set(i,   dh_pre_mean_fast.Get(i)   - dh_pre_mean_slow.Get(i));
		
		t_pre_stddev.Set(i,  t_pre_stddev_fast.Get(i)  - t_pre_stddev_slow.Get(i));
		h_pre_stddev.Set(i,  h_pre_stddev_fast.Get(i)  - h_pre_stddev_slow.Get(i));
		d_pre_stddev.Set(i,  d_pre_stddev_fast.Get(i)  - d_pre_stddev_slow.Get(i));
		dh_pre_stddev.Set(i, dh_pre_stddev_fast.Get(i) - dh_pre_stddev_slow.Get(i));
		
	}
	*/
}




















WdayHourForecastErrors::WdayHourForecastErrors() {
	var_period = 10;
	
}

void WdayHourForecastErrors::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void WdayHourForecastErrors::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, t_pre_mean);
	SetIndexBuffer ( 1, t_pre_stddev);
	SetIndexBuffer ( 2, h_pre_mean);
	SetIndexBuffer ( 3, h_pre_stddev);
	SetIndexBuffer ( 4, d_pre_mean);
	SetIndexBuffer ( 5, d_pre_stddev);
	SetIndexBuffer ( 6, dh_pre_mean);
	SetIndexBuffer ( 7, dh_pre_stddev);
	
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
	
	if (RequireIndicator("whstat")) throw DataExc();
	
	VAR_PERIOD_VEC(total);
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(wday);
	VAR_PERIOD_VEC(hour);
	*/
}

void WdayHourForecastErrors::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& wdayhourstats = At(0);
	
	const FloatVector& stat_t_pre_mean		= wdayhourstats.GetBuffer(0);
	const FloatVector& stat_t_pre_stddev	= wdayhourstats.GetBuffer(1);
	const FloatVector& stat_h_pre_mean		= wdayhourstats.GetBuffer(2);
	const FloatVector& stat_h_pre_stddev	= wdayhourstats.GetBuffer(3);
	const FloatVector& stat_d_pre_mean		= wdayhourstats.GetBuffer(4);
	const FloatVector& stat_d_pre_stddev	= wdayhourstats.GetBuffer(5);
	const FloatVector& stat_dh_pre_mean		= wdayhourstats.GetBuffer(6);
	const FloatVector& stat_dh_pre_stddev	= wdayhourstats.GetBuffer(7);
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
		
		double change = open.Get(i+1) - open.Get(i);
		
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
	*/
}



























WdayHourErrorAdjusted::WdayHourErrorAdjusted() {
	
	
}

void WdayHourErrorAdjusted::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WdayHourErrorAdjusted::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, t_pre_mean);
	SetIndexBuffer ( 1, t_pre_stddev);
	SetIndexBuffer ( 2, h_pre_mean);
	SetIndexBuffer ( 3, h_pre_stddev);
	SetIndexBuffer ( 4, d_pre_mean);
	SetIndexBuffer ( 5, d_pre_stddev);
	SetIndexBuffer ( 6, dh_pre_mean);
	SetIndexBuffer ( 7, dh_pre_stddev);
	
	if (RequireIndicator("whstat")) throw DataExc();
	if (RequireIndicator("whfe")) throw DataExc();
	*/
}

void WdayHourErrorAdjusted::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& wdayhourstats = At(0);
	Core& wdayhourforecasterrors = At(1);
	
	const FloatVector& stat_t_pre_mean		= wdayhourstats.GetBuffer(0);
	const FloatVector& stat_t_pre_stddev	= wdayhourstats.GetBuffer(1);
	const FloatVector& stat_h_pre_mean		= wdayhourstats.GetBuffer(2);
	const FloatVector& stat_h_pre_stddev	= wdayhourstats.GetBuffer(3);
	const FloatVector& stat_d_pre_mean		= wdayhourstats.GetBuffer(4);
	const FloatVector& stat_d_pre_stddev	= wdayhourstats.GetBuffer(5);
	const FloatVector& stat_dh_pre_mean		= wdayhourstats.GetBuffer(6);
	const FloatVector& stat_dh_pre_stddev	= wdayhourstats.GetBuffer(7);
	
	const FloatVector& err_t_pre_mean		= wdayhourforecasterrors.GetBuffer(0);
	const FloatVector& err_t_pre_stddev		= wdayhourforecasterrors.GetBuffer(1);
	const FloatVector& err_h_pre_mean		= wdayhourforecasterrors.GetBuffer(2);
	const FloatVector& err_h_pre_stddev		= wdayhourforecasterrors.GetBuffer(3);
	const FloatVector& err_d_pre_mean		= wdayhourforecasterrors.GetBuffer(4);
	const FloatVector& err_d_pre_stddev		= wdayhourforecasterrors.GetBuffer(5);
	const FloatVector& err_dh_pre_mean		= wdayhourforecasterrors.GetBuffer(6);
	const FloatVector& err_dh_pre_stddev	= wdayhourforecasterrors.GetBuffer(7);
	
	for ( int i = counted; i < bars; i++ ) {
		
		t_pre_mean.Set(i,    stat_t_pre_mean.Get(i) + stat_t_pre_stddev.Get(i) * err_t_pre_mean.Get(i));
		h_pre_mean.Set(i,    stat_h_pre_mean.Get(i) + stat_h_pre_stddev.Get(i) * err_h_pre_mean.Get(i));
		d_pre_mean.Set(i,    stat_d_pre_mean.Get(i) + stat_d_pre_stddev.Get(i) * err_d_pre_mean.Get(i));
		dh_pre_mean.Set(i,   stat_dh_pre_mean.Get(i) + stat_dh_pre_stddev.Get(i) * err_dh_pre_mean.Get(i));
		
		t_pre_stddev.Set(i,  stat_t_pre_stddev.Get(i) * err_t_pre_stddev.Get(i));
		h_pre_stddev.Set(i,  stat_h_pre_stddev.Get(i) * err_h_pre_stddev.Get(i));
		d_pre_stddev.Set(i,  stat_d_pre_stddev.Get(i) * err_d_pre_stddev.Get(i));
		dh_pre_stddev.Set(i, stat_dh_pre_stddev.Get(i) * err_dh_pre_stddev.Get(i));
		
	}
	*/
}




























WeekStats::WeekStats() {
	var_period = 10;
	
	
}

void WeekStats::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void WeekStats::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, w_pre_mean);
	SetIndexBuffer ( 1, w_pre_stddev);
	SetIndexBuffer ( 2, m_pre_mean);
	SetIndexBuffer ( 3, m_pre_stddev);
	SetIndexBuffer ( 4, q_pre_mean);
	SetIndexBuffer ( 5, q_pre_stddev);
	SetIndexBuffer ( 6, y_pre_mean);
	SetIndexBuffer ( 7, y_pre_stddev);
	
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	month.SetCount(force_d0 ? 1 : 5);
	quarter.SetCount(force_d0 ? 1 : 14);
	year.SetCount(force_d0 ? 1 : 54);
	
	VAR_PERIOD(week);
	VAR_PERIOD_VEC(month);
	VAR_PERIOD_VEC(quarter);
	VAR_PERIOD_VEC(year);
	*/
}

void WeekStats::Start() {
	/*
	int bars = GetBars() - 1;
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		throw DataExc();
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	for (int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
	*/
}





































WeekStatsForecastErrors::WeekStatsForecastErrors() {
	var_period = 10;
	
}

void WeekStatsForecastErrors::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void WeekStatsForecastErrors::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, w_pre_mean);
	SetIndexBuffer ( 1, w_pre_stddev);
	SetIndexBuffer ( 2, m_pre_mean);
	SetIndexBuffer ( 3, m_pre_stddev);
	SetIndexBuffer ( 4, q_pre_mean);
	SetIndexBuffer ( 5, q_pre_stddev);
	SetIndexBuffer ( 6, y_pre_mean);
	SetIndexBuffer ( 7, y_pre_stddev);
	
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
	
	if (RequireIndicator("ws")) throw DataExc();
	
	VAR_PERIOD_VEC(week);
	VAR_PERIOD_VEC(month);
	VAR_PERIOD_VEC(quarter);
	VAR_PERIOD_VEC(year);
	*/
}

void WeekStatsForecastErrors::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		throw DataExc();
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& weekstats = At(0);
	
	FloatVector& stat_w_pre_mean		= weekstats.GetBuffer(0);
	FloatVector& stat_w_pre_stddev	= weekstats.GetBuffer(1);
	FloatVector& stat_m_pre_mean		= weekstats.GetBuffer(2);
	FloatVector& stat_m_pre_stddev	= weekstats.GetBuffer(3);
	FloatVector& stat_q_pre_mean		= weekstats.GetBuffer(4);
	FloatVector& stat_q_pre_stddev	= weekstats.GetBuffer(5);
	FloatVector& stat_y_pre_mean		= weekstats.GetBuffer(6);
	FloatVector& stat_y_pre_stddev	= weekstats.GetBuffer(7);
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
		
		double change = open.Get(i+1) - open.Get(i);
		if (change != 0.0) {
			w_var.AddResult( min(2.0, max(-2.0, (change - stat_w_pre_mean.Get(i)) / stat_w_pre_stddev.Get(i) )) );
			m_var.AddResult( min(2.0, max(-2.0, (change - stat_m_pre_mean.Get(i)) / stat_m_pre_stddev.Get(i) )) );
			q_var.AddResult( min(2.0, max(-2.0, (change - stat_q_pre_mean.Get(i)) / stat_q_pre_stddev.Get(i) )) );
			y_var.AddResult( min(2.0, max(-2.0, (change - stat_y_pre_mean.Get(i)) / stat_y_pre_stddev.Get(i) )) );
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
	*/
}



























WeekStatsErrorAdjusted::WeekStatsErrorAdjusted() {
	
	
}

void WeekStatsErrorAdjusted::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WeekStatsErrorAdjusted::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, w_pre_mean);
	SetIndexBuffer ( 1, w_pre_stddev);
	SetIndexBuffer ( 2, m_pre_mean);
	SetIndexBuffer ( 3, m_pre_stddev);
	SetIndexBuffer ( 4, q_pre_mean);
	SetIndexBuffer ( 5, q_pre_stddev);
	SetIndexBuffer ( 6, y_pre_mean);
	SetIndexBuffer ( 7, y_pre_stddev);
	
	if (RequireIndicator("ws")) throw DataExc();
	if (RequireIndicator("wsfe")) throw DataExc();
	
	if (GetTime().GetEndTS() - GetTime().GetBeginTS()
		< 365*24*60*60) {
		LOG("WeekStatsErrorAdjusted Init error: requires data at least one year");
		throw DataExc();
	}
	*/
}

void WeekStatsErrorAdjusted::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	bool force_d0 = period < 7*24*60;
	
	if (force_d0)
		throw DataExc();
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& weekstats = At(0);
	Core& weekstatsforecasterrors = At(1);
	
	FloatVector& stat_w_pre_mean	= weekstats.GetBuffer(0);
	FloatVector& stat_w_pre_stddev	= weekstats.GetBuffer(1);
	FloatVector& stat_m_pre_mean	= weekstats.GetBuffer(2);
	FloatVector& stat_m_pre_stddev	= weekstats.GetBuffer(3);
	FloatVector& stat_q_pre_mean	= weekstats.GetBuffer(4);
	FloatVector& stat_q_pre_stddev	= weekstats.GetBuffer(5);
	FloatVector& stat_y_pre_mean	= weekstats.GetBuffer(6);
	FloatVector& stat_y_pre_stddev	= weekstats.GetBuffer(7);
	
	FloatVector& err_w_pre_mean		= weekstatsforecasterrors.GetBuffer(0);
	FloatVector& err_w_pre_stddev	= weekstatsforecasterrors.GetBuffer(1);
	FloatVector& err_m_pre_mean		= weekstatsforecasterrors.GetBuffer(2);
	FloatVector& err_m_pre_stddev	= weekstatsforecasterrors.GetBuffer(3);
	FloatVector& err_q_pre_mean		= weekstatsforecasterrors.GetBuffer(4);
	FloatVector& err_q_pre_stddev	= weekstatsforecasterrors.GetBuffer(5);
	FloatVector& err_y_pre_mean		= weekstatsforecasterrors.GetBuffer(6);
	FloatVector& err_y_pre_stddev	= weekstatsforecasterrors.GetBuffer(7);
	
	
	
	for ( int i = counted; i < bars; i++ ) {
		
		w_pre_mean.Set(i,    stat_w_pre_mean.Get(i) + stat_w_pre_stddev.Get(i) * err_w_pre_mean.Get(i));
		m_pre_mean.Set(i,    stat_m_pre_mean.Get(i) + stat_m_pre_stddev.Get(i) * err_m_pre_mean.Get(i));
		q_pre_mean.Set(i,    stat_q_pre_mean.Get(i) + stat_q_pre_stddev.Get(i) * err_q_pre_mean.Get(i));
		y_pre_mean.Set(i,    stat_y_pre_mean.Get(i) + stat_y_pre_stddev.Get(i) * err_y_pre_mean.Get(i));
		
		w_pre_stddev.Set(i,    stat_w_pre_stddev.Get(i) * err_w_pre_stddev.Get(i));
		m_pre_stddev.Set(i,    stat_m_pre_stddev.Get(i) * err_m_pre_stddev.Get(i));
		q_pre_stddev.Set(i,    stat_q_pre_stddev.Get(i) * err_q_pre_stddev.Get(i));
		y_pre_stddev.Set(i,    stat_y_pre_stddev.Get(i) * err_y_pre_stddev.Get(i));
		
	}
	*/
}






















WdayHourDiffWeek::WdayHourDiffWeek() {
	
}

void WdayHourDiffWeek::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WdayHourDiffWeek::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(2);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(0,127,0));
	SetBufferLineWidth(1, 2);
	
	SetIndexCount(2);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, cdf);
	
	SetCoreLevelCount(2);
	SetCoreLevel(0,  0.5);
	SetCoreLevel(1, -0.5);
	SetCoreLevelsColor(Color(192, 192, 192));
	SetCoreLevelsStyle(STYLE_DOT);
	
	if (RequireIndicator("whea")) throw DataExc();
	if (RequirePeriodIndicator(7*24*60*60, "wsea")) throw DataExc();
	
	SetCoreMaximum(1.0);
	SetCoreMinimum(-1.0);
	*/
}

void WdayHourDiffWeek::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	
	if (period >= 7*24*60*60) throw DataExc();
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& wdayhourerradj = At(0);
	Core& weekstatserradj = At(1);
	
	LOG("WdayHourDiffWeek::Start");
	int a = wdayhourerradj.GetPeriod();
	int b = GetPeriod();
	ASSERT(a == b);
	
	FloatVector& wd_t_pre_mean		= wdayhourerradj.GetBuffer(0);
	FloatVector& wd_t_pre_stddev	= wdayhourerradj.GetBuffer(1);
	FloatVector& wd_h_pre_mean		= wdayhourerradj.GetBuffer(2);
	FloatVector& wd_h_pre_stddev	= wdayhourerradj.GetBuffer(3);
	FloatVector& wd_d_pre_mean		= wdayhourerradj.GetBuffer(4);
	FloatVector& wd_d_pre_stddev	= wdayhourerradj.GetBuffer(5);
	FloatVector& wd_dh_pre_mean		= wdayhourerradj.GetBuffer(6);
	FloatVector& wd_dh_pre_stddev	= wdayhourerradj.GetBuffer(7);
	
	FloatVector& week_w_pre_mean	= weekstatserradj.GetBuffer(0);
	FloatVector& week_w_pre_stddev	= weekstatserradj.GetBuffer(1);
	FloatVector& week_m_pre_mean	= weekstatserradj.GetBuffer(2);
	FloatVector& week_m_pre_stddev	= weekstatserradj.GetBuffer(3);
	FloatVector& week_q_pre_mean	= weekstatserradj.GetBuffer(4);
	FloatVector& week_q_pre_stddev	= weekstatserradj.GetBuffer(5);
	FloatVector& week_y_pre_mean	= weekstatserradj.GetBuffer(6);
	FloatVector& week_y_pre_stddev	= weekstatserradj.GetBuffer(7);
	
	int this_period = GetPeriod();
	int w1_period = weekstatserradj.GetPeriod();
	int w1_count = GetTime().GetCount(w1_period);
	int week_w_pre_mean_count = week_w_pre_mean.GetCount();
	ASSERT(week_w_pre_mean_count == w1_count);
	
	if (counted) counted--;
	
	double div = (5*24*60 / period);
	
	for ( int i = counted; i < bars; i++ ) {
		int w1_shift = GetTime().GetShift(this_period, w1_period, i);
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
		
		double mean = (tf_mean_av - w1_tf_mean_av) / fabs(w1_tf_mean_av) / 100.0;
		this->mean.Set(i, mean);
		
		if (w1_stddev_av != 0.0) {
			i = i;
		}
		double w1val = 1 - NormalCDF(0, w1_mean_av, w1_stddev_av) - 0.5;
		double tfval = 1 - NormalCDF(0, tf_mean_av, tf_stddev_av) - 0.5;
		double cdf = tfval - w1val;
		this->cdf.Set(i, cdf);
		
	}
	*/
}






















WdayHourWeekAdjusted::WdayHourWeekAdjusted() {
	
	
}

void WdayHourWeekAdjusted::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WdayHourWeekAdjusted::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,127,0));
	SetBufferColor(4, Color(0,0,127));
	SetBufferColor(5, Color(0,0,127));
	SetBufferColor(6, Color(0,127,127));
	SetBufferColor(7, Color(0,127,127));
	SetBufferLineWidth(7, 2);
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, t_pre_mean);
	SetIndexBuffer ( 1, t_pre_stddev);
	SetIndexBuffer ( 2, h_pre_mean);
	SetIndexBuffer ( 3, h_pre_stddev);
	SetIndexBuffer ( 4, d_pre_mean);
	SetIndexBuffer ( 5, d_pre_stddev);
	SetIndexBuffer ( 6, dh_pre_mean);
	SetIndexBuffer ( 7, dh_pre_stddev);
	
	
	if (RequireIndicator("whea")) throw DataExc();
	if (RequirePeriodIndicator(7*24*60*60, "wsea")) throw DataExc();
	*/
}

void WdayHourWeekAdjusted::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	
	if (period >= 7*24*60*60) throw DataExc();
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	Core& wdayhourerradj = At(0);
	Core& weekstatserradj = At(1);
	
	FloatVector& wd_t_pre_mean		= wdayhourerradj.GetBuffer(0);
	FloatVector& wd_t_pre_stddev		= wdayhourerradj.GetBuffer(1);
	FloatVector& wd_h_pre_mean		= wdayhourerradj.GetBuffer(2);
	FloatVector& wd_h_pre_stddev		= wdayhourerradj.GetBuffer(3);
	FloatVector& wd_d_pre_mean		= wdayhourerradj.GetBuffer(4);
	FloatVector& wd_d_pre_stddev		= wdayhourerradj.GetBuffer(5);
	FloatVector& wd_dh_pre_mean		= wdayhourerradj.GetBuffer(6);
	FloatVector& wd_dh_pre_stddev	= wdayhourerradj.GetBuffer(7);
	
	FloatVector& week_w_pre_mean		= weekstatserradj.GetBuffer(0);
	FloatVector& week_w_pre_stddev	= weekstatserradj.GetBuffer(1);
	FloatVector& week_m_pre_mean		= weekstatserradj.GetBuffer(2);
	FloatVector& week_m_pre_stddev	= weekstatserradj.GetBuffer(3);
	FloatVector& week_q_pre_mean		= weekstatserradj.GetBuffer(4);
	FloatVector& week_q_pre_stddev	= weekstatserradj.GetBuffer(5);
	FloatVector& week_y_pre_mean		= weekstatserradj.GetBuffer(6);
	FloatVector& week_y_pre_stddev	= weekstatserradj.GetBuffer(7);
	
	int this_period = GetPeriod();
	int w1_period = weekstatserradj.GetPeriod();
	
	double div = (5*24*60 / period);
	
	for ( int i = counted; i < bars; i++ ) {
		int w1_shift = GetTime().GetShift(this_period, w1_period, i);
		
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
	*/
}




















SubTfChanges::SubTfChanges() {
	other_counted = 0;
	var_period = 10;
	
}

void SubTfChanges::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void SubTfChanges::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(4);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,0,127));
	SetBufferColor(3, Color(0,0,127));
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(3, 2);
	
	SetIndexCount(4);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, stddev);
	SetIndexBuffer ( 2, absmean);
	SetIndexBuffer ( 3, absstddev);
	
	// TODO get tf-id from periods
	
	int tf_period = GetMinutePeriod();
	int sub_period;
	if (tf_period <= 60) sub_period = 15 * 60;
	else if (tf_period <= 240) sub_period = 60 * 60;
	else if (tf_period == 1440) sub_period = 240 * 60;
	else sub_period = 1440 * 60;
	
	if (RequirePeriod(sub_period)) throw DataExc();
	
	bool force_d0 = tf_period >= 7*24*60;
	wdayhour   .SetCount(force_d0 ? 1 : 5*24*60 / tf_period);
	abswdayhour.SetCount(force_d0 ? 1 : 5*24*60 / tf_period);
	
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(abswdayhour);
	*/
}

void SubTfChanges::Start() {
	/*
	Core& other_values = dynamic_cast<Core&>(At(0));
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	if (period >= 7*24*60*60) throw DataExc();
	
	FloatVector& other_open  = other_values.GetBuffer(0);
	
	int this_period = GetPeriod();
	int other_period = other_values.GetPeriod();
	
	// If already processed
	int other_bars = other_values.GetBars();
	if (other_counted >= other_bars)
		throw DataExc();
	
	other_bars--;
	
	double prev_open = 0;
	int prev_shift = GetTime().GetShift(other_period, this_period, other_counted);
	prev_open = other_open.Get(prev_shift);
	
	//int tc = other_time.GetCount();
	for ( int i = other_counted; i < other_bars; i++ ) {
		Time time = GetTime().GetTime(other_period, i);
		
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
		
		int shift = GetTime().GetShift(other_period, this_period, i);
		if (shift >= mean.GetCount()) // The counter may be the exact size of buffer but not more
			continue;
		
		double change = other_open.Get(i+1) - prev_open;
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
	*/
}





















MetaTfChanges::MetaTfChanges() {
	offset = 0;
	prev_open_time = Time(1970,1,1);
	prev_open = 0;
	var_period = 10;
}

void MetaTfChanges::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
}

void MetaTfChanges::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(4);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,0,127));
	SetBufferColor(3, Color(0,0,127));
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(3, 2);
	
	SetIndexCount(4);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, stddev);
	SetIndexBuffer ( 2, absmean);
	SetIndexBuffer ( 3, absstddev);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	abswdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(wdayhour);
	VAR_PERIOD_VEC(abswdayhour);
	*/
}

void MetaTfChanges::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	if (period >= 24*60)
		throw DataExc();
	
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	Core& bd = *GetSource().Get<Core>();
	const FloatVector& open  = bd.GetOpen();
	
	// If already processed
	if (counted >= bars)
		return 0;
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
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
	*/
}


































HourHeat::HourHeat() {
	
}

void HourHeat::SetArguments(const VectorMap<String, Value>& args) {
	
}

void HourHeat::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(3);
	SetBufferColor(0, Color(127,127,0));
	SetBufferColor(1, Color(0,127,127));
	SetBufferColor(2, Color(127,0,127));
	SetBufferLineWidth(2, 2);
	
	SetIndexCount(3);
	
	SetIndexBuffer (0, sub);
	SetIndexBuffer (1, day);
	SetIndexBuffer (2, sum);
	
	if (RequireIndicator("stfc")) throw DataExc();
	if (RequireIndicator("mtfc")) throw DataExc();
	*/
}

void HourHeat::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	
	if (period >= 7*24*60*60) throw DataExc();
	
	// If already processed
	if (counted >= sub.GetCount())
		throw DataExc();
	
	Core& subtf = At(0);
	Core& metatf = At(1);
	
	FloatVector& submean = subtf.GetBuffer(2);
	FloatVector& substddev = subtf.GetBuffer(3);
	FloatVector& metamean = metatf.GetBuffer(2);
	FloatVector& metastddev = metatf.GetBuffer(3);
	
	for (int i = counted; i < bars; i++ ) {
		Time t = GetTime().GetTime(GetPeriod(), i);
		
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
	*/
}



























MetaTfCDF::MetaTfCDF() {
	offset = 0;
	prev_open_time = Time(1970,1,1);
	prev_open = 0;
	var_period = 10;
}

void MetaTfCDF::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("offset");
	if (i != -1)
		offset = args[i];
	
	i = args.Find("prev_open_time");
	if (i != -1)
		prev_open_time = args[i];
	
	i = args.Find("prev_open");
	if (i != -1)
		prev_open = args[i];
	
	i = args.Find("period");
	if (i != -1)
		var_period = args[i];
	
}

void MetaTfCDF::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(1);
	SetBufferColor(0, Color(0,127,0));
	
	SetIndexCount(1);
	
	SetIndexBuffer ( 0, value);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	wdayhour.SetCount(force_d0 ? 1 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(wdayhour);
	*/
}

void MetaTfCDF::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	if (period >= 24*60)
		throw DataExc();
	
	bool force_d0 = period >= 7*24*60;
	int h_count = 24 * 60 / period; // originally hour only
	
	Core& bd = *GetSource().Get<Core>();
	const FloatVector& open = bd.GetOpen();
	
	// If already processed
	if (counted >= bars)
		throw DataExc();
	
	if (counted) counted--;
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
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
	*/
}



























WdayHourTrending::WdayHourTrending() {
	var_period = 10;
}

void WdayHourTrending::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
	
}

void WdayHourTrending::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(8);
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
	
	SetIndexCount(8);
	
	SetIndexBuffer ( 0, shift1_mean);
	SetIndexBuffer ( 1, shift1_stddev);
	SetIndexBuffer ( 2, shift2_mean);
	SetIndexBuffer ( 3, shift2_stddev);
	SetIndexBuffer ( 4, shift3_mean);
	SetIndexBuffer ( 5, shift3_stddev);
	SetIndexBuffer ( 6, mean_av);
	SetIndexBuffer ( 7, stddev_av);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	s1.SetCount(force_d0 ? 54 : 5*24*60 / period);
	s2.SetCount(force_d0 ? 54 : 5*24*60 / period);
	s3.SetCount(force_d0 ? 54 : 5*24*60 / period);
	
	VAR_PERIOD_VEC(s1);
	VAR_PERIOD_VEC(s2);
	VAR_PERIOD_VEC(s3);
	*/
}

void WdayHourTrending::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
	*/
}








WdayHourTrendSuccess::WdayHourTrendSuccess() {
	var_period = 10;
}

void WdayHourTrendSuccess::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("period");
	if (i != -1)
		var_period = args[i];
	
}

void WdayHourTrendSuccess::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(5);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	SetBufferColor(2, Color(0,127,0));
	SetBufferColor(3, Color(0,0,127));
	SetBufferColor(4, Color(127,0,127));
	SetBufferLineWidth(4, 2);
	
	SetIndexCount(5);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, stddev);
	SetIndexBuffer ( 2, success);
	SetIndexBuffer ( 3, erradj);
	SetIndexBuffer ( 4, dir);
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	vars.SetCount(force_d0 ? 54 : 5*24*60 / period);
	
	if (RequireIndicator("whtr")) throw DataExc();
	
	VAR_PERIOD_VEC(vars);
	*/
}

void WdayHourTrendSuccess::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	if (counted < 3) counted = 3;
	
	Core& whtrend = At(0);
	
	FloatVector& shift1_mean = whtrend.GetBuffer(0);
	FloatVector& shift2_mean = whtrend.GetBuffer(2);
	FloatVector& shift3_mean = whtrend.GetBuffer(4);
	
	FloatVector& shift1_stddev = whtrend.GetBuffer(1);
	FloatVector& shift2_stddev = whtrend.GetBuffer(3);
	FloatVector& shift3_stddev = whtrend.GetBuffer(5);
	
	Core& pb = *GetSource().Get<Core>();
	const FloatVector& open  = pb.GetOpen();
	
	bars--;
	
	for ( int i = counted; i < bars; i++ ) {
		Time time = GetTime().GetTime(GetPeriod(), i);
		
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
	*/
}








OpportinityQuality::OpportinityQuality() {
	weights.SetCount(4, 1);
	weights[0] = 0.5;
	weights[1] = 100;
	weights[2] = 0;
	weights[3] = 0.5;
}

void OpportinityQuality::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("w0");
	if (i != -1)
		weights[0] = args[i];
	
	i = args.Find("w1");
	if (i != -1)
		weights[1] = args[i];
	
	i = args.Find("w2");
	if (i != -1)
		weights[2] = args[i];
	
	i = args.Find("w3");
	if (i != -1)
		weights[3] = args[i];
	
}

void OpportinityQuality::Init() {
	/*
	SetCoreSeparateWindow();
	SetBufferCount(1+weights.GetCount());
	SetBufferColor(0, Color(0,127,127));
	SetBufferLineWidth(0, 2);
	
	SetIndexCount(1+weights.GetCount());
	
	SetIndexBuffer (0, quality);
	
	buffers.SetCount(weights.GetCount());
	for(int i = 0; i < buffers.GetCount(); i++) {
		SetIndexBuffer(1+i, buffers[i]);
		SetBufferColor(1+i, Color(127+i*20,0,127-i*20));
	}
	
	if (RequireIndicator("mtcdf")) throw DataExc();
	if (RequireIndicator("hh")) throw DataExc();
	if (RequireIndicator("whts")) throw DataExc();
	if (RequireIndicator("costprob")) throw DataExc();
	*/
}

void OpportinityQuality::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	Core& mtfcdf = At(0);
	Core& hourheat = At(1);
	Core& trendsucc = At(2);
	Core& spreadprob = At(3);
	
	FloatVector& abuf = mtfcdf.GetBuffer(0);
	FloatVector& bbuf = hourheat.GetBuffer(2);
	FloatVector& cbuf_mean = trendsucc.GetBuffer(0);
	FloatVector& cbuf_stddev = trendsucc.GetBuffer(1);
	FloatVector& dbuf = spreadprob.GetBuffer(0);
	
	for ( int i = counted; i < bars; i++ ) {
		
		// More common value change is better
		// Only some big fundamental events make larger changes than distribution.
		//  - MetaTFCDF ... prefer low value
		double a = (1 - abuf.Get(i)) * weights[0];
		buffers[0].Set(i, a);
		
		// Prefer high activity times to overcome spreads
		//  - HourHeat  ... prefer high value
		double b = bbuf.Get(i) * weights[1];
		buffers[1].Set(i, b);
		
		// Prefer successful trending forecasts
		//   - WdayHourTrendSuccess ... prefer high values
		double c_cdf = 1 - NormalCDF(0, cbuf_mean.Get(i), cbuf_stddev.Get(i));
		double c = c_cdf * weights[2];
		buffers[2].Set(i, c);
		
		// Prefer low probability to lose against spreads
		//   - SpreadProbability ... prefer low values
		double d = dbuf.Get(i) * weights[3];
		buffers[3].Set(i, d);
		
		double value = a + b + c + d;
		
		quality.Set(i, value);
		
	}
	*/
}



EdgeStatistics::EdgeStatistics()
{
	period = 15;
	method = 0;
	slowing = 10;
	es_counted = 0;
	SetCoreSeparateWindow();
}

void EdgeStatistics::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("period");
	if (i != -1)
		period = args[i];
	i = args.Find("method");
	if (i != -1)
		method = args[i];
	i = args.Find("slowing");
	if (i != -1)
		slowing = args[i];
}

void EdgeStatistics::Init()
{
	/*
	int draw_begin;
	if (period < 2)
		throw DataExc();
	draw_begin = period - 1;
	SetBufferCount(1);
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	SetIndexCount(2);
	SetIndexBuffer(0, mean);
	SetIndexBuffer(1, dev);
	
	if (RequireIndicator("symlre", "period", period, "method", method, "slowing", slowing)) throw DataExc();
	
	vars.SetCount(period);
	*/
}

void EdgeStatistics::Start()
{
	/*
	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();
	
	int shift = period / 2;
	bars -= shift;
	
	Core& cont = At(0);
	cont.Refresh();
	
	FloatVector& dbl = cont.GetIndex(0);
	
	// Loop unprocessed data
	int begin = max(es_counted, period);
	for(int i = begin; i < bars; i++) {
		double d = dbl.Get(i);
		ASSERT(IsFin(d));
		int j = i % period;
		OnlineVariance& ov = vars[j];
		double mean = ov.GetMean();
		ASSERT(IsFin(mean));
		//if (IsNaN(mean)) mean = 0;
		this->mean.Set(i, mean);
		this->dev.Set(i, ov.GetDeviation());
		ov.AddResult(d);
	}
	
	// Find the most probable edge phase
	double highest_mean = -DBL_MAX;
	int highest_mean_pos = -1;
	for(int i = 0; i < period; i++) {
		OnlineVariance& ov = vars[i];
		double mean = ov.GetMean();
		if (mean > highest_mean) {
			highest_mean = mean;
			highest_mean_pos = i;
		}
	}
	ASSERT(highest_mean_pos != -1);
	
	
	
	// Update keypoints
	const int phase = highest_mean_pos;
	
	// - Get next phase position
	int phase_begin = (begin % period) == phase ?
		begin : // use current begin pos if in correct phase
		begin - (begin % period) + period + phase; // next periodical begin + phase
	
	es_counted = bars;
	*/
}























TideStatistics::TideStatistics()
{
	period = 15;
	//cont = 0;
	ts_counted = 0;
	SetCoreSeparateWindow();
}

void TideStatistics::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("period");
	if (i != -1)
		period = args[i];
}

void TideStatistics::Init()
{
	/*
	int draw_begin;
	if (period < 2)
		period = 15;
	draw_begin = period - 1;
	SetBufferCount(1);
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	SetIndexCount(2);
	SetIndexBuffer(0, mean);
	SetIndexBuffer(1, dev);
	
	vars.SetCount(period);
	*/
}

void TideStatistics::Start()
{
	/*
	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();
	
	// Can't process the newest data, until 'shift' amount of offset
	bars -= period;
	
	// Prepare values
	const FloatVector& dbl = GetSource().Get<Core>()->GetOpen();
	
	// Loop unprocessed data
	for(int i = ts_counted; i < bars; i++) {
		
		// Calculate values
		double d0 = dbl.Get(i);
		double d1 = dbl.Get(i+1);
		double d2 = dbl.Get(i+period);
		double diff1 = d1 - d0;
		double diff2 = d2 - d0;
		double mul = diff1 * diff2 * 100000; // Lower 'mul' means changing trend
		
		// Set values
		int j = i % period;
		OnlineVariance& var = vars[j];
		double m = var.GetMean();
		double d = var.GetDeviation();
		mean.Set(i, m);
		dev.Set(i, d);
		var.AddResult(mul);
	}
	
	// Find the most probable edge phase
	double highest_mean = -DBL_MAX;
	int highest_mean_pos = -1;
	for(int i = 0; i < period; i++) {
		OnlineVariance& ov = vars[i];
		double mean = -1.0 * ov.GetMean(); // <-- inverts value!!!
		if (mean > highest_mean) {
			highest_mean = mean;
			highest_mean_pos = i;
		}
	}
	ASSERT(highest_mean_pos != -1);
	
	
	// Update keypoints
	const int begin = ts_counted;
	const int phase = highest_mean_pos;
	
	// - Get next phase position
	int phase_begin = (begin % period) == phase ?
		begin : // use current begin pos if in correct phase
		begin - (begin % period) + period + phase; // next periodical begin + phase
	
	ts_counted = bars;
	*/
}



















Disconnections::Disconnections()
{
	period = 15;
	dis_counted = 0;
	prev_disc_pos = 0;
	SetCoreSeparateWindow();
}

void Disconnections::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("period");
	if (i != -1)
		period = args[i];
}

void Disconnections::Init()
{
	/*
	int draw_begin;
	if (period < 2)
		throw DataExc();
	draw_begin = period - 1;
	SetBufferCount(1);
	SetBufferColor(0, Green());
	SetBufferBegin(0, draw_begin );
	SetBufferLineWidth(0, 2);
	SetIndexCount(1);
	SetIndexBuffer(0, discbuf);
	SetCoreMaximum(1.0);
	SetCoreMinimum(0.0);
	
	int method = 0;
	
	if (RequireIndicator("edstat", "period", period, "method", method, "slowing", period * 11 / 12)) throw DataExc();
	if (RequireIndicator("tidstat", "period", period)) throw DataExc();
	*/
}

void Disconnections::Start()
{
	/*
	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();
	
	Core& cont1 = At(0);
	Core& cont2 = At(1);
	cont1.Refresh();
	cont2.Refresh();
	
	FloatVector& dbl1 = cont1.GetIndex(0);
	FloatVector& dbl2 = cont2.GetIndex(0);
	
	Vector<double> buf1, buf2;
	Vector<bool> discs;
	buf1.SetCount(period, 0);
	buf2.SetCount(period, 0);
	discs.SetCount(period, false);
	ASSERT(dis_counted >= 0);
	int limit = bars + period + 1;
	dis_counted = max(1, dis_counted - period);
	
	bars--;
	
	for(int i = dis_counted; i < limit; i++) {
		int mod = i % period;
		
		if (i < bars - period) {
			// Edge filter
			double d1 = dbl1.Get(i);
			double d2 = dbl2.Get(i);
			double d1a = dbl1.Get(i+1);
			double d1b = dbl1.Get(i-1);
			double d2a = dbl2.Get(i+1);
			double d2b = dbl2.Get(i-1);
			double edge1 = (d1 - d1b) + (d1 - d1a);
			double edge2 = (d2 - d2b) + (d2 - d2a);
			
			// Get maximum peak value in previous period
			double max1 = 0;
			double max2 = 0;
			for(int j = 0; j < period; j++) {
				max1 = max(max1, buf1[j]);
				max2 = max(max2, buf2[j]);
			}
			
			buf1[mod] = edge1;
			buf2[mod] = edge2;
			
			// Use as disconnections values over 80% peak edge value
			bool disconnection =
				(bool)(edge1 > max1 * 0.8) +
				(bool)(edge2 > max2 * 0.8);
				
			discs[mod] = disconnection;
		}
		
		if (discs[mod]) {
			
			int len = i - prev_disc_pos;
			if (len <= 1) continue;
			double mul = 1.0 / pow(100.0, 1.0 / (len / 2.0));
			int len2 = len / 2;
			double value = 1.0;
			for(int j = 0; j <= len2; j++) {
				int a = prev_disc_pos + j;
				int b = i - j;
				if (a < bars) discbuf.Set(a, value);
				if (b < bars) discbuf.Set(b, value);
				value *= mul;
			}
			
			if (i >= bars)
				break;
			
			prev_disc_pos = i;
		}
		
	}
	
	dis_counted = bars;
	*/
}






















EventOsc::EventOsc() {
	mul = 0.8;
	emgr = NULL;
	counted_events = 0;
}

void EventOsc::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("mul");
	if (i != -1)
		mul = args[i];
}

void EventOsc::Init() {
	/*
	Core::Init();
	
	if (mul <= 0 || mul > 0.9) throw DataExc();
	
	int id = GetId();
	if (id == -1) throw DataExc();
	
	PathResolver& res = GetResolver();
	PathLink* link = res.FindLinkPath("/id/id" + IntStr(id) + "/tf" + IntStr(GetPeriod()));
	if (!link) throw DataExc();
	if (!link->link.Is()) throw DataExc();
	
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
	
	//DataVar dv = res.ResolvePath("/emgr");
	if (!dv.Is()) throw DataExc();
	emgr = dv.Get<EventManager>();
	if (!emgr) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferCount(4);
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
	SetIndexCount(4);
	SetIndexBuffer(3, info);
	SetIndexBuffer(2, low);
	SetIndexBuffer(1, med);
	SetIndexBuffer(0, high);
	*/
}

void EventOsc::Start() {
	/*
	int bars = GetBars();
	
	int count = emgr->GetCount();
	
	if (!count) {
		emgr->Refresh();
		count = emgr->GetCount();
	}
	
	for(int i = counted_events; i < count; i++) {
		Event& e = emgr->GetEvent(i);
		if (keys.Find(e.currency) != -1) {
			FloatVector& buf = (e.impact == 0 ? info : (e.impact == 1 ? low : (e.impact == 2 ? med : high)));
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
	*/
}












GroupEventOsc::GroupEventOsc() {
	mul = 0.8;
	feat_count = 0;
	ownmul = 10;
}

void GroupEventOsc::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("mul");
	if (i != -1)
		mul = args[i];
	i = args.Find("ownmul");
	if (i != -1)
		ownmul = args[i];
}

void GroupEventOsc::Init() {
	/*
	Core::Init();
	
	if (mul <= 0 || mul > 0.9) throw DataExc();
	if (ownmul < 0.01) throw DataExc();
	
	int period = GetPeriod();
	int id = GetId();
	if (id == -1) throw DataExc();
	
	PathResolver& res = GetResolver();
	PathLink* link;
	
	// Nodes only
	link = res.FindLinkPath("/nodes");
	if (!link) throw DataExc();
	
	feat_count = link->keys.GetCount();
	for(int i = 0; i < feat_count; i++) {
		int id = link->keys[i].keys[0].link->GetId();
		if (id == -1) throw DataExc();
		if (RequireIdIndicator(id, period, "eosc", "mul", mul)) throw DataExc();
	}
	
	if (feat_count <= 1) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferCount(4);
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
	SetIndexCount(4);
	SetIndexBuffer(3, info);
	SetIndexBuffer(2, low);
	SetIndexBuffer(1, med);
	SetIndexBuffer(0, high);
	*/
}

void GroupEventOsc::Start() {
	/*
	int counted = GetCounted();
	int bars = GetBars();
	int id = GetId();
	double mul = 1.0 / (feat_count - 1.0);
	double ownmul = mul * this->ownmul;
	for (int i = counted; i < bars; i++) {
		
		for(int j = 0; j < feat_count; j++) {
			Core& cont = At(j);
			if (j == id) {
				info.Inc(i, ownmul * cont.GetIndex(3).Get(i));
				low.Inc(i,  ownmul * cont.GetIndex(2).Get(i));
				med.Inc(i,  ownmul * cont.GetIndex(1).Get(i));
				high.Inc(i, ownmul * cont.GetIndex(0).Get(i));
			} else {
				info.Inc(i, mul * cont.GetIndex(3).Get(i));
				low.Inc(i,  mul * cont.GetIndex(2).Get(i));
				med.Inc(i,  mul * cont.GetIndex(1).Get(i));
				high.Inc(i, mul * cont.GetIndex(0).Get(i));
			}
		}
	}
	*/
}




















FeatureOsc::FeatureOsc() {
	mul = 0.8;
	counted_features = 0;
}

void FeatureOsc::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("mul");
	if (i != -1)
		mul = args[i];
}

void FeatureOsc::Init() {
	/*
	Core::Init();
	
	if (mul <= 0 || mul > 0.9) throw DataExc();
	
	int id = GetId();
	if (id == -1) throw DataExc();
	
	if (RequireIndicator("feat")) throw DataExc();
	
	SetCoreSeparateWindow();
	SetBufferCount(4);
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
	SetIndexCount(4);
	SetIndexBuffer(3, info);
	SetIndexBuffer(2, low);
	SetIndexBuffer(1, med);
	SetIndexBuffer(0, high);
	*/
}

void FeatureOsc::Start() {
	/*
	int bars = GetBars();
	
	FeatureDetector& feat = dynamic_cast<FeatureDetector&>(At(0));
	
	int count = feat.GetKeypointCount();
	for(int i = counted_features; i < count; i++) {
		const FeatureKeypoint& kp = feat.GetKeypoint(i);
		int shift = kp.GetShift();
		int impact = kp.GetImpact();
		FloatVector& buf = (impact == 0 ? info : (impact == 1 ? low : (impact == 2 ? med : high)));
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
	*/
}






















GroupFeatureOsc::GroupFeatureOsc() {
	mul = 0.8;
	feat_count = 0;
	ownmul = 10;
}

void GroupFeatureOsc::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("mul");
	if (i != -1)
		mul = args[i];
	i = args.Find("ownmul");
	if (i != -1)
		ownmul = args[i];
}

void GroupFeatureOsc::Init() {
	/*
	Core::Init();
	
	if (mul <= 0 || mul > 0.9) throw DataExc();
	if (ownmul < 0.01) throw DataExc();
	
	int period = GetPeriod();
	int id = GetId();
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
	SetBufferCount(4);
	SetBufferColor(3, Blue());
	SetBufferColor(2, Green());
	SetBufferColor(1, Yellow());
	SetBufferColor(0, Red());
	SetBufferLineWidth(2, 2);
	SetBufferLineWidth(1, 3);
	SetBufferLineWidth(0, 4);
	SetIndexCount(4);
	SetIndexBuffer(3, info);
	SetIndexBuffer(2, low);
	SetIndexBuffer(1, med);
	SetIndexBuffer(0, high);
	*/
}

void GroupFeatureOsc::Start() {
	/*
	int counted = GetCounted();
	int bars = GetBars();
	int id = GetId();
	double mul = 1.0 / (feat_count - 1.0);
	double ownmul = mul * this->ownmul;
	for (int i = counted; i < bars; i++) {
		
		for(int j = 0; j < feat_count; j++) {
			Core& cont = At(j);
			if (j == id) {
				info.Inc(i, ownmul * cont.GetIndex(3).Get(i));
				low.Inc(i,  ownmul * cont.GetIndex(2).Get(i));
				med.Inc(i,  ownmul * cont.GetIndex(1).Get(i));
				high.Inc(i, ownmul * cont.GetIndex(0).Get(i));
			} else {
				info.Inc(i, mul * cont.GetIndex(3).Get(i));
				low.Inc(i,  mul * cont.GetIndex(2).Get(i));
				med.Inc(i,  mul * cont.GetIndex(1).Get(i));
				high.Inc(i, mul * cont.GetIndex(0).Get(i));
			}
		}
	}
	*/
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
	
}

void ChannelPredicter::SetArguments(const VectorMap<String, Value>& args) {
	int i = args.Find("length");
	if (i != -1)
		length = args[i];
}

void ChannelPredicter::Init() {
	//AddDependency("/whstat_slow", 0, 0);
}

bool ChannelPredicter::Process(const CoreProcessAttributes& attr) {
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
}








	
}
