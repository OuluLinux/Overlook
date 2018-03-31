#include "Overlook.h"
#include <plugin/cctz/civil_time.h>
#include <plugin/cctz/time_zone.h>

namespace Overlook {

SyncedPrice::SyncedPrice() {
	data.SetCount(USEDSYMBOL_COUNT);
}

String SyncedPrice::GetPath() {
	String dir = ConfigFile("SyncedPrice");
	RealizeDirectory(dir);
	String file = AppendFileName(dir, IntStr(period) + ".bin");
	return file;
}

void SyncedPrice::LoadThis() {
	LoadFromFile(*this, GetPath());
}

void SyncedPrice::StoreThis() {
	StoreToFile(*this, GetPath());
}

void SyncedPrice::Refresh() {
	lock.Enter();

	System& sys = GetSystem();
	Vector<int>& used_symbols_id = sys.used_symbols_id;
	ASSERT(period > 0 && tf >= 0);
	
	for(int i = 0; i < used_symbols_id.GetCount(); i++) {
		int sym = used_symbols_id[i];
		sys.data[sym][0].db.Start();
		sys.data[sym][tf].db.Start();
	}
	
	int initial_cursor = loadsource_cursor;
	
	while (true) {
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		int smallest_ids[sym_count];
		int smallest_id_count = 0;
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int count = db.open.GetCount();
			int next = data[i].loadsource_pos + 1;
			if (next >= count) continue;
			
			int time = db.time[next];
			if (time < smallest_time) {
				smallest_time = time;
				smallest_id_count = 1;
				smallest_ids[0] = i;
			}
			else if (time == smallest_time) {
				smallest_ids[smallest_id_count++] = i;
			}
		}
		if (smallest_time == INT_MAX)
			break;
		
		
		// Increase cursor with all which has the same time next
		for(int i = 0; i < smallest_id_count; i++) {
			data[smallest_ids[i]].loadsource_pos++;
		}
		
		int reserve = loadsource_cursor - loadsource_cursor % 100000 + 100000;
		
		Time t = Time(1970,1,1) + smallest_time;
		time.Reserve(reserve);
		time.SetCount(loadsource_cursor+1);
		time[loadsource_cursor] = t;
		
		for(int i = 0; i < sym_count; i++) {
			Data& d = data[i];
			
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int pos = d.loadsource_pos;
			//LOG(loadsource_cursor << "\t" << i << "\t" << pos << "/" << db.open.GetCount());
			
			d.open.Reserve(reserve);
			d.low.Reserve(reserve);
			d.high.Reserve(reserve);
			
			d.open.SetCount(loadsource_cursor+1);
			d.low.SetCount(loadsource_cursor+1);
			d.high.SetCount(loadsource_cursor+1);
			
			d.open[loadsource_cursor] = db.open[pos];
			d.low[loadsource_cursor]  = db.low[pos];
			d.high[loadsource_cursor] = db.high[pos];
		}
		
		loadsource_cursor++;
	}
	
	for(int i = 0; i < sym_count; i++) {
		int sym = used_symbols_id[i];
		DataBridge& db = sys.data[sym][tf].db;
		data[i].point = db.GetPoint();
		data[i].spread = db.GetSpread();
		if (data[i].point <= 0) Panic("Invalid point");
		if (data[i].spread < data[i].point * 2)
			data[i].spread = data[i].point * 4;
		ReleaseLog("Automation::LoadSource sym " + IntStr(sym) + " point " + DblStr(data[i].point) + " spread " + DblStr(data[i].spread));
	}
	
	if (initial_cursor != loadsource_cursor)
		StoreThis();
	
	lock.Leave();
}

SyncedPriceManager::SyncedPriceManager() {
	
}

SyncedPrice& SyncedPriceManager::Get(int tf) {
	int i = data.Find(tf);
	if (i != -1) return data[i];
	SyncedPrice& p = data.Add(tf);
	p.tf = tf;
	p.period = GetSystem().GetPeriod(tf);
	p.LoadThis();
	return p;
}







BitProcess::BitProcess() {
	
}

