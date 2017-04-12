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
	total_slot_bytes = 0;
	for(int i = 0; i < slot.GetCount(); i++) {
		int bytes = slot[i]->GetReservedBytes();
		if (bytes == 0 && !slot[i]->IsWithoutData()) throw DataExc();
		slot_bytes[i] = bytes;
		total_slot_bytes += bytes;
	}
	
	// Reserve data for 'ready' flags
	slot_flag_bytes = slot.GetCount() / 8 + (slot.GetCount() % 8) == 0 ? 0 : 1;
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
	if (!enable_cache) throw DataExc();
	int64 file_pos = GetPersistencyCursor(sym_id, tf_id, pos);
	SlotData& slot_data = data[sym_id][tf_id][pos];
	if (!locked) cache_lock.Enter();
	if (slot_data.GetCount()) {
		if (!locked) cache_lock.Leave();
		return; // Should be empty. Possibility of threaded duplicate load exists.
	}
	reserved_memory += total_slot_bytes;
	slot_data.SetCount(total_slot_bytes); // don't lose lock for this, not worth it
	cache_file.Seek(file_pos);
	cache_file.Get(slot_data.Begin(), total_slot_bytes);
	if (!locked) cache_lock.Leave();
}

}
