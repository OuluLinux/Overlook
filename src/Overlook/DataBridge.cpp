#include "Overlook.h"
#include <plugin/zip/zip.h>

namespace Overlook {

DataBridge::DataBridge() {
	SetSkipAllocate();
	SetSkipSetCount(true);
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
	
	slow_volume = GetMinutePeriod() >= 1440;
	day_volume = GetMinutePeriod() == 1440;
	
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
}

void DataBridge::Start() {
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	
	if (once) {
		once = false;
		
		common.InspectInit();
		
	}
	
	int sym_count = common.GetSymbolCount();
	int cur_count = mt.GetCurrencyCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	int mt_period = GetPeriod();
	
	// Regular symbols
	if (mt_period > 1 && sym < sym_count) {
		RefreshFromFasterTime();
		// NOTE: SyncData has broken other refresh functions for these periods. Disable sync to
		// use them.
	}
	else if (sym < sym_count) {
		bool init_round = GetCounted() == 0 && GetBuffer(0).GetCount() == 0;
		if (init_round) {
			const Symbol& mtsym = mt.GetSymbol(sym);
			RefreshFromHistory(true);
			RefreshFromHistory(false);
		}
		RefreshFromAskBid(init_round);
	}
	
	
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

void DataBridge::RefreshFromAskBid(bool init_round) {
	MetaTrader& mt = GetMetaTrader();
	System& sys = GetSystem();
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	DataBridgeCommon& common = GetDataBridgeCommon();
	
	common.RefreshAskBidData();
	
	int id = GetSymbol();
	int tf = GetTimeframe();
	int counted = GetCounted();
	ASSERTEXC(id >= 0);
	double half_point = point * 0.5;
	
	const Vector<DataBridgeCommon::AskBid>& data = common.data[id];
		
	int64 step = GetMinutePeriod() * 60;
	int64 epoch_begin = Time(1970,1,1).Get();
	int shift = open_buf.GetCount() - 1;
	
	for(; cursor < data.GetCount(); cursor++) {
		
		// Get local references
		const DataBridgeCommon::AskBid& askbid = data[cursor];
		const Time& t = askbid.a;
		const double& ask = askbid.b;
		const double& bid = askbid.c;
		Time utc_time = mt.GetTimeToUtc(t);
		if (ask == 0.0 || bid == 0.0)
			continue;
		
		int64 time = utc_time.Get();
		time += 4*24*60*60; // 1.1.1970 is thursday, move to monday
		time = time - time % step;
		time -= 4*24*60*60;
		time -= epoch_begin;
		
		if (!SyncData(time, shift, ask))
			continue;
		
		// or if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		// Find min/max
		double diff = ask - bid;
		int step = (int)(diff / point);
		AddSpread(step);
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		SetSafetyLimit(shift+1);
		if (shift >= open_buf.GetCount()) {
			if (shift >= open_buf.GetCount()) {
				int res = shift + 1000;
				res -= res % 1000;
				res += 100;
				
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
			
			open_buf.Set(shift, ask);
			low_buf.Set(shift, ask);
			high_buf.Set(shift, ask);
			time_buf.Set(shift, time);
		}
		else {
			double low  = low_buf.Get(shift);
			double high = high_buf.Get(shift);
			if (ask < low)  {low_buf	.Set(shift, ask);}
			if (ask > high) {high_buf	.Set(shift, ask);}
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

void DataBridge::RefreshFromHistory(bool use_internet_data) {
	MetaTrader& mt = GetMetaTrader();
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	if (!use_internet_data) common.DownloadHistory(GetSymbol(), GetTf(), false);
	System& sys = GetSystem();
	
	LOG(Format("sym=%d tf=%d pos=%d", Core::GetSymbol(), GetTimeframe(), GetBars()));
	
	
	// Open data-file
	int id = GetSymbol();
	int tf = GetTf();
	int mt_period = GetPeriod();
	String symbol = sys.GetSymbol(GetSymbol());
	String history_dir = ConfigFile("history");
	String filename = symbol + IntStr(mt_period) + ".hst";
	String local_history_file = AppendFileName(history_dir, filename);
	
	
	bool old_filetype = false;
	
	if (use_internet_data) {
		System& sys = GetSystem();
		String symbol = sys.GetSymbol(GetSymbol());
		
		String url = "http://tools.fxdd.com/tools/M1Data/" + symbol + ".zip";
		
		String data_dir = ConfigFile("m1data");
		RealizeDirectory(data_dir);
		
		String local_zip = AppendFileName(data_dir, symbol + ".zip");
		
		if (!FileExists(local_zip)) {
			LOG("Downloading " << url);
			TimeStop ts;
			
			Downloader dl;
			dl.url = url;
			dl.path = local_zip;
			dl.Perform();
			
			LOG("Downloading took " << ts.ToString());
		}
		
		String local_hst = AppendFileName(data_dir, symbol + ".hst");
		String exp_fname = symbol + ".hst";
		
		if (!FileExists(local_hst)) {
			LOG("Opening zip " << local_zip);
			FileUnZip unzip(local_zip);
			bool found = false;
			while(!(unzip.IsEof() || unzip.IsError())) {
				String fname = GetFileName(unzip.GetPath());
				LOG("Zip has file " << unzip.GetPath() << " (" << fname << ")");
				if (fname == exp_fname) {
					String dst = AppendFileName(data_dir, fname);
					FileOut fout(dst);
					fout << unzip.ReadFile();
					local_history_file = dst;
					old_filetype = true;
					found = true;
					LOG("Found file " << dst);
					break;
				}
				else
					unzip.SkipFile();
			}
			if (!found) {LOG("History file not found for " + symbol);}
		} else {
			local_history_file = local_hst;
			old_filetype = true;
		}
	}
	else if (!FileExists(local_history_file)) {
		DUMP(local_history_file);
		throw DataExc();
	}
	
	FileIn src(local_history_file);
	if (!src.IsOpen() || !src.GetSize())
		return;
	
	// Read the history file
	int digits;
	src.Seek(4+64+12+4);
	src.Get(&digits, 4);
	if (digits > 20)
		throw DataExc();
	double point = 1.0 / pow(10.0, digits);
	common.points[GetSymbol()] = point;
	int data_size = (int)src.GetSize();
	int struct_size = 8 + 4*8 + 8 + 4 + 8;
	if (old_filetype) struct_size = 4 + 4*8 + 8;
	byte row[0x100];
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	int expected_count = open_buf.GetCount() + (int)((src.GetSize() - cursor) / struct_size);
	src.Seek(cursor);
	
	open_buf.Reserve(expected_count);
	low_buf.Reserve(expected_count);
	high_buf.Reserve(expected_count);
	volume_buf.Reserve(expected_count);
	time_buf.Reserve(expected_count);
	
	bool overwrite = false;
	double prev_open = 0.0;
	
	int64 step = GetMinutePeriod() * 60;
	int shift = open_buf.GetCount() - 1;
	
	Time limit = GetUtcTime() + 60*60;
	
	while ((cursor + struct_size) <= data_size) {
		if ((open_buf.GetCount() % 10) == 0) {
			GetSystem().WhenSubProgress(open_buf.GetCount(), bars*2);
		}
		
		int64 time;
		double open, high, low, close;
		int64 tick_volume, real_volume;
		int spread;
		src.Get(row, struct_size);
		byte* current = row;
		
		//TODO: endian swap in big endian machines
		
		if (!old_filetype) {
			time  = *((uint64*)current);			current += 8;
			open  = *((double*)current);			current += 8;
			high  = *((double*)current);			current += 8;
			low   = *((double*)current);			current += 8;
			close = *((double*)current);			current += 8;
			tick_volume  = *((int64*)current);		current += 8;
			spread       = *((int32*)current);		current += 4;
			real_volume  = *((int64*)current);		current += 8;
		} else {
			time  = *((int32*)current);				current += 4;
			open  = *((double*)current);			current += 8;
			high  = *((double*)current);			current += 8;
			low   = *((double*)current);			current += 8;
			close = *((double*)current);			current += 8;
			tick_volume  = *((double*)current);		current += 8;
			spread       = 0;
			real_volume  = 0;
		}
		
		cursor += struct_size;
		
		Time utc_time = mt.GetTimeToUtc(Time(1970,1,1) + time);
		time = utc_time.Get() - Time(1970,1,1).Get();
		time += 4*24*60*60;
		time = time - time % step;
		time -= 4*24*60*60;
		
		if (utc_time >= limit)
			continue;
		
		if (!SyncData(time, shift, open))
			continue;
		
		// or if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		if (shift >= open_buf.GetCount()) {
			int res = shift + 10000;
			res -= res % 10000;
			res += 10000;
			
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
		SetSafetyLimit(shift+1);
		
		
		//ASSERT(bs.GetTime(tf, count) == TimeFromTimestamp(time));
		open_buf.Set(shift, open);
		low_buf.Set(shift, low);
		high_buf.Set(shift, high);
		volume_buf.Set(shift, tick_volume);
		time_buf.Set(shift, time);
		
		
		int count = open_buf.GetCount();
		double diff = count >= 2 ? open_buf.Get(count-1) - open_buf.Get(count-2) : 0.0;
		int step = (int)(diff / point);
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
	
	ForceSetCounted(open_buf.GetCount());
}

bool DataBridge::SyncData(int64 time, int& shift, double ask) {
	Time utc_time = Time(1970,1,1) + time;
	ASSERT(utc_time.second == 0);
	int wday = DayOfWeek(utc_time);
	if (wday == 0 || wday == 6)
		return false;
	
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	Time t;
	if (shift < 0) {
		shift = -1;
		t = Time(2005,1,1) - 60;
	} else {
		shift = time_buf.GetCount() - 1;
		t = Time(1970,1,1) + time_buf.Top();
		ask = open_buf.Top();
		ASSERT(t.second == 0);
	}
	if (utc_time < t)
		return false;
	
	if (t < utc_time) {
		shift++;
		t += 60;
		while (t < utc_time) {
			int wday = DayOfWeek(t);
			if (wday > 0 && wday < 6) {
				int time = t.Get() - Time(1970,1,1).Get();
				
				int res = shift + 10000;
				res -= res % 10000;
				res += 10000;
				
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
				
				open_buf.Set(shift, ask);
				low_buf.Set(shift, ask);
				high_buf.Set(shift, ask);
				time_buf.Set(shift, time);
				
				shift++;
			}
			
			t += 60;
		}
	}
	
	return true;
}

int DataBridge::GetChangeStep(int shift, int steps) {
	int change = 0;
	if (shift > 0) {
		ConstBuffer& open_buf = GetBuffer(0);
		change = (int)((open_buf.Get(shift) - open_buf.Get(shift-1)) / point);
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

void DataBridge::RefreshFromFasterTime() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int shift = open_buf.GetCount() - 1;
	
	int sym = GetSymbol();
	int tf = GetTf();
	ASSERT(tf > 0);
	
	DataBridge& m1_db = dynamic_cast<DataBridge&>(*GetInputCore(0, sym, 0));
	ConstBuffer& src_open = m1_db.GetBuffer(0);
	ConstBuffer& src_low  = m1_db.GetBuffer(1);
	ConstBuffer& src_high = m1_db.GetBuffer(2);
	ConstBuffer& src_vol  = m1_db.GetBuffer(3);
	ConstBuffer& src_time  = m1_db.GetBuffer(4);
	int faster_bars = src_open.GetCount();
	
	int period_mins = GetMinutePeriod();
	int period_secs = period_mins * 60;
	Time epoch(1970,1,1);
	int64 tdiff = 4*24*60*60; // seconds between thursday and monday
	Time t0 = epoch + tdiff; // thursday -> next monday, for W1 data
	int64 t0int = t0.Get();
	//   int64 t1int = epoch.Get();
	//   tdiff == t0int - t1int
	
	int exp_count = faster_bars / period_mins;
	for(int i = 0; i < GetBufferCount(); i++)
		GetBuffer(i).Reserve(exp_count);
	
	for(; cursor2 < faster_bars; cursor2++) {
		double open_value = src_open.Get(cursor2);
		double low_value  = src_low.Get(cursor2);
		double high_value = src_high.Get(cursor2);
		double volume_sum = src_vol.Get(cursor2);
		int ts = src_time.Get(cursor2);
		if (!ts) return;
		Time t = epoch + ts;
		int steps = (t.Get() - t0int) / period_secs; // steps from monday 5.1.1970
		//    int time = (t0int + steps * period_secs) - t1int;
		//    t0int - t1int == tdiff
		int time = steps * period_secs + tdiff;
		
		if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		SetSafetyLimit(shift+1);
		if (shift >= open_buf.GetCount()) {
			if (shift >= open_buf.GetCount()) {
				open_buf.SetCount(shift+1);
				low_buf.SetCount(shift+1);
				high_buf.SetCount(shift+1);
				volume_buf.SetCount(shift+1);
				time_buf.SetCount(shift+1);
			}
			
			open_buf.Set(shift, open_value);
			low_buf.Set(shift, low_value);
			high_buf.Set(shift, high_value);
			volume_buf.Set(shift, volume_sum);
			time_buf.Set(shift, time);
		}
		else {
			if (low_buf.Get(shift)  > low_value)  {low_buf	.Set(shift, low_value);}
			if (high_buf.Get(shift) < high_value) {high_buf	.Set(shift, low_value);}
			volume_buf.Set(shift, volume_sum + volume_buf.Get(shift));
		}
	}
	
	ForceSetCounted(open_buf.GetCount());
}

void DataBridge::RefreshFromFasterChange() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int shift = open_buf.GetCount() - 1;
	
	int sym = GetSymbol();
	int tf = GetTf();
	ASSERT(tf > 0);
	
	DataBridge& m1_db = dynamic_cast<DataBridge&>(*GetInputCore(0, sym, 0));
	ConstBuffer& src_open = m1_db.GetBuffer(0);
	ConstBuffer& src_low  = m1_db.GetBuffer(1);
	ConstBuffer& src_high = m1_db.GetBuffer(2);
	ConstBuffer& src_vol  = m1_db.GetBuffer(3);
	ConstBuffer& src_time  = m1_db.GetBuffer(4);
	int faster_bars = src_open.GetCount();
	
	int period_mins = GetMinutePeriod();
	int period_secs = period_mins * 60;
	Time epoch(1970,1,1);
	int64 tdiff = 4*24*60*60; // seconds between thursday and monday
	Time t0 = epoch + tdiff; // thursday -> next monday, for W1 data
	int64 t0int = t0.Get();
	//   int64 t1int = epoch.Get();
	//   tdiff == t0int - t1int
	
	int exp_count = faster_bars / period_mins;
	for(int i = 0; i < GetBufferCount(); i++)
		GetBuffer(i).Reserve(exp_count);
	
	double chk_change = period_mins * point;
	
	for(; cursor2 < faster_bars; cursor2++) {
		double open_value = src_open.Get(cursor2);
		double low_value  = src_low.Get(cursor2);
		double high_value = src_high.Get(cursor2);
		double volume_sum = src_vol.Get(cursor2);
		int time = src_time.Get(cursor2);
		if (!time) return;
		
		if (!time_buf.GetCount())
			shift++;
		else {
			double diff = open_buf.Top() - open_value;
			if (fabs(diff) >= chk_change) shift++;
		}
		
		SetSafetyLimit(shift+1);
		if (shift >= open_buf.GetCount()) {
			if (shift >= open_buf.GetCount()) {
				open_buf.SetCount(shift+1);
				low_buf.SetCount(shift+1);
				high_buf.SetCount(shift+1);
				volume_buf.SetCount(shift+1);
				time_buf.SetCount(shift+1);
			}
			
			open_buf.Set(shift, open_value);
			low_buf.Set(shift, low_value);
			high_buf.Set(shift, high_value);
			volume_buf.Set(shift, volume_sum);
			time_buf.Set(shift, time);
		}
		else {
			if (low_buf.Get(shift)  > low_value)  {low_buf	.Set(shift, low_value);}
			if (high_buf.Get(shift) < high_value) {high_buf	.Set(shift, low_value);}
			volume_buf.Set(shift, volume_sum + volume_buf.Get(shift));
		}
	}
	
	ForceSetCounted(open_buf.GetCount());
}

void DataBridge::Assist(int cursor, VectorBool& vec) {
	if (cursor < 5 || cursor >= GetBars()) return;
	
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	// Open change
	{
		double cur = open.Get(cursor);
		for(int i = 0; i < 5; i++) {
			double prev = open.Get(cursor - i - 1);
			if (prev < cur)
				vec.Set(DB_UP1 + i, true);
			else
				vec.Set(DB_DOWN1 + i, true);
		}
	}
	
	
	// Open/Close trend
	{
		int dir = 0;
		int len = 0;
		for (int i = cursor-1; i >= 0; i--) {
			int idir = open.Get(i+1) > open.Get(i) ? +1 : -1;
			if (dir != 0 && idir != dir) break;
			dir = idir;
			len++;
		}
		if (len > 1) {
			if (dir == +1)
				vec.Set(DB_UPTREND, true);
			else
				vec.Set(DB_DOWNTREND, true);
		}
	}
	
	// High trend
	{
		int dir = 0;
		int len = 0;
		for (int i = cursor-2; i >= 0; i--) {
			int idir = high.Get(i+1) > high.Get(i) ? +1 : -1;
			if (dir != 0 && idir != dir) break;
			dir = idir;
			len++;
		}
		if (len > 1) {
			if (dir == +1)
				vec.Set(DB_HIGHUPTREND, true);
			else
				vec.Set(DB_HIGHDOWNTREND, true);
		}
	}
	
	// Low trend
	{
		int dir = 0;
		int len = 0;
		for (int i = cursor-2; i >= 0; i--) {
			int idir = low.Get(i+1) < low.Get(i) ? -1 : +1;
			if (dir != 0 && idir != dir) break;
			dir = idir;
			len++;
		}
		if (len > 1) {
			if (dir == +1)
				vec.Set(DB_LOWUPTREND, true);
			else
				vec.Set(DB_LOWDOWNTREND, true);
		}
	}
	
	// Sideways trend
	{
		int dir = 0;
		int len = 0;
		for (int i = cursor-1; i >= 0; i--) {
			double lowhigh_av = 0.0;
			for(int j = i; j < cursor; j++)
				lowhigh_av += high.Get(j) - low.Get(j);
			lowhigh_av /= cursor - i;
			
			double change = fabs(open.Get(cursor) - open.Get(i));
			bool is_less = change <= lowhigh_av;
			if (dir != 0 && !is_less) break;
			dir = 1;
			len++;
		}
		if (len > 1)
			vec.Set(DB_SIDEWAYSTREND, true);
	}
	
	// High break
	{
		int dir = 0;
		int len = 0;
		double hi = high.Get(cursor-1);
		for (int i = cursor-2; i >= 0; i--) {
			int idir = hi > high.Get(i) ? +1 : -1;
			if (dir != 0 && idir != +1) break;
			dir = idir;
			len++;
		}
		if (len > 1)
			vec.Set(DB_HIGHBREAK, true);
		if (len > 32)
			vec.Set(DB_LONGHIGHBREAK, true);
	}
	
	// Low break
	{
		int dir = 0;
		int len = 0;
		double lo = low.Get(cursor-1);
		for (int i = cursor-2; i >= 0; i--) {
			int idir = lo < low.Get(i) ? +1 : -1;
			if (dir != 0 && idir != +1) break;
			dir = idir;
			len++;
		}
		if (len > 1)
			vec.Set(DB_LOWBREAK, true);
		if (len > 32)
			vec.Set(DB_LONGLOWBREAK, true);
	}
	
	// Trend reversal
	if (cursor >= 4) {
		double t0_diff		= open.Get(cursor-0) - open.Get(cursor-1);
		double t1_diff		= open.Get(cursor-1) - open.Get(cursor-2);
		double t2_diff		= open.Get(cursor-2) - open.Get(cursor-3);
		int t0 = t0_diff > 0 ? +1 : -1;
		int t1 = t1_diff > 0 ? +1 : -1;
		int t2 = t2_diff > 0 ? +1 : -1;
		if (t0 * t1 == -1 && t1 * t2 == +1) {
			double t0_hilodiff	= high.Get(cursor-1) - low.Get(cursor-1);
			if (fabs(t0_hilodiff) >= fabs(t1_diff)) {
				if (t0 == +1)
					vec.Set(DB_REVERSALUP, true);
				else
					vec.Set(DB_REVERSALDOWN, true);
			} else {
				if (t0 == +1)
					vec.Set(DB_STOPUP, true);
				else
					vec.Set(DB_STOPDOWN, true);
			}
		}
	}
}

}