void BitProcess::Refresh() {
	SyncedPrice& sp = GetSyncedPriceManager().Get(tf);
	sp.Refresh();
	int loadsource_cursor = sp.loadsource_cursor;
	
	int initial_cursor = processbits_cursor;
	
	for (; processbits_cursor < loadsource_cursor; processbits_cursor++) {
		#ifdef flagDEBUG
		if (processbits_cursor == 10000) {
			processbits_cursor = loadsource_cursor - 1;
			break;
		}
		#endif
		
		int reserve = processbits_cursor - processbits_cursor % 100000 + 100000;
		bits_buf.Reserve(reserve);
		bits_buf.SetCount((processbits_cursor + 1) * processbits_row_size, 0);
		
		int bit_pos = 0;
		for(int j = 0; j < processbits_period_count; j++) {
			ProcessBitsSingle(sp, j, bit_pos);
		}
		ASSERT(bit_pos == processbits_inputrow_size);
	}
	
	if (initial_cursor != processbits_cursor)
		StoreThis();
}

String BitProcess::GetPath() {
	String dir = ConfigFile("BitProcess");
	RealizeDirectory(dir);
	String file = AppendFileName(dir, IntStr(used_sym) + "_" + IntStr(tf) + ".bin");
	return file;
}

void BitProcess::Serialize(Stream& s) {
	s % bits_buf
	  % av_wins0
	  % av_wins1
	  % av_wins2
	  % av_wins3
	  % av_wins4
	  % av_wins5
	  % ec0
	  % ec1
	  % ec2
	  % ec3
	  % ec4
	  % ec5
	  % used_sym % tf
	  % processbits_cursor;
}

void BitProcess::LoadThis() {
	LoadFromFile(*this, GetPath());
}

void BitProcess::StoreThis() {
	StoreToFile(*this, GetPath());
}

void BitProcess::SetBit(int pos, int bit, bool b) {
	ASSERT(bit >= 0 && bit < processbits_row_size);
	int64 i = pos* processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < bits_buf.GetCount());
	uint64* it = bits_buf.Begin() + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

bool BitProcess::GetBit(int pos, int bit) const {
	int64 i = pos * processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < bits_buf.GetCount());
	ConstU64* it = bits_buf.Begin() + j;
	return *it & (1ULL << k);
}

