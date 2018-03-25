#include "Overlook.h"

namespace Overlook {

Automation::Automation() {
	memset(this, 0, sizeof(Automation));
	ASSERT(sym_count > 0);
	
	
	tf = 4; // H1
	output_fmlevel = 0.8;
	
	if (FileExists(ConfigFile("Automation.bin")))
		LoadThis();
	
	not_stopped = 0;
}

Automation::~Automation() {
	
}

int Automation::GetSymGroupJobId(int symbol) const {
	for(int i = 0; i < GROUP_COUNT; i++) {
		const JobGroup& jobgroup = jobgroups[i];
		const Job& job = jobgroup.jobs[symbol];
		if (!job.is_finished)
			return i;
	}
	return GROUP_COUNT;
}

void Automation::StartJobs() {
	if (running || not_stopped > 0) return;
	
	running = true;
	int thrd_count = GetUsedCpuCores();
	not_stopped = 0;
	for(int i = 0; i < thrd_count; i++) {
		not_stopped++;
		Thread::Start(THISBACK1(JobWorker, i));
	}
}

void Automation::StopJobs() {
	
	running = false;
	while (not_stopped > 0) Sleep(100);
	
	StoreThis();
}

void Automation::Serialize(Stream& s) {
	if (s.IsLoading()) {
		s.Get(this, sizeof(Automation));
	} else {
		s.Put(this, sizeof(Automation));
	}
}

void Automation::JobWorker(int i) {
	
	while (running) {
		
		int group_id = 0;
		for (;group_id < jobgroup_count; group_id++)
			if (!jobgroups[group_id].is_finished)
				break;
		if (group_id >= jobgroup_count) {
			for(int i = 0; i < GROUP_COUNT; i++) {
				jobgroups[i].is_finished = false;
				for(int j = 0; j < sym_count; j++)
					jobgroups[i].jobs[j].is_finished = false;
			}
			continue;
		}
		
		JobGroup& jobgroup = jobgroups[group_id];
		
		workitem_lock.Enter();
		bool found = false;
		int job_id;
		for(int i = 0; i < sym_count; i++) {
			job_id = worker_cursor++;
			if (worker_cursor >= sym_count)
				worker_cursor = 0;
			Job& job = jobgroup.jobs[job_id];
			if (job.is_finished || job.is_processing)
				continue;
			job.is_processing = true;
			found = true;
			break;
		}
		
		if (!found) {
			bool all_finished = true;
			for(int i = 0; i < sym_count; i++)
				all_finished &= jobgroup.jobs[i].is_finished;
			if (all_finished) {
				jobgroup.is_finished = true;
			}
		}
		
		workitem_lock.Leave();
		
		if (!found) {
			Sleep(100);
			continue;
		}
		
		Process(group_id, job_id);
		jobgroups[group_id].jobs[job_id].is_processing = false;
		
	}
	
	
	
	not_stopped--;
}

void Automation::Process(int group_id, int job_id) {
	JobGroup& jobgroup	= jobgroups[group_id];
	Job& job			= jobgroup.jobs[job_id];
	
	if (group_id == GROUP_SOURCE) {
		if (job_id == 0)
			LoadSource();
		else
			Sleep(1000);
	}
	else if (group_id == GROUP_BITS) {
		if (job_id == 0)
			ProcessBits();
		else
			Sleep(1000);
	}
	else if (group_id == GROUP_EVOLVE) {
		Evolve(job_id);
	}
	else if (group_id == GROUP_TRIM) {
		Trim(job_id);
	}
	
	if (running) job.is_finished = true;
}

void Automation::LoadSource() {
	System& sys = GetSystem();
	Vector<int>& used_symbols_id = sys.used_symbols_id;
	
	for(int i = 0; i < used_symbols_id.GetCount(); i++) {
		int sym = used_symbols_id[i];
		sys.data[sym][0].db.Start();
		sys.data[sym][tf].db.Start();
	}
	
	while (running) {
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		int smallest_ids[sym_count];
		int smallest_id_count = 0;
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int count = db.open.GetCount();
			int next = loadsource_pos[i] + 1;
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
			loadsource_pos[smallest_ids[i]]++;
		}
		
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int pos = loadsource_pos[i];
			//LOG(loadsource_cursor << "\t" << i << "\t" << pos << "/" << db.open.GetCount());
			double open = db.open[pos];
			open_buf[i][loadsource_cursor] = open;
		}
		
		time_buf[loadsource_cursor] = smallest_time;
		
