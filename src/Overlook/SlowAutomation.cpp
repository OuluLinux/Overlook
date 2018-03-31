#if 0
#include "Overlook.h"

namespace Overlook {


void SlowAutomation::ProcessBits() {
	bool initial = processbits_cursor == 0;
	
	for (; processbits_cursor < loadsource_cursor; processbits_cursor++) {
		#ifdef flagDEBUG
		if (processbits_cursor == 10000) {
			processbits_cursor = loadsource_cursor - 1;
			break;
		}
		#endif
		
		int bit_pos = 0;
		for(int j = 0; j < processbits_period_count; j++) {
			ProcessBitsSingle(j, bit_pos);
		}
		if (!(bit_pos == processbits_inputrow_size)) {
			DUMP(bit_pos);
			DUMP(processbits_inputrow_size);
		}
		ASSERT(bit_pos == processbits_inputrow_size);
		
		int pos = processbits_cursor;
		int next = pos + 1;
		if (next < loadsource_cursor) {
			double open = open_buf[pos];
			double close = open_buf[next];
			double change = fabs(close / open - 1.0);
			Time time = Time(1970,1,1) + time_buf[pos];
			int wday = DayOfWeek(time) - 1;
			int wdaymins = (wday * 24 + time.hour) * 60 + time.minute;
			int wdayslot = wdaymins / period;
			if (wdayslot >= 0 && wdayslot < wdayhours)
				slot_stats[wdayslot].Add(change);
		}
	}
	
	if (initial) {
		VectorMap<int, double> slots;
		for(int i = 0; i < wdayhours; i++) {
			slots.Add(i, slot_stats[i].mean);
		}
		SortByValue(slots, StdGreater<double>());
		DUMPM(slots);
		int count = wdayhours * 2 / 3;
		for(int i = 0; i < count; i++) {
			enabled_slot[slots.GetKey(i)] = true;
		}
	}
}

void SlowAutomation::SetBit(int pos, int bit, bool b) {
	ASSERT(bit >= 0 && bit < processbits_row_size);
	int64 i = pos* processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < processbits_reserved_bytes);
	uint64* it = bits_buf + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

bool SlowAutomation::GetBit(int pos, int bit) const {
	int64 i = pos * processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < processbits_reserved_bytes);
	ConstU64* it = bits_buf + j;
	return *it & (1ULL << k);
}

