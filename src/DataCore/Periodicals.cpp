#include "DataCore.h"

namespace DataCore {



WdayHourChanges::WdayHourChanges() {
	
}

void WdayHourChanges::SetArguments(const VectorMap<String, Value>& args) {
	
}

void WdayHourChanges::Init() {
	/*SetContainerSeparateWindow();
	SetBufferCount(2);
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(0,127,0));
	
	SetIndexCount(2);
	
	SetIndexBuffer ( 0, mean);
	SetIndexBuffer ( 1, stddev)
	
	int period = GetMinutePeriod();
	bool force_d0 = period >= 7*24*60;
	
	//wdayhour.SetCount(force_d0 ? 1 : 7*24*60 / period);;*/
}

bool WdayHourChanges::Process(const SlotProcessAttributes& attr) {
/*	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	BarData& pb = *GetSource().Get<BarData>();
	const Data32f& open  = pb.GetOpen();
	
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
	return true;
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
	/*
	SetContainerSeparateWindow();
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
	
	return 0;*/
}

bool WdayHourStats::Process(const SlotProcessAttributes& attr) {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	BarData& pb = *GetSource().Get<BarData>();
	const Data32f& open  = pb.GetOpen();
	
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
	
	return 0;*/
	return true;
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
	/*BarDataContainer::Init();
	
	SetContainerSeparateWindow();
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
	
	if (RequireIndicator("whstat", "period", var_period_fast)) return 1;
	if (RequireIndicator("whstat", "period", var_period_fast + var_period_diff)) return 1;
	
	return 0;*/
}

bool WdayHourDiff::Process(const SlotProcessAttributes& attr) {
	/*
	int counted = GetCounted();
	
	if (counted > 0) counted--;
	
	Container& a = At(0);
	const Data32f& t_pre_stddev_fast	= a.GetIndex(0);
	const Data32f& t_pre_mean_fast		= a.GetIndex(1);
	const Data32f& h_pre_stddev_fast	= a.GetIndex(2);
	const Data32f& h_pre_mean_fast		= a.GetIndex(3);
	const Data32f& d_pre_stddev_fast	= a.GetIndex(4);
	const Data32f& d_pre_mean_fast		= a.GetIndex(5);
	const Data32f& dh_pre_stddev_fast	= a.GetIndex(6);
	const Data32f& dh_pre_mean_fast		= a.GetIndex(7);
	
	Container& b = At(1);
	const Data32f& t_pre_stddev_slow	= b.GetIndex(0);
	const Data32f& t_pre_mean_slow		= b.GetIndex(1);
	const Data32f& h_pre_stddev_slow	= b.GetIndex(2);
	const Data32f& h_pre_mean_slow		= b.GetIndex(3);
	const Data32f& d_pre_stddev_slow	= b.GetIndex(4);
	const Data32f& d_pre_mean_slow		= b.GetIndex(5);
	const Data32f& dh_pre_stddev_slow	= b.GetIndex(6);
	const Data32f& dh_pre_mean_slow		= b.GetIndex(7);
	
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
	
	return 0;
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
	/*BarDataContainer::Init();
	
	ASSERTEXC(!(mul <= 0 || mul > 0.9));
	
	int id = GetId();
	ASSERTEXC(id != -1);
	
	PathResolver& res = GetResolver();
	PathLink* link = res.FindLinkPath("/id/id" + IntStr(id) + "/tf" + IntStr(GetPeriod()));
	if (!link) return 1;
	if (!link->link.Is()) return 1;
	
	BridgeBarData* bbd = link->link.Get<BridgeBarData>();
	if (bbd) {
		keys.Add(bbd->GetKey0());
		keys.Add(bbd->GetKey1());
	}
	
	BridgeMeshConverter* bmc = link->link.Get<BridgeMeshConverter>();
	if (bmc) {
		keys.Add(bmc->GetKey());
	}
	
	if (keys.IsEmpty()) return 1;
	
	DataVar dv = res.ResolvePath("/emgr");
	if (!dv.Is()) return 1;
	emgr = dv.Get<EventManager>();
	if (!emgr) return 1;
	
	SetContainerSeparateWindow();
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
	
	return 0;
	*/
}

bool EventOsc::Process(const SlotProcessAttributes& attr) {
	/*int bars = GetBars();
	
	int count = emgr->GetCount();
	
	if (!count) {
		emgr->Refresh();
		count = emgr->GetCount();
	}
	
	for(int i = counted_events; i < count; i++) {
		Event& e = emgr->GetEvent(i);
		if (keys.Find(e.currency) != -1) {
			Data32f& buf = (e.impact == 0 ? info : (e.impact == 1 ? low : (e.impact == 2 ? med : high)));
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
	
	return 0;*/
	return true;
}


}