void BitProcess::ProcessBitsSingle(const SyncedPrice& data, int period_id, int& bit_pos) {
	double open1 = data.data[used_sym].open[processbits_cursor];
	double point = data.data[used_sym].point;
	ASSERT(point >= 0.000001 && point < 0.1);
	
	int cursor = processbits_cursor;
	const double* open_buf = data.data[used_sym].open.Begin();
	
	
	// MovingAverage
	double prev, ma;
	switch (period_id) {
		case 0: prev = av_wins0.GetMean(); av_wins0.Add(open1); ma = av_wins0.GetMean(); break;
		case 1: prev = av_wins1.GetMean(); av_wins1.Add(open1); ma = av_wins1.GetMean(); break;
		case 2: prev = av_wins2.GetMean(); av_wins2.Add(open1); ma = av_wins2.GetMean(); break;
		case 3: prev = av_wins3.GetMean(); av_wins3.Add(open1); ma = av_wins3.GetMean(); break;
		case 4: prev = av_wins4.GetMean(); av_wins4.Add(open1); ma = av_wins4.GetMean(); break;
		case 5: prev = av_wins5.GetMean(); av_wins5.Add(open1); ma = av_wins5.GetMean(); break;
		default: Panic("Invalid period id");
	}
	SetBitCurrent(bit_pos++, ma < prev);
	SetBitCurrent(bit_pos++, open1 < prev);
	
	
	// OnlineMinimalLabel
	double cost	 = point * (1 + period_id);
	const int count = 1;
	bool sigbuf[count];
	int begin = Upp::max(0, cursor - 200);
	int end = cursor + 1;
	OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
	bool label = sigbuf[count - 1];
	SetBitCurrent(bit_pos++, label);

	
	// TrendIndex
	bool bit_value;
	int period = 1 << (1 + period_id);
	double err, av_change, buf_value;
	TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
	SetBitCurrent(bit_pos++, buf_value > 0.0);
	
	
	// Momentum
	begin = Upp::max(0, cursor - period);
	double open2 = open_buf[begin];
	double value = open1 / open2 - 1.0;
	label = value < 0.0;
	SetBitCurrent(bit_pos++, label);
	
	
	// Open/Close trend
	period = 1 << period_id;
	int dir = 0;
	int len = 0;
	if (cursor >= period * 3) {
		for (int i = cursor-period; i >= 0; i -= period) {
			int idir = open_buf[i+period] > open_buf[i] ? +1 : -1;
			if (dir != 0 && idir != dir) break;
			dir = idir;
			len++;
		}
	}
	SetBitCurrent(bit_pos++, len > 2);


	// High break
	dir = 0;
	len = 0;
	if (cursor >= period * 3) {
		double hi = open_buf[cursor-period];
		for (int i = cursor-1-period; i >= 0; i -= period) {
			int idir = hi > open_buf[i] ? +1 : -1;
			if (dir != 0 && idir != +1) break;
			dir = idir;
			len++;
		}
	}
	SetBitCurrent(bit_pos++, len > 2);
	
	
	// Low break
	dir = 0;
	len = 0;
	if (cursor >= period * 3) {
		double lo = open_buf[cursor-period];
		for (int i = cursor-1-period; i >= 0; i -= period) {
			int idir = lo < open_buf[i] ? +1 : -1;
			if (dir != 0 && idir != +1) break;
			dir = idir;
			len++;
		}
	}
	SetBitCurrent(bit_pos++, len > 2);
	
	
	// Trend reversal
	int t0 = +1;
	int t1 = +1;
	int t2 = -1;
	if (cursor >= 4*period) {
		double t0_diff		= open_buf[cursor-0*period] - open_buf[cursor-1*period];
		double t1_diff		= open_buf[cursor-1*period] - open_buf[cursor-2*period];
		double t2_diff		= open_buf[cursor-2*period] - open_buf[cursor-3*period];
		t0 = t0_diff > 0 ? +1 : -1;
		t1 = t1_diff > 0 ? +1 : -1;
		t2 = t2_diff > 0 ? +1 : -1;
	}
	if (t0 * t1 == -1 && t1 * t2 == +1) {
		SetBitCurrent(bit_pos++, t0 == +1);
		SetBitCurrent(bit_pos++, t0 != +1);
	} else {
		SetBitCurrent(bit_pos++, false);
		SetBitCurrent(bit_pos++, false);
	}
	
	
	// ChannelOscillator
	int high_pos, low_pos;
	switch (period_id) {
		case 0: ec0.Add(open1, open1); high_pos = ec0.GetHighest(); low_pos = ec0.GetLowest(); break;
		case 1: ec1.Add(open1, open1); high_pos = ec1.GetHighest(); low_pos = ec1.GetLowest(); break;
		case 2: ec2.Add(open1, open1); high_pos = ec2.GetHighest(); low_pos = ec2.GetLowest(); break;
		case 3: ec3.Add(open1, open1); high_pos = ec3.GetHighest(); low_pos = ec3.GetLowest(); break;
		case 4: ec4.Add(open1, open1); high_pos = ec4.GetHighest(); low_pos = ec4.GetLowest(); break;
		case 5: ec5.Add(open1, open1); high_pos = ec5.GetHighest(); low_pos = ec5.GetLowest(); break;
		default: Panic("Invalid period id");
	}
	double ch_high = open_buf[high_pos];
	double ch_low = open_buf[low_pos];
	double ch_diff = ch_high - ch_low;
	value = (open1 - ch_low) / ch_diff * 2.0 - 1.0;
	SetBitCurrent(bit_pos++, value < 0.0);
	SetBitCurrent(bit_pos++, value < 0.0 ? (value < -0.5) : (value > +0.5));
	
	
	// BollingerBands
	double tmp = 0.0;
	for (int j = 0; j < period; j++) {
		double value = open_buf[Upp::max(0, cursor - j)];
		tmp += pow ( value - ma, 2 );
	}
	tmp = sqrt( tmp / period );
	double bb_top = ma + tmp;
	double bb_bot = ma - tmp;
	bool bb_a = open1 < ma;
	bool bb_b = open1 >= bb_top;
	bool bb_c = open1 <= bb_bot;
	SetBitCurrent(bit_pos++, open1 < ma);
	SetBitCurrent(bit_pos++, open1 < ma ? open1 <= bb_bot : open1 >= bb_top);
	
	
	// Descriptor bits
	period = 1 << period_id;
	for(int i = 0; i < processbits_period_count; i++) {
		int pos = Upp::max(0, cursor - period * (i + 1));
		double open2 = open_buf[pos];
		bool value = open1 < open2;
		SetBitCurrent(bit_pos++, value);
	}
	
	
	// Symbol change order
	int pos = Upp::max(0, cursor - period);
	open2 = open_buf[pos];
	double change_a = open1 / open2 - 1.0;
	for(int i = 0; i < sym_count; i++) {
		if (i == used_sym) continue;
		double change_b = data.data[i].open[cursor] / data.data[i].open[pos] - 1.0;
		bool value = change_a > change_b;
		SetBitCurrent(bit_pos++, value);
		SetBitCurrent(bit_pos++, change_b > 0);
	}
	
	
	// Correlation
	const int corr_count = 4;
	const int corr_step = 1 << period_id;
	
	for(int j = 0; j < sym_count; j++) {
		if (j == used_sym) continue;
		
		double x[corr_count], y[corr_count], xy[corr_count], xsquare[corr_count], ysquare[corr_count];
		double xsum, ysum, xysum, xsqr_sum, ysqr_sum;
		double coeff, num, deno;
		
		xsum = ysum = xysum = xsqr_sum = ysqr_sum = 0;
		
		// find the needed data to manipulate correlation coeff
		for (int i = 0; i < corr_count; i++) {
			int pos = Upp::max(0, cursor - i * corr_step);
			x[i] = open_buf[pos];
			y[i] = data.data[j].open[pos];
			
			xy[i] = x[i] * y[i];
			xsquare[i] = x[i] * x[i];
			ysquare[i] = y[i] * y[i];
			xsum = xsum + x[i];
			ysum = ysum + y[i];
			xysum = xysum + xy[i];
			xsqr_sum = xsqr_sum + xsquare[i];
			ysqr_sum = ysqr_sum + ysquare[i];
		}
		
		num = 1.0 * (((double)corr_count * xysum) - (xsum * ysum));
		deno = 1.0 * (((double)corr_count * xsqr_sum - xsum * xsum)* ((double)corr_count * ysqr_sum - ysum * ysum));
		
		// calculate correlation coefficient
		coeff = deno > 0.0 ? num / sqrt(deno) : 0.0;
		//LOG("xsum " << xsum << " ysum " << ysum << " xysum " << xysum << " xsqr_sum " << xsqr_sum << " ysqr_sum " << ysqr_sum);
		//LOG("num " << num << " deno " << deno << " coeff " << coeff);
		bool value = coeff < 0.0;
		SetBitCurrent(bit_pos++, value);
	}
	
}


