#include "Overlook.h"
#include <plugin/zip/zip.h>

namespace Overlook {

DataBridge::DataBridge() {
	//SetSkipAllocate();
	//SetSkipSetCount(true);
	cursor = 0;
	max_value = 0;
	min_value = 0;
	median_max = 0;
	median_min = 0;
	point = 0.00001;
	spread_mean = 0;
	spread_count = 0;
}

DataBridge::~DataBridge() {
	
}

void DataBridge::Init() {
	System& sys = GetSystem();
	
	point = GetMetaTrader().GetSymbol(sym_id).point;
	
}

void DataBridge::Clear() {
	median_max_map.Clear();
	median_min_map.Clear();
	spread_mean = 0;
	spread_count = 0;
	median_max = -DBL_MAX;
	median_min = DBL_MAX;
	cursor = 0;
	cursor2 = 0;
	counted = 0;
	open.Clear();
	low.Clear();
	high.Clear();
	volume.Clear();
	time.Clear();
}

void DataBridge::Start() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	
	lock.Enter();
	
	if (once) {
		once = false;
		
		common.InspectInit();
	}
	
	int sym_count = common.GetSymbolCount();
	
	// Regular symbols
	if (tf_id > 0 && sym_id < sym_count) {
		RefreshFromFaster();
	}
	else if (sym_id < sym_count) {
		RefreshFromAskBid();
	}
	
	lock.Leave();
}

void DataBridge::AddSpread(double a) {
	if (spread_count == 0) {
		spread_mean = a;
	} else {
		double delta = a - spread_mean;
		spread_mean += delta / spread_count;
	}
	spread_count++;
}

void DataBridge::RefreshFromAskBid() {
	MetaTrader& mt = GetMetaTrader();
	System& sys = GetSystem();
	Vector<double>& open_buf = open;
	Vector<double>& low_buf = low;
	Vector<double>& high_buf = high;
	Vector<double>& volume_buf = volume;
	Vector<int>& time_buf = time;
	DataBridgeCommon& common = GetDataBridgeCommon();
	
	common.RefreshAskBidData();
	
	int id = sym_id;
	int tf = tf_id;
	int counted = GetCounted();
	ASSERTEXC(id >= 0);
	double half_point = point * 0.5;
	
	const Vector<DataBridgeCommon::AskBid>& data = common.data[id];
		
	int64 step = GetPeriod();
	int64 epoch_begin = Time(1970,1,1).Get();
	int shift = open_buf.GetCount() - 1;
	
	Time max_time = GetUtcTime() + 5 * 60;
	int max_time_ts = max_time.Get() - Time(1970,1,1).Get();
	
	for(; cursor < data.GetCount(); cursor++) {
		
		// Get local references
		const DataBridgeCommon::AskBid& askbid = data[cursor];
		const Time& t = askbid.a;
		const double& ask = askbid.b;
		const double& bid = askbid.c;
		Time utc_time = mt.GetTimeToUtc(t);
		
		
		int64 time = utc_time.Get();
		time += 4*24*60*60; // 1.1.1970 is thursday, move to monday
		time = time - time % step;
		time -= 4*24*60*60;
		time -= epoch_begin;
		if (time > max_time_ts)
			continue;
		
		// Find min/max
		double diff = ask - bid;
		int step = (int)(diff / point);
		AddSpread(step);
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		if (shift >= open_buf.GetCount()) {
			if (shift >= open_buf.GetCount()) {
				int res = shift + 1000;
				res -= res % 1000;
				
				open_buf.Reserve(res);
				low_buf.Reserve(res);
				high_buf.Reserve(res);
				volume_buf.Reserve(res);
				time_buf.Reserve(res);
				
				open_buf.SetCount(shift+1);
				low_buf.SetCount(shift+1);
				high_buf.SetCount(shift+1);
				volume_buf.SetCount(shift+1);
				time_buf.SetCount(shift+1);
			}
			
			open_buf[shift] = ask;
			low_buf[shift] = ask;
			high_buf[shift] = ask;
			time_buf[shift] = time;
		}
		else if (time_buf.Top() == time) {
			double low  = low_buf[shift];
			double high = high_buf[shift];
			if (ask < low)  {low_buf[shift] = ask;}
			if (ask > high) {high_buf[shift] = ask;}
		}
	}
	
	
	// Very weird bug of volume not being updated
	volume_buf.SetCount(open_buf.GetCount());
	
	
	RefreshMedian();
	ForceSetCounted(open_buf.GetCount());
}