		loadsource_cursor++;
		if (loadsource_cursor >= maxcount)
			Panic("Reserved memory exceeded");
	}
	
	for(int i = 0; i < sym_count; i++) {
		int sym = used_symbols_id[i];
		DataBridge& db = sys.data[sym][tf].db;
		point[i] = db.GetPoint();
		spread[i] = db.GetSpread();
		if (point[i] <= 0) Panic("Invalid point");
		if (spread[i] < point[i] * 2)
			spread[i] = point[i] * 4;
		ReleaseLog("Automation::LoadSource sym " + IntStr(sym) + " point " + DblStr(point[i]) + " spread " + DblStr(spread[i]));
	}
}

void Automation::ProcessBits() {
	for (; processbits_cursor < loadsource_cursor; processbits_cursor++) {
		#ifdef flagDEBUG
		if (processbits_cursor == 10000) {
			processbits_cursor = loadsource_cursor - 1;
			break;
		}
		#endif
		for(int i = 0; i < sym_count; i++) {
			int bit_pos = 0;
			for(int j = 0; j < processbits_period_count; j++) {
				ProcessBitsSingle(i, j, bit_pos);
			}
			if (!(bit_pos == processbits_inputrow_size)) {
				DUMP(bit_pos);
				DUMP(processbits_inputrow_size);
			}
			ASSERT(bit_pos == processbits_inputrow_size);
		}
	}
}

void Automation::SetBit(int pos, int sym, int bit, bool b) {
	ASSERT(bit >= 0 && bit < processbits_row_size);
	int64 i = (pos * sym_count + sym) * processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < processbits_reserved_bytes);
	uint64* it = bits_buf + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

bool Automation::GetBit(int pos, int sym, int bit) const {
	int64 i = (pos * sym_count + sym) * processbits_row_size + bit;
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < processbits_reserved_bytes);
	ConstU64* it = bits_buf + j;
	return *it & (1ULL << k);
}

