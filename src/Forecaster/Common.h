#ifndef _Forecaster_Common_h_
#define _Forecaster_Common_h_

namespace Forecast {

typedef Exc ConfExc;
typedef Exc DataExc;




class OnlineAverageWindow1 : Moveable<OnlineAverageWindow1> {
	Vector<double> win_a;
	double sum_a = 0.0;
	int period = 0, cursor = 0;
	int count = 0;
	
public:
	OnlineAverageWindow1() {}
	void SetPeriod(int i) {period = i; win_a.SetCount(i,0);}
	void Add(double a) {
		double& da = win_a[cursor];
		sum_a -= da;
		da = a;
		sum_a += da;
		count++;
		cursor = (cursor + 1) % period;
	}
	double GetMean() const {return sum_a / min(count, period);}
	int GetPeriod() const {return period;}
	void Serialize(Stream& s) {s % win_a % sum_a % period % cursor % count;}
	const Vector<double>& GetWindow() const {return win_a;}
};


inline int PopCount64(uint64 i) {
	#ifdef flagMSC
	#if CPU_64
	return __popcnt64(i);
	#elif CPU_32
	return __popcnt(i) + __popcnt(i >> 32);
	#endif
	#else
	return __builtin_popcountll(i);
	#endif
}

inline int PopCount32(uint32 i) {
	#ifdef flagMSC
	#if CPU_64
	return __popcnt64(i);
	#elif CPU_32
	return __popcnt(i);
	#endif
	#else
	return __builtin_popcountl(i);
	#endif
}

















struct ValueBase {
	int count=0, visible=0, data_type=-1, min=-1, max=-1, factory=-1;
	const char* s0 = "";
	void* data = NULL;
	void* data2 = NULL;
	ValueBase() {}
	virtual ~ValueBase() {}
	enum {IN_, INOPT_, OUT_, LBL_, INT_, PERS_};
	void operator = (const ValueBase& vb) {
		count		= vb.count;
		visible		= vb.visible;
		data_type	= vb.data_type;
		min			= vb.min;
		max			= vb.max;
		factory		= vb.factory;
		s0			= vb.s0;
		data		= vb.data;
		data2		= vb.data2;
	}
};

struct ValueRegister {
	ValueRegister() {}
	
	virtual void IO(const ValueBase& base) = 0;
	virtual ValueRegister& operator % (const ValueBase& base) {IO(base); return *this;}
};

struct FactoryDeclaration : Moveable<FactoryDeclaration> {
	int args[8];
	int factory = -1;
	int arg_count = 0;
	
	FactoryDeclaration() {}
	FactoryDeclaration(const FactoryDeclaration& src) {*this = src;}
	FactoryDeclaration& Set(int i) {factory = i; return *this;}
	FactoryDeclaration& AddArg(int i) {ASSERT(arg_count >= 0 && arg_count < 8); args[arg_count++] = i; return *this;}
	FactoryDeclaration& operator=(const FactoryDeclaration& src) {
		factory = src.factory;
		arg_count = src.arg_count;
		for(int i = 0; i < 8; i++) args[i] = src.args[i];
		return *this;
	}
	
	unsigned GetHashValue() {
		CombineHash ch;
		ch << factory << 1;
		for(int i = 0; i < arg_count; i++)
			ch << args[i] << 1;
		return ch;
	}
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(args, sizeof(int) * 8);
		}
		else if (s.IsStoring()) {
			s.Put(args, sizeof(int) * 8);
		}
		s % factory % arg_count;
	}
};











typedef const double ConstDouble;

class Buffer : public Moveable<Buffer> {
	
public:
	Vector<double> value;
	String label;
	Color clr;
	int style, line_style, line_width, chr, begin, shift, earliest_write;
	bool visible;
	
public:
	Buffer() : clr(Black()), style(0), line_width(1), chr('^'), begin(0), shift(0), line_style(0), visible(true), earliest_write(INT_MAX) {}
	void Serialize(Stream& s) {s % value % label % clr % style % line_style % line_width % chr % begin % shift % visible;}
	void SetCount(int i) {value.SetCount(i, 0.0);}
	void Add(double d) {value.Add(d);}
	void Reserve(int n) {value.Reserve(n);}
	
	int GetResetEarliestWrite() {int i = earliest_write; earliest_write = INT_MAX; return i;}
	int GetCount() const {return value.GetCount();}
	bool IsEmpty() const {return value.IsEmpty();}
	double GetUnsafe(int i) const {return value[i];}
	double Top() const {return value.Top();}
	
	ConstDouble* Begin() const {return value.Begin();}
	ConstDouble* End()   const {return value.End();}
	double* Begin() {return value.Begin();}
	double* End()   {return value.End();}
	
	// Some utility functions for checking that indicator values are strictly L-R
	double Get(int i) const {return value[i];}
	void Set(int i, double value) {this->value[i] = value; if (i < earliest_write) earliest_write = i;}
	void Inc(int i, double value) {this->value[i] += value;}
	
	
};


