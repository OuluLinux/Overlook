#include "DataCore.h"

namespace DataCore {


DataExc::DataExc() {
	#ifdef flagDEBUG
	Panic("debug DataExc");
	#endif
}

DataExc::DataExc(String msg) : Exc(msg) {
	#ifdef flagDEBUG
	Panic("debug DataExc");
	#endif
}




Slot::Slot() {
	vector = 0;
	source = 0;
	forced_without_data = false;
	id = -1;
	reserved = 0;
	other_symbols = false;
	other_timeframes = false;
	has_attack = false;
	has_release = false;
	is_processed_once = true;
}

String Slot::GetFileDir() {
	if (!filedir.IsEmpty())
		return filedir;
	Session& ses = GetSession();
	String name = ses.GetName();
	ASSERT(!name.IsEmpty());
	String dir = AppendFileName(ConfigFile("profiles"), name);
	RealizeDirectory(dir);
	ASSERT(DirectoryExists(dir));
	String slot_dir = linkpath.Mid(1);
	slot_dir.Replace("/", "__");
	filedir = AppendFileName(dir, slot_dir);
	return filedir;
}

SlotPtr Slot::FindLinkSlot(const String& path) {
	ASSERTEXC(vector != NULL);
	PathLinkPtr link = vector->FindLinkPath(path);
	if (!link) return SlotPtr();
	return link->link;
}

SlotPtr Slot::ResolvePath(const String& path) {
	ASSERTEXC(vector != NULL);
	return vector->ResolvePath(path);
}

void Slot::LinkPath(String dest, String src) {
	ASSERTEXC(vector != NULL);
	vector->LinkPath(dest, src);
}

void Slot::AddValue(uint16 bytes, String name, String description) {
	SlotValue& value = values.Add();
	value.bytes = bytes;
	value.name = name;
	value.description = description;
	data.Add();
}

void Slot::LoadCache(int sym_id, int tf_id) {
	// only load to empty vector
	cache_lock.Enter();
	int existing = data[sym_id][tf_id][0].GetCount();
	if (existing > 0) {
		cache_lock.Leave();
		return;
	}
	
	String filename;
	filename << sym_id << "-" << tf_id << ".bin";
	String dir = GetFileDir();
	RealizeDirectory(dir);
	String path = AppendFileName(dir, filename);
	FileIn in(path);
	in % data[sym_id][tf_id] % ready[sym_id][tf_id];
	SerializeCache(in, sym_id, tf_id);
	
	// Recalculate reserved
	reserved = 0;
	for(int i = 0; i < data.GetCount(); i++) {
		Vector<Vector<Vector<byte> > >& sym = data[i];
		Vector<Vector<byte> >& r0 = ready[i];
		for(int j = 0; j < sym.GetCount(); j++) {
			Vector<Vector<byte> >& tf = sym[j];
			reserved += r0[j].GetCount();
			for(int k = 0; k < tf.GetCount(); k++)
				reserved += tf[k].GetCount();
		}
	}
	
	cache_lock.Leave();
}

void Slot::StoreCache(int sym_id, int tf_id) {
	cache_lock.Enter();
	String filename;
	filename << sym_id << "-" << tf_id << ".bin";
	String dir = GetFileDir();
	RealizeDirectory(dir);
	String path = AppendFileName(dir, filename);
	FileOut out(path);
	out % data[sym_id][tf_id] % ready[sym_id][tf_id];
	SerializeCache(out, sym_id, tf_id);
	cache_lock.Leave();
}

void Slot::Reserve(int sym_id, int tf_id) {
	int64 added = 0;
	TimeVector& tv = GetTimeVector();
	Vector<Vector<byte> >& d = data[sym_id][tf_id];
	Vector<byte>& r = ready[sym_id][tf_id];
	int count = tv.GetCount(tv.GetPeriod(tf_id));
	for(int i = 0; i < values.GetCount(); i++) {
		Vector<byte>& d0 = d[i];
		int64 size = count * values[i].bytes;
		added += size - d0.GetCount();
		d0.SetCount(size, 0);
	}
	int rsize = count / 8;
	added += rsize - r.GetCount();
	r.SetCount(rsize, 0); // 1 bit for every time-position
	if (count % 8 != 0) {
		r.Add(0); // add remaining
		added++;
	}
	reserved += added;
}

void Slot::Free(int64 target, int64& reserved_memory) {
	Session& ses = GetSession();
	for(int i = 0; i < data.GetCount() && reserved_memory > target; i++) {
		Vector<Vector<Vector<byte> > >& sym = data[i];
		Vector<Vector<byte> >& r0 = ready[i];
		for(int j = 0; j < sym.GetCount() && reserved_memory > target; j++) {
			Vector<Vector<byte> >& tf = sym[j];
			Vector<byte>& r1 = r0[j];
			
			// Don't clear data which is being used currently
			if (ses.IsLocked(id, i, j))
				continue;
			
			// Free memory
			int64 freed = r1.GetCount();
			r1.Clear();
			for(int k = 0; k < tf.GetCount(); k++) {
				Vector<byte>& vec = tf[k];
				freed += vec.GetCount();
				vec.Clear();
			}
			reserved -= freed;
			reserved_memory -= freed;
		}
	}
}

void Slot::DataInit() {
	TimeVector& tv = GetTimeVector();
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	data.SetCount(sym_count);
	ready.SetCount(sym_count);
	for(int i = 0; i < data.GetCount(); i++) {
		Vector<Vector<Vector<byte> > >& sym = data[i];
		Vector<Vector<byte> >& r0 = ready[i];
		sym.SetCount(tf_count);
		r0.SetCount(tf_count);
		for(int j = 0; j < sym.GetCount(); j++) {
			Vector<Vector<byte> >& tf = sym[j];
			tf.SetCount(values.GetCount());
		}
	}
}

void Slot::SetReady(int sym_id, int tf_id, int pos, const SlotProcessAttributes& attr, bool ready) {
	byte* ready_slot = this->ready[sym_id][tf_id].Begin();
	ASSERT(ready_slot);
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	if (ready)
		*ready_slot |= ready_mask;
	else
		*ready_slot &= !ready_mask;
}

void Slot::SetReady(int pos, const SlotProcessAttributes& attr, bool ready) {
	SetReady(attr.sym_id, attr.tf_id, pos, attr, ready);
}

bool Slot::IsReady(int pos, const SlotProcessAttributes& attr) {
	byte* ready_slot = ready[attr.sym_id][attr.tf_id].Begin();
	if (!ready_slot)
		return false;
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	return ready_mask & *ready_slot;
}

void Slot::AddDependency(String slot_path, bool other_symbols, bool other_timeframes) {
	TimeVector& tv = GetTimeVector();
	SlotPtr slot = tv.FindLinkSlot(slot_path);
	ASSERTEXC_(slot, "Could not link slot: " + slot_path);
	Dependency& dep = dependencies.Add(slot_path);
	dep.slot = slot;
	dep.other_symbols = other_symbols;
	dep.other_timeframes = other_timeframes;
}

const Slot& Slot::GetDependency(int i) {
	return *dependencies[i].slot;
}

void Slot::Enter(BatchPartStatus& stat) {
	Session& ses = GetSession();
	TimeVector& tv = GetTimeVector();
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	
	
	// Lock memory, load cache and reserve memory for current class according to settings
	if (!other_symbols) {
		if (!other_timeframes) {
			// Lock source memory
			ses.Enter(id, stat.sym_id, stat.tf_id);
			
			// Load cache
			LoadCache(stat.sym_id, stat.tf_id);
			
			// Reserve rest of the memory
			Reserve(stat.sym_id, stat.tf_id);
		} else {
			for(int i = 0; i < tf_count; i++) {
				ses.Enter(id, stat.sym_id, i);
				LoadCache(stat.sym_id, i);
				Reserve(stat.sym_id, i);
			}
		}
	} else {
		if (!other_timeframes) {
			for(int i = 0; i < sym_count; i++) {
				ses.Enter(id, i, stat.tf_id);
				LoadCache(i, stat.tf_id);
				Reserve(i, stat.tf_id);
			}
		} else {
			for(int i = 0; i < sym_count; i++) {
				for(int j = 0; j < tf_count; j++) {
					ses.Enter(id, i, j);
					LoadCache(i, j);
					Reserve(i, j);
				}
			}
		}
	}
	
	
	// Check if memory release is needed
	tv.CheckMemoryLimit();
	
	
	// Load caches of dependencies
	for(int i = 0; i < dependencies.GetCount(); i++) {
		Dependency& dep = dependencies[i];
		Slot& slot = *dep.slot;
		if (!dep.other_symbols) {
			if (!dep.other_timeframes) {
				slot.LoadCache(stat.sym_id, stat.tf_id);
			} else {
				for(int j = 0; j < tf_count; j++)
					slot.LoadCache(stat.sym_id, j);
			}
		}
		else {
			if (!dep.other_timeframes) {
				for(int j = 0; j < sym_count; j++)
					slot.LoadCache(j, stat.tf_id);
			} else {
				for(int j = 0; j < sym_count; j++)
					for(int k = 0; k < tf_count; k++)
						slot.LoadCache(j, k);
			}
		}
	}
	
}

void Slot::Leave(BatchPartStatus& stat) {
	Session& ses = GetSession();
	TimeVector& tv = GetTimeVector();
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	
	// Unlock memory and store cache for current class according to settings
	if (!other_symbols) {
		if (!other_timeframes) {
			// Release source memory
			ses.Leave(id, stat.sym_id, stat.tf_id);
			
			// Store changes
			StoreCache(stat.sym_id, stat.tf_id);
		} else {
			for(int i = 0; i < tf_count; i++) {
				ses.Leave(id, stat.sym_id, i);
				StoreCache(stat.sym_id, i);
			}
		}
	} else {
		if (!other_timeframes) {
			for(int i = 0; i < sym_count; i++) {
				ses.Leave(id, i, stat.tf_id);
				StoreCache(i, stat.tf_id);
			}
		} else {
			for(int i = 0; i < sym_count; i++) {
				for(int j = 0; j < tf_count; j++) {
					ses.Leave(id, i, j);
					StoreCache(i, j);
				}
			}
		}
	}
}

int Slot::GetReservedBytes() const {
	int bytes = 0;
	for(int i = 0; i < values.GetCount(); i++)
		bytes += values[i].bytes;
	return bytes;
}

}
