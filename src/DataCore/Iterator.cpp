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
	//DLOG(pos[0] << "/" << tv->bars[0]);
	
	// Get attributes of the TimeVector
	int total_slot_bytes = tv->total_slot_bytes;
	int slot_flag_offset = tv->slot_flag_offset;
	int64 memory_limit = tv->memory_limit;
	int64& reserved_memory = tv->reserved_memory;
	Vector<int>& slot_bytes = tv->slot_bytes;
	Vector<SlotPtr>& slot = tv->slot;
	
	// Set attributes of the current time-position
	SlotProcessAttributes attr;
	ASSERTEXC(tv->bars.GetCount() <= 16);
	for(int i = 0; i < tv->bars.GetCount(); i++) {
		attr.pos[i] = pos[i];
		attr.bars[i] = tv->bars[i];
		attr.periods[i] = tv->periods[i];
	}
	
	// Set attributes of TimeVector::Iterator
	attr.it = this;
	attr.sym_count = tv->symbols.GetCount();
	attr.tf_count = tv->periods.GetCount();
	
	// Set attributes of how many timeframes are processed currently
	// E.g. when a year changes, also month, day, hour, minute, second timeframes changes
	//      but at wednesday 11:23am only minutes and seconds changes
	attr.processing_tf_count = attr.tf_count;
	for(int i = 0; i < attr.tf_count; i++) {
		int& pos = this->pos[i];
		if (i < attr.tf_count-1 && (pos % tv->tfbars_in_slowtf[i]) != 0) {
			attr.processing_tf_count = i+1;
			break;
		}
	}
	
	// Get symbol iterator
	Vector<Vector< Vector<SlotData>::Iterator > >::Iterator sym_begin, sym_iter, sym_iter_end;
	Vector< Vector<SlotData>::Iterator >::Iterator symtf_begin, symtf_iter, symtf_iter_end;
	sym_begin = iter.Begin();
	sym_iter = sym_begin;
	sym_iter_end = iter.End();
	
	// Set attributes of all symbol iterators (time position)
	attr.sym_it = &sym_begin;
	attr.tf_it = &symtf_begin;
	
	bool enable_cache = tv->enable_cache;
	bool check_memory_limit = memory_limit != 0;
	bool process_failed = false;
	
	// Loop symbols
	for (int i = 0; sym_iter != sym_iter_end; i++, sym_iter++) {
		
		// Get symbol/tf iterator
		symtf_begin = sym_iter->Begin();
		symtf_iter = symtf_begin;
		symtf_iter_end = sym_iter->End();
		
		// Set attributes of current symbol
		attr.sym_id = i;
		
		// Loop timeframes of the symbol
		for(int j = 0; symtf_iter != symtf_iter_end && j < attr.processing_tf_count; j++, symtf_iter++) {
			
			// Get slot iterator
			Vector<SlotData>::Iterator& slot_it = *symtf_iter;
			SlotData& slot_data = *slot_it;
			
			// Set attributes of current symbol/tf
			attr.tf_id = j;
			attr.slot_it = &slot_it;
			ASSERT((int64)&tv->data[i][j][attr.pos[j]] == (int64)&*slot_it);
			
			// Resize total slot memory if it is not the target size (first time only always?)
			int64 file_pos;
			bool has_file_pos = false;
			if (slot_data.GetCount() != total_slot_bytes) {
				
				// Load slot memory if caching is enabled
				if (enable_cache) {
					tv->cache_lock.Enter();
					
					// If nobody loaded cache while we waited entering locked state, then we still load it
					if (slot_data.GetCount() != total_slot_bytes) {
						// Check memory limits
						reserved_memory += total_slot_bytes;
						if (check_memory_limit && reserved_memory >= memory_limit) {
							ReleaseMemory(); // must be locked externally
						}
						
						// Cache should be loaded only to empty slot memory
						if (!slot_data.IsEmpty()) {
							tv->cache_lock.Leave();
							throw DataExc("slot_data.IsEmpty()"); // might throw legal duplicate load, though...
						}
						
						// Load slot memory from cache
						slot_data.SetCount(total_slot_bytes); // don't lose lock for this
						file_pos = tv->GetPersistencyCursor(i, j, attr.pos[j]);
						has_file_pos = true;
						tv->cache_file.Seek(file_pos);
						tv->cache_file.Get(slot_data.Begin(), total_slot_bytes);
					}
					// else you could check memory limits, but this is a rare occasion anyway
					
					tv->cache_lock.Leave();
				}
				else {
					// Check memory limits
					reserved_memory += total_slot_bytes;
					if (check_memory_limit && reserved_memory >= memory_limit) {
						ReleaseMemory();
					}
					
					// Reserve memory (without zeroing for performance reasons)
					ASSERTEXC(slot_data.IsEmpty());
					slot_data.SetCount(total_slot_bytes);
				}
			}
			
			// Loop slot memory area and slot processors
			SlotData::Iterator it = slot_data.Begin();
			Vector<SlotPtr>::Iterator sp = slot.Begin();
			
			// Set attributes of the begin of the total memory area of slots
			attr.slot_vector = it;
			
			// Get a ready-flag iterator (=is-processed-flags), which are in the end of the memory area
			byte* ready_slot = it + slot_flag_offset;
			int ready_bit = 0;
			
			// Loop slot processors
			for(int k = 0; k < slot_bytes.GetCount(); k++) {
				
				// Set attributes of the begin of the slot memory area
				attr.slot_bytes = slot_bytes[k];
				attr.data = it;
				
				// Check if slot is already processed
				byte ready_mask = 1 << ready_bit;
				bool is_ready = *ready_slot & ready_mask;
				
				// Process slot if not yet
				if (!is_ready) {
					Slot& slot = **sp;
					bool ready = slot.Process(attr);
					if (ready)
						*ready_slot |= ready_mask;
					else {
						process_failed = true; // return error;
						break; // don't process slots after this
					}
				}
				
				// Increase the ready-bit position (position in a byte)
				ready_bit++;
				if (ready_bit == 8) {
					ready_slot++;
					ready_bit = 0;
				}
				
				// Increase iterators to move next slot
				sp++;
				it += attr.slot_bytes;
			}
			
			// Store processed values immediately to the persistent cache
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
	
	// Store cache to hard drive if cache is enabled
	if (enable_cache) {
		tv->cache_lock.Enter();
		tv->cache_file.Flush();
		tv->cache_lock.Leave();
	}
	
	return process_failed;
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
	//LOG("TimeVector::Iterator::ReleaseMemory()");
	
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