void DataBridge::RefreshMedian() {
	// Get median values
	{
		SortByKey(median_max_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_max_map.GetCount(); i++)
			total += median_max_map[i];
		int64 target = total / 2;
		total = 0;
		for(int i = 0; i < median_max_map.GetCount(); i++) {
			total += median_max_map[i];
			if (total > target) {
				median_max = median_max_map.GetKey(i);
				break;
			}
		}
	}
	{
		SortByKey(median_min_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_min_map.GetCount(); i++)
			total += median_min_map[i];
		int64 target = total / 2;
		total = 0;
		for(int i = 0; i < median_min_map.GetCount(); i++) {
			total += median_min_map[i];
			if (total > target) {
				median_min = median_min_map.GetKey(i);
				break;
			}
		}
	}
}

int DataBridge::GetChangeStep(int shift, int steps) {
	int change = 0;
	if (shift > 0) {
		change = (int)((open[shift] - open[shift-1]) / point);
	}
	int max_change = median_max * 2;
	int min_change = median_min * 2;
	int diff = max_change - min_change;
	if (!diff) return steps / 2;
	double step = (double)diff / steps;
	int v = (int)((change - min_change) / step);
	if (v < 0) v = 0;
	if (v >= steps) v = steps -1;
	return v;
}

void DataBridge::RefreshFromFaster() {
	Vector<double>& open_buf = open;
	Vector<double>& low_buf = low;
	Vector<double>& high_buf = high;
	Vector<double>& volume_buf = volume;
	Vector<int>& time_buf = time;
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int shift = open_buf.GetCount() - 1;
	
	int sym = sym_id;
	int tf = tf_id;
	ASSERT(tf > 0);
	
	DataBridge& m1_db = sys.data[sym][0].db;
	Vector<double>& src_open = m1_db.open;
	Vector<double>& src_low  = m1_db.low;
	Vector<double>& src_high = m1_db.high;
	Vector<double>& src_vol  = m1_db.volume;
	Vector<int>& src_time  = m1_db.time;
	int faster_bars = src_open.GetCount();
	
	int period_secs = GetPeriod();
	Time epoch(1970,1,1);
	int64 tdiff = 4*24*60*60; // seconds between thursday and monday
	Time t0 = epoch + tdiff; // thursday -> next monday, for W1 data
	int64 t0int = t0.Get();
	//   int64 t1int = epoch.Get();
	//   tdiff == t0int - t1int
	
	int expected_count = faster_bars / period_secs * 1.2;
	open_buf.Reserve(expected_count);
	low_buf.Reserve(expected_count);
	high_buf.Reserve(expected_count);
	volume_buf.Reserve(expected_count);
	time_buf.Reserve(expected_count);
	
	for(; cursor2 < faster_bars; cursor2++) {
		double open_value = src_open[cursor2];
		double low_value  = src_low[cursor2];
		double high_value = src_high[cursor2];
		double volume_sum = src_vol[cursor2];
		int ts = src_time[cursor2];
		if (!ts) return;
		Time t = epoch + ts;
		int steps = (t.Get() - t0int) / period_secs; // steps from monday 5.1.1970
		//    int time = (t0int + steps * period_secs) - t1int;
		//    t0int - t1int == tdiff
		int time = steps * period_secs + tdiff;
		
		if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		if (shift >= open_buf.GetCount()) {
			if (shift >= open_buf.GetCount()) {
				open_buf.SetCount(shift+1);
				low_buf.SetCount(shift+1);
				high_buf.SetCount(shift+1);
				volume_buf.SetCount(shift+1);
				time_buf.SetCount(shift+1);
			}
			
			open_buf[shift] = open_value;
			low_buf[shift] = low_value;
			high_buf[shift] = high_value;
			volume_buf[shift] = volume_sum;
			time_buf[shift] = time;
		}
		else {
			if (low_buf[shift]  > low_value)  {low_buf[shift] = low_value;}
			if (high_buf[shift] < high_value) {high_buf[shift] = low_value;}
			volume_buf[shift] = volume_sum + volume_buf[shift];
		}
	}
	
	ForceSetCounted(open_buf.GetCount());
}

}
