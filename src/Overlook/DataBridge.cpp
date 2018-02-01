#include "Overlook.h"
#include <plugin/zip/zip.h>

namespace Overlook {

DataBridge::DataBridge() {
	SetSkipAllocate();
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
	#if ONLY_M1_SOURCE
	if (mt_period > 1 && sym < sym_count) {
		RefreshFromFaster();
	} else
	#endif

	if (sym < sym_count) {
		bool init_round = GetCounted() == 0;
		if (init_round) {
			const Symbol& mtsym = mt.GetSymbol(sym);
			bool use_internet_data =
				Config::use_internet_m1_data &&
				mt_period == 1 &&
				mtsym.IsForex();
			if (use_internet_data) {
				RefreshFromHistory(true);
				RefreshFromHistory(false);
			} else {
				RefreshFromHistory(false);
			}
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
	System& sys = GetSystem();
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	DataBridgeCommon& common = GetDataBridgeCommon();
	
	common.RefreshAskBidData();
	
	int id = GetSymbol();
	int tf = GetTimeframe();
	int counted = GetCounted();
	ASSERTEXC(id >= 0);
	double half_point = point * 0.5;
	
	const Vector<DataBridgeCommon::AskBid>& data = common.data[id];
	
	if (!counted) sys.DataTimeBegin(id, tf);
	
	int64 step = GetMinutePeriod() * 60;
	
	for(; cursor < data.GetCount(); cursor++) {
		
		// Get local references
		const DataBridgeCommon::AskBid& askbid = data[cursor];
		const Time& t = askbid.a;
		const double& ask = askbid.b;
		const double& bid = askbid.c;
		Time utc_time = sys.TimeFromBroker(t);
		utc_time.second = 0;
		if (utc_time >= sys.GetEnd())
			break;
		
		
		int64 time = utc_time.Get();
		time += 4*24*60*60;
		time = time - time % step;
		time -= 4*24*60*60;
		utc_time.Set(time);
		
		
		int shift = sys.DataTimeAdd(id, tf, utc_time);
		if (shift == -1)
			continue;
		
		
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
				open_buf.SetCount(shift+1);
				low_buf.SetCount(shift+1);
				high_buf.SetCount(shift+1);
				volume_buf.SetCount(shift+1);
			}
			
			open_buf.Set(shift, ask);
			low_buf.Set(shift, ask);
			high_buf.Set(shift, ask);
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
	
	sys.DataTimeEnd(id, tf);
	
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
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	common.DownloadHistory(GetSymbol(), GetTf(), false);
	
	MetaTrader& mt = GetMetaTrader();
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
	if (!FileExists(local_history_file)) {
		DUMP(local_history_file);
		throw DataExc();
	}
	
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
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	int expected_count = (int)((src.GetSize() - cursor) / struct_size);
	src.Seek(cursor);
	
	open_buf.Reserve(expected_count);
	low_buf.Reserve(expected_count);
	high_buf.Reserve(expected_count);
	volume_buf.Reserve(expected_count);
	
	bool overwrite = false;
	double prev_open = 0.0;
	
	if (!GetCounted()) sys.DataTimeBegin(id, tf);
	ASSERT(!GetCounted());
	ASSERT(open_buf.GetCount() == 0);
	
	int64 step = GetMinutePeriod() * 60;
	
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
			time  = *((int64*)current);				current += 4;
			open  = *((double*)current);			current += 8;
			high  = *((double*)current);			current += 8;
			low   = *((double*)current);			current += 8;
			close = *((double*)current);			current += 8;
			tick_volume  = *((double*)current);		current += 8;
			spread       = 0;
			real_volume  = 0;
		}
		
		time += 4*24*60*60;
		time = time - time % step;
		time -= 4*24*60*60;
		
		Time utc_time = sys.TimeFromBroker(Time(1970,1,1) + time);
		if (utc_time >= sys.GetEnd()) break;
		
		cursor += struct_size;
		int shift = sys.DataTimeAdd(id, tf, utc_time);
		if (shift == -1) break;
		
		if (shift >= open_buf.GetCount()) {
			open_buf.SetCount(shift+1);
			low_buf.SetCount(shift+1);
			high_buf.SetCount(shift+1);
			volume_buf.SetCount(shift+1);
		}
		SetSafetyLimit(shift+1);
		
		
		//ASSERT(bs.GetTime(tf, count) == TimeFromTimestamp(time));
		open_buf.Set(shift, open);
		low_buf.Set(shift, low);
		high_buf.Set(shift, high);
		volume_buf.Set(shift, tick_volume);
		
		
		int count = open_buf.GetCount();
		double diff = count >= 2 ? open_buf.Get(count-1) - open_buf.Get(count-2) : 0.0;
		int step = (int)(diff / point);
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	sys.DataTimeEnd(id, tf);
	
	RefreshMedian();
	
	// Fill volume querytable
	bool five_mins = GetMinutePeriod() < 5;
	int steps = five_mins ? 5 : 1;
	
	ForceSetCounted(open_buf.GetCount());
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

void DataBridge::RefreshFromFaster() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int counted = GetCounted();
	int bars = GetBars();
	
	int sym = GetSymbol();
	int tf = GetTf();
	ASSERT(tf > 0);
	
	DataBridge& m1_db = dynamic_cast<DataBridge&>(*GetInputCore(0, sym, 0));
	ConstBuffer& src_open = m1_db.GetBuffer(0);
	ConstBuffer& src_low  = m1_db.GetBuffer(1);
	ConstBuffer& src_high = m1_db.GetBuffer(2);
	ConstBuffer& src_vol  = m1_db.GetBuffer(3);
	int src_count = src_open.GetCount();

	
	// Allocate memory
	ASSERT(bars > 0);
	SetSafetyLimit(bars);
	for(int i = 0; i < outputs[0].buffers.GetCount(); i++)
		outputs[0].buffers[i].SetCount(bars);
	double prev_open = counted ? open.Get(counted-1) : src_open.Get(0);
	for(int i = counted; i < bars; i++) {
		open.Set(i, prev_open);
		low.Set(i, prev_open);
		high.Set(i, prev_open);
	}
	
	
	for(int i = counted; i < bars; i++) {
		int pos = sys.GetShiftTf(sym, tf, sym, 0, i);
		int nextpos = sys.GetShiftTf(sym, tf, sym, 0, i+1);
		
		if ((i % 10) == 0) {
			sys.WhenSubProgress(i, bars);
		}
		
		SetSafetyLimit(i);
		
		pos = Upp::min(pos, src_open.GetCount() - 1);
		double open_value = src_open.Get(pos);
		double low_value = src_low.Get(pos);
		double high_value = src_high.Get(pos);
		double volume_sum = src_vol.Get(pos);
		
		for(int j = pos+1; j < nextpos && j < src_count; j++) {
			low_value  = Upp::min(low_value,  src_low.Get(j));
			high_value = Upp::max(high_value, src_high.Get(j));
			volume_sum += src_vol.Get(j);
		}
		
		open.Set(i, open_value);
		low.Set(i, low_value);
		high.Set(i, high_value);
		volume.Set(i, volume_sum);
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

}
