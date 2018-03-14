#include "Overlook.h"

namespace Overlook {

Automation::Automation() {
	if (FileExists(ConfigFile("Automation.bin")))
		LoadThis();
	
	not_stopped = 0;
}

Automation::~Automation() {
	
}

void Automation::StartJobs() {
	if (running || not_stopped > 0) return;
	
	AddJournal("Starting system jobs");
	
	running = true;
	int thrd_count = GetUsedCpuCores();
	not_stopped = 0;
	for(int i = 0; i < thrd_count; i++) {
		not_stopped++;
		Thread::Start(THISBACK1(JobWorker, i));
	}
}

void Automation::StopJobs() {
	AddJournal("Stopping system jobs");
	
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
		if (group_id >= jobgroup_count)
			break;
		JobGroup& jobgroup = jobgroups[group_id];
		
		workitem_lock.Enter();
		bool found = false;
		int job_id;
		for(int i = 0; i < sym_count; i++) {
			job_id = worker_cursor++;
			if (worker_cursor >= sym_count)
				worker_cursor = 0;
			Job& job = jobgroup.jobs[job_id];
			if (job.is_finished)
				continue;
			found = true;
			break;
		}
		workitem_lock.Leave();
		if (!found) continue;
		
		Process(group_id, job_id);
	}
	
	
	
	not_stopped--;
}

void Automation::Process(int group_id, int job_id) {
	JobGroup& jobgroup	= jobgroups[group_id];
	Job& job			= jobgroup.jobs[job_id];
	
	if (group_id == GROUP_SOURCE) {
		if (job_id == 0) {
			LoadSource();
		}
		else Sleep(1000);
		job.is_finished = true;
	}
	else if (group_id == GROUP_BITS) {
		ProcessBits();
	}
	else {
		Evolve(group_id, job_id);
	}
}

void Automation::LoadSource() {
	System& sys = GetSystem();
	Vector<int>& used_symbols_id = sys.used_symbols_id;
	
	while (running) {
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		int smallest_ids[sym_count];
		int smallest_id_count = 0;
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			int count = db.open.GetCount();
			int next = loadsource_state[i].a + 1;
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
			loadsource_state[smallest_ids[i]].a++;
		}
		
		for(int i = 0; i < sym_count; i++) {
			int sym = used_symbols_id[i];
			DataBridge& db = sys.data[sym][tf].db;
			State& state = loadsource_state[sym];
			double open = db.open[state.a];
			open_buf[i][loadsource_cursor] = open;
		}
		
		
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
		int bit_pos = 0;
		for(int i = 0; i < processbits_period_count; i++) {
			for(int j = 0; j < sym_count; j++) {
				ProcessBitsSingle(i, j, bit_pos);
			}
		}
		ASSERT(bit_pos == processbits_inputrow_size);
	}
}

