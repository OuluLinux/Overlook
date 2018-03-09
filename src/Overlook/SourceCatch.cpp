#include "Overlook.h"

namespace Overlook {

void Job::LoadCatchStrands() {
	System& sys = GetSystem();
	auto& strands = this->catch_strands;
	
	if (strands.IsEmpty())
		strands.Add();
	
	int iter_count = MAX_ITERS;
	#ifdef flagDEBUG
	iter_count = 2;
	#endif
	
	for (; strands.cursor < iter_count && sys.running; strands.cursor++) {
		bool total_added = false;
		
		StrandList meta_added;
		int evolve_count = strands.GetCount();
		for(int i = 0; i < evolve_count && sys.running; i++) {
			Strand& st = strands[i];
			
			StrandList single_added;
			
			for(int j = 0; j < Job::row_size && sys.running; j++) {
				
				for(int k = 0; k < 6; k++) {
					Strand test;
					test.Clear();
					
					bool fail = false;
					if      (k == 0)	fail = st.enabled			.Evolve(j, test.enabled);
					else if (k == 1)	fail = st.signal_true		.Evolve(j, test.signal_true);
					else if (k == 2)	fail = st.signal_false		.Evolve(j, test.signal_false);
					else if (k == 3)	fail = st.trigger_true		.Evolve(j, test.trigger_true);
					else if (k == 4)	fail = st.trigger_false		.Evolve(j, test.trigger_false);
					else if (k == 5)	{test = st; test.sig_bit = j;}
					if (fail) continue;
					
					TestCatchStrand(test);
					
					if (test.result == 1.0)
						continue;
					
					if (test.result < strands.Top().result && strands.GetCount() >= MAX_STRANDS)
						continue;
					
					single_added.Add(test);
				}
				
			}
			
			if (!single_added.IsEmpty()) {
				total_added = true;
				
				single_added.Sort();
				if (single_added.GetCount() > MAX_STRANDS)
					single_added.SetCount(MAX_STRANDS);
				
				for(int i = 0, added_count = 0; i < single_added.GetCount() && added_count < 2; i++) {
					auto& s = single_added.strands[i];
					if (!meta_added.Has(s)) {
						meta_added.Add(s);
						added_count++;
					}
				}
				
				
				meta_added.Sort();
				if (meta_added.GetCount() > MAX_STRANDS)
					meta_added.SetCount(MAX_STRANDS);
			}
		}
		
		if (!sys.running) break;
		
		if (total_added == false)
			break;
		
		
		int count = min(MAX_STRANDS*2, meta_added.GetCount());
		for(int i = 0; i < count; i++) {
			auto& s = meta_added.strands[i];
			if (!strands.Has(s))
				strands.Add(s);
		}
		
		// Sort
		strands.Sort();
		
		
		//DUMP(strands.cursor);
		//strands.Dump();
		
		
		// Trim
		if (strands.GetCount() > MAX_STRANDS)
			strands.SetCount(MAX_STRANDS);
	}
	
	
	strand_lock.Enter();
	strand_data.SetCount(end);
	strand_lock.Leave();
	
	for(int i = strands.GetCount() -1; i >= 0; i--) {
		auto& st = strands[i];
		if (st.result < 1.1) continue;
		TestCatchStrand(st, true);
	}
}

void Job::TestCatchStrand(Strand& st, bool write) {
	int begin = 200;
	#ifdef flagDEBUG
	end = min(100000, end);
	#endif
	
	long double result = 1.0;
	double spread = GetSpread();
	const Vector<double>& open_buf = GetOpen();
	int tf = GetTf();
	
	bool prev_enabled = false, prev_signal;
	double prev_open;
	for(int i = begin; i < end; i++) {
		Snap& snap = main_booleans[i];
		
		//bool signal = strand_data.Get(i, 0);
		bool signal = snap.Get(st.sig_bit);
		bool enabled = strand_data.Get(i, 1);
		bool triggered = false;
		
		for(int j = 0; j < st.enabled.count && enabled; j++) {
			int bit = st.enabled.bits[j];
			bool is_bit_enabled = snap.Get(bit);
			enabled &= is_bit_enabled;
		}
		
		if (enabled) {
			for(int j = 0; j < st.signal_true.count && enabled; j++)
				enabled &= snap.Get(st.signal_true.bits[j]) == signal;
			for(int j = 0; j < st.signal_false.count && enabled; j++)
				enabled &= snap.Get(st.signal_false.bits[j]) != signal;
		}
		
		if (enabled) {
			bool signal_triggered = true;
			for(int j = 0; j < st.trigger_true.count && signal_triggered; j++)
				signal_triggered &= snap.Get(st.trigger_true.bits[j]) == signal;
			for(int j = 0; j < st.trigger_false.count && signal_triggered; j++)
				signal_triggered &= snap.Get(st.trigger_false.bits[j]) != signal;
			triggered |= signal_triggered;
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
			if (!prev_signal)	change = current / (prev_open + spread);
			else				change = 1.0 - (current / (prev_open - spread) - 1.0);
			if (fabs(change - 1.0) > 0.5) change = 1.0;
			result *= change;
		}
		if (do_open) {
			double current = open_buf[i];
			prev_open = current;
		}
		
		prev_signal = signal;
		prev_enabled = enabled;
		
		if (write && enabled) {
			strand_data.Set(i, 2, signal);
			strand_data.Set(i, 3, enabled);
		}
	}
	
	st.result = result;
}

int Job::GetCatchSignal(int pos) {
	if (pos < 0)
		pos = strand_data.GetCount() - 1;
	if (pos < 0) return 0;
	
	strand_lock.Enter();
	bool signal = strand_data.Get(pos, 2);
	bool enabled = strand_data.Get(pos, 3);
	strand_lock.Leave();
	
	return enabled ? (signal ? -1 : +1) : 0;
}

}
