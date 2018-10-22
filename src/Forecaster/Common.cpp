#include "Forecaster.h"

namespace Forecast {

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

VectorBool& VectorBool::SetCount(int i, bool b) {
	if (count == i) return *this;
	int c64 = i / 64;
	if (i % 64 != 0) c64++;
	count = i;
	data.SetCount(c64, b * 0xFFFFFFFFFFFFFFFF);
	return *this;
}

VectorBool& VectorBool::Reserve(int i) {
	if (count == i) return *this;
	int c64 = i / 64;
	if (i % 64 != 0) c64++;
	count = i;
	data.Reserve(c64);
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

VectorBool& VectorBool::SetInverse(const VectorBool& b) {
	SetCount(b.GetCount());
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 = ~(*it1);
	return *this;
}

VectorBool& VectorBool::InverseAnd(const VectorBool& b) {
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 &= ~(*it1);
	return *this;
}

VectorBool& VectorBool::And(const VectorBool& b) {
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 &= *it1;
	return *this;
}

VectorBool& VectorBool::Or(const VectorBool& b) {
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 |= *it1;
	return *this;
}

double VectorBool::GetOverlapFactor(const VectorBool& b) const {ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		pop_count += PopCount64(*it0 & *it1);
	return Upp::min(1.0, (double)pop_count / count);
}

int VectorBool::Hamming(const VectorBool& b) const {
	if (b.GetCount() != GetCount()) Panic("Hamming distance cannot be calculated");
	ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		pop_count += PopCount64(*it0 ^ *it1);
	return pop_count;
}

int VectorBool::PopCountAnd(const VectorBool& b) const {
	ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		pop_count += PopCount64(*it0 & *it1);
	return pop_count;
}

int VectorBool::PopCountNotAnd(const VectorBool& b) const {
	ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		pop_count += PopCount64(*it0 & (~*it1));
	return pop_count;
}

bool VectorBool::IsEqual(const VectorBool& b) const {
	if (b.GetCount() != GetCount()) Panic("Equality cannot be calculated");
	ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		if (*it0 != *it1)
			return false;
	return true;
}

bool VectorBool::Get(int64 i) const {
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < data.GetCount());
	ConstU64* it = Begin() + j;
	return *it & (1ULL << k);
}

void VectorBool::Set(int64 i, bool b) {
	int64 j = i / 64;
	int64 k = i % 64;
	ASSERT(j >= 0 && j < data.GetCount());
	uint64* it = data.Begin() + j;
	if (b)	*it |=  (1ULL << k);
	else	*it &= ~(1ULL << k);
}

void VectorBool::LimitLeft(int i) {
	if (i < 0 || i >= count)
		return;
	int byt = i / 64;
	int bit = i % 64;
	uint64* it = data.Begin() + byt;
	ASSERT(byt >= 0 && byt < data.GetCount());
	if (bit > 0) {
		for(int i = bit; i < 64; i++)
			*it &= ~(1ULL << i);
		it++;
	}
	ConstU64* end = data.End();
	for (; it != end; it++)
		*it = 0;
}

void VectorBool::LimitRight(int i) {
	if (i < 0 || i >= count)
		return;
	int byt = i / 64;
	int bit = i % 64;
	uint64* begin = data.Begin();
	uint64* it = begin + byt;
	ASSERT(byt >= 0 && byt < data.GetCount());
	if (bit > 0) {
		for(int i = bit-1; i >= 0; i--)
			*it &= ~(1ULL << i);
	}
	while (it != begin) {
		it--;
		*it = 0;
	}
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
