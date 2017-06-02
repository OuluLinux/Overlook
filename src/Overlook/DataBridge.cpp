#include "Overlook.h"

namespace Overlook {

DataBridge::DataBridge()  {
	SetSkipAllocate();
	cursor = 0;
	buffer_cursor = 0;
	max_value = 0;
	min_value = 0;
	median_max = 0;
	median_min = 0;
	point = 0.00001;
}

DataBridge::~DataBridge()  {
	
}

void DataBridge::Init() {
	spread_qt.AddColumn("Spread points", 1024);
	spread_qt.EndTargets();
	spread_qt.AddColumn("Wday",			7);
	spread_qt.AddColumn("Hour",			24);
	spread_qt.AddColumn("5-min",		12);
	
	volume_qt.AddColumn("Volume", 524288);
	volume_qt.EndTargets();
	
	slow_volume = GetMinutePeriod() >= 1440;
	day_volume = GetMinutePeriod() == 1440;
	if (!slow_volume) {
		volume_qt.AddColumn("Wday",			7);
		volume_qt.AddColumn("Hour",			24);
		volume_qt.AddColumn("5-min",		12);
	} else {
		if (day_volume)
			volume_qt.AddColumn("Wday",			7);
		volume_qt.AddColumn("1-change",		16);
	}
	
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
}

void DataBridge::Start() {
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	int sym_count = common.GetSymbolCount();
	
	// Regular symbols
	if (GetSymbol() < sym_count) {
		if (GetCounted() == 0)
			RefreshFromHistory();
		RefreshFromAskBid();
	}
	
	// Generated symbols
	else {
		RefreshVirtualNode();
	}
}

void DataBridge::RefreshFromAskBid() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& spread_buf = GetBuffer(4);
		
	BaseSystem& bs = GetBaseSystem();
	int id = GetSymbol();
	int tf = GetTimeframe();
	int bars = GetBars();
	int counted = GetCounted();
	ASSERTEXC(id >= 0);
	double half_point = point * 0.5;
	
	String id_str = bs.GetSymbol(id).Left(6);
	char id_chr[6] = {0,0,0,0,0,0};
	for(int i = 0; i < id_str.GetCount(); i++)
		id_chr[i] = id_str[i];
	
	// Allocate memory
	ASSERT(bars > 0);
	if (counted) {
		SetSafetyLimit(bars);
		for(int i = 0; i < outputs[0].buffers.GetCount(); i++)
			outputs[0].buffers[i].SetCount(bars);
		double prev_open = open_buf.Get(counted-1);
		for(int i = counted; i < bars; i++) {
			open_buf.Set(i, prev_open);
			low_buf.Set(i, prev_open);
			high_buf.Set(i, prev_open);
		}
	}
	
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	// Open askbid-file
	String local_askbid_file = ConfigFile("askbid.bin");
	FileIn src(local_askbid_file);
	ASSERTEXC(src.IsOpen() && src.GetSize());
	int data_size = src.GetSize();
	
	src.Seek(cursor);
	
	
	
	
	// TODO: common askbid source and spread_qt.Reserve
	
	
	
	int struct_size = 4 + 6 + 8 + 8;
	
	while ((cursor + struct_size) <= data_size) {
		int timestamp;
		double ask, bid;
		src.Get(&timestamp, 4);
		
		bool equal = true;
		for(int i = 0; i < 6; i++) {
			char c;
			src.Get(&c, 1);
			if (!c) {
				src.SeekCur(6-1-i);
				break;
			}
			if (!equal) continue;
			if (c != id_chr[i])
				equal = false;
		}
		if (!equal) {
			src.SeekCur(8+8);
			cursor += struct_size;
			continue;
		}
		src.Get(&ask, 8);
		src.Get(&bid, 8);
		cursor += struct_size;
		
		
		Time t = TimeFromTimestamp(timestamp);
		int h, d, dh;
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		} else {
			h = (t.minute + t.hour * 60) / period;
			d = DayOfWeek(t) - 1;
			dh = h + d * h_count;
		}
		int shift = bs.GetShiftFromTimeTf(timestamp, tf);
		
		
		// Add row
		int row = spread_qt.GetCount();
		spread_qt.SetCount(row+1);
		
