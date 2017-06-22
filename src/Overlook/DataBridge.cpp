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
	spread_qt.AddColumn("Spread points", 4096);
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
	MetaTrader& mt = GetMetaTrader();
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	common.CheckInit(this);
	
	int sym_count = common.GetSymbolCount();
	int cur_count = mt.GetCurrencyCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	
	// Regular symbols
	if (sym < sym_count) {
		bool init_round = GetCounted() == 0;
		if (init_round)
			RefreshFromHistory();
		RefreshFromAskBid(init_round);
	}
	
	// Generated symbols
	else if (cur < cur_count) {
		RefreshVirtualNode();
	}
	
	// Baskets
	else if (cur >= cur_count) {
		// Correlation for baskets
		if (cur == cur_count) RefreshCorrelation();
		RefreshBasket();
	}
}

void DataBridge::RefreshFromAskBid(bool init_round) {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& spread_buf = GetBuffer(4);
		
	System& bs = GetSystem();
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	
	common.RefreshAskBidData();
	
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
	
	const Vector<DataBridgeCommon::AskBid>& data = common.data[id];
	spread_qt.Reserve(data.GetCount());
	
	for(; cursor < data.GetCount(); cursor++) {
		
		// Get local references
		const DataBridgeCommon::AskBid& askbid = data[cursor];
		const Time& t = askbid.a;
		const double& ask = askbid.b;
		const double& bid = askbid.c;
		
		
		// Get shift in data from time
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
		int shift = bs.GetShiftFromTimeTf(t, tf);
		
		
		// Add data row to spread querytable
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
	
	bool five_mins = GetMinutePeriod() < 5;
	
	
	// Fill missing spread data. Set spread value based on querytable at the first refresh.
	if (init_round) {
		double prev_spread = 0;
		for(int i = 0; i < counted; i += 5) {
			Time t = bs.GetTimeTf(tf, i);
			int dow = DayOfWeek(t);
			int hour = t.hour;
			int minute = t.minute;
			
			// Set spread value
			spread_qt.ClearQuery();
			int pos = 1;
			spread_qt.SetQuery(pos++, dow);
			spread_qt.SetQuery(pos++, hour);
			spread_qt.SetQuery(pos++, minute / 5);
			double average_diff_point = spread_qt.QueryAverage(0);
			double diff = average_diff_point * point;
			if (diff == 0)
				diff = prev_spread;
			else
				prev_spread = diff;
			spread_buf.Set(i, diff);
			
			
			// Make it faster
			int end = i + 5;
			SetSafetyLimit(end);
			for(int j = i+1; j < end && j < bars; j++) {
				spread_buf.Set(j, diff);
			}
		}
	}
	
	// Fill spread and volume data based on query table (actual data would be better, though)
	double prev_spread = 0;
	int begin = counted;
	if (init_round) // Fix unfinished volume bar at history -> askbid switching point
		counted--;
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double spread = 0;
		
		Time t = bs.GetTimeTf(tf, i);
		int dow = DayOfWeek(t);
		int hour = t.hour;
		int minute = t.minute;
		
		
		// Find min/max
		double diff = i ? open_buf.Get(i) - open_buf.Get(i-1) : 0.0;
		int step = diff / point;
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		
		// Set volume value
		int i0 = GetChangeStep(i, 16);
		volume_qt.ClearQuery();
		int pos = 1;
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
		
		
		// Set spread value
		spread_qt.ClearQuery();
		pos = 1;
		spread_qt.SetQuery(pos++, dow);
		spread_qt.SetQuery(pos++, hour);
		spread_qt.SetQuery(pos++, minute / 5);
		double average_diff_point = spread_qt.QueryAverage(0);
		diff = average_diff_point * point;
		if (diff == 0)
			diff = prev_spread;
		else
			prev_spread = diff;
		spread_buf.Set(i, diff);
		
		
		// Make 1M tf faster
		if (five_mins && (minute % 5) == 0) {
			int end = i + 5;
			SetSafetyLimit(end);
			for(int j = i+1; j < end && j < bars; j++) {
				spread_buf.Set(j, diff);
				volume_buf.Set(j, average_volume);
			}
			i += 4;
		}
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
	
	MetaTrader& mt = GetMetaTrader();
	System& bs = GetSystem();
	
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
		if ((count % 10) == 0) {
			GetSystem().WhenSubProgress(count, bars*2);
		}
		
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
	bool five_mins = GetMinutePeriod() < 5;
	int steps = five_mins ? 5 : 1;
	volume_qt.Reserve(count / steps);
	for(int i = 0; i < count; i += steps) {
		if ((count % 10) == 0) {
			GetSystem().WhenSubProgress(bars + count, bars*2);
		}
		
		Time t = bs.GetTimeTf(tf, i);
		int row = volume_qt.GetCount();
		volume_qt.SetCount(row+1);
		int dow = DayOfWeek(t);
		int hour = t.hour;
		int minute = t.minute;
		int i0 = GetChangeStep(i, 16);
		int pos = 0;
		int tick_volume = volume_buf.Get(i);
		if (tick_volume > 0) {
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
	
	System& bs = GetSystem();
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
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, false));
	}
	for(int i = 0; i < c.pairs1.GetCount(); i++) {
		int id = c.pairs1[i];
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
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
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
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

void DataBridge::RefreshCorrelation() {
	MetaTrader& mt = GetMetaTrader();
	int sym_count = mt.GetSymbolCount();
	int tf = GetTf();
	
	int counted = GetCounted();
	int bars = GetBars();
	
	const int period = 16;
	Vector<double> cache_a, cache_b;
	cache_a.SetCount(period);
	cache_b.SetCount(period);
	
	Vector<ConstBuffer*> bufs;
	bufs.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++)
		bufs[i] = &GetInputBuffer(0, i, tf, 0);
	
	Vector<double> values, tmp1, tmp2, coeffs;
	Vector<bool> in_group;
	values.SetCount(sym_count);
	tmp1.SetCount(period);
	tmp2.SetCount(period);
	coeffs.SetCount(sym_count * (sym_count-1));
	
	bool initial = counted < period;
	if (initial)
		counted = period;
	
	// Allocate memory
	int ext_data_count = bars / 2 + bars % 2;
	ext_data.SetCount(sym_count);
	sym_group_stats.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++) {
		ext_data[i].SetCount(ext_data_count, 0);
		sym_group_stats[i].SetCount(sym_count, 0);
	}
	
	TimeStop ts;
	for(int i = counted; i < bars; i++) {
		if ((i % 10) == 0) {
			GetSystem().WhenSubProgress(i, bars);
		}
		
		// Calculate correlation values
		SetSafetyLimit(i);
		int l = 0;
		for(int j = 0; j < sym_count-1; j++) {
			ConstBuffer& a = *bufs[j];
			
			double avg1 = 0;
		    double sumSqr1 = 0;
			for(int j = 0; j < period; j++) {
				int pos = i-j;
				double v1 = a.Get(pos);
				tmp1[j] = v1;
				avg1 += v1;
		    }
		    avg1 /= period;
		    
			for(int j = 0; j < period; j++) {
				double v1 = tmp1[j];
				sumSqr1 += pow(v1 - avg1, 2.0);
			}
			
			for(int k = j+1; k < sym_count; k++) {
				ConstBuffer& b = *bufs[k];
				
				double avg2 = 0;
				double sum1 = 0;
			    double sumSqr2 = 0;
			    for(int j = 0; j < period; j++) {
					int pos = i-j;
					double v2 = b.Get(pos);
					tmp2[j] = v2;
					avg2 += v2;
			    }
			    avg2 /= period;
			    
				for(int j = 0; j < period; j++) {
					double v1 = tmp1[j];
					double v2 = tmp2[j];
					sum1 += (v1 - avg1) * (v2 - avg2);
					sumSqr2 += pow(v2 - avg2, 2.0);
				}
		    
				double mul = sumSqr1 * sumSqr2;
				double correlation_coef = mul != 0.0 ? sum1 / sqrt(mul) : 0;
				coeffs[l++] = correlation_coef;
			}
		}
		
		// Find groups
		Vector<Index<int> > groups;
		in_group.SetCount(0);
		in_group.SetCount(sym_count, false);
		int without_group = sym_count;
		while (without_group > 0) {
			Index<int>& group = groups.Add();
			l = 0;
			for(int j = 0; j < sym_count-1; j++) {
				if ((!group.IsEmpty() && group.Find(j) == -1) || in_group[j]) {
					l += j;
					continue;
				}
				for(int k = j+1; k < sym_count; k++) {
					if (in_group[k]) {l++; continue;}
					double correlation_coef = coeffs[l++];
					if (correlation_coef >= 0.75) {
						if (group.IsEmpty()) {
							in_group[j] = true;
							group.Add(j);
							without_group--;
						}
						in_group[k] = true;
						group.Add(k);
						without_group--;
					}
				}
			}
			if (group.IsEmpty())
				break;
		}
		
		
		// Sort groups by descending size
		struct Sorter {
			bool operator()(const Index<int>& a, const Index<int>& b) const {
				return a.GetCount() > b.GetCount();
			}
		};
		Sort(groups, Sorter());
		
		
		// Add groups in the result vector
		if (groups.GetCount() > 0xF)
			groups.SetCount(0xF);
		int pos = i / 2;
		int sub = i % 2;
		for(int j = 0; j < groups.GetCount(); j++) {
			const Index<int>& group = groups[j];
			for(int k = 0; k < group.GetCount(); k++) {
				int sym = group[k];
				
				byte& buf = ext_data[sym][pos];
				if (sub == 0)
					buf = (buf & 0xF0) | j;
				else
					buf = (buf & 0x0F) | (j << 4);
			}
			
			// Count symbols in same group
			for(int k0 = 0; k0 < group.GetCount(); k0++) {
				int s0 = group[k0];
				Vector<int>& stat = sym_group_stats[s0];
				for(int k1 = 0; k1 < group.GetCount(); k1++) {
					if (k0 == k1) continue;
					int s1 = group[k1];
					stat[s1]++;
				}
			}
		}
		
		//LOG(i << "/" << bars << ": " << (100 * i / bars));
	}
	//LOG("Processing correlations for " << bars - counted << " bars took " << ts.ToString());
	
	
	// Get strongest pairs
	typedef Tuple2<int,int> Pair;
	Vector<Pair> strongest_pairs;
	for(int i = 0; i < sym_count; i++) {
		int strongest_id = -1;
		int strongest_count = 0;
		const Vector<int>& counts = sym_group_stats[i];
		for(int j = 0; j < sym_count; j++) {
			if (i == j) continue;
			int count = counts[j];
			if (count > strongest_count) {
				strongest_count = count;
				strongest_id = j;
			}
		}
		if (strongest_id != -1)
			strongest_pairs.Add(Pair(i, strongest_id));
	}
	ASSERT(!strongest_pairs.IsEmpty());
	//DUMPC(strongest_pairs);
	
	
	// Create groups from pairs
	Vector<Index<int> > groups;
	for(int i = 0; i < strongest_pairs.GetCount(); i++) {
		const Pair& p = strongest_pairs[i];
		bool added = false;
		for(int j = 0; j < groups.GetCount(); j++) {
			Index<int>& group = groups[j];
			// If A is in the group and B is not in the group, add B to the group
			if (group.Find(p.a) != -1) {
				if (group.Find(p.b) == -1) {
					group.Add(p.b);
					added = true;
				}
			}
			else if (group.Find(p.b) != -1) {
				if (group.Find(p.a) == -1) {
					group.Add(p.a);
					added = true;
				}
			}
			if (added) break;
		}
		if (!added) {
			Index<int>& group = groups.Add();
			group.Add(p.a);
			group.Add(p.b);
		}
	}
	
	
	// Sort groups by their size descending
	struct GroupSorter {
		bool operator() (const Index<int>& a, const Index<int>& b) const {
			return a.GetCount() > b.GetCount();
		}
	};
	Sort(groups, GroupSorter());
	//DUMPCC(groups);
	
	
	// Copy group to compatible and persistent variable
	sym_groups.SetCount(groups.GetCount());
	for(int i = 0; i < groups.GetCount(); i++) {
		const Index<int>& src = groups[i];
		Vector<int>& dst = sym_groups[i];
		dst.SetCount(src.GetCount());
		for(int j = 0; j < src.GetCount(); j++)
			dst[j] = src[j];
	}
	
	// Copy symbols to the system
	GetSystem().basket_symbols <<= sym_groups;
}

