#include "DataCore.h"

namespace DataCore {

TimeVector::Iterator::Iterator(TimeVectorPtr tv) {
	this->tv = tv;
}

TimeVector::Iterator::Iterator(const Iterator& it) {
	*this = it;
}

void TimeVector::Iterator::operator =(const Iterator& it) {
	tv = it.tv;
	// Less safe (faster)
	if (1) {
		iter <<= it.iter;
		pos <<= it.pos;
	}
	// Safe (slower)
	else {
		iter.Clear();
		pos.Clear();
		if (it.pos.IsEmpty()) return;
		SetPosition(it.pos[0]);
	}
}

void TimeVector::Iterator::SetPosition(int p) {
	int sym_count = tv->symbols.GetCount();
	int tf_count = tv->periods.GetCount();
	
	pos.SetCount(tf_count, 0);
	for(int i = 0; i < tf_count; i++) {
		pos[i] = p;
		if (i == tf_count-1) break;
		p = tv->GetShift(tv->periods[i], tv->periods[i+1], p);
	}
	
	iter.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++) {
		Vector< Vector<SlotData>::Iterator >& sym_iter = iter[i];
		sym_iter.SetCount(tf_count);
		for(int j = 0; j < tf_count; j++) {
			Vector<SlotData>::Iterator& symtf_iter = sym_iter[j];
			Vector<SlotData>::Iterator it = tv->data[i][j].Begin();
			
			symtf_iter = it + pos[j];
		}
	}
	
}

bool TimeVector::Iterator::Process() {
	DLOG(pos[0] << "/" << tv->bars[0]);
	
	int total_slot_bytes = tv->total_slot_bytes;
	int slot_flag_offset = tv->slot_flag_offset;
	int64 memory_limit = tv->memory_limit;
	int64& reserved_memory = tv->reserved_memory;
	Vector<int>& slot_bytes = tv->slot_bytes;
	Vector<SlotPtr>& slot = tv->slot;
	
	SlotProcessAttributes attr;
	
	ASSERTEXC(tv->bars.GetCount() <= 16);
	for(int i = 0; i < tv->bars.GetCount(); i++) {
		attr.pos[i] = pos[i];
		attr.bars[i] = tv->bars[i];
		attr.periods[i] = tv->periods[i];
	}
	
	
	attr.it = this;
	attr.sym_count = tv->symbols.GetCount();
	attr.tf_count = tv->periods.GetCount();
	
	attr.processing_tf_count = attr.tf_count;
	for(int i = 0; i < attr.tf_count; i++) {
		int& pos = this->pos[i];
		if (i < attr.tf_count-1 && (pos % tv->tfbars_in_slowtf[i]) != 0) {
			attr.processing_tf_count = i+1;
			break;
		}
	}
	
	Vector<Vector< Vector<SlotData>::Iterator > >::Iterator sym_begin, sym_iter, sym_iter_end;
	Vector< Vector<SlotData>::Iterator >::Iterator symtf_begin, symtf_iter, symtf_iter_end;
	
	sym_begin = iter.Begin();
	sym_iter = sym_begin;
	sym_iter_end = iter.End();
	
	attr.sym_it = &sym_begin;
	attr.tf_it = &symtf_begin;
	
	bool enable_cache = tv->enable_cache;
	bool check_memory_limit = memory_limit != 0;
	
	for (int i = 0; sym_iter != sym_iter_end; i++, sym_iter++) {
		symtf_begin = sym_iter->Begin();
		symtf_iter = symtf_begin;
		symtf_iter_end = sym_iter->End();
		
		attr.sym_id = i;
		
		for(int j = 0; symtf_iter != symtf_iter_end && j < attr.processing_tf_count; j++, symtf_iter++) {
			
			Vector<SlotData>::Iterator& slot_it = *symtf_iter;
			
			attr.tf_id = j;
			attr.slot_it = &slot_it;
			
			ASSERT((int64)&tv->data[i][j][attr.pos[j]] == (int64)&*slot_it);
			
			SlotData& slot_data = *slot_it;
			
			int64 file_pos;
			bool has_file_pos = false;
			if (slot_data.GetCount() != total_slot_bytes) {
								
				if (enable_cache) {
					tv->cache_lock.Enter();
					
					// Check memory limits
					reserved_memory += total_slot_bytes;
					if (check_memory_limit && reserved_memory >= memory_limit) {
						ReleaseMemory(); // must be locked externally
					}
					
					ASSERTEXC(slot_data.IsEmpty()); // might throw legal duplicate load, though...
					slot_data.SetCount(total_slot_bytes); // don't lose lock for this
					file_pos = tv->GetPersistencyCursor(i, j, attr.pos[j]);
					has_file_pos = true;
					tv->cache_file.Seek(file_pos);
					tv->cache_file.Get(slot_data.Begin(), total_slot_bytes);
					tv->cache_lock.Leave();
				}
				else {
					// Check memory limits
					reserved_memory += total_slot_bytes;
					if (check_memory_limit && reserved_memory >= memory_limit) {
						ReleaseMemory();
					}

					ASSERTEXC(slot_data.IsEmpty());
					slot_data.SetCount(total_slot_bytes);
				}
			}
			
			
			SlotData::Iterator it = slot_data.Begin();
			Vector<SlotPtr>::Iterator sp = slot.Begin();
			
			attr.slot_vector = it;
			
			byte* ready_slot = it + slot_flag_offset;
			int ready_bit = 0;
			
			for(int k = 0; k < slot_bytes.GetCount(); k++) {
				attr.slot_bytes = slot_bytes[k];
				attr.data = it;
				
				byte ready_mask = 1 << ready_bit;
				bool is_ready = *ready_slot & ready_mask;
				
				if (!is_ready) {
					Slot& slot = **sp;
					bool ready = slot.Process(attr);
					*ready_slot |= ready_mask;
				}
				
				ready_bit++;
				if (ready_bit == 8) {
					ready_slot++;
					ready_bit = 0;
				}
				
				sp++;
				it += attr.slot_bytes;
			}
			
			if (enable_cache) {
				if (!has_file_pos)
					file_pos = tv->GetPersistencyCursor(i, j, attr.pos[j]);
				tv->cache_lock.Enter();
				tv->cache_file.Seek(file_pos);
				tv->cache_file.Put(attr.slot_vector, total_slot_bytes);
				tv->cache_lock.Leave();
			}
		}
	}
	
	if (enable_cache) {
		tv->cache_lock.Enter();
		tv->cache_file.Flush();
		tv->cache_lock.Leave();
	}
	
	return false;
}