		double diff = ask - bid + half_point;
		int diff_points = diff / point;
		int dow = DayOfWeek(t);
		int hour = t.hour;
		int minute = t.minute;
		
		int pos = 0;
		spread_qt.Set(row, pos++, diff_points);
		spread_qt.Set(row, pos++, dow);
		spread_qt.Set(row, pos++, hour);
		spread_qt.Set(row, pos++, minute / 5);
		
		
		// Add value to the buffers
		if (shift >= buffer_cursor) {
			SetSafetyLimit(shift+1);
			if (shift > buffer_cursor) {
				if (shift > buffer_cursor+1) {
					double prev_value = open_buf.Get(buffer_cursor);
					for(int i = buffer_cursor+1; i < shift; i++) {
						open_buf.Set(i, prev_value);
						low_buf.Set(i, prev_value);
						high_buf.Set(i, prev_value);
					}
				}
				open_buf.Set(shift, ask);
				low_buf.Set(shift, ask);
				high_buf.Set(shift, ask);
				buffer_cursor = shift;
			}
			else {
				double low  = low_buf.Get(shift);
				double high = high_buf.Get(shift);
				if (ask < low)  {low_buf	.Set(shift, ask);}
				if (ask > high) {high_buf	.Set(shift, ask);}
			}
		}
	}
	
	// Clone the last value to the end of the vector
	if (buffer_cursor) {
		double prev_open = open_buf.Get(buffer_cursor-1);
		SetSafetyLimit(bars);
		for(int i = buffer_cursor+1; i < bars; i++) {
			open_buf.Set(i, prev_open);
			low_buf.Set(i, prev_open);
			high_buf.Set(i, prev_open);
		}
	}
	/*
	VectorMap<int, int> stats;
	for(int i = 0; i < volume_qt.GetCount(); i++) {
		int t = volume_qt.Get(i, 0);
		stats.GetAdd(t,0)++;
	}
	DUMPM(stats);*/
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double spread = 0;
		
		Time t = bs.GetTimeTf(tf, i);
		int dow = DayOfWeek(t);
		int hour = t.hour;
		int minute = t.minute;
		
		
		// Get spread value
		spread_qt.ClearQuery();
		int pos = 1;
		spread_qt.SetQuery(pos++, dow);
		spread_qt.SetQuery(pos++, hour);
		spread_qt.SetQuery(pos++, minute / 5);
		double average_diff_point = spread_qt.QueryAverage(0);
		double diff = average_diff_point * point;
		spread_buf.Set(i, diff);
		
		
		// Get volume value
		int i0 = GetChangeStep(i, 16);
		volume_qt.ClearQuery();
		pos = 1;
		if (!slow_volume) {
			volume_qt.SetQuery(pos++, dow);
			volume_qt.SetQuery(pos++, hour);
			volume_qt.SetQuery(pos++, minute / 5);
		} else {
			if (day_volume)
				volume_qt.SetQuery(pos++, dow);
			volume_qt.SetQuery(pos++, i0);
		}
		double average_volume = volume_qt.QueryAverage(0);
		volume_buf.Set(i, average_volume);
		
		
		// Find min/max
		diff = i ? open_buf.Get(i) - open_buf.Get(i-1) : 0.0;
		int step = diff / point;
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
	}
	
	RefreshMedian();
	ForceSetCounted(bars);
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