void Automation::ProcessBitsSingle(int sym, int period_id, int& bit_pos) {
	double open1 = open_buf[sym][processbits_cursor];
	double point = this->point[sym];
	ASSERT(point >= 0.000001 && point < 0.1);
	
	int cursor = processbits_cursor;
	double* open_buf = this->open_buf[sym];
	
	
	// MovingAverage
	double prev, ma;
	switch (period_id) {
		case 0: prev = av_wins0[sym].GetMean(); av_wins0[sym].Add(open1); ma = av_wins0[sym].GetMean(); break;
		case 1: prev = av_wins1[sym].GetMean(); av_wins1[sym].Add(open1); ma = av_wins1[sym].GetMean(); break;
		case 2: prev = av_wins2[sym].GetMean(); av_wins2[sym].Add(open1); ma = av_wins2[sym].GetMean(); break;
		case 3: prev = av_wins3[sym].GetMean(); av_wins3[sym].Add(open1); ma = av_wins3[sym].GetMean(); break;
		case 4: prev = av_wins4[sym].GetMean(); av_wins4[sym].Add(open1); ma = av_wins4[sym].GetMean(); break;
		case 5: prev = av_wins5[sym].GetMean(); av_wins5[sym].Add(open1); ma = av_wins5[sym].GetMean(); break;
		default: Panic("Invalid period id");
	}
	SetBitCurrent(sym, bit_pos++, ma < prev);
	SetBitCurrent(sym, bit_pos++, open1 < prev);
	
	
	// OnlineMinimalLabel
	double cost	 = point * (1 + period_id);
	const int count = 1;
	bool sigbuf[count];
	int begin = Upp::max(0, cursor - 200);
	int end = cursor + 1;
	OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
	bool label = sigbuf[count - 1];
	SetBitCurrent(sym, bit_pos++, label);

	
	// TrendIndex
	bool bit_value;
	int period = 1 << (1 + period_id);
	double err, av_change, buf_value;
	TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
	SetBitCurrent(sym, bit_pos++, buf_value > 0.0);
	
	
	// Momentum
	begin = Upp::max(0, cursor - period);
	double open2 = open_buf[begin];
	double value = open1 / open2 - 1.0;
	label = value < 0.0;
	SetBitCurrent(sym, bit_pos++, label);
	
	
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
	SetBitCurrent(sym, bit_pos++, len > 2);


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
	SetBitCurrent(sym, bit_pos++, len > 2);
	
	
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
	SetBitCurrent(sym, bit_pos++, len > 2);
	
	
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
		SetBitCurrent(sym, bit_pos++, t0 == +1);
		SetBitCurrent(sym, bit_pos++, t0 != +1);
	} else {
		SetBitCurrent(sym, bit_pos++, false);
		SetBitCurrent(sym, bit_pos++, false);
	}
	
	
	// ChannelOscillator
	int high_pos, low_pos;
	switch (period_id) {
		case 0: ec0[sym].Add(open1, open1); high_pos = ec0[sym].GetHighest(); low_pos = ec0[sym].GetLowest(); break;
		case 1: ec1[sym].Add(open1, open1); high_pos = ec1[sym].GetHighest(); low_pos = ec1[sym].GetLowest(); break;
		case 2: ec2[sym].Add(open1, open1); high_pos = ec2[sym].GetHighest(); low_pos = ec2[sym].GetLowest(); break;
		case 3: ec3[sym].Add(open1, open1); high_pos = ec3[sym].GetHighest(); low_pos = ec3[sym].GetLowest(); break;
		case 4: ec4[sym].Add(open1, open1); high_pos = ec4[sym].GetHighest(); low_pos = ec4[sym].GetLowest(); break;
		case 5: ec5[sym].Add(open1, open1); high_pos = ec5[sym].GetHighest(); low_pos = ec5[sym].GetLowest(); break;
		default: Panic("Invalid period id");
	}
	double ch_high = open_buf[high_pos];
	double ch_low = open_buf[low_pos];
	double ch_diff = ch_high - ch_low;
	value = (open1 - ch_low) / ch_diff * 2.0 - 1.0;
	SetBitCurrent(sym, bit_pos++, value < 0.0);
	SetBitCurrent(sym, bit_pos++, value < 0.0 ? (value < -0.5) : (value > +0.5));
	
	
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
	SetBitCurrent(sym, bit_pos++, open1 < ma);
	SetBitCurrent(sym, bit_pos++, open1 < ma ? open1 <= bb_bot : open1 >= bb_top);
	
	
	// Descriptor bits
	period = 1 << period_id;
	for(int i = 0; i < processbits_period_count; i++) {
		int pos = Upp::max(0, cursor - period * (i + 1));
		double open2 = open_buf[pos];
		bool value = open1 < open2;
		SetBitCurrent(sym, bit_pos++, value);
	}
	
	
	// Symbol change order
	int pos = Upp::max(0, cursor - period);
	open2 = open_buf[pos];
	double change_a = open1 / open2 - 1.0;
	for(int i = 0; i < sym_count; i++) {
		if (i == sym) continue;
		double change_b = this->open_buf[i][cursor] / this->open_buf[i][pos] - 1.0;
		bool value = change_a > change_b;
		SetBitCurrent(sym, bit_pos++, value);
		SetBitCurrent(sym, bit_pos++, change_b > 0);
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
			y[i] = this->open_buf[j][pos];
			
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
		SetBitCurrent(sym, bit_pos++, value);
	}
	
}

void Automation::LoadInput(Dqn::MatType& input, int sym, int pos) {
	for(int i = 0; i < processbits_inputrow_size; i++) {
		bool value = GetBit(pos, sym, i);
		input.Set(i, value ? 0.0 : 1.0);
	}
}