void Automation::SetBit(int pos, int sym, int bit, bool b) {
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

void Automation::ProcessBitsSingle(int period_id, int sym, int& bit_pos) {
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
	for(int i = 0; i < processbits_descriptor_count; i++) {
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
	
}

void Automation::Evolve(int group_id, int job_id) {
	
	if (strands.IsEmpty())
		strands.Add();
	
	int iter_count = MAX_ITERS;
	#ifdef flagDEBUG
	iter_count = 2;
	#endif
	
	int res_id = group_id - GROUP_ENABLE;
	
	for (; strands.cursor < iter_count && running; strands.cursor++) {
		bool total_added = false;
		
		StrandList meta_added;
		int evolve_count = strands.GetCount();
		for(int i = 0; i < evolve_count && running; i++) {
			Strand& st = strands[i];
			
			StrandList single_added;
			
			for(int j = 0; j < processbits_inputrow_size && running; j++) {
				
				for(int k = 0; k < 14; k++) {
					switch (group_id) {
						case GROUP_ENABLE:	if (k >= 4) continue;
						case GROUP_TRIGGER:	if (k < 4 || k >= 6) continue;
						case GROUP_LIMIT:	if (k < 6 || k >= 10) continue;
						case GROUP_WEIGHT:	if (k < 10) continue;
						default: Panic("Invalid group");
					}
					
					Strand test;
					test.Clear();
					
					bool fail = false;
					if      (k == 0)	fail = st.enabled			.Evolve(j, test.enabled);
					else if (k == 1)	fail = st.signal_true		.Evolve(j, test.signal_true);
					else if (k == 2)	fail = st.signal_false		.Evolve(j, test.signal_false);
					else if (k == 3)	{test = st; test.sig_bit = j;}
					else if (k == 4)	fail = st.trigger_true		.Evolve(j, test.trigger_true);
					else if (k == 5)	fail = st.trigger_false		.Evolve(j, test.trigger_false);
					else if (k == 6)	fail = st.limit_inc_true	.Evolve(j, test.limit_inc_true);
					else if (k == 7)	fail = st.limit_inc_false	.Evolve(j, test.limit_inc_false);
					else if (k == 8)	fail = st.limit_dec_true	.Evolve(j, test.limit_dec_true);
					else if (k == 9)	fail = st.limit_dec_false	.Evolve(j, test.limit_dec_false);
					else if (k == 10)	fail = st.weight_inc_true	.Evolve(j, test.weight_inc_true);
					else if (k == 11)	fail = st.weight_inc_false	.Evolve(j, test.weight_inc_false);
					else if (k == 12)	fail = st.weight_dec_true	.Evolve(j, test.weight_dec_true);
					else if (k == 13)	fail = st.weight_dec_false	.Evolve(j, test.weight_dec_false);
					if (fail) continue;
					
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
					if (!meta_added.Has(s)) {
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
			if (!strands.Has(s))
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
		if (st.result[res_id] < 1.0) continue;
		TestStrand(group_id, job_id, st, true);
	}
}

void Automation::TestStrand(int group_id, int job_id, Strand& st, bool write) {
	int begin = 200;
	
	const int sym = job_id;
	
	long double result = 1.0;
	double spread = this->spread[sym];
	double* open_buf = this->open_buf[sym];
	
	bool prev_enabled = false, prev_signal;
	double prev_open;
	
	int order_limit = sym_count / 2;
	int weight = 1;
	const int max_order_limit = sym_count;
	const int min_order_limit = 1;
	const int max_weight = 4;
	const int min_weight = 1;
	
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
				
			case GROUP_LIMIT:
				signal    = GetBitOutput(i, sym, 2);
				enabled   = GetBitOutput(i, sym, 3);
				triggered = true;
				check_resultorder = true;
				break;
				
			case GROUP_WEIGHT:
				signal    = GetBitOutput(i, sym, 4);
				enabled   = GetBitOutput(i, sym, 5);
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
		
		if (check_resultorder && enabled) {
			
			bool increase = true;
			for(int j = 0; j < st.limit_inc_true.count && increase; j++)
				increase &= GetBit(i, sym, st.limit_inc_true.bits[j]) == signal;
			for(int j = 0; j < st.limit_inc_false.count && increase; j++)
				increase &= GetBit(i, sym, st.limit_inc_false.bits[j]) != signal;
			
			
			bool decrease = true;
			for(int j = 0; j < st.limit_dec_true.count && decrease; j++)
				decrease &= GetBit(i, sym, st.limit_dec_true.bits[j]) == signal;
			for(int j = 0; j < st.limit_dec_false.count && decrease; j++)
				decrease &= GetBit(i, sym, st.limit_dec_false.bits[j]) != signal;
			
			if (increase && decrease)
				;
			else if (increase) order_limit = min(max_order_limit, order_limit + 1);
			else if (decrease) order_limit = max(min_order_limit, order_limit - 1);
			
			int prev_pos = max(0, i - 12);
			double diff_a = trigger_result[sym][i] - trigger_result[sym][prev_pos];
			int better_count = 0;
			for(int j = 0; j < sym_count; j++) {
				if (j == sym) continue;
				double diff_b = trigger_result[j][i] - trigger_result[j][prev_pos];
				if (diff_b > diff_a)
					better_count++;
			}
			if (better_count >= order_limit)
				enabled = false;
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
			double change;
			if (!prev_signal)	change = +(current / (prev_open + spread) - 1.0);
			else				change = -(current / (prev_open - spread) - 1.0);
			change *= weight;
			if (fabs(change - 1.0) > 0.5) change = 1.0;
			result *= 1.0 + change;
		}
		if (do_open) {
			double current = open_buf[i];
			prev_open = current;
		}
		
		prev_signal = signal;
		prev_enabled = enabled;
		
		if (write && enabled) {
			if (write_result) {
				trigger_result[sym][i] = result;
			}
			
			SetBitOutput(i, sym, group_id * 2 + 0, signal);
			SetBitOutput(i, sym, group_id * 2 + 1, enabled);
		}
	}
	
	st.result[group_id - GROUP_ENABLE] = result;
}








bool StrandList::Has(Strand& s) {
	for(int i = 0; i < strand_count; i++) {
		if (s.result == strands[i].result)
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

bool StrandItem::Evolve(int bit, StrandItem& dst) {
	dst = *this;
	bool fail = false;
	for(int i = 0; i < dst.count; i++) {
		if (dst.bits[i] == bit) {
			fail = true;
			break;
		}
	}
	dst.Add(bit);
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
	return out;
}

}
