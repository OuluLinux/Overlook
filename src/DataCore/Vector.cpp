#include "DataCore.h"

namespace DataCore {

TimeVector::TimeVector() {
	timediff = 0;
	base_period = 60;
	reversed = false;
	cache_file_path = ConfigFile("TimeVector.bin");
	enable_cache = false;
	memory_limit = 0;
	header_size = 0;
	bars_total = 0;
	reserved_memory = 0;
	slot_flag_bytes = 0;
	current_slotattr = NULL;
}

TimeVector::~TimeVector() {
	for(int i = 0; i < slot.GetCount(); i++) {
		SlotPtr sp = slot[i];
		if (sp) {
			delete sp;
		}
	}
}

void TimeVector::AddPeriod(int period) {
	int count = periods.GetCount();
	
	// Currently first period must match base period
	if (count == 0 && base_period != period)
		throw DataExc();
	
	period /= base_period;
	
	if (count) {
		int i = period / periods[count-1];
		if (i <= 1) throw DataExc();
		if (count-1 != tfbars_in_slowtf.GetCount()) throw DataExc();
		tfbars_in_slowtf.Add(i);
	}
	
	periods.Add(period);
}

void TimeVector::AddSymbol(String sym) {
	symbols.Add(sym);
}

int TimeVector::GetCount(int period) const {
	int div = base_period * period;
	int count = timediff / div;
	if (count % div != 0) count++;
	return count * (reversed ? -1 : 1);
}

void TimeVector::RefreshData() {
	int64 sym_count = symbols.GetCount();
	int64 tf_count = periods.GetCount();
	
	if (sym_count == 0) throw DataExc();
	if (tf_count == 0)  throw DataExc();
	if (slot.IsEmpty()) throw DataExc();
	
	bars.SetCount(tf_count);
	bars_total = 0;
	for(int i = 0; i < bars.GetCount(); i++) {
		int period = periods[i];
		int count = GetCount(period);
		if (!count) throw DataExc();
		bars[i] = count;
		bars_total += count;
	}
	
	reserved_memory = 0;
	data.Clear();
	data.SetCount(sym_count);
	for(int i = 0; i < data.GetCount(); i++) {
		Vector<Vector<SlotData> >& sym_data = data[i];
		sym_data.SetCount(tf_count);
		for(int j = 0; j < sym_data.GetCount(); j++) {
			Vector<SlotData>& symtf_data = sym_data[j];
			symtf_data.SetCount(bars[j]);
		}
	}
	
	slot_bytes.SetCount(slot.GetCount(), 0);
	total_slot_bytes = 1; // one byte for "changed" flag
	for(int i = 0; i < slot.GetCount(); i++) {
		int bytes = slot[i]->GetReservedBytes();
		if (bytes == 0 && !slot[i]->IsWithoutData()) throw DataExc();
		slot_bytes[i] = bytes;
		total_slot_bytes += bytes;
	}
	
	// Reserve data for 'ready' flags
	slot_flag_bytes = slot.GetCount() / 8 + ((slot.GetCount() % 8) == 0 ? 0 : 1);
	slot_flag_offset = total_slot_bytes;
	total_slot_bytes += slot_flag_bytes;
	
	if (enable_cache) {
		if (cache_file.IsOpen()) cache_file.Close();
		
		cache_file.Open(cache_file_path);
		ASSERTEXC(cache_file.IsOpen());
		
		header_size = 4 * sizeof(int64) + 2 * sizeof(Time);
		
		// Compare header
		cache_file.Seek(0);
		if (cache_file.GetSize() >= header_size) {
			int64 header_sym_count, header_tf_count, header_total_slot_bytes, header_bars_total;
			Time begin, end;
			cache_file.Get(&header_sym_count, sizeof(int64));
			cache_file.Get(&header_tf_count, sizeof(int64));
			cache_file.Get(&header_total_slot_bytes, sizeof(int64));
			cache_file.Get(&header_bars_total, sizeof(int64));
			cache_file.Get(&begin, sizeof(Time));
			cache_file.Get(&end, sizeof(Time));
			
			if (header_sym_count != sym_count ||
				header_tf_count != tf_count ||
				header_total_slot_bytes != total_slot_bytes ||
				header_bars_total != bars_total ||
				begin != this->begin ||
				end != this->end) {
				throw DataExc();
			}
		}
		
		// Write header
		else {
			cache_file.Put(&sym_count, sizeof(int64));
			cache_file.Put(&tf_count, sizeof(int64));
			cache_file.Put(&total_slot_bytes, sizeof(int64));
			cache_file.Put(&bars_total, sizeof(int64));
			cache_file.Put(&begin, sizeof(Time));
			cache_file.Put(&end, sizeof(Time));
		}
		
		// Reserve file
		int64 total_size = header_size + sym_count * total_slot_bytes * bars_total;
		int64 append_size = total_size - cache_file.GetSize();
		ASSERTEXC(total_size > 0);
		cache_file.SeekEnd();
		ASSERTEXC(append_size <= INT_MAX);
		if (append_size > 0)
			cache_file.Put0((int)append_size);
	}
}

int64 TimeVector::GetShift(int src_period, int dst_period, int shift) {
	int64 timediff = shift * src_period; //		* base_period
	return (int)(timediff / dst_period); //			/ base_period
}

int64 TimeVector::GetShiftFromTime(int timestamp, int period) {
	int64 timediff = timestamp - begin_ts;
	return (int)(timediff / period / base_period * (reversed ? -1 : 1));
}

int TimeVector::GetTfFromSeconds(int period_seconds) {
	return period_seconds / base_period;
}

int64 TimeVector::GetPersistencyCursor(int sym_id, int tf_id, int shift) {
	if (!enable_cache) throw DataExc();
	ASSERTEXC(sym_id >= 0 && sym_id < symbols.GetCount());
	ASSERTEXC(tf_id >= 0 && tf_id < periods.GetCount());
	ASSERTEXC(shift >= 0 && shift < bars[tf_id]);
	int64 bars = sym_id * bars_total;
	for(int i = 0; i < tf_id; i++) {
		bars += this->bars[i];
	}
	bars += shift;
	int64 pos = header_size + total_slot_bytes * bars;
	return pos;
}

TimeVector::Iterator TimeVector::Begin() {
	Iterator it(this);
	
	it.SetPosition(0);
	
	return it;
}

void TimeVector::AddCustomSlot(String key, SlotFactory f) {
	GetFactories().Add(key, f);
}

void TimeVector::LoadCache(int sym_id, int tf_id, int pos, bool locked) {
	if (!locked) cache_lock.Enter();
	
	int64 file_pos = GetPersistencyCursor(sym_id, tf_id, pos);
	SlotData& slot_data = data[sym_id][tf_id][pos];
	if (slot_data.GetCount()) {
		if (!locked) cache_lock.Leave();
		return; // Should be empty. Possibility of threaded duplicate load exists.
	}
	
	// Check memory limits
	reserved_memory += total_slot_bytes;
	if (memory_limit && reserved_memory >= memory_limit) {
		StoreChangedCache();
		ReleaseMemory(tf_id, pos);
	}
	
	// Reserve memory (without zeroing for performance reasons)
	ASSERTEXC(slot_data.IsEmpty());
	slot_data.SetCount(total_slot_bytes);
	
	if (enable_cache) {
		slot_data.SetCount(total_slot_bytes); // don't lose lock for this, not worth it
		cache_file.Seek(file_pos);
		cache_file.Get(slot_data.Begin(), total_slot_bytes);
	}
	
	if (!locked) cache_lock.Leave();
}

void TimeVector::ReleaseMemory(int tf_id, int pos) {
	int fast_pos = pos;
	// Change position, if the current timeframe is not the smallest
	if (tf_id != 0) {
		fast_pos = GetShift(GetPeriod(tf_id), 1, pos);
	}
	ReleaseMemory(fast_pos);
}

void TimeVector::StoreChangedCache() {
	
	// Store only if cache is enabled
	if (!enable_cache)
		return;
	
	// Loop data range
	Vector<Vector<Vector<SlotData> > >::Iterator sym_it  = data.Begin();
	Vector<Vector<Vector<SlotData> > >::Iterator sym_end = data.End();
	for (int i = 0; sym_it != sym_end; sym_it++, i++) {
		Vector<Vector<SlotData> >::Iterator tf_it = sym_it->Begin();
		Vector<Vector<SlotData> >::Iterator tf_end = sym_it->End();
		for (int j = 0; tf_it != tf_end; tf_it++, j++) {
			Vector<SlotData>::Iterator slotdata_it = tf_it->Begin();
			Vector<SlotData>::Iterator slotdata_end = tf_it->End();
			for (int k = 0; slotdata_it != slotdata_end; slotdata_it++, k++) {
				byte* data = slotdata_it->Begin();
				
				// If slot has reserved memory and the content has been flagged as 'changed'
				if (data && *data) {
					
					// Data should have constant size
					ASSERT(slotdata_it->GetCount() == total_slot_bytes);
					
					// Write data to the persistent file
					int64 file_pos = GetPersistencyCursor(i, j, k);
					cache_file.Seek(file_pos);
					cache_file.Put(data, total_slot_bytes);
					
					// Mark data as 'unchanged'
					*data = false;
				}
			}
		}
	}
}

void TimeVector::ReleaseMemory(int fastest_pos) {
	LOG("TimeVector::ReleaseMemory()");
	
	int tf_count = periods.GetCount();
	int p = fastest_pos;
	Vector<int> pos, begin_pos, end_pos;
	pos.SetCount(tf_count, 0);
	for(int i = 0; i < tf_count; i++) {
		pos[i] = p;
		if (i == tf_count-1) break;
		p = GetShift(periods[i], periods[i+1], p);
	}
	begin_pos <<= pos;
	end_pos <<= pos;
	
	int sym_count = symbols.GetCount();
	int64 memory_limit = (int64)(this->memory_limit * 0.5);
	int64 memory_size = 0;
	int64 tf_row_size = sym_count * total_slot_bytes;
	
	if (!tf_count || !sym_count) throw DataExc();
	
	bool growing = true;
	while (growing) {
		growing = false;
		
		for(int i = 0; i < tf_count; i++) {
			
			int& begin = begin_pos[i];
			int& end = end_pos[i];
			
			if (begin > 0) {
				begin--;
				memory_size += tf_row_size;
				growing = true;
			}
			
			if (end < bars[i]) {
				end++;
				memory_size += tf_row_size;
				growing = true;
			}
			
			if (memory_size >= memory_limit) {
				growing = false;
				break;
			}
		}
	}
	
	memory_size = 0;
	for(int i = 0; i < sym_count; i++) {
		for(int j = 0; j < tf_count; j++) {
			int begin = begin_pos[j];
			int end = end_pos[j];
			int limit = bars[j];
			
			Vector<Vector<byte> >::Iterator it = data[i][j].Begin();
			for(int k = 0; k < begin; k++, it++)
				it->Clear();
			for(int k = begin; k < end; k++, it++) {
				if (!it->IsEmpty())
					memory_size += it->GetCount();
			}
			for(int k = end; k < limit; k++, it++)
				it->Clear();
		}
	}
	
	reserved_memory = memory_size;
}

}