void DataBridge::RefreshFromHistory() {
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	if (!common.IsInited()) {
		common.lock.Enter();
		if (!common.IsInited()) {
			common.Init(this);
		}
		common.lock.Leave();
	}
	
	MetaTrader& mt = GetMetaTrader();
	BaseSystem& bs = GetBaseSystem();
	
	LOG(Format("sym=%d tf=%d pos=%d", Core::GetSymbol(), GetTimeframe(), GetBars()));
	
	
	int bars = GetBars();
	ASSERT(bars > 0);
	for(int i = 0; i < outputs.GetCount(); i++)
		for(int j = 0; j < outputs[i].buffers.GetCount(); j++)
			outputs[i].buffers[j].value.SetCount(bars, 0);
	
	
	// Open data-file
	int period = GetPeriod();
	int tf = GetTf();
	int mt_period = period * bs.GetBasePeriod() / 60;
	String symbol = bs.GetSymbol(GetSymbol());
	String history_dir = ConfigFile("history");
	String filename = symbol + IntStr(mt_period) + ".hst";
	String local_history_file = AppendFileName(history_dir, filename);
	if (!FileExists(local_history_file))
		throw DataExc();
	FileIn src(local_history_file);
	if (!src.IsOpen() || !src.GetSize())
		return;
	
	
	// Init destination time vector settings
	int begin = bs.GetBeginTS(tf);
	int step = bs.GetBasePeriod() * period;
	int cur = begin;
	bool inc_month = period == 43200 && bs.GetBasePeriod() == 60;
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		throw DataExc();
	double point = 1.0 / pow(10.0, digits);
	common.points[GetSymbol()] = point;
	int data_size = src.GetSize();
	const int struct_size = 8 + 4*8 + 8 + 4 + 8;
	byte row[struct_size];
	double prev_close;
	double open = 0;
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	int count = 0;
	int expected_count = (src.GetSize() - cursor) / struct_size;
	src.Seek(cursor);
	
	volume_qt.Reserve(expected_count);
	
	while ((cursor + struct_size) <= data_size && count < bars) {
		int time;
		double high, low, close;
		int64 tick_volume, real_volume;
		int spread;
		src.Get(row, struct_size);
		byte* current = row;
		
		//TODO: endian swap in big endian machines
		
		time  = *((uint64*)current);		current += 8;
		open  = *((double*)current);		current += 8;
		high  = *((double*)current);		current += 8;
		low   = *((double*)current);		current += 8;
		close = *((double*)current);		current += 8;
		tick_volume  = *((int64*)current);		current += 8;
		spread       = *((int32*)current);		current += 4;
		real_volume  = *((int64*)current);		current += 8;
		
		cursor += struct_size;
		
		// At first value
		if (count == 0) {
			prev_close = close;
			
			// Check that value is in the range of 1*point - UINT16_MAX*point
			double base_point = point;
			while (close >= (UINT16_MAX*point)) {
				// sacrifice a little bit of accuracy for optimal memory usage
				point += base_point;
			}
			common.points[GetSymbol()] = point;
			// TODO: check all data and don't rely on close
		}
		
		while (cur < time && count < bars) {
			SetSafetyLimit(count);
			open_buf.Set(count, prev_close);
			low_buf.Set(count, prev_close);
			high_buf.Set(count, prev_close);
			volume_buf.Set(count, 0);
			if (!inc_month)		cur += step;
			else				cur = IncreaseMonthTS(cur);
			count++;
		}
		
		prev_close = close;
		
		if (count < bars && time == cur) {
			SetSafetyLimit(count);
			//ASSERT(bs.GetTime(tf, count) == TimeFromTimestamp(time));
			open_buf.Set(count, open);
			low_buf.Set(count, low);
			high_buf.Set(count, high);
			volume_buf.Set(count, tick_volume);
			if (!inc_month)		cur += step;
			else				cur = IncreaseMonthTS(cur);
			count++;
		}
		
		
		// Find min/max
		double diff = count >= 2 ? open_buf.Get(count-1) - open_buf.Get(count-2) : 0.0;
		int step = diff / point;
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	RefreshMedian();
	
	// Fill volume querytable
	volume_qt.Reserve(count);
	for(int i = 0; i < count; i++) {
		Time t = bs.GetTimeTf(tf, i);
		int row = volume_qt.GetCount();
		volume_qt.SetCount(row+1);
		int dow = DayOfWeek(t);
		int hour = t.hour;
		int minute = t.minute;
		int i0 = GetChangeStep(i, 16);
		int pos = 0;
		int tick_volume = volume_buf.Get(i);
		volume_qt.Set(row, pos++, Upp::min(tick_volume, 524287));
		if (!slow_volume) {
			volume_qt.Set(row, pos++, dow);
			volume_qt.Set(row, pos++, hour);
			volume_qt.Set(row, pos++, minute / 5);
		} else {
			if (day_volume)
				volume_qt.Set(row, pos++, dow);
			volume_qt.Set(row, pos++, i0);
		}
	}
	
	ForceSetCounted(count);
	buffer_cursor = count-1;
	
	while (count < bars) {
		SetSafetyLimit(count);
		open_buf.Set(count, open);
		low_buf.Set(count, open);
		high_buf.Set(count, open);
		volume_buf.Set(count, 0);
		if (!inc_month)		cur += step;
		else				cur = IncreaseMonthTS(cur);
		count++;
	}
	
}

void DataBridge::RefreshVirtualNode() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	BaseSystem& bs = GetBaseSystem();
	MetaTrader& mt = GetMetaTrader();
	int sym_count = mt.GetSymbolCount();
	int cur = GetSymbol() - sym_count;
	if (cur < 0)
		return;
	ASSERT(cur >= 0 && cur < mt.GetCurrencyCount());
	
	int bars = GetBars();
	ASSERT(bars > 0);
	for(int i = 0; i < outputs.GetCount(); i++)
		for(int j = 0; j < outputs[i].buffers.GetCount(); j++)
			outputs[i].buffers[j].value.SetCount(bars, 0);
		
	const Currency& c = mt.GetCurrency(cur);
	int tf = GetTimeframe();
	
	typedef Tuple3<ConstBuffer*,ConstBuffer*,bool> Source;
	Vector<Source> sources;
	for(int i = 0; i < c.pairs0.GetCount(); i++) {
		int id = c.pairs0[i];
		ConstBuffer& open_buf = GetInputBuffer(1,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(1,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, false));
	}
	for(int i = 0; i < c.pairs1.GetCount(); i++) {
		int id = c.pairs1[i];
		ConstBuffer& open_buf = GetInputBuffer(1,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(1,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, true));
	}
	
	int counted = GetCounted();
	if (!counted) {
		open.Set(0, 1.0);
		low.Set(0, 1.0);
		high.Set(0, 1.0);
		volume.Set(0, 0);
		counted = 1;
	}
	else counted--;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double change_sum = 0;
		double volume_sum = 0;
		
		for(int j = 0; j < sources.GetCount(); j++) {
			Source& s = sources[j];
			ConstBuffer& open_buf = *s.a;
			ConstBuffer& vol_buf  = *s.b;
			bool inverse = s.c;
			
			double open   = open_buf.Get(i-1);
			double close  = open_buf.Get(i);
			double vol    = vol_buf.Get(i);
			double change = open != 0.0 ? (close / open) - 1.0 : 0.0;
			if (inverse) change *= -1.0;
			
			change_sum += change;
			volume_sum += vol;
		}
		
		double prev = open.Get(i-1);
		double change = change_sum / sources.GetCount();
		double value = prev * (1.0 + change);
		double volume_av = volume_sum / sources.GetCount();
		
		open.Set(i, value);
		low.Set(i, value);
		high.Set(i, value);
		volume.Set(i, volume_av);
		
		//LOG(Format("pos=%d open=%f volume=%f", i, value, volume_av));
		
		if (i) {
			low		.Set(i-1, Upp::min(low		.Get(i-1), value));
			high	.Set(i-1, Upp::max(high		.Get(i-1), value));
		}
		
		
		// Find min/max
		double diff = i ? open.Get(i) - open.Get(i-1) : 0.0;
		int step = diff / point;
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (diff > max_value) max_value = diff;
		if (diff < min_value) min_value = diff;
	}
	
	RefreshMedian();
}

int DataBridge::GetChangeStep(int shift, int steps) {
	int change = 0;
	if (shift > 0) {
		ConstBuffer& open_buf = GetBuffer(0);
		change = (open_buf.Get(shift) - open_buf.Get(shift-1)) / point;
	}
	int max_change = median_max * 2;
	int min_change = median_min * 2;
	int diff = max_change - min_change;
	if (!diff) return steps / 2;
	double step = (double)diff / steps;
	int v = (change - min_change) / step;
	if (v < 0) v = 0;
	if (v >= steps) v = steps -1;
	return v;
}

}
