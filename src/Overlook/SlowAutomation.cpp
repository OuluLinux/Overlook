#include "Overlook.h"

namespace Overlook {


void SlowAutomation::ProcessBits() {
	ASSERT(processbits_row_size < processbits_row_size_aliased);
	ASSERT(processbits_row_size_aliased % 64 == 0);
	
	bool initial = processbits_cursor == 0;
	bool new_data = processbits_cursor < loadsource_cursor;
	
	if (!initial) processbits_cursor--;
	
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
	
	if (new_data) {
		int min_diff = INT_MAX;
		int min_pos = 0;
		int last = processbits_cursor - 120;
		
		for(int i = 1; i < last; i++) {
			#ifdef flagDEBUG
			if (i == 10000)
				break;
			#endif
			int diff = GetBitDiff(last, i);
			if (diff < min_diff) {
				min_diff = diff;
				min_pos = i;
			}
			//LOG(i << "\t" << diff);
		}
		most_matching_pos = min_pos;
	}
}

int SlowAutomation::GetBitDiff(int a, int b) {
	static const int row_bytes = processbits_row_size_aliased / 64;
	int hamming_distance = 0;
	int a_pos = a * row_bytes;
	int b_pos = b * row_bytes;
	uint64* a_ptr = bits_buf + a_pos;
	uint64* b_ptr = bits_buf + b_pos;
	for(int i = 0; i < row_bytes; i++) {
		hamming_distance += PopCount64(a_ptr[i] ^ b_ptr[i]);
	}
	return hamming_distance;
}

void SlowAutomation::SetBit(int pos, int bit, bool b) {
	ASSERT(bit >= 0 && bit < processbits_row_size);
	int64 i = pos* processbits_row_size_aliased + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < processbits_reserved_bytes);
	uint64* it = bits_buf + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

bool SlowAutomation::GetBit(int pos, int bit) const {
	int64 i = pos * processbits_row_size_aliased + bit;
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
	double diff = open_buf[pos + 1] - open_buf[pos];
	if (diff >= 0) {
		output[0] = 0.0;
		output[1] = 1.0;
	} else {
		output[0] = 1.0;
		output[1] = 0.0;
	}
	
	diff = open_buf[pos + dqn_rightoffset - 1] - open_buf[pos];
	if (diff >= 0) {
		output[2] = 0.0;
		output[3] = 1.0;
	} else {
		output[2] = 1.0;
		output[3] = 0.0;
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
		if (cursor > 0)
			cursor -= 3;
		for(; cursor < processbits_cursor; cursor++) {
			#ifdef flagDEBUG
			if (cursor == 10000)
				cursor = processbits_cursor - 1;
			#endif
			
			Dqn::MatType input;
			double output[dqn_output_size];
			
			LoadInput(input, cursor);
			
			dqn.Evaluate(input, output, dqn_output_size);
			
			bool signal = output[1] < output[0];
			bool enabled = output[0] < 0.5 || output[1] < 0.5;
			
			if (!enabled && cursor > 0) {
				bool prev_signal  = GetBitOutput(cursor-1, OUT_EVOLVE_SIG);
				bool prev_enabled = GetBitOutput(cursor-1, OUT_EVOLVE_ENA);
				if (prev_enabled && prev_signal == signal)
					enabled = true;
			}
			
			SetBitOutput(cursor, OUT_EVOLVE_SIG, signal);
			SetBitOutput(cursor, OUT_EVOLVE_ENA, enabled);
		}
	}
}

bool SlowAutomation::GetSignal() {
	int cursor = dqn_cursor - 1;
	if (cursor < 0) return 0;
	
	Dqn::MatType input;
	double output[dqn_output_size];
	
	LoadInput(input, cursor);
	
	dqn.Evaluate(input, output, dqn_output_size);
	
	bool signal = output[1] < output[0];
	return signal;
}

int SlowAutomation::GetLevel() {
	int cursor = dqn_cursor - 1;
	if (cursor < 0) return 0;
	
	Dqn::MatType input;
	double output[dqn_output_size];
	
	LoadInput(input, cursor);
	
	dqn.Evaluate(input, output, dqn_output_size);
	
	bool signal = output[1] < output[0];
	if (!signal)
		return (output[0] - 0.5) * -20;
	else
		return (output[1] - 0.5) * -20;
}

void SlowAutomation::GetOutputValues(bool& signal, int& level, bool& slow_signal) {
	int cursor = dqn_cursor - 1;
	if (cursor < 0) return;
	
	Dqn::MatType input;
	double output[dqn_output_size];
	
	LoadInput(input, cursor);
	
	dqn.Evaluate(input, output, dqn_output_size);
	
	signal = output[1] < output[0];
	if (!signal)
		level = (output[0] - 0.5) * -20;
	else
		level = (output[1] - 0.5) * -20;
	
	slow_signal = output[3] < output[2];
}

}
