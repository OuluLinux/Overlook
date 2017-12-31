#include "Overlook.h"
#include <plugin/zip/zip.h>

namespace Overlook {

DataBridge::DataBridge() {
	SetSkipAllocate();
	cursor = 0;
	buffer_cursor = 0;
	data_begin = 0;
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
		
		// Correlation unit setup
		if (GetSymbol() == sys.GetStrongSymbol()) {
			if (corr.IsEmpty()) {
				corr.Add();
				
				corr[0].sym_ids.SetCount(SYM_COUNT, -1);
				for(int i = 0; i < SYM_COUNT; i++)
					corr[0].sym_ids[i] = sys.GetPrioritySymbol(i);
				
				corr[0].averages.SetCount(SYM_COUNT-1);
				corr[0].buffer.SetCount(SYM_COUNT-1);
			}
			
			corr[0].period = 10;
			
			corr[0].opens.SetCount(SYM_COUNT, 0);
			for(int i = 0; i < SYM_COUNT; i++) {
				ConstBuffer& open = GetInputBuffer(0, corr[0].sym_ids[i], GetTimeframe(), 0);
				corr[0].opens[i] = &open;
			}
		}
	}
	
	int sym_count = common.GetSymbolCount();
	int cur_count = mt.GetCurrencyCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	int mt_period = GetPeriod() * GetSystem().GetBasePeriod() / 60;
	
	// Account symbol
	if (sym == sys.GetAccountSymbol()) {
		RefreshAccount();
	}
	else if (sym == sys.GetStrongSymbol()) {
		RefreshStrong();
	}
	else
	
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
				RefreshFromHistory(false, true);
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


void DataBridge::RefreshAccount() {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	
	int bars = GetBars();
	int counted = GetCounted();
	if (counted > bars) counted = bars; // weird bug
	
	// Allocate memory
	ASSERT(bars > 0);
	SetSafetyLimit(bars);
	for(int i = 0; i < outputs[0].buffers.GetCount(); i++)
		outputs[0].buffers[i].SetCount(bars);
	double prev_open = counted ? open_buf.Get(counted-1) : 0.0;
	for(int i = counted; i < bars; i++) {
		open_buf.Set(i, prev_open);
		low_buf.Set(i, prev_open);
		high_buf.Set(i, prev_open);
	}
}

void DataBridge::RefreshStrong() {
	RefreshCorrelation();
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	
	
	int bars = GetBars();
	int counted = GetCounted();
	SetSafetyLimit(bars);
	
	open_buf.SetCount(bars);
	low_buf.SetCount(bars);
	high_buf.SetCount(bars);
	volume_buf.SetCount(bars);
	
	if (!counted) {
		open_buf.Set(0, 1.0);
		low_buf.Set(0, 1.0);
		high_buf.Set(0, 1.0);
		counted++;
	}
	for(int i = counted; i < bars; i++) {
		double chng_sum = 0.0;
		for(int j = 0; j < SYM_COUNT; j++) {
			ConstBuffer& buf = *corr[0].opens[j];
			double prev = buf.Get(i-1);
			double curr = buf.Get(i);
			double chng = curr / prev - 1.0;
			double corr = j > 0 ? this->corr[0].buffer[j-1].Get(i) : +1.0;
			double valu = chng * corr;
			chng_sum += valu;
		}
		
		chng_sum /= SYM_COUNT;
		
		double prev		= open_buf.Get(i-1);
		double value	= prev * (1.0 + chng_sum);
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
		
		low_buf		.Set(i-1, Upp::min(value, prev));
		high_buf	.Set(i-1, Upp::max(value, prev));
	}
}

void DataBridge::ProcessCorrelation(int output) {
	int counted = GetCounted();
	int bars = GetBars();
	
	ConstBuffer& a		= *corr[0].opens[0];
	ConstBuffer& b		= *corr[0].opens[output+1];
	Buffer& buf			=  corr[0].buffer[output];
	OnlineAverage2& s	=  corr[0].averages[output];
	
	if (b.GetCount() == 0) {
		LOG("CorrelationOscillator error: No data for output " << output);
		return;
	}
	
	int period = corr[0].period;
	
	if (counted < period) {
		ASSERT(counted == 0);
		s.mean_a = a.Get(0);
		s.mean_b = b.Get(0);
		s.count = 1;
		for(int i = 1; i < period; i++) {
			SetSafetyLimit(i);
			s.Add(a.Get(i), b.Get(i));
		}
		counted = period;
	}
	SetSafetyLimit(counted);
	
	Vector<double> cache_a, cache_b;
	cache_a.SetCount(period);
	cache_b.SetCount(period);
	
	int cache_offset = 0;
	for(int i = 1; i < period; i++) {
		cache_a[i] = a.Get(counted - period + i);
		cache_b[i] = b.Get(counted - period + i);
	}
	
	buf.SetCount(bars);
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		double da = a.Get(i);
		double db = b.Get(i);
		
		s.Add(da, db);
		cache_a[cache_offset] = da;
		cache_b[cache_offset] = db;
		cache_offset = (cache_offset + 1) % period;
		
		double avg1 = s.mean_a;
		double avg2 = s.mean_b;
		double sum1 = 0;
	    double sumSqr1 = 0;
	    double sumSqr2 = 0;
		for(int j = 0; j < period; j++) {
			double v1 = cache_a[j];
			double v2 = cache_b[j];
			sum1 += (v1 - avg1) * (v2 - avg2);
			sumSqr1 += pow(v1 - avg1, 2.0);
			sumSqr2 += pow(v2 - avg2, 2.0);
		}
    
		double mul = sumSqr1 * sumSqr2;
		double correlation_coef = mul != 0.0 ? sum1 / sqrt(mul) : 0;
		
		buf.Set(i, correlation_coef);
	}
}