BitProcessManager::BitProcessManager() {
	
}

BitProcess& BitProcessManager::Get(int used_sym, int tf) {
	int id = used_sym * 0x100 + tf;
	int i = data.Find(id);
	if (i != -1) return data[i];
	BitProcess& p = data.Add(id);
	p.used_sym = used_sym;
	p.tf = tf;
	p.LoadThis();
	return p;
}









ExchangeSlots::ExchangeSlots() {
	
	
	
	/*
	
	Approximation in UTC
	
	NZD 22:00	05:00
	AUD 00:00	06:00
	JPY 00:00	06:00		02:30–03:30
	EUR 08:00	16:30
	    07:00	19:00
	    07:00	21:00
	CHF 08:00	16:30
	USD 14:30	21:00
	CAD 14:30	21:00
	
	00:00 - 02:00	NZD,AUD,JPY
	02:00 - 04:00	NZD,AUD,JPY		Japan lunch
	04:00 - 05:00	NZD,AUD,JPY
	05:00 - 06:00	AUD,JPY
	06:00 - 07:00	
	07:00 - 08:00	EUR
	08:00 - 10:00	EUR,CHF
	10:00 - 12:00	EUR,CHF
	12:00 - 14:30	EUR,CHF
	14:30 - 16:30	EUR,CHF,USD,CAD
	16:30 - 19:00	EUR,CHF,USD,CAD
	19:00 - 21:00	EUR,CHF,USD,CAD
	21:00 - 22:00	
	22:00 - 20:00	NZD
	
	However, exchanges are open in local time, which fluctuate with daylight savings time.
	
	NZD	      10:00	16:45	NZST
	AUD	      10:00	16:00	AEST
	JPY	      09:00	15:00	JST
	EUR	Paris 09:00	17:30	CET
	EUR FSX   08:00	20:00	CET
	EUR EUREX 08:00	22:00	CET
	GBP       08:00	16:30	GMT
	USD NYSE  09:30	16:00	EST
	USD NASQ  09:30	16:00	EST
	CAD       09:30	16:00	EST
	*/
	
	currencies.Add().Set("NZD").AddOpenLocal(1000, 1645).SetTZ("NZ");
	currencies.Add().Set("AUD").AddOpenLocal(1000, 1600).SetTZ("Australia/Sydney");
	currencies.Add().Set("JPY").AddOpenLocal( 900, 1500).SetTZ("Japan");
	currencies.Add().Set("EUR").SetTZ("Europe/Amsterdam")
		.AddOpenLocal( 900, 1730)
		.AddOpenLocal( 800, 2000)
		.AddOpenLocal( 800, 2200);
	currencies.Add().Set("GBP").SetTZ("Europe/London").AddOpenLocal( 800, 1630);
	currencies.Add().Set("CHF").SetTZ("Europe/Zurich").AddOpenLocal( 930, 1600);
	currencies.Add().Set("USD").SetTZ("America/New_York").AddOpenLocal( 930, 1600);
	currencies.Add().Set("CAD").SetTZ("America/Toronto").AddOpenLocal( 930, 1600);
	
	
	
	
	
	cursor = GetSysTime();
	cursor.year -= 10;
	
	
}