void Automation::LoadOutput(double output[dqn_output_size], int sym, int pos) {
	
	double change = open_buf[sym][pos + dqn_rightoffset - 1] / open_buf[sym][pos] - 1.0;
	
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

void Automation::Evolve(int job_id) {
	int& iters = this->dqn_iters[job_id];
	
	const double max_alpha = 0.01;
	
	if (!iters) {
		dqn.Init();
	}
	
	dqn.SetEpsilon(0);
	dqn.SetGamma(0);
	
	while (running && iters < max_iters) {
		int pos = dqn_leftoffset + Random(processbits_cursor - dqn_rightoffset - dqn_leftoffset);
		Dqn::MatType input;
		double output[dqn_output_size];
		
		double alpha = max_alpha * (double)(max_iters-1-iters) / (double)max_iters;
		dqn.SetAlpha(alpha);
		
		LoadInput(input, job_id, pos);
		LoadOutput(output, job_id, pos);
		
		dqn.Learn(input, output);
		
		iters++;
	}
	
	if (iters >= max_iters) {
		int& cursor = dqn_cursor[job_id];
		for(; cursor < processbits_cursor; cursor++) {
			#ifdef flagDEBUG
			if (cursor == 10000)
				cursor = processbits_cursor - 1;
			#endif
			
			Dqn::MatType input;
			double output[dqn_output_size];
			
			LoadInput(input, job_id, cursor);
			
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
			
			SetBitOutput(cursor, job_id, OUT_EVOLVE_SIG, signal);
			SetBitOutput(cursor, job_id, OUT_EVOLVE_ENA, enabled);
			SetBitOutput(cursor, job_id, OUT_COUNT + level, true);
		}
	}
}

void Automation::Trim(int job_id) {
	
	if (trim_cursor[job_id] == 0) {
		
		// Find enabled, +signal, -signal bits, which allows trading
		VectorMap<int, double> enable_results, possig_results, negsig_results;
		for(int i = 0; i < processbits_inputrow_size; i++) {
			double enable_result = TestTrim(job_id, i, 0);
			double possig_result = TestTrim(job_id, i, 1);
			double negsig_result = TestTrim(job_id, i, 2);
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
	int& cursor = trim_cursor[job_id];
	for(; cursor < dqn_cursor[job_id]; cursor++) {
		
		bool signal = GetBitOutput(cursor, job_id, OUT_EVOLVE_SIG);
		bool enable = false; //GetBitOutput(cursor, job_id, OUT_EVOLVE_ENA);
		
		#ifdef flagDEBUG
		signal = (cursor / 10) % 2;
		enable = (cursor / 20) % 2;
		#endif
		
		
		for(int i = 0; i < trimbit_count && !enable; i++) {
			enable |= GetBit(cursor, job_id, enable_bits[i]);
			enable |= GetBit(cursor, job_id, possig_bits[i]) == signal;
			enable |= GetBit(cursor, job_id, negsig_bits[i]) != signal;
		}
		
		SetBitOutput(cursor, job_id, OUT_TRIM_SIG, signal);
		SetBitOutput(cursor, job_id, OUT_TRIM_ENA, enable);
	}
}

double Automation::TestTrim(int job_id, int bit, int type) {
	
	// Loop range and sum pips
	double res = 0;
	int end = dqn_cursor[job_id] - 1;
	for(int cursor = 100; cursor < end; cursor++) {
		#ifdef flagDEBUG
		if (cursor == 10000)
			break;
		#endif
		
		
		bool signal = GetBitOutput(cursor, job_id, OUT_EVOLVE_SIG);
		bool enable = false; //GetBitOutput(cursor, job_id, OUT_EVOLVE_ENA);
		
		if      (type == 0)
			enable |= GetBit(cursor, job_id, bit);
		else if (type == 1)
			enable |= GetBit(cursor, job_id, bit) == signal;
		else if (type == 2)
			enable |= GetBit(cursor, job_id, bit) != signal;
		
		if (enable) {
			bool signal = GetBitOutput(cursor, job_id, OUT_EVOLVE_SIG);
			double change = open_buf[job_id][cursor+1] - open_buf[job_id][cursor];
			if (signal) change *= -1.0;
			res += change;
		}
	}
	
	return res;
}

int Automation::GetSignal(int sym) {
	int last = trim_cursor[sym] - 1;
	if (last < 0) return 0;
	bool signal  = GetBitOutput(last, sym, OUT_TRIM_SIG);
	bool enabled = GetBitOutput(last, sym, OUT_TRIM_ENA);
	int sig = enabled ? (signal ? -1 : +1) : 0;
	int mult = 1;
	for(int level = 0; level < dqn_levels; level++) {
		if (GetBitOutput(last, sym, OUT_COUNT + level))
			mult = level + 1;
	}
	sig *= mult;
	
	System& sys = GetSystem();
	int sys_sym = sys.used_symbols_id[sym];
	DataBridge& db = sys.GetSource(sys_sym, tf).db;
	if (sig > 0) {
		double change = +(db.GetLatestAsk() / (db.open.Top() + spread[sym]) - 1.0);
		if (change < 0) sig = 0;
	}
	else if (sig < 0) {
		double change = -(db.GetLatestAsk() / (db.open.Top() - spread[sym]) - 1.0);
		if (change < 0) sig = 0;
	}
	
	Time now = GetUtcTime();
	if (!enabled && prev_sig[sym] != 0 && now.Get() - prev_sig_time[sym].Get() < 10*60) {
		sig = prev_sig[sym];
	} else {
		prev_sig_time[sym] = now;
		prev_sig[sym] = sig;
	}
	
	return sig;
}


}

