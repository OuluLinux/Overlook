#include "Overlook.h"

namespace Overlook {

int VectorBool::GetCount() const {
	return count;
}

int VectorBool::PopCount() const {
	ConstU64* it = Begin();
	ConstU64* end = End();
	int count = 0;
	for (; it != end; it++)
		count += PopCount64(*it);
	return count;
}

VectorBool& VectorBool::SetCount(int i) {
	int c64 = i / 64;
	if (i % 64 != 0) c64++;
	count = i;
	data.SetCount(c64, 0);
	return *this;
}

VectorBool& VectorBool::Zero() {
	uint64* it = data.Begin();
	ConstU64* end = data.End();
	for (; it != end; it++)
		*it = 0;
	return *this;
}

VectorBool& VectorBool::One() {
	uint64* it = data.Begin();
	ConstU64* end = data.End();
	for (; it != end; it++)
		*it = 0xFFFFFFFFFFFFFFFF;
	return *this;
}

bool VectorBool::Get(int i) const {
	int j = i / 64;
	int k = i % 64;
	ConstU64* it = Begin() + j;
	return *it & (1ULL << k);
}

void VectorBool::Set(int i, bool b) {
	int j = i / 64;
	int k = i % 64;
	uint64* it = data.Begin() + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

ConstU64* VectorBool::Begin() const {
	return data.Begin();
}

ConstU64* VectorBool::End() const {
	return data.End();
}

uint64* VectorBool::Begin() {
	return data.Begin();
}

uint64* VectorBool::End() {
	return data.End();
}

void VectorBool::operator=(const VectorBool& src) {
	data.SetCount(src.data.GetCount());
	
	uint64* ait  = data.Begin();
	uint64* aend = data.End();
	ConstU64* bit  = src.data.Begin();
	ConstU64* bend = src.data.End();
	for (;ait != aend; ait++, bit++)
		*ait = *bit;
	
	count = src.count;
}

}