ExchangeSlots::~ExchangeSlots() {
	
}

String ExchangeSlots::GetPath() {
	return ConfigFile("ExchangeSlots.bin");
}

void ExchangeSlots::GetDaySlots(Date date, DaySlots& slot) {
	
	slot.Clear();
	
	cctz::time_zone utc;
	ASSERT(cctz::load_time_zone("UTC", &utc));
	
	for(int i = 0; i < currencies.GetCount(); i++) {
		Currency& cur = currencies[i];
		String tzpath = cur.GetTZ();
		LOG(cur.GetName());
		LOG(tzpath);
		
		cctz::time_zone tz;
		if(!cctz::load_time_zone(tzpath.Begin(), &tz))
			Panic("Include timezone files with executable");
	
		for(int j = 0; j < cur.GetLocalCount(); j++) {
			Time t(date.year, date.month, date.day);
			
			{
				cur.GetLocalOpen(j, t);
				const auto utc_open = cctz::convert(cctz::civil_second(t.year, t.month, t.day, t.hour, t.minute, 0), tz);
				const cctz::time_zone::absolute_lookup al = utc.lookup(utc_open);
				Time open(al.cs.year(), al.cs.month(), al.cs.day(), al.cs.hour(), al.cs.minute());
				DUMP(open);
				slot.time_points.GetAdd(open).AddOpen(cur.GetName() + IntStr(j));
			}
			
			{
				cur.GetLocalClose(j, t);
				const auto utc_close = cctz::convert(cctz::civil_second(t.year, t.month, t.day, t.hour, t.minute, 0), tz);
				const cctz::time_zone::absolute_lookup al = utc.lookup(utc_close);
				Time close(al.cs.year(), al.cs.month(), al.cs.day(), al.cs.hour(), al.cs.minute());
				DUMP(close);
				slot.time_points.GetAdd(close).AddClose(cur.GetName() + IntStr(j));
			}
			
		}
	}
	
	
	SortByKey(slot.time_points, StdLess<Time>());
	DUMPM(slot.time_points);
	
	Index<String> is_open;
	for(int i = 0; i < slot.time_points.GetCount(); i++) {
		TimeSlot& ts = slot.time_points[i];
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.was_open.Add(is_open[j]);
		}
		
		for(int j = 0; j < ts.close.GetCount(); j++) {
			int k = is_open.Find(ts.close[j]);
			if (k == -1) Panic("ExchangeSlots: Expected open exchange");
			is_open.Remove(k);
		}
		
		
		for(int j = 0; j < ts.open.GetCount(); j++) {
			is_open.Add(ts.open[j]);
		}
		SortIndex(is_open, StdLess<String>());
		
		for(int j = 0; j < is_open.GetCount(); j++) {
			ts.is_open.Add(is_open[j]);
		}
		
		
	}
	
	
	DUMPM(slot.time_points);
}

