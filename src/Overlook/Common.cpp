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
	if (count == i) return *this;
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

VectorBool& VectorBool::InverseAnd(const VectorBool& b) {
	ASSERT(data.GetCount() == b.data.GetCount());
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 &= ~(*it1);
	return *this;
}

VectorBool& VectorBool::And(const VectorBool& b) {
	ASSERT(data.GetCount() == b.data.GetCount());
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 &= *it1;
	return *this;
}

VectorBool& VectorBool::Or(const VectorBool& b) {
	ASSERT(data.GetCount() == b.data.GetCount());
	uint64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		*it0 |= *it1;
	return *this;
}

double VectorBool::GetOverlapFactor(const VectorBool& b) const {
	ASSERT(data.GetCount() == b.data.GetCount());
	ConstU64* it0 = data.Begin();
	ConstU64* it1 = b.Begin();
	ConstU64* end0 = data.End();
	ConstU64* end1 = b.data.End();
	int pop_count = 0;
	for (; it0 != end0 && it1 != end1; it0++, it1++)
		pop_count += PopCount64(*it0 & *it1);
	return Upp::min(1.0, (double)pop_count / count);
}

bool VectorBool::Get(int i) const {
	int j = i / 64;
	int k = i % 64;
	ASSERT(j >= 0 && j < data.GetCount());
	ConstU64* it = Begin() + j;
	return *it & (1ULL << k);
}

void VectorBool::Set(int i, bool b) {
	int j = i / 64;
	int k = i % 64;
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




void TestExtremumCache() {
	
	Vector<double> data;
	
	for(int i = 0; i < 100000; i++) {
		data.Add(Randomf());
	}
	
	int period = 100;
	ExtremumCache ec(period);
	for(int i = 0; i < data.GetCount(); i++) {
		
		// Less complex
		ec.Add(data[i], data[i]);
		int ec_highest = ec.GetHighest();
		int ec_lowest = ec.GetLowest();
		
		// More complex
		int highest = -1, lowest = -1;
		double highestv = -DBL_MAX, lowestv = DBL_MAX;
		for(int j = 0; j < period; j++) {
			int pos = i - j;
			if (pos < 0) break;
			
			double d = data[pos];
			if (d < lowestv) {
				lowest = pos;
				lowestv = d;
			}
			if (d > highestv) {
				highest = pos;
				highestv = d;
			}
		}
		
		// Must match
		LOG(Format("%d: %d != %d, %d != %d", i, highest, ec_highest, lowest, ec_lowest));
		if (highest != ec_highest || lowest != ec_lowest)
			Panic("Invalid value");
	}
	
}

}