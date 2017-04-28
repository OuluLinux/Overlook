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
	String filename;
	filename << sym_id << "-" << tf_id << ".bin";
	String path = AppendFileName(GetFileDir(), filename);
	FileIn in(path);
	in % data[sym_id][tf_id] % ready[sym_id][tf_id];
	SerializeCache(in, sym_id, tf_id);
}

void Slot::StoreCache(int sym_id, int tf_id) {
	String filename;
	filename << sym_id << "-" << tf_id << ".bin";
	String path = AppendFileName(GetFileDir(), filename);
	FileOut out(path);
	out % data[sym_id][tf_id] % ready[sym_id][tf_id];
	SerializeCache(out, sym_id, tf_id);
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
	ASSERT(ready_slot);
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
	dependencies.Add(slot_path, slot);
}

const Slot& Slot::GetDependency(int i) {
	return *dependencies[i];
}

}