void DataBridge::RefreshBasket() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	Buffer& spread = GetBuffer(4);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int counted = GetCounted();
	int bars = GetBars();
	int sym_count = mt.GetSymbolCount();
	int cur_count = mt.GetCurrencyCount();
	int corr_sym = sym_count + cur_count;
	int sym = GetSymbol();
	int tf = GetTf();
	int basket_alltime_group = sym - corr_sym;
	
	DataBridge& corr_db = sym == corr_sym ? *this : dynamic_cast<DataBridge&>(*GetInputCore(0, corr_sym, tf));
	
	if (symbols.IsEmpty()) {
		ASSERT_(basket_alltime_group < corr_db.sym_groups.GetCount(), "Somehow correlation groups have not been found enough");
		const Vector<int>& group = corr_db.sym_groups[basket_alltime_group];
		for(int j = 0; j < group.GetCount(); j++) {
			int sym = group[j];
			ASSERT(sym < sym_count);
			symbols.Add(sym, +1); // add group member as positively correlating member
		}
		ASSERT(!symbols.IsEmpty());
	}
	
	// Allocate memory
	ASSERT(bars > 0);
	SetSafetyLimit(bars);
	for(int i = 0; i < outputs[0].buffers.GetCount(); i++)
		outputs[0].buffers[i].SetCount(bars);
	if (counted) {
		double prev_open = open.Get(counted-1);
		for(int i = counted; i < bars; i++) {
			open.Set(i, prev_open);
			low.Set(i, prev_open);
			high.Set(i, prev_open);
		}
	}
	
	for(int i = counted; i < bars; i++) {
		if ((i % 10) == 0) {
			GetSystem().WhenSubProgress(i, bars);
		}
		
		SetSafetyLimit(i);
		Time t = sys.GetTimeTf(tf, i);
		int day_of_week = DayOfWeek(t);
		
		// Continue from previous value
		double value = 1.0, low_value = 1.0, high_value = 1.0, volume_value = 0.0, spread_value = 0.0;
		if (i > 1) {
			
			// Get changes from every symbol in the timeslot
			const int prev_pos = i-1;
			double prev = open.Get(prev_pos);
			double mul = 0.0, low_mul = 0.0, high_mul = 0.0, spread_mul = 0.0;
			for(int j = 0; j < symbols.GetCount(); j++) {
				int sym = symbols.GetKey(j);
				ConstBuffer& src_open = GetInputBuffer(0, sym, tf, 0);
				ConstBuffer& src_low  = GetInputBuffer(0, sym, tf, 1);
				ConstBuffer& src_high = GetInputBuffer(0, sym, tf, 2);
				ConstBuffer& src_vol  = GetInputBuffer(0, sym, tf, 3);
				ConstBuffer& src_sprd = GetInputBuffer(0, sym, tf, 4);
				double open_value = src_open.Get(i);
				mul += open_value / src_open.Get(i-1) - 1.0;
				low_mul += src_low.Get(i-1) / src_open.Get(i-1) - 1.0;
				high_mul += src_high.Get(i-1) / src_open.Get(i-1) - 1.0;
				volume_value += src_vol.Get(i);
				double spread_value = src_sprd.Get(i);
				spread_mul += (open_value + spread_value) / open_value - 1.0;
			}
			// Get average
			mul /= symbols.GetCount();
			low_mul /= symbols.GetCount();
			high_mul /= symbols.GetCount();
			spread_mul /= symbols.GetCount();
			
			// Calculate new values
			value = prev * (1.0 + mul);
			low_value = prev * (1.0 + low_mul);
			high_value = prev * (1.0 + high_mul);
			low.Set(prev_pos, low_value);
			high.Set(prev_pos, high_value);
			spread_value = (value * (spread_mul + 1.0)) - value;
			if (spread_value < 0.0) spread_value = 0.0; // Some very small rounding errors
			ASSERT(value > 0.0);
		}
		open.Set(i, value);
		low.Set(i, value);
		high.Set(i, value);
		volume.Set(i, volume_value);
		spread.Set(i, spread_value);
	}
}

}