class CompatBuffer {
	const Buffer* b;
	int pos, shift;
	
public:
	CompatBuffer(const Buffer& b, int pos, int shift=0) {
		this->b = &b;
		this->pos = pos;
		this->shift = shift;
	}
	
	double operator[] (int i) const {int j = pos - i + shift; ASSERT(j <= pos); return b->Get(j);}
};


typedef const uint64 ConstU64;

class VectorBool : Moveable<VectorBool> {
	Vector<uint64> data;
	int count = 0;
	
public:
	VectorBool() {}
	VectorBool(const VectorBool& src) {*this = src;}
	void operator=(const VectorBool& src);
	
	int GetCount() const;
	int PopCount() const;
	VectorBool& SetCount(int i, bool b=false);
	VectorBool& Reserve(int i);
	VectorBool& Zero();
	VectorBool& One();
	VectorBool& SetInverse(const VectorBool& b);
	VectorBool& InverseAnd(const VectorBool& b);
	VectorBool& And(const VectorBool& b);
	VectorBool& Or(const VectorBool& b);
	double GetOverlapFactor(const VectorBool& b) const;
	int Hamming(const VectorBool& b) const;
	int PopCountAnd(const VectorBool& b) const;
	int PopCountNotAnd(const VectorBool& b) const;
	bool IsEqual(const VectorBool& b) const;
	bool IsEmpty() const {return count == 0;}
	
	bool Top() const {return Get(GetCount()-1);}
	bool Get(int64 i) const;
	void Set(int64 i, bool b);
	void LimitLeft(int i);
	void LimitRight(int i);
	
	ConstU64* Begin() const;
	ConstU64* End() const;
	uint64*   Begin();
	uint64*   End();
	
	void Serialize(Stream& s) {s % data % count;}
};

typedef const VectorBool	ConstVectorBool;
typedef const Buffer		ConstBuffer;

struct Output : Moveable<Output> {
	Output() {}
	Vector<Buffer> buffers;
	int phase = 0, type = 0, visible = 0;
};

struct LabelSignal : Moveable<LabelSignal> {
	VectorBool signal, enabled;
	void Serialize(Stream& s) {s % signal % enabled;}
};

struct Label : Moveable<Label> {
	Label() {}
	Vector<LabelSignal> buffers;
	void Serialize(Stream& s) {s % buffers;}
};

typedef const LabelSignal ConstLabelSignal;
typedef const Label ConstLabel;













class BitStream {
	VectorBool data;
	int bit = 0;
	int cols = 0;
	
public:
	BitStream() {}
	BitStream(const BitStream& bs) {data = bs.data; bit = bs.bit; cols = bs.cols;}
	
	void Clear() {data.SetCount(0); bit = 0;}
	void SetCount(int i) {data.SetCount(i * cols);}
	void SetColumnCount(int i) {cols = i;}
	void SetBit(int i) {bit = i;}
	int GetColumnCount() const {return cols;}
	int GetBit() const {return bit;}
	
	bool Read() {return data.Get(bit++);}
	void Write(bool b) {data.Set(bit, b); bit++;}
	
};












struct ExtremumCache {
	Vector<double> max, min;
	double max_value = -DBL_MAX, min_value = DBL_MAX;
	int pos = -1, size = 0, max_left = 0, min_left = 0;
	
	ExtremumCache(int size=0) {
		SetSize(size);
	}
	
	void SetSize(int size) {
		this->size = size;
		max.SetCount(size, -DBL_MAX);
		min.SetCount(size, DBL_MAX);
	}
	
	void Serialize(Stream& s) {
		s % max % min % max_value % min_value % pos % size % max_left % min_left;
	}
	
	void Add(double low, double high) {
		pos++;
		int write_pos = pos % size;
		
		max_left--;
		max[write_pos] = high;
		if (max_left <= 0) {
			max_value = -DBL_MAX;
			for(int i = 0; i < size; i++) {
				double d = max[i];
				if (d > max_value) {
					if (i <= write_pos)	max_left = size - (write_pos - i);
					else				max_left = i - write_pos;
					max_value = d;
				}
			}
		}
		else if (high >= max_value) {
			max_left = size;
			max_value = high;
		}
		
		min_left--;
		min[write_pos] = low;
		if (min_left <= 0) {
			min_value = DBL_MAX;
			for(int i = 0; i < size; i++) {
				double d = min[i];
				if (d < min_value) {
					if (i <= write_pos)	min_left = size - (write_pos - i);
					else				min_left = i - write_pos;
					min_value = d;
				}
			}
		}
		else if (low <= min_value) {
			min_left = size;
			min_value = low;
		}
	}
	
	int GetHighest() {
		return pos - (size - max_left);
	}
	
	int GetLowest() {
		return pos - (size - min_left);
	}
};





}

#endif
