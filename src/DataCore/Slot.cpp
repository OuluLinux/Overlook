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
	reserved_bytes = 0;
	slot_offset = 0;
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
	value.offset = reserved_bytes;
	reserved_bytes += bytes;
}

void Slot::LoadCache(int sym_id, int tf_id, int pos) const {
	vector->LoadCache(sym_id, tf_id, pos);
}

const SlotData& Slot::GetData(int sym_id, int tf_id, int pos) const {
	return vector->data[sym_id][tf_id][pos];
}

void Slot::SetReady(int sym_id, int tf_id, int pos, const SlotProcessAttributes& attr, bool ready) {
	const SlotData& data = GetData(sym_id, tf_id, pos);
	if (!data.GetCount())
		LoadCache(sym_id, tf_id, pos);
	const byte* slot_vector = data.Begin();
	byte* ready_slot = (byte*)slot_vector + vector->slot_flag_offset;
	ASSERT(ready_slot);
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	if (ready)
		*ready_slot |= ready_mask;
	else
		*ready_slot &= !ready_mask;
	
	// The first byte in the slot (sym/tf/pos) vector is reserved for the 'changed' flag. Set it true also.
	*(bool*)slot_vector = true;
}

void Slot::SetReady(int pos, const SlotProcessAttributes& attr, bool ready) {
	SetReady(attr.sym_id, attr.tf_id, pos, attr, ready);
}

bool Slot::IsReady(int pos, const SlotProcessAttributes& attr) {
	const SlotData& data = GetData(attr.sym_id, attr.tf_id, pos);
	if (!data.GetCount())
		LoadCache(attr.sym_id, attr.tf_id, pos);
	const byte* ready_slot = data.Begin() + vector->slot_flag_offset;
	ASSERT(ready_slot);
	int slot_pos = attr.slot_pos;
	int ready_bit = slot_pos % 8;
	ready_slot += slot_pos / 8;
	byte ready_mask = 1 << ready_bit;
	return *ready_slot & ready_mask;
}

void Slot::AddDependency(String slot_path, String description) {
	TimeVector& tv = GetTimeVector();
	SlotPtr slot = tv.FindLinkSlot(slot_path);
	ASSERTEXC_(slot, "Could not link slot: " + slot_path);
	dependencies.Add(slot_path, slot);
}

const Slot& Slot::GetDependency(int i) {
	return *dependencies[i];
}

}