void SlowAutomation::ProcessBitsSingle(int period_id, int& bit_pos) {
	double open1 = open_buf[processbits_cursor];
	double point = this->point;
	ASSERT(point >= 0.000001 && point < 0.1);
	
	int cursor = processbits_cursor;
	double* open_buf = this->open_buf;
	
	
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
		if (i == sym) continue;
		double change_b = this->other_open_buf[i][cursor] / this->other_open_buf[i][pos] - 1.0;
		bool value = change_a > change_b;
		SetBitCurrent(bit_pos++, value);
		SetBitCurrent(bit_pos++, change_b > 0);
	}
	
	
	// Correlation
	const int corr_count = 4;
	const int corr_step = 1 << period_id;
	
	for(int j = 0; j < sym_count; j++) {
		if (j == sym) continue;
		
		double x[corr_count], y[corr_count], xy[corr_count], xsquare[corr_count], ysquare[corr_count];
		double xsum, ysum, xysum, xsqr_sum, ysqr_sum;
		double coeff, num, deno;
		
		xsum = ysum = xysum = xsqr_sum = ysqr_sum = 0;
		
		// find the needed data to manipulate correlation coeff
		for (int i = 0; i < corr_count; i++) {
			int pos = Upp::max(0, cursor - i * corr_step);
			x[i] = open_buf[pos];
			y[i] = this->other_open_buf[j][pos];
			
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

void SlowAutomation::LoadInput(Dqn::MatType& input, int pos) {
	for(int i = 0; i < processbits_inputrow_size; i++) {
		bool value = GetBit(pos, i);
		input.Set(i, value ? 0.0 : 1.0);
	}
}

void SlowAutomation::LoadOutput(double output[dqn_output_size], int pos) {
	
	double change = open_buf[pos + dqn_rightoffset - 1] / open_buf[pos] - 1.0;
	
	int level = fabs(change) / 0.001;
	if (level >= dqn_levels) level = dqn_levels-1;
	
	for(int i = 0; i < dqn_output_size; i++)
		output[i] = 1.0;
	
	if (change >= 0.0) {
		for(int i = 0; i <= level; i++)
			output[i] = 0.0;
	} else {
		for(int i = 0; i <= level; i++)
			output[dqn_levels + i] = 0.0;
	}
	
}

void SlowAutomation::Evolve() {
	int& iters = this->dqn_iters;
	
	const double max_alpha = 0.01;
	
	if (!iters) {
		dqn.Init();
	}
	
	dqn.SetEpsilon(0);
	dqn.SetGamma(0);
	
	while (*running && iters < max_iters) {
		int pos = dqn_leftoffset + Random(processbits_cursor - dqn_rightoffset - dqn_leftoffset);
		Dqn::MatType input;
		double output[dqn_output_size];
		
		double alpha = max_alpha * (double)(max_iters-1-iters) / (double)max_iters;
		dqn.SetAlpha(alpha);
		
		LoadInput(input, pos);
		LoadOutput(output, pos);
		
		dqn.Learn(input, output);
		
		iters++;
	}
	
	if (iters >= max_iters) {
		int& cursor = dqn_cursor;
		for(; cursor < processbits_cursor; cursor++) {
			#ifdef flagDEBUG
			if (cursor == 10000)
				cursor = processbits_cursor - 1;
			#endif
			
			Dqn::MatType input;
			double output[dqn_output_size];
			
			LoadInput(input, cursor);
			
			dqn.Evaluate(input, output, dqn_output_size);
			
			double pos_sensor = output[0];
			double neg_sensor = output[dqn_levels];
			
			bool signal = neg_sensor < pos_sensor;
			int level;
			if (!signal) {
				for(level = 0; level < dqn_levels-1; level++)
					if (output[level] > 0.5)
						break;
			} else {
				for(level = 0; level < dqn_levels-1; level++)
					if (output[dqn_levels + level] > 0.5)
						break;
			}
			
			bool enabled = true;
			
			SetBitOutput(cursor, OUT_EVOLVE_SIG, signal);
			SetBitOutput(cursor, OUT_EVOLVE_ENA, enabled);
			SetBitOutput(cursor, OUT_COUNT + level, true);
		}
	}
}

void SlowAutomation::Trim() {
	
	if (trim_cursor == 0) {
		
		// Find enabled, +signal, -signal bits, which allows trading
		VectorMap<int, double> enable_results, possig_results, negsig_results;
		for(int i = 0; i < processbits_inputrow_size; i++) {
			double enable_result = TestTrim(i, 0);
			double possig_result = TestTrim(i, 1);
			double negsig_result = TestTrim(i, 2);
			enable_results.Add(i, enable_result);
			possig_results.Add(i, possig_result);
			negsig_results.Add(i, negsig_result);
		}
		SortByValue(enable_results, StdGreater<double>());
		SortByValue(possig_results, StdGreater<double>());
		SortByValue(negsig_results, StdGreater<double>());
		DUMPM(enable_results);
		DUMPM(possig_results);
		DUMPM(negsig_results);
		
		// Gather the most positive bits
		for(int i = 0; i < trimbit_count; i++) {
			enable_bits[i] = enable_results.GetKey(i);
			possig_bits[i] = possig_results.GetKey(i);
			negsig_bits[i] = negsig_results.GetKey(i);
		}
	}
	
	
	// Write output
	int& cursor = trim_cursor;
	for(; cursor < dqn_cursor; cursor++) {
		
		bool signal = GetBitOutput(cursor, OUT_EVOLVE_SIG);
		bool enable = false; //GetBitOutput(cursor, job_id, OUT_EVOLVE_ENA);
		
		#ifdef flagDEBUG
		signal = (cursor / 10) % 2;
		enable = (cursor / 20) % 2;
		#endif
		
		
		for(int i = 0; i < trimbit_count && !enable; i++) {
			enable |= GetBit(cursor, enable_bits[i]);
			enable |= GetBit(cursor, possig_bits[i]) == signal;
			enable |= GetBit(cursor, negsig_bits[i]) != signal;
		}
		
		SetBitOutput(cursor, OUT_TRIM_SIG, signal);
		SetBitOutput(cursor, OUT_TRIM_ENA, enable);
	}
}

double SlowAutomation::TestTrim(int bit, int type) {
	
	// Loop range and sum pips
	double res = 0;
	int end = dqn_cursor - 1;
	for(int cursor = 100; cursor < end; cursor++) {
		#ifdef flagDEBUG
		if (cursor == 10000)
			break;
		#endif
		
		
		bool signal = GetBitOutput(cursor, OUT_EVOLVE_SIG);
		bool enable = false; //GetBitOutput(cursor, OUT_EVOLVE_ENA);
		
		if      (type == 0)
			enable |= GetBit(cursor, bit);
		else if (type == 1)
			enable |= GetBit(cursor, bit) == signal;
		else if (type == 2)
			enable |= GetBit(cursor, bit) != signal;
		
		if (enable) {
			bool signal = GetBitOutput(cursor, OUT_EVOLVE_SIG);
			double change = open_buf[cursor+1] - open_buf[cursor];
			if (signal) change *= -1.0;
			res += change;
		}
	}
	
	return res;
}

int SlowAutomation::GetSignal() {
	int last = trim_cursor - 1;
	if (last < 0) return 0;
	bool signal  = GetBitOutput(last, OUT_TRIM_SIG);
	bool enabled = GetBitOutput(last, OUT_TRIM_ENA);
	int sig = enabled ? (signal ? -1 : +1) : 0;
	
	System& sys = GetSystem();
	MetaTrader& mt = GetMetaTrader();
	int sys_sym = sys.used_symbols_id[sym];
	DataBridge& db = sys.GetSource(sys_sym, tf).db;
	if (sig > 0) {
		double change = +(mt.GetAskBid()[sys_sym].ask / (open_buf[last] + spread) - 1.0);
		ReleaseLog(sys.GetSymbol(sys_sym) + " sig " + IntStr(sig) + " change " + DblStr(change) + " ask " + DblStr(mt.GetAskBid()[sys_sym].ask)
			+ " open " + DblStr(open_buf[last]) + " spread " + DblStr(spread));
		if (change < 0) sig = 0;
	}
	else if (sig < 0) {
		double change = -(mt.GetAskBid()[sys_sym].ask / (open_buf[last] - spread) - 1.0);
		ReleaseLog(sys.GetSymbol(sys_sym) + " sig " + IntStr(sig) + " change " + DblStr(change) + " ask " + DblStr(mt.GetAskBid()[sys_sym].ask)
			+ " open " + DblStr(open_buf[last]) + " spread " + DblStr(spread));
		if (change < 0) sig = 0;
	}
	
	Time time = GetUtcTime();
	if (not_first && sig != prev_sig && time.Get() - prev_sig_time.Get() < 10*60) {
		sig = prev_sig;
	} else {
		prev_sig_time = time;
		prev_sig = sig;
		not_first = true;
	}
	
	int wday = DayOfWeek(time) - 1;
	int wdaymins = (wday * 24 + time.hour) * 60 + time.minute;
	int wdayslot = wdaymins / period;
	if (wdayslot >= 0 && wdayslot < wdayhours && !enabled_slot[wdayslot])
		sig = 0;
	
	sig *= GetLevel();
	
	return sig;
}

int SlowAutomation::GetLevel() {
	int last = trim_cursor - 1;
	if (last < 0) return 0;
	int mult = 0;
	for(int level = 0; level < dqn_levels; level++) {
		if (GetBitOutput(last, OUT_COUNT + level))
			mult = level + 1;
	}
	return mult;
}

}
#endif
