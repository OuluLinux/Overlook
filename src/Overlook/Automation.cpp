#include "Overlook.h"

namespace Overlook {

Automation::Automation() {
	memset(this, 0, sizeof(Automation));
	ASSERT(sym_count > 0);
	
	tf = 2; // M15
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
	else if (group_id == GROUP_CORRELATION) {
		ProcessCorrelation(job_id);
	}
	else {
		Evolve(group_id, job_id);
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
	}
}

void Automation::ProcessBits() {
	for (; processbits_cursor < loadsource_cursor; processbits_cursor++) {
		#ifdef flagDEBUG
		if (processbits_cursor == 10000) {
			processbits_cursor = loadsource_cursor;
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

void Automation::ProcessCorrelation(int job_id) {
	
	int& cursor = correlation_cursor[job_id];
	
	for (; cursor < weight_cursor[job_id]; cursor++) {
		
		bool signal  = GetBitOutput(cursor, job_id, 4);
		bool enabled = GetBitOutput(cursor, job_id, 5);
		
		if (enabled) {
			// Prefer coherence in symbol correlations
			
			// Calculate for and against from other symbols
			int same = 0, opposite = 0;
			
			int bit_pos = processbits_generic_row - processbits_correlation_count;
			
			for(int i = 0; i < sym_count; i++) {
				if (i == job_id) continue;
				
				bool correlation_bit = GetBit(cursor, job_id, bit_pos);
				bool other_signal  = GetBitOutput(cursor, i, 4);
				bool other_enabled = GetBitOutput(cursor, i, 5);
				
				if (other_enabled) {
					if (!correlation_bit) {
						if (other_signal == signal)		same++;
						else							opposite++;
					} else {
						if (other_signal != signal)		same++;
						else							opposite++;
					}
				}
				
				bit_pos++;
			}


			// Cancel signal
			if (same == opposite)
				enabled = false;
			else if (same < opposite)
				signal = !signal;
		}
		
		
		// Write output
		SetBitOutput(cursor, job_id, 6, signal);
		SetBitOutput(cursor, job_id, 7, enabled);
		
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

void Automation::Evolve(int group_id, int job_id) {
	StrandList& meta_added = this->tmp_meta_added[job_id];
	StrandList& single_added = this->tmp_single_added[job_id];
	StrandList& strands = this->strands[job_id];
	
	if (strands.IsEmpty())
		strands.Add();
	
	int iter_count = MAX_ITERS;
	#ifdef flagDEBUG
	iter_count = 2;
	#endif
	
	int res_id = group_id - GROUP_ENABLE;
	strands.Sort(res_id);
	
	Index<unsigned> visited_hashes;
	
	for (; strands.cursor[group_id] < iter_count && running; strands.cursor[group_id]++) {
		bool total_added = false;
		
		meta_added.Clear();
		
		int evolve_count = strands.GetCount();
		for(int i = 0; i < evolve_count && running; i++) {
			Strand& st = strands[i];
			
			single_added.Clear();
			
			for(int j = 0; j < processbits_inputrow_size && running; j++) {
				
				for(int k = 0; k < 10; k++) {
					switch (group_id) {
						case GROUP_ENABLE:	if (k >= 4) continue; break;
						case GROUP_TRIGGER:	if (k < 4 || k >= 6) continue; break;
						case GROUP_WEIGHT:	if (k < 6) continue; break;
						default: Panic("Invalid group");
					}
					
					Strand test = st;
					
					bool fail = false;
					if      (k == 0)	fail = test.enabled				.Evolve(j);
					else if (k == 1)	fail = test.signal_true			.Evolve(j);
					else if (k == 2)	fail = test.signal_false		.Evolve(j);
					else if (k == 3)	test.sig_bit = j;
					else if (k == 4)	fail = test.trigger_true		.Evolve(j);
					else if (k == 5)	fail = test.trigger_false		.Evolve(j);
					else if (k == 6)	fail = test.weight_inc_true		.Evolve(j);
					else if (k == 7)	fail = test.weight_inc_false	.Evolve(j);
					else if (k == 8)	fail = test.weight_dec_true		.Evolve(j);
					else if (k == 9)	fail = test.weight_dec_false	.Evolve(j);
					if (fail) continue;
					
					unsigned hash = test.GetHashValue();
					if (visited_hashes.Find(hash) != -1)
						continue;
					visited_hashes.FindAdd(hash);
					
					TestStrand(group_id, job_id, test);
					
					if (test.result[res_id] == 1.0)
						continue;
					
					if (test.result[res_id] < strands.Top().result[res_id] && strands.GetCount() >= MAX_STRANDS)
						continue;
					
					single_added.Add(test);
					
				}
				
			}
			
			if (!single_added.IsEmpty()) {
				total_added = true;
				
				single_added.Sort(res_id);
				if (single_added.GetCount() > MAX_STRANDS)
					single_added.SetCount(MAX_STRANDS);
				
				for(int i = 0, added_count = 0; i < single_added.GetCount() && added_count < 2; i++) {
					auto& s = single_added.strands[i];
					if (!meta_added.Has(s, res_id)) {
						meta_added.Add(s);
						added_count++;
					}
				}
				
				
				meta_added.Sort(res_id);
				if (meta_added.GetCount() > MAX_STRANDS)
					meta_added.SetCount(MAX_STRANDS);
			}
		}
		
		if (!running) break;
		
		if (total_added == false)
			break;
		
		
		int count = min(MAX_STRANDS*2, meta_added.GetCount());
		for(int i = 0; i < count; i++) {
			auto& s = meta_added.strands[i];
			if (!strands.Has(s, res_id))
				strands.Add(s);
		}
		
		// Sort
		strands.Sort(res_id);
		
		
		//DUMP(strands.cursor);
		//strands.Dump(res_id);
		
		
		// Trim
		if (strands.GetCount() > MAX_STRANDS)
			strands.SetCount(MAX_STRANDS);
	}
	
	
	for(int i = strands.GetCount() -1; i >= 0; i--) {
		auto& st = strands[i];
		//if (st.result[res_id] < 1.0) continue;
		TestStrand(group_id, job_id, st, true);
	}
}

void Automation::TestStrand(int group_id, int job_id, Strand& st, bool write) {
	int begin = 0;
	
	const int sym = job_id;
	const int res_id = group_id - GROUP_ENABLE;
	
	long double result = 1.0;
	double spread = this->spread[sym];
	double* open_buf = this->open_buf[sym];
	
	bool prev_enabled = false, prev_signal;
	double prev_open;
	
	int weight = 1;
	int prev_weight = 1;
	
	
	for(int i = begin; i < processbits_cursor; i++) {
		
		#ifdef flagDEBUG
		if (i == 100000) break;
		#endif
		
		bool signal = false, enabled = false, triggered = false, check_enabled = false, check_triggered = false, write_result = false, check_resultorder = false, check_weight = false;
		switch (group_id) {
			case GROUP_ENABLE:
				signal    = GetBit(i, sym, st.sig_bit);
				enabled   = true;
				triggered = true;
				check_enabled = true;
				break;
			
			case GROUP_TRIGGER:
				signal    = GetBitOutput(i, sym, 0);
				enabled   = GetBitOutput(i, sym, 1);
				check_triggered = true;
				write_result = true;
				break;
				
			case GROUP_WEIGHT:
				signal    = GetBitOutput(i, sym, 2);
				enabled   = GetBitOutput(i, sym, 3);
				triggered = true;
				check_weight = true;
				break;
			
			default: Panic("Invalid group id");
		}
		
		if (check_enabled) {
			for(int j = 0; j < st.enabled.count && enabled; j++) {
				int bit = st.enabled.bits[j];
				bool is_bit_enabled = GetBit(i, sym, bit);
				enabled &= is_bit_enabled;
			}
			
			if (enabled) {
				for(int j = 0; j < st.signal_true.count && enabled; j++)
					enabled &= GetBit(i, sym, st.signal_true.bits[j]) == signal;
				for(int j = 0; j < st.signal_false.count && enabled; j++)
					enabled &= GetBit(i, sym, st.signal_false.bits[j]) != signal;
			}
			
		}
		
		if (check_triggered && enabled) {
			bool signal_triggered = true;
			for(int j = 0; j < st.trigger_true.count && signal_triggered; j++)
				signal_triggered &= GetBit(i, sym, st.trigger_true.bits[j]) == signal;
			for(int j = 0; j < st.trigger_false.count && signal_triggered; j++)
				signal_triggered &= GetBit(i, sym, st.trigger_false.bits[j]) != signal;
			triggered |= signal_triggered;
		}
		
		
		if (check_weight && enabled) {
			
			bool increase = true;
			for(int j = 0; j < st.weight_inc_true.count && increase; j++)
				increase &= GetBit(i, sym, st.weight_inc_true.bits[j]) == signal;
			for(int j = 0; j < st.weight_inc_false.count && increase; j++)
				increase &= GetBit(i, sym, st.weight_inc_false.bits[j]) != signal;
			
			
			bool decrease = true;
			for(int j = 0; j < st.weight_dec_true.count && decrease; j++)
				decrease &= GetBit(i, sym, st.weight_dec_true.bits[j]) == signal;
			for(int j = 0; j < st.weight_dec_false.count && decrease; j++)
				decrease &= GetBit(i, sym, st.weight_dec_false.bits[j]) != signal;
			
			if (increase && decrease)
				;
			else if (increase) weight = min(max_weight, weight + 1);
			else if (decrease) weight = max(min_weight, weight - 1);
		}
		
		bool do_open = false, do_close = false;
		
		if (prev_enabled) {
			if (enabled) {
				if (signal != prev_signal) {
					if (triggered)
						do_open = true;
					do_close = true;
				}
			} else {
				do_close = true;
			}
		}
		else if (enabled) {
			if (triggered)
				do_open = true;
		}
		
		
		if (do_close) {
			double current = open_buf[i];
			long double change;
			if (!prev_signal)	change = +(current / (prev_open + spread) - 1.0);
			else				change = -(current / (prev_open - spread) - 1.0);
			change *= weight * 0.001;
			if (change > +0.5 || change < -0.5) change = 0.0;
			result *= 1.0 + change;
		}
		if (do_open) {
			double current = open_buf[i];
			prev_open = current;
		}
		
		prev_signal = signal;
		prev_enabled = enabled;
		prev_weight = weight;
		
		if (write) {
			if (enabled) {
				SetBitOutput(i, sym, res_id * 2 + 0, signal);
				SetBitOutput(i, sym, res_id * 2 + 1, enabled);
				if (check_weight) {
					SetBitOutput(i, sym, GROUP_RESULTS * 2 + weight - min_weight, true);
					if (i + 1 > weight_cursor[sym]) weight_cursor[sym] = i + 1;
				}
			}
		}
	}
	
	st.result[res_id] = result;
}

int Automation::GetSignal(int sym) {
	int last = correlation_cursor[sym] - 1;
	if (last < 0) return 0;
	bool signal  = GetBitOutput(last, sym, 6);
	bool enabled = GetBitOutput(last, sym, 7);
	int sig = enabled ? (signal ? -1 : +1) : 0;
	int mult = 0;
	for(int i = min_weight; i <= max_weight; i++) {
		int bit = i - min_weight;
		if (GetBitOutput(last, sym, 8 + bit))
			mult = i;
	}
	sig *= mult;
	return sig;
}






bool StrandList::Has(Strand& s, int res_id) {
	for(int i = 0; i < strand_count; i++) {
		if (s.result[res_id] == strands[i].result[res_id])
			return true;
	}
	return false;
}

void StrandList::Sort(int res_id) {
	ASSERT(res_id >= 0 && res_id < GROUP_RESULTS);
	for(int i = 0; i < strand_count; i++) {
		int pos = i;
		double max = strands[i].result[res_id];
		for(int j = i+1; j < strand_count; j++) {
			Strand& s = strands[j];
			if (s.result[res_id] > max) {
				max = s.result[res_id];
				pos = j;
			}
		}
		Swap(strands[i], strands[pos]);
	}
}

void StrandList::Dump(int res_id) {
	DUMP(strand_count);
	DUMP(cursor);
	for(int i = 0; i < strand_count; i++) {
		LOG("   " << i << ": " << strands[i].ToString(res_id));
	}
}

bool StrandItem::Evolve(int bit) {
	bool fail = false;
	for(int i = 0; i < count; i++) {
		if (bits[i] == bit) {
			fail = true;
			break;
		}
	}
	Add(bit);
	return fail;
}

String Strand::ToString(int res_id) const {
	String out;
	out << "result=" << (double)result[res_id] << ", " << BitString();
	return out;
}

String Strand::BitString() const {
	String out;
	for(int i = 0; i < enabled.count; i++)
		out << "e" << i << "=" << enabled.bits[i] << ", ";
	for(int i = 0; i < signal_true.count; i++)
		out << "s+" << i << "=" << signal_true.bits[i] << ", ";
	for(int i = 0; i < signal_false.count; i++)
		out << "s-" << i << "=" << signal_false.bits[i] << ", ";
	for(int i = 0; i < trigger_true.count; i++)
		out << "t+" << i << "=" << trigger_true.bits[i] << ", ";
	for(int i = 0; i < trigger_false.count; i++)
		out << "t-" << i << "=" << trigger_false.bits[i] << ", ";
	for(int i = 0; i < weight_inc_true.count; i++)
		out << "w++" << i << "=" << weight_inc_true.bits[i] << ", ";
	for(int i = 0; i < weight_inc_false.count; i++)
		out << "w+-" << i << "=" << weight_inc_false.bits[i] << ", ";
	for(int i = 0; i < weight_dec_true.count; i++)
		out << "w-+" << i << "=" << weight_dec_true.bits[i] << ", ";
	for(int i = 0; i < weight_dec_false.count; i++)
		out << "w--" << i << "=" << weight_dec_false.bits[i];
	return out;
}

}
