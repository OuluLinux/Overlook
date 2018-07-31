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
	point = 0.0001;
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
		ASSERT(point == 0.01 || point == 0.0001);
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
	
	int sym_count = mt.GetSymbolCount();
	int cur_count = mt.GetCurrencyCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	int mt_period = GetPeriod();
	int tf = GetTf();
	
	if (tf == VTF) {
		RefreshFromFasterTime();
	}
	
	// Regular symbols
	#if REFRESH_FROM_FASTER
	else if (mt_period > 1) {
		RefreshFromFasterTime();
	}
	else
	#endif
	if (sym < sym_count) {
		bool init_round = GetCounted() == 0 && GetBuffer(0).GetCount() == 0;
		#ifndef flagSECONDS
		if (init_round) {
			String symstr = sys.GetSymbol(sym);
			cursor = 0;
			cursor2 = 0;
			for(int i = 0; i < buffers.GetCount(); i++)
				buffers[i]->SetCount(0);
			
			const Symbol& mtsym = mt.GetSymbol(sym);
			if (mt_period == 1)
				RefreshFromHistory(true);
			RefreshFromHistory(false);
		}
		#endif
		RefreshFromAskBid(init_round);
	}
	else if (sym < sym_count + cur_count) {
		RefreshCurrency();
	}
	else {
		RefreshNet();
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
	
	common.RefreshAskBidData(open_buf.IsEmpty());
	
	int id = GetSymbol();
	int tf = GetTimeframe();
	int counted = GetCounted();
	ASSERTEXC(id >= 0);
	double half_point = point * 0.5;
	
	const Vector<DataBridgeCommon::AskBid>& data = common.data[id];
	
	if (sys.GetSymbol(GetSymbol()) == "AUDSEK") {
		LOG("");
	}
	
	#ifdef flagSECONDS
	int64 step = GetMinutePeriod();
	#else
	int64 step = GetMinutePeriod() * 60;
	#endif
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
		#ifndef flagSECONDS
		time += 4*24*60*60; // 1.1.1970 is thursday, move to monday
		time = time - time % step;
		time -= 4*24*60*60;
		#endif
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
			
			open_buf.Set(shift, ask);
			low_buf.Set(shift, ask);
			high_buf.Set(shift, ask);
			time_buf.Set(shift, time);
		}
		else {
			double low  = low_buf.Get(shift);
			double high = high_buf.Get(shift);
			if (ask < low  && fabs((ask - low) / point) < 50)  {low_buf		.Set(shift, ask);}
			if (ask > high && fabs((ask - high) / point) < 50) {high_buf	.Set(shift, ask);}
		}
	}
	
	int64 time = GetUtcTime().Get() - epoch_begin;
	#ifndef flagSECONDS
	time -= time % 60;
	#endif
	SyncData(time, shift, open_buf.Top());
	
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
			FileIn fin(local_zip);
			UnZip unzip(fin);
			bool found = false;
			while(!(unzip.IsEof() || unzip.IsError())) {
				String fname = GetFileName(unzip.GetPath());
				LOG("Zip has file " << unzip.GetPath() << " (" << fname << ")");
				if (fname == exp_fname) {
					String dst = AppendFileName(data_dir, fname);
					FileOut fout(dst);
					unzip.ReadFile(fout);
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
	#ifndef flagSECONDS
	ASSERT(utc_time.second == 0);
	#endif
	int wday = DayOfWeek(utc_time);
	if (wday == 6 || (wday == 0 && utc_time.hour < 21) || (wday == 5 && utc_time.hour >= 22))
		return false;
	
	int minperiod = GetMinutePeriod();
	bool is_vtf = GetTf() == VTF;
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	Time t;
	if (shift < 0) {
		shift = -1;
		#ifndef flagSECONDS
		t = Time(1970,1,1) + Config::start_time - 60 * minperiod;
		#else
		t = Time(1970,1,1) + Config::start_time - 1 * minperiod;
		#endif
	} else {
		if (time_buf.IsEmpty())
			return false;
		shift = time_buf.GetCount() - 1;
		t = Time(1970,1,1) + time_buf.Top();
		ask = open_buf.Top();
		#ifndef flagSECONDS
		ASSERT(t.second == 0);
		#endif
	}
	if (utc_time < t)
		return false;
	
	if (t < utc_time) {
		shift++;
		#ifdef flagSECONDS
		t += 1 * minperiod;
		#else
		t += 60 * minperiod;
		#endif
		while (t < utc_time) {
			int wday = DayOfWeek(t);
			if ((!is_vtf && !(wday == 6 || (wday == 0 && t.hour < 22) || (wday == 5 && t.hour >= 21))) || (is_vtf && IsVtfTime(wday, t))) {
				int time = t.Get() - Time(1970,1,1).Get();
				
				int res = shift + 100000;
				res -= res % 100000;
				res += 100000;
				
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
			#ifdef flagSECONDS
			t += 1 * minperiod;
			#else
			t += 60 * minperiod;
			#endif
		}
	}
	else if (shift == -1) {
		shift++;
	}
	
	if (is_vtf && !IsVtfTime(wday, t))
		return false;
	
	return true;
}

bool DataBridge::IsVtfTime(int wday, const Time& t) {
	#define IS(h, m) {if (t.hour == h && t.minute == m) return true;}
	
	// NOTE: update GetVtfWeekbars if changed
	
	if (wday == 0) {
		IS(22,00);
		IS(22,59);
	}
	// Monday
	else if (wday == 1) {
		IS(2,00);
		IS(4,10);
		IS(4,16);
		IS(4,54);
		IS(7,00); // EU ses
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(12,00); // US ses
		IS(12,30); // first US M30 has gone
		IS(14,00);
		IS(15,00);
		IS(17,59);
		IS(18,57);
		IS(21,40);
		IS(23,37);
	}
	if (wday == 2) {
		IS(1,00);
		IS(2,00);
		IS(4,00);
		IS(5,30);
		IS(7,00); // EU ses
		IS(8,30);
		IS(10,00); // Strong
		IS(12,00); // US ses
		IS(12,30);
		IS(14,00);
		IS(14,57);
		IS(16,30);
		IS(18,00);
		IS(19,30);
		IS(20,42);
		IS(22,30);
		IS(22,59);
		IS(23,59);
	}
	else if (wday == 3) {
		IS(0,59);
		IS(1,30);
		IS(2,00);
		IS(2,46);
		IS(3,30);
		IS(4,00);
		IS(4,30);
		IS(5,30);
		IS(7,00); //EU ses
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(11,29);
		IS(12,00); //US ses
		IS(12,30);
		IS(13,30);
		IS(15,00);
		IS(16,30);
		IS(17,00);
		IS(18,00);
		IS(20,45);
		IS(22,00);
	}
	else if (wday == 4) {
		IS(0,59);
		IS(2,00);
		IS(3,05);
		IS(4,00);
		IS(5,30);
		IS(7,00);
		IS(8,30);
		IS(10,00);
		IS(11,00);
		IS(12,00);
		IS(12,30);
		IS(15,00);
		IS(16,30);
		IS(17,15);
		IS(18,00);
		IS(19,00);
		IS(20,00);
		IS(20,49);
		IS(21,35);
		IS(22,49);
		IS(23,59);
	}
	else if (wday == 5) {
		IS(0,59);
		IS(2,00);
		IS(3,00);
		IS(4,10);
		IS(4,54);
		IS(6,35);
		IS(7,00);
		IS(8,12);
		IS(9,00);
		IS(10,00);
		IS(11,00);
		IS(12,00);
		IS(12,30);
		IS(14,05);
		IS(15,29);
		IS(16,30);
		IS(18,00);
		IS(18,57);
		IS(19,59);
	}
	#undef IS
	return false;
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
	
	m1_db.refresh_lock.Enter();
	
	spread_mean = m1_db.spread_mean;
	point = m1_db.point;
	
	int period_mins = GetMinutePeriod();
	#ifdef flagSECONDS
	int period_secs = period_mins;
	#else
	int period_secs = period_mins * 60;
	#endif
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
		
		if (!SyncData(time, shift, open_value))
			continue;
		
		//if (!time_buf.GetCount() || time_buf.Top() < time) shift++;
		
		int count = min(min(min(min(open_buf.GetCount(), low_buf.GetCount()), high_buf.GetCount()), volume_buf.GetCount()), time_buf.GetCount());
		SetSafetyLimit(shift+1);
		if (shift >= count) {
			int res = shift - shift % 100000;
			res += 100000;
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
	
	m1_db.refresh_lock.Leave();
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
			open_buf.SetCount(shift+1);
			low_buf.SetCount(shift+1);
			high_buf.SetCount(shift+1);
			volume_buf.SetCount(shift+1);
			time_buf.SetCount(shift+1);
			
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

void DataBridge::RefreshCurrency() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int shift = open_buf.GetCount() - 1;
	
	typedef Tuple<int, bool, ConstBuffer*, double> Src;
	Vector<Src> src_open;
	const Index<int>& syms = sys.currency_sym_dirs.Get(sys.GetSymbol(GetSymbol()));
	src_open.SetCount(syms.GetCount());
	int bars = INT_MAX;
	
	DUMP(inputs[0].GetCount());
	for(int i = 0; i < inputs[0].GetCount(); i++) {
		LOG(i << " " << inputs[0][i].sym);
	}
	String symstr = sys.GetSymbol(GetSymbol());
	LOG(symstr);
	
	spread_mean = syms.GetCount() * 3;
	
	for(int i = 0; i < syms.GetCount(); i++) {
		Src& src = src_open[i];
		src.a = syms[i]; // symbol
		if (src.a >= 0) {
			src.b = false; // dir (0 pos, 1 neg)
		} else {
			src.b = true;
			src.a = -src.a-1;
		}
		String sym = sys.GetSymbol(src.a);
		src.c = &GetInputBuffer(0, src.a, GetTf(), 0);
		bars = min(bars, src.c->GetCount());
		src.d = dynamic_cast<DataBridge&>(*GetInputCore(0, src.a, GetTf())).GetPoint();
		ASSERT(src.d == 0.0001 || src.d == 0.01);
	}
	for(int i = 0; i < src_open.GetCount(); i++) {ASSERT(src_open[i].d != 0);}
	
	if (bars == INT_MAX) return;
	
	point = 0.0001;
	
	int counted = open_buf.GetCount();
	open_buf	.SetCount(bars);
	low_buf		.SetCount(bars);
	high_buf	.SetCount(bars);
	volume_buf	.SetCount(bars);
	time_buf	.SetCount(bars);
	ConstBuffer& src_time_buf = GetInputBuffer(0, src_open[0].a, GetTf(), 4);
	for(int i = counted; i < bars; i++) {
		
		time_buf.Set(i, src_time_buf.Get(i));
		
		int pip_sum = 0;
		if (i > 0) for(int j = 0; j < src_open.GetCount(); j++) {
			Src& src = src_open[j];
			ConstBuffer& open = *src.c;
			double o0 = open.Get(i);
			double o1 = open.Get(i-1);
			ASSERT(src.d > 0.0);
			int pips = (o0 - o1) / src.d;
			if (src.b == false)
				pip_sum += pips;
			else
				pip_sum -= pips;
		}
		
		double d = i == 0 ? 1.0 : open_buf.Get(i-1);
		d += pip_sum * point;
		if (i > 0 && d < low_buf.Get(i-1))	low_buf.Set(i-1, d);
		if (i > 0 && d > high_buf.Get(i-1))	high_buf.Set(i-1, d);
		open_buf.Set(i, d);
		low_buf.Set(i, d);
		high_buf.Set(i, d);
	}
	
}

void DataBridge::RefreshNet() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	Buffer& time_buf = GetBuffer(4);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int shift = open_buf.GetCount() - 1;
	
	struct Src : Moveable<Src> {
		int a;
		int b;
		ConstBuffer* c = NULL;
		double d;
		ConstBuffer* low = NULL, *high = NULL;
	};
	Vector<Src> src_open;
	const System::NetSetting& net = sys.GetSymbolNet(GetSymbol());
	src_open.SetCount(net.symbol_ids.GetCount());
	int bars = INT_MAX;
	
	spread_mean = net.symbol_ids.GetCount() * 3;
	
	for(int i = 0; i < net.symbol_ids.GetCount(); i++) {
		Src& src = src_open[i];
		src.a = net.symbol_ids.GetKey(i); // symbol
		src.b = net.symbol_ids[i];
		src.c = &GetInputBuffer(0, src.a, GetTf(), 0);
		src.low = &GetInputBuffer(0, src.a, GetTf(), 1);
		src.high = &GetInputBuffer(0, src.a, GetTf(), 2);
		bars = min(bars, src.c->GetCount());
		src.d = dynamic_cast<DataBridge&>(*GetInputCore(0, src.a, GetTf())).GetPoint();
		ASSERT(src.d == 0.0001 || src.d == 0.01);
	}
	for(int i = 0; i < src_open.GetCount(); i++) {ASSERT(src_open[i].d != 0);}
	
	if (bars == INT_MAX) return;
	
	point = 0.0001;
	
	int counted = open_buf.GetCount();
	if (counted) counted--;
	open_buf	.SetCount(bars);
	low_buf		.SetCount(bars);
	high_buf	.SetCount(bars);
	volume_buf	.SetCount(bars);
	time_buf	.SetCount(bars);
	ConstBuffer& src_time_buf = GetInputBuffer(0, src_open[0].a, GetTf(), 4);
	for(int i = counted; i < bars; i++) {
		
		time_buf.Set(i, src_time_buf.Get(i));
		
		int pip_sum = 0, low_pip_sum = 0, high_pip_sum = 0;
		if (i > 0) for(int j = 0; j < src_open.GetCount(); j++) {
			Src& src = src_open[j];
			ConstBuffer& open = *src.c;
			ConstBuffer& low = *src.low;
			ConstBuffer& high = *src.high;
			double o0 = open.Get(i);
			double o1 = open.Get(i-1);
			double l = low.Get(i);
			double h = high.Get(i);
			ASSERT(src.d > 0.0);
			int pips = (o0 - o1) / src.d;
			if (src.b > 0)			pip_sum += pips;
			else if (src.b < 0)		pip_sum -= pips;
			int low_pips = (l - o0) / src.d;
			if (src.b > 0)			low_pip_sum += low_pips;
			else if (src.b < 0)		low_pip_sum -= low_pips;
			int high_pips = (h - o0) / src.d;
			if (src.b > 0)			high_pip_sum += high_pips;
			else if (src.b < 0)		high_pip_sum -= high_pips;
		}
		
		double d = i == 0 ? 1.0 : open_buf.Get(i-1);
		d += pip_sum * point;
		double l = d + low_pip_sum * point;
		double h = d + high_pip_sum * point;
		if (i > 0 && d < low_buf.Get(i-1))	low_buf.Set(i-1, d);
		if (i > 0 && d > high_buf.Get(i-1))	high_buf.Set(i-1, d);
		open_buf.Set(i, d);
		low_buf.Set(i, min(l, d));
		high_buf.Set(i, max(h, d));
	}
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

double DataBridge::GetDigits() const {
	int Digits = 1;
	for (;; Digits++) {
		double d = point * pow(10, Digits);
		if (d >= 1.0)
			break;
	}
	return Digits;
}

}
