#include "Overlook.h"

namespace Overlook {

void SourceImage::LoadTryStrands() {
	System& sys = GetSystem();
	auto& strands = this->try_strands;
	
	if (strands.IsEmpty())
		strands.Add();
	
	int iter_count = 20;
	#ifdef flagDEBUG
	iter_count = 2;
	#endif
	
	for (; strands.cursor < iter_count; strands.cursor++) {
		bool total_added = false;
		
		StrandList meta_added;
		int evolve_count = strands.GetCount();
		for(int i = 0; i < evolve_count; i++) {
			Strand& st = strands[i];
			
			StrandList single_added;
			
			for(int j = 0; j < SourceImage::row_size; j++) {
				
				for(int k = 0; k < 5; k++) {
					Strand test;
					test.Clear();
					
					bool fail = false;
					if      (k == 0)	fail = st.enabled			.Evolve(j, test.enabled);
					else if (k == 1)	fail = st.signal_true		.Evolve(j, test.signal_true);
					else if (k == 2)	fail = st.signal_false		.Evolve(j, test.signal_false);
					else if (k == 3)	fail = st.trigger_true		.Evolve(j, test.trigger_true);
					else if (k == 4)	fail = st.trigger_false		.Evolve(j, test.trigger_false);
					if (fail) continue;
					
					TestTryStrand(test);
					
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
	
	
	strand_data.SetCount(end);
	
	for(int i = strands.GetCount() -1; i >= 0; i--) {
		auto& st = strands[i];
		if (st.result < 1.0) continue;
		TestTryStrand(st, true);
	}
}

void SourceImage::TestTryStrand(Strand& st, bool write) {
	int begin = 200;
	#ifdef flagDEBUG
	end = min(100000, end);
	#endif
	
	double result = 1.0;
	double point = db.GetPoint();
	
	bool prev_enabled = false, prev_signal;
	double prev_open;
	for(int i = begin; i < end; i++) {
		Snap& snap = main_booleans[i];
		
		bool signal = snap.Get((SourceImage::period_count-1) * SourceImage::generic_row);
		bool enabled = true;
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
			double current = db.open[i];
			double change;
			if (!prev_signal)	change = current / (prev_open + STRAND_COSTMULT * point);
			else				change = 1.0 - (current / (prev_open - STRAND_COSTMULT * point) - 1.0);
			result *= change;
		}
		if (do_open) {
			double current = db.open[i];
			prev_open = current;
		}
		
		prev_signal = signal;
		prev_enabled = enabled;
		
		if (write && enabled) {
			strand_data.Set(i, 0, signal);
			strand_data.Set(i, 1, enabled);
		}
	}
	
	st.result = result;
}

}
