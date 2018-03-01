#include "Overlook.h"

namespace Overlook {

void SourceImage::LoadTryStrands() {
	System& sys = GetSystem();
	auto& strands = this->try_strands;
	
	if (strands.IsEmpty())
		strands.Add();
	
	int iter_count = 6;
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
				
				for(int k = 0; k < 3; k++) {
					Strand test;
					test.Clear();
					
					bool fail = false;
					if (k == 0)			fail = st.EvolveSignal(j, test);
					else if (k == 1)	fail = st.EvolveEnabled(j, test);
					else				fail = st.EvolveOppositeSignal(j, test);
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
	
	for(; strand_data.try_cursor < end; strand_data.try_cursor++) {
		int sig = GetTrySignal(strand_data.try_cursor);
		bool enabled = sig != 0;
		bool signal = sig == -1;
		
		strand_data.Set(strand_data.try_cursor, 0, signal);
		strand_data.Set(strand_data.try_cursor, 1, enabled);
	}
}

void SourceImage::TestTryStrand(Strand& st) {
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
		for(int j = 0; j < st.enabled_count && enabled; j++) {
			int bit = st.enabled[j];
			bool is_bit_enabled = snap.Get(bit);
			enabled &= is_bit_enabled;
		}
		
		if (enabled) {
			bool signal_enabled = true;
			for(int j = 0; j < st.signal_count && signal_enabled; j++) {
				int bit = st.signal[j];
				bool is_bit_equal = snap.Get(bit) == signal;
				signal_enabled &= is_bit_equal;
			}
			enabled &= signal_enabled;
		}
		
		if (enabled) {
			bool oppositesignal_enabled = true;
			for(int j = 0; j < st.oppositesignal_count && oppositesignal_enabled; j++) {
				int bit = st.oppositesignal[j];
				bool is_bit_equal = snap.Get(bit) != signal;
				oppositesignal_enabled &= is_bit_equal;
			}
			enabled &= oppositesignal_enabled;
		}
		
		bool do_open = false, do_close = false;
		
		if (prev_enabled) {
			if (enabled) {
				if (signal != prev_signal) {
					do_open = true;
					do_close = true;
				}
			} else {
				do_close = true;
			}
		}
		else if (enabled) {
			do_open = true;
		}
		
		
		if (do_close) {
			double current = db.open[i];
			double change;
			if (!prev_signal)	change = current / (prev_open + point);
			else				change = 1.0 - (current / (prev_open - point) - 1.0);
			result *= change;
		}
		if (do_open) {
			double current = db.open[i];
			prev_open = current;
		}
		
		prev_signal = signal;
		prev_enabled = enabled;
	}
	
	st.result = result;
}

int SourceImage::GetTrySignal(int pos) {
	auto& strands = this->try_strands;
	
	if (main_booleans.IsEmpty())
		return 0;
	
	Snap& snap = pos == -1 ? main_booleans.Top() : main_booleans[pos];
	
	for(int i = 0; i < strands.GetCount(); i++) {
		Strand& st = strands.strands[i];
		if (st.result <= 1.0)
			continue;
		
		bool signal = snap.Get((SourceImage::period_count-1) * SourceImage::generic_row);
			
		bool enabled = true;
		for(int j = 0; j < st.enabled_count && enabled; j++) {
			int bit = st.enabled[j];
			bool is_bit_enabled = snap.Get(bit);
			enabled &= is_bit_enabled;
		}
		
		if (enabled) {
			bool signal_enabled = true;
			for(int j = 0; j < st.signal_count && signal_enabled; j++) {
				int bit = st.signal[j];
				bool is_bit_equal = snap.Get(bit) == signal;
				signal_enabled &= is_bit_equal;
			}
			enabled &= signal_enabled;
		}
		
		if (enabled) {
			bool oppositesignal_enabled = true;
			for(int j = 0; j < st.oppositesignal_count && oppositesignal_enabled; j++) {
				int bit = st.oppositesignal[j];
				bool is_bit_equal = snap.Get(bit) != signal;
				oppositesignal_enabled &= is_bit_equal;
			}
			enabled &= oppositesignal_enabled;
		}
		
		if (enabled) {
			int sig = 0;
			sig = signal ? -1.0 : +1.0;
			return sig;
		}
	}
	
	return 0;
}

}