void ExchangeSlots::Refresh() {
	SyncedPrice& sp = GetSyncedPriceManager().Get(2); // M15
	
	sp.Refresh();
	
	if (!is_loaded) {
		LoadThis();
		is_loaded = true;
	}
	
	Time end = GetUtcTime();
	end.hour = 0;
	end.minute = 0;
	end.second = 0;
	end += 24*60*60;
	DaySlots ds;
	Time initial_cursor = cursor;
	
	while (cursor < end) {
		
		Date d(cursor.year, cursor.month, cursor.day);
		GetDaySlots(d, ds);
		
		for(int i = 0; i < ds.time_points.GetCount(); i++) {
			Time open = ds.time_points.GetKey(i);
			unsigned hash = ds.time_points[i].GetHashValue();
			Slot& slot = slots.GetAdd(hash);
			if (slot.id.IsEmpty()) {
				slot.id = Join(ds.time_points[i].is_open, ", ");
			}
			slot.open_time.Add(open);
			
			Time close;
			if (i < ds.time_points.GetCount()-1)
				close = ds.time_points.GetKey(i+1);
			else {
				close = ds.time_points.GetKey(0);
				close += 24*60*60;
			}
			slot.close_time.Add(close);
		}
		cursor += 24*60*60;
	}
	
	int count = sp.loadsource_cursor;
	
	for (; findtime_cursor < count; findtime_cursor++) {
		Time t = sp.time[findtime_cursor];
		
		for(int i = 0; i < slots.GetCount(); i++) {
			Slot& slot = slots[i];
			
			for(int j = 0; j < slot.open_time.GetCount(); j++) {
				if (slot.open_time[j] == t) {
					slot.open_pos.Add(j);
				}
			}
			
			for(int j = 0; j < slot.close_time.GetCount(); j++) {
				if (slot.close_time[j] == t) {
					int count = slot.open_pos.GetCount();
					slot.close_pos.SetCount(count, -1);
					slot.close_pos[count-1] = j;
				}
			}
		}
		
	}
	for(int i = 0; i < slots.GetCount(); i++) {
		Slot& slot = slots[i];
		slot.close_pos.SetCount(slot.open_pos.GetCount(), -1);
	}
	
	if (cursor != initial_cursor)
		StoreThis();
}


void TestExchangeSlots() {
	ExchangeSlots& es = GetExchangeSlots();
	
	Date d(2018,3,29);
	DaySlots ds;
	es.GetDaySlots(d, ds);
	
	
}







SlotSignals::SlotSignals() {
	
}

SlotSignals::~SlotSignals() {
	
}

void SlotSignals::Refresh() {
	ExchangeSlots& es = GetExchangeSlots();
	
	es.Refresh();
	
	if (!is_loaded) {
		LoadThis();
		is_loaded = true;
	}
	
	bool train = slots.IsEmpty();
	bool changes = false;
	
	for(int i = 0; i < es.slots.GetCount(); i++) {
		unsigned key = es.slots.GetKey(i);
		Slot& src = es.slots[i];
		SlotSignal& dst = slots.GetAdd(key);
		dst.id = src.id;
		
		int begin = dst.data.GetCount();
		int end = src.open_pos.GetCount();
		changes |= begin != end;
		dst.data.SetCount(end);
		for(int j = begin; j < end; j++) {
			SlotSignal::Data& dstd = dst.data[j];
			dstd.open_time  = src.open_time[j];
			dstd.close_time = src.close_time[j];
			dstd.open_pos   = src.open_pos[j];
			dstd.close_pos  = src.close_pos[j];
		}
		
		/*if (train) {
			int count = BitProcess::processbits_inputrow_size;
			dst.correlation.SetCount(count);
			for(int j = 0; j < count; j++) {
				dst.correlation[j].SetCount(sym_count);
				for(int k = 0; k < sym_count; k++) {
					dst.correlation[j][k] = Correlation(slot, j, k);
				}
			}
		}
		
		
		for(int j = begin; j < end; j++) {
			for(int k = 0; k < sym_count; k++) {
				
			}
		}*/
		
	}
	
	
	if (changes)
		StoreThis();
}


}