bool TimeVector::Iterator::IsEnd() {
	if (!tv) throw DataExc();
	if (pos.IsEmpty()) throw DataExc();
	int pos = this->pos[0];
	int bars = tv->bars[0]; //GetCount(1);
	if (pos < 0 || pos > bars) throw DataExc();
	return pos == bars;
}

void TimeVector::Iterator::operator ++(int i) {
	if (IsEnd()) throw DataExc();
	
	Vector<Vector< Vector<SlotData>::Iterator > >::Iterator sym_iter, sym_iter_end;
	Vector< Vector<SlotData>::Iterator >::Iterator symtf_iter, symtf_iter_end;
	
	int tfs = tv->periods.GetCount();
	int changing_tfs = tfs;
	for(int i = 0; i < tfs; i++) {
		int& pos = this->pos[i];
		pos++;
		if (i < tfs-1 && (pos % tv->tfbars_in_slowtf[i]) != 0) {
			changing_tfs = i+1;
			break;
		}
	}
	
	sym_iter = iter.Begin();
	sym_iter_end = iter.End();
	for (int i = 0; sym_iter != sym_iter_end; i++, sym_iter++) {
		symtf_iter = sym_iter->Begin();
		symtf_iter_end = sym_iter->End();
		
		for(int j = 0; j < changing_tfs; j++, symtf_iter++) {
			//ASSERT(symtf_iter != symtf_iter_end);
			
			Vector<SlotData>::Iterator& it = *symtf_iter;
			it++;
			
			//ASSERT(it == tv->data[i][j].Begin() + pos[j]);
		}
	}
}

void TimeVector::Iterator::ReleaseMemory() {
	LOG("TimeVector::Iterator::ReleaseMemory()");
	
	Vector<int> begin_pos, end_pos, bars;
	begin_pos <<= pos;
	end_pos <<= pos;
	bars <<= tv->bars;
	
	int sym_count = tv->symbols.GetCount();
	int tf_count = begin_pos.GetCount();
	int total_slot_bytes = tv->total_slot_bytes;
	int64 memory_limit = (int64)(tv->memory_limit * 0.5);
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
			
			Vector<Vector<byte> >::Iterator it = tv->data[i][j].Begin();
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
	
	tv->reserved_memory = memory_size;
}

}
