#if 0
#include "Overlook.h"

namespace Overlook {

bool MarginImage::LoadSources() {
	System& sys = GetSystem();
	int sym_count = sys.GetSymbolCount();
	int tf_count = MAX_REAL_TFID - MIN_REAL_TFID + 1;
	
	
	// Gather Job pointers
	if (jobs.IsEmpty()) {
		sym_jobs.SetCount(sym_count, NULL);
		int j = 0;
		for(int i = 0; i < sys.jobs.GetCount(); i++) {
			Job& job = *sys.jobs[i];
			if (dynamic_cast<SourceImage*>(&job)) {
				jobs.Add(&job);
				src_jobs.Add(j++, &job);
				if (job.GetTf() == MAX_REAL_TFID)
					sym_jobs[job.GetSymbol()] = &job;
			}
			if (dynamic_cast<AccountImage*>(&job)) {
				jobs.Add(&job);
				acc_jobs.Add(j++, &job);
			}
		}
	}
	ASSERT(jobs.GetCount());
	
	
	// Sort jobs by value to prefer the first signal correctly
	struct JobResultSorter {
		bool operator() (const Job* a, const Job* b) const {
			return a->GetBestResult() > b->GetBestResult();
		}
	};
	SortByValue(src_jobs, JobResultSorter());
	SortByValue(acc_jobs, JobResultSorter());
	
	
	// Check that jobs are finished
	for(int i = 0; i < jobs.GetCount(); i++) if (!jobs[i]->IsFinished()) return false;
	
	
	// States for tfs
	ASSERT(jobs.GetCount() == tf_count * (1 + sym_count));
	if (current_state.IsEmpty()) {
		current_state.SetCount(jobs.GetCount());
		
		for(int i = 0; i < jobs.GetCount(); i++) {
			Job& job = *jobs[i];
			current_state[i] = State(0, job.GetTime()[0], job.GetOpen()[0], 0);
		}
	}
	
	
	// Gather symbol signals again, but with joined tfs
	signals.row_size = sym_count * 2;
	tmp_sym_change.SetCount(sym_count, 0);
	
	
	// Reserve memory for current symbol signals and account signals
	src_signal.SetCount(sym_count, 0);
	acc_signal.SetCount(tf_count, 0);
	
	
	while (sys.running) {
		int pos = this->gain.GetCount();
		int reserve_step = 100000;
		int reserve = pos - (pos % reserve_step) + reserve_step;
		signals.Reserve(reserve);
		gain.Reserve(reserve);
		signals.SetCount(pos+1);
		
		
		// Find next smallest time
		int smallest_time = INT_MAX;
		for(int i = 0; i < jobs.GetCount(); i++) {
			int count = jobs[i]->GetOpen().GetCount();
			int next = current_state[i].a + 1;
			if (next >= count) continue;
			int time = jobs[i]->GetTime()[next];
			if (time < smallest_time)
				smallest_time = time;
		}
		if (smallest_time == INT_MAX)
			break;
		
		
		// Increase cursor with all which has the same time next
		for(int i = 0; i < jobs.GetCount(); i++) {
			int count = jobs[i]->GetOpen().GetCount();
			int next = current_state[i].a + 1;
			if (next >= count) continue;
			int time = jobs[i]->GetTime()[next];
			if (time == smallest_time)
				current_state[i].a++;
		}
		
		double gain = balance;
		
		
		// Get AccountImage signals
		for(int i = 0; i < acc_jobs.GetCount(); i++) {
			Job& job = *acc_jobs[i];
			State& state = current_state[acc_jobs.GetKey(i)];
			
			int src_pos = min(job.strand_data.GetCount() - 1, state.a);
			int sig = 0;
			if (src_pos != -1) {
				bool signal = job.strand_data.Get(src_pos, 2);
				bool enabled = job.strand_data.Get(src_pos, 3);
				sig = enabled ? (signal ? -1 : +1) : 0;
			}
			acc_signal[job.GetTf()] = sig;
		}
		
		
		// Reset previous SourceImage signals
		for(int i = 0; i < sym_count; i++) src_signal[i] = 0;
		
		// Get SourceImage signals (src_jobs is sorted, so first signal can be used)
		for(int i = 0; i < src_jobs.GetCount(); i++) {
			Job& job = *src_jobs[i];
			State& state = current_state[src_jobs.GetKey(i)];
			
			int src_pos = min(job.strand_data.GetCount() - 1, state.a);
			int acc_sig = acc_signal[job.GetTf()];
			int& sig = src_signal[job.GetSymbol()];
			if (src_pos != -1 && sig == 0 && acc_sig != 0) {
				bool signal = job.strand_data.Get(src_pos, 2);
				bool enabled = job.strand_data.Get(src_pos, 3);
				sig = enabled ? (signal ? -1 : +1) : 0;
				sig *= acc_sig;
			}
		}
		
		
		// Use signals
		for(int i = 0; i < sym_count; i++) {
			int sig = src_signal[i];
			State& state = current_state[src_jobs.GetKey(i)];
			double open = sym_jobs[i]->GetOpen()[state.a];
			int& prev_sig = state.d;
			
			
			// Write signals to memory for further use
			signals.Set(pos, i*2 + 0, sig == -1);
			signals.Set(pos, i*2 + 1, sig !=  0);
			
			
			// Write also the open value to avoid time syncing in fast optimization loop
			sym_opens[pos * sym_count + i] = open;
			
			
			// Add gain, when signal is continuing
			if (prev_sig == sig) {
				if (sig) {
					double change;
					if (sig > 0)	change = +(open / state.c - 1.0);
					else			change = -(open / state.c - 1.0);
					gain += change;
				}
			}
			// Close and open. Add to balance
			else {
				if (prev_sig) {
					double change;
					if (sig > 0)	change = +(open / state.c - 1.0);
					else			change = -(open / state.c - 1.0);
					balance += change;
					gain += change;
				}
				if (sig) {
					state.c = open;
				}
			}
			prev_sig = sig;
			ASSERT(gain > 0);
			
			
			tmp_sym_change[i] = gain;
		}
		
		
		// Gather overwriting input values
		int row = 0;
		for(int i = 0; i < tmp_sym_change.GetCount(); i++) {
			double a = tmp_sym_change[i];
			for(int j = i+1; j < tmp_sym_change.GetCount(); j++) {
				double b = tmp_sym_change[j];
				bool value1 = a >= b;
				overwrite_bits.Set(pos, row++, value1);
			}
			bool value2 = a >= 0;
			overwrite_bits.Set(pos, row++, value2);
		}
		ASSERT(row == overwrite_row_count);
		
		
		this->gain.Add(gain);
	}
	
	return true;
}

void MarginImage::LoadBooleans() {
	
	// Take cursor
	int begin = main_booleans.GetCount();
	
	
	Job::LoadBooleans();
	
	
	// Overwrite symbol order and +/- side
	int overwrite_begin = row_size - extra_row - overwrite_row_count;
	
	for(int cursor = begin; cursor < end; cursor++) {
		Snap& snap = main_booleans[cursor];
		int row = overwrite_begin;
		for(int i = 0; i < overwrite_row_count; i++)
			snap.Set(row++, overwrite_bits.Get(cursor, i));
	}
}

void MarginImage::TestTryStrand(Strand& st, bool write) {
	// Tune margin level
	
	int begin = 200;
	
	if (!write) {
		#ifdef flagDEBUG
		end = min(100000, end);
		#else
		end = min(500000, end);
		#endif
	}
	
	long double result = 1.0;
	double spread = GetSpread();
	const Vector<double>& open_buf = GetOpen();
	const Vector<int>& time_buf = GetTime();
	int tf = GetTf();
	
	const int sym_count = USEDSYMBOL_COUNT;
	const int min_symopen = 1;
	const int max_symopen = sym_count;
	int symopen = max_symopen;
	
	
	// Reset states
	tmp_current_state.SetCount(sym_count);
	for(int i = 0; i < tmp_current_state.GetCount(); i++)
		tmp_current_state[i] = State(0, sym_times[i], sym_opens[i], 0);
	
	
	for(int i = begin; i < end; i++) {
		Snap& snap = main_booleans[i];
		
		bool signal = snap.Get(st.sig_bit);
		
		
		// increase fm level
		bool increase = true;
		for(int j = 0; j < st.signal_true.count && increase; j++)
			increase &= snap.Get(st.signal_true.bits[j]) == signal;
		for(int j = 0; j < st.signal_false.count && increase; j++)
			increase &= snap.Get(st.signal_false.bits[j]) != signal;
		
		
		// decrease fm level
		bool decrease = true;
		for(int j = 0; j < st.trigger_true.count && decrease; j++)
			decrease &= snap.Get(st.trigger_true.bits[j]) == signal;
		for(int j = 0; j < st.trigger_false.count && decrease; j++)
			decrease &= snap.Get(st.trigger_false.bits[j]) != signal;
		
		// Change fmlevel
		if (increase && decrease)
			;
		else if (increase)
			symopen = min(max_symopen, symopen + 1);
		else if (decrease)
			symopen = max(min_symopen, symopen - 1);
		
		
		// loop all symbols, get gain
		double gain = balance;
		
		int open_cursor = i * sym_count;
		for(int j = 0; j < sym_count; j++) {
			bool force_close = j >= symopen;
			double open = sym_opens[open_cursor];
			byte sym_id = sym_orders[open_cursor];
			open_cursor++;
			State& state = tmp_current_state[sym_id];
			
			bool signal  = signals.Get(i, sym_id*2 + 0);
			bool enabled = signals.Get(i, sym_id*2 + 1);
			int sig = enabled ? (signal ? -1 : +1) : 0;
			
			if (force_close)
				sig = 0;
			
			int& prev_sig = state.d;
			
			if (prev_sig == sig) {
				if (sig) {
					double change;
					if (sig > 0)	change = +(open / (state.c + spread) - 1.0);
					else			change = -(open / (state.c - spread) - 1.0);
					gain += change;
				}
			}
			else {
				if (prev_sig) {
					double change;
					if (sig > 0)	change = +(open / (state.c + spread) - 1.0);
					else			change = -(open / (state.c - spread) - 1.0);
					balance += change;
					gain += change;
				}
				if (sig) {
					state.c = open;
				}
			}
			prev_sig = sig;
			ASSERT(gain > 0);
		}
	}
	
	st.result = result;
}

void MarginImage::TestCatchStrand(Strand& st, bool write) {
	// Tune used symbols
	
	int begin = 200;
	
	if (!write) {
		#ifdef flagDEBUG
		end = min(100000, end);
		#else
		end = min(500000, end);
		#endif
	}
	
	const int sym_count = USEDSYMBOL_COUNT;
	
	long double result = 1.0;
	double spread = GetSpread();
	const Vector<double>& open_buf = GetOpen();
	const Vector<int>& time_buf = GetTime();
	int tf = GetTf();
	
	int sigmul = 1;
	const int min_sigmul = 1;
	const int max_sigmul = 4;
	
	
	// Reset states
	tmp_current_state.SetCount(sym_count);
	for(int i = 0; i < tmp_current_state.GetCount(); i++)
		tmp_current_state[i] = State(0, sym_times[i], sym_opens[i], 0);
	
	
	for(int i = begin; i < end; i++) {
		Snap& snap = main_booleans[i];
		
		bool signal = snap.Get(st.sig_bit);
		
		// increase fm level
		bool increase = true;
		for(int j = 0; j < st.signal_true.count && increase; j++)
			increase &= snap.Get(st.signal_true.bits[j]) == signal;
		for(int j = 0; j < st.signal_false.count && increase; j++)
			increase &= snap.Get(st.signal_false.bits[j]) != signal;
		
		
		// decrease fm level
		bool decrease = true;
		for(int j = 0; j < st.trigger_true.count && decrease; j++)
			decrease &= snap.Get(st.trigger_true.bits[j]) == signal;
		for(int j = 0; j < st.trigger_false.count && decrease; j++)
			decrease &= snap.Get(st.trigger_false.bits[j]) != signal;
		
		// Change fmlevel
		if (increase && decrease)
			;
		else if (increase)
			sigmul = min(max_sigmul, sigmul + 1);
		else if (decrease)
			sigmul = max(min_sigmul, sigmul - 1);
		
		
		// loop all symbols, get gain
		double gain = balance;
		
		int open_cursor = i * sym_count;
		for(int j = 0; j < sym_count; j++) {
			double open = sym_opens[open_cursor];
			byte sym_id = sym_orders[open_cursor];
			open_cursor++;
			State& state = tmp_current_state[sym_id];
			
			bool signal  = signals.Get(i, sym_id*2 + 0);
			bool enabled = signals.Get(i, sym_id*2 + 1);
			int sig = enabled ? (signal ? -1 : +1) : 0;
			
			int& prev_sig = state.d;
			
			if (prev_sig == sig) {
				if (sig) {
					double change;
					if (sig > 0)	change = +(open / (state.c + spread) - 1.0);
					else			change = -(open / (state.c - spread) - 1.0);
					change *= sigmul;
					gain += change;
				}
			}
			else {
				if (prev_sig) {
					double change;
					if (sig > 0)	change = +(open / (state.c + spread) - 1.0);
					else			change = -(open / (state.c - spread) - 1.0);
					change *= sigmul;
					balance += change;
					gain += change;
				}
				if (sig) {
					state.c = open;
				}
			}
			prev_sig = sig;
			ASSERT(gain > 0);
		}
		
		/*if (write) {
			strand_data.Set(i, 0, signal);
			strand_data.Set(i, 1, enabled);
		}*/
	}
	
	st.result = result;
}

}
#endif
