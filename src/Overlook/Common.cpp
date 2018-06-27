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







void DrawVectorPoints(Draw& id, Size sz, const Vector<double>& data) {
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int max_steps = 0;
	int count = data.GetCount();
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	if (count > max_steps)
		max_steps = count;
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		int count = data.GetCount();
		if (count >= 2) {
			for(int j = 0; j < count; j++) {
				double v = data[j];
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				id.DrawRect(x, y, 2, 2, Color(88, 114, 210));
			}
		}
	}
}

void DrawVectorPolyline(Draw& id, Size sz, const Vector<double>& data, Vector<Point>& polyline, int max_count) {
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = 0.0;
	
	int max_steps = 0;
	int count = data.GetCount();
	if (max_count > 0)
		count = Upp::min(count, max_count);
	for(int j = 0; j < count; j++) {
		double d = data[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	if (count > max_steps)
		max_steps = count;
	
	
	if (max_steps > 1 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		if (count >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < count; j++) {
				double v = data[j];
				last = v;
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				polyline.Add(Point(x, y));
				if (v > peak) peak = v;
			}
			if (polyline.GetCount() >= 2)
				id.DrawPolyline(polyline, 1, Color(81, 145, 137));
		}
		
		{
			int y = 0;
			String str = DblStr(peak);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(16, y, str, fnt, Black());
		}
		{
			int y = 0;
			String str = DblStr(last);
			Size str_sz = GetTextSize(str, fnt);
			id.DrawRect(sz.cx - 16 - str_sz.cx, y, str_sz.cx, str_sz.cy, White());
			id.DrawText(sz.cx - 16 - str_sz.cx, y, str, fnt, Black());
		}
	}
}

// Check LOCK() macro always for optimization break-ups
void TestLockMacro() {
	struct FakeLock {
		bool is_locked = false, is_unlocked = false;
		void Enter() {is_locked = true;}
		void Leave() {if (is_locked) is_unlocked = true;}
	};
	FakeLock fake;
	int count = 0;
	LOCK(fake) {
		count++;
	}
	if (!fake.is_locked || !fake.is_unlocked || count != 1)
		Panic("Test failed: LOCK macro is broken in this build (because of optimization). TODO: create lock wrapper and use it in the macro.");
}



const int tab64[64] = {
    63,  0, 58,  1, 59, 47, 53,  2,
    60, 39, 48, 27, 54, 33, 42,  3,
    61, 51, 37, 40, 49, 18, 28, 20,
    55, 30, 34, 11, 43, 14, 22,  4,
    62, 57, 46, 52, 38, 26, 32, 41,
    50, 36, 17, 19, 29, 10, 13, 21,
    56, 45, 25, 31, 35, 16,  9, 12,
    44, 24, 15,  8, 23,  7,  6,  5
};

int log2_64 (uint64 value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return tab64[((uint64_t)((value - (value >> 1))*0x07EDD5E59A4E28C2)) >> 58];
}

}