void DataBridge::RefreshCorrelation() {
	int counted = GetCounted();
	int bars = GetBars();
	
	if (counted == bars)
		return;
	
	if (counted > 0) counted = Upp::max(counted - period, 0);
	
	int k = 0;
	for(int i = 0; i < SYM_COUNT-1; i++) {
		ProcessCorrelation(i);
	}
}

void DataBridge::RefreshFromAskBid(bool init_round) {
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
		
	System& bs = GetSystem();
	DataBridgeCommon& common = GetDataBridgeCommon();
	
	common.RefreshAskBidData();
	
	int id = GetSymbol();
	int tf = GetTimeframe();
	int bars = GetBars();
	int counted = GetCounted();
	if (counted > bars) counted = bars; // weird bug
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
	
	for(; cursor < data.GetCount(); cursor++) {
		
		// Get local references
		const DataBridgeCommon::AskBid& askbid = data[cursor];
		const Time& t = askbid.a;
		const double& ask = askbid.b;
		const double& bid = askbid.c;
		
		
		// Find min/max
		double diff = ask - bid;
		int step = (int)(diff / point);
		AddSpread(step);
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (step > max_value) max_value = step;
		if (step < min_value) min_value = step;
		
		
		// Get shift in data from time
		int h, d, dh;
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		} else {
			int wday = DayOfWeek(t);
			h = (t.minute + t.hour * 60) / period;
			d = wday - 1;
			dh = h + d * h_count;
			if (wday == 0 || wday == 6) continue;
		}
		int shift = bs.GetShiftFromTimeTf(t, tf);
		if (shift >= bars) {
			break;
		}
		
		
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
		SetSafetyLimit(bars);
		double prev_open = open_buf.Get(buffer_cursor-1);
		for(int i = buffer_cursor+1; i < bars; i++) {
			open_buf.Set(i, prev_open);
			low_buf.Set(i, prev_open);
			high_buf.Set(i, prev_open);
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

void DataBridge::RefreshFromHistory(bool use_internet_data, bool update_only) {
	Time now = GetSysTime();
	
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	common.DownloadHistory(GetSymbol(), GetTf(), false);
	
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
	bool join_bars = false;
	int sub_count = 0;
	if (tf >= mt.GetTimeframeCount()) {
		sub_count = mt_period / 240;
		ASSERT(mt_period % 240 == 0);
		mt_period = 240;
		join_bars = true;
	}
	String symbol = bs.GetSymbol(GetSymbol());
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
	double prev_close;
	double open = 0;
	
	Buffer& open_buf = GetBuffer(0);
	Buffer& low_buf = GetBuffer(1);
	Buffer& high_buf = GetBuffer(2);
	Buffer& volume_buf = GetBuffer(3);
	
	// Seek to begin of the data
	int cursor = (4+64+12+4+4+4+4 +13*4);
	int count = 0;
	int expected_count = (int)((src.GetSize() - cursor) / struct_size);
	src.Seek(cursor);
	
	bool overwrite = false;
	double prev_open = 0.0;
	
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
		
		cursor += struct_size;
		Time time2 = TimeFromTimestamp(time);
		int wday = DayOfWeek(time2);
		if (wday == 0 || wday == 6) continue;
		int shift = bs.GetShiftFromTimeTf(time2, tf);
		if (shift < 0) continue;
		if (shift >= bars) break;
		
		// At first value
		bool set_data_begin = count == 0 && !update_only;
		if (set_data_begin) {
			prev_close = close;
		}
		else {
			if (join_bars) {
				if (shift < count) {
					double prev_low = low_buf.Get(count-1);
					if (low < prev_low) low_buf.Set(count-1, low);
					
					double prev_high = high_buf.Get(count-1);
					if (high > prev_high) high_buf.Set(count-1, high);
					
					volume_buf.Inc(count-1, (int)tick_volume);
					continue;
				}
			} else {
				if (shift < count)
					continue;
			}
		}
		
		while (count < shift && count < bars) {
			if (!update_only || overwrite) {
				SetSafetyLimit(count);
				open_buf.Set(count, prev_close);
				low_buf.Set(count, prev_close);
				high_buf.Set(count, prev_close);
				volume_buf.Set(count, 0);
			}
			count++;
		}
		
		if (set_data_begin) {
			data_begin = count;
		}
		
		prev_close = close;
		
		SetSafetyLimit(count);
		double cur_open = open_buf.Get(count);
		
		bool updated_bar = false;
		if (count < bars && shift == count) {
			SetSafetyLimit(count);
			if (update_only && count) {
				if (open_buf.Get(count) == prev_open &&
					open != prev_open &&
					TimeFromTimestamp(time) > (now - 2*7*24*60*60))
					overwrite = true;
			}
			if (!update_only || overwrite) {
				//ASSERT(bs.GetTime(tf, count) == TimeFromTimestamp(time));
				open_buf.Set(count, open);
				low_buf.Set(count, low);
				high_buf.Set(count, high);
				volume_buf.Set(count, tick_volume);
				updated_bar = true;
			}
			count++;
		}
		
		prev_open = cur_open;
		
		
		// Find min/max
		if (updated_bar) {
			double diff = count >= 2 ? open_buf.Get(count-1) - open_buf.Get(count-2) : 0.0;
			int step = (int)(diff / point);
			if (step >= 0) median_max_map.GetAdd(step, 0)++;
			else median_min_map.GetAdd(step, 0)++;
			if (step > max_value) max_value = step;
			if (step < min_value) min_value = step;
		}
		
		//LOG(Format("%d: %d %f %f %f %f %d %d %d", cursor, (int)time, open, high, low, close, tick_volume, spread, real_volume));
	}
	
	
	if (data_begin > bars -10000) {
		LOG("WARNING " << symbol);
	}
	
	RefreshMedian();
	
	// Fill volume querytable
	bool five_mins = GetMinutePeriod() < 5;
	int steps = five_mins ? 5 : 1;
	
	
	ForceSetCounted(count);
	buffer_cursor = count-1;
	ASSERT(buffer_cursor >= 0);
	
	while (count < bars) {
		SetSafetyLimit(count);
		open_buf.Set(count, open);
		low_buf.Set(count, open);
		high_buf.Set(count, open);
		volume_buf.Set(count, 0);
		count++;
	}
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
		int pos = sys.GetShiftTf(tf, 0, i);
		int nextpos = sys.GetShiftTf(tf, 0, i+1);
		
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

void DataBridge::Assist(AssistBase& ab, int cursor) {
	if (cursor < 1 || cursor >= GetBars()) return;
	
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
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
		if (len > 1)
			ab.Add(String(dir == +1 ? "Up " : "Down ") + "trend of " + IntStr(len));
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
		if (len > 1)
			ab.Add("High " + String(dir == +1 ? "up" : "down") + " trend of " + IntStr(len));
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
		if (len > 1)
			ab.Add("Low " + String(dir == +1 ? "up" : "down") + " trend of " + IntStr(len));
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
			ab.Add("Sideways trend of " + IntStr(len));
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
			ab.Add("High break of " + IntStr(len));
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
			ab.Add("Low break of " + IntStr(len));
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
				ab.Add("Trend reversal " + String(t0 == +1 ? "up" : "down"));
			} else {
				ab.Add("Trend stop, start sideways " + String(t0 == +1 ? "up" : "down"));
			}
		}
	}
}

}
