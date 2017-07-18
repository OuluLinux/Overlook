#ifndef _Overlook_Common_h_
#define _Overlook_Common_h_

#include <Core/Core.h>
#include <PlotCtrl/PlotCtrl.h>
#include <ConvNet/ConvNet.h>
#include <ConvNetCtrl/ConvNetCtrl.h>
#include <CoreUtils/Optimizer.h>

#undef ASSERTEXC

namespace Overlook {
using namespace Upp;

struct OnlineAverage2 : Moveable<OnlineAverage2> {
	double mean_a, mean_b;
	int count;
	OnlineAverage2() : mean_a(0), mean_b(0), count(0) {}
	void Add(double a, double b) {
		if (count == 0) {
			mean_a = a;
			mean_b = b;
		} else {
			double delta_a = a - mean_a; mean_a += delta_a / count;
			double delta_b = b - mean_b; mean_b += delta_b / count;
		}
		count++;
	}
	void Serialize(Stream& s) {s % mean_a % mean_b % count;}
};

struct OnlineAverage1 : Moveable<OnlineAverage1> {
	double mean;
	int count;
	OnlineAverage1() : mean(0), count(0) {}
	void Add(double a) {
		if (count == 0) {
			mean = a;
		} else {
			double delta = a - mean;
			mean += delta / count;
		}
		count++;
	}
	void Serialize(Stream& s) {s % mean % count;}
};

struct ValueBase {
	int count, visible, data_type, min, max, factory;
	const char* s0;
	void* data;
	ValueBase() {count=0; visible=0; s0=0; data=0; data_type = -1; min = -1; max = -1; factory = -1;}
	enum {IN_, INOPT_, OUT_, BOOL_, INT_, PERS_BOOL_, PERS_INT_, PERS_DOUBLE_, PERS_INTVEC_, PERS_DBLVEC_, PERS_INTMAP_, PERS_QUERYTABLE_, PERS_BYTEGRID_, PERS_INTGRID_};
};

struct ValueRegister {
	ValueRegister() {}
	
	virtual void IO(const ValueBase& base) = 0;
	virtual ValueRegister& operator % (const ValueBase& base) {IO(base); return *this;}
};

struct DataExc : public Exc {
	DataExc() {
		#ifdef flagDEBUG
		Panic("debug DataExc");
		#endif
	}
	
	DataExc(String msg) : Exc(msg) {
		#ifdef flagDEBUG
		Panic(msg);
		#endif
	}
};
#define ASSERTEXC(x) if (!(x)) throw ::Overlook::DataExc(#x);
#define ASSERTEXC_(x, msg) if (!(x)) throw ::Overlook::DataExc(msg);

// Helper macros for indicator short names
#define SHORTNAME0(x) x
#define SHORTNAME1(x, a1) x "(" + DblStr(a1) + ")"
#define SHORTNAME2(x, a1, a2) x "(" + DblStr(a1) + "," + DblStr(a2) + ")"
#define SHORTNAME3(x, a1, a2, a3) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + ")"
#define SHORTNAME4(x, a1, a2, a3, a4) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + ")"
#define SHORTNAME5(x, a1, a2, a3, a4, a5) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + ")"
#define SHORTNAME6(x, a1, a2, a3, a4, a5, a6) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) +  ")"
#define SHORTNAME7(x, a1, a2, a3, a4, a5, a6, a7) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) + "," + DblStr(a7) + ")"
#define SHORTNAME8(x, a1, a2, a3, a4, a5, a6, a7, a8) x "(" + DblStr(a1) + "," + DblStr(a2) + "," + DblStr(a3) + "," + DblStr(a4) + "," + DblStr(a5) + "," + DblStr(a6) + "," + DblStr(a7) + "," + DblStr(a8) + ")"

typedef Vector<byte> CoreData;

class Core;

struct BatchPartStatus : Moveable<BatchPartStatus> {
	BatchPartStatus() {slot = NULL; begin = Time(1970,1,1); end = begin; sym_id = -1; tf_id = -1; actual = 0; total = 1; complete = false; batch_slot = 0;}
	Core* slot;
	Time begin, end;
	int sym_id, tf_id, actual, total;
	byte batch_slot;
	bool complete;
	
	void Serialize(Stream& s) {
		s % begin % end % sym_id % tf_id % actual % total % batch_slot % complete;
	}
};

template <class T>
String HexVector(const Vector<T>& vec) {
	String s;
	int byts = sizeof(T);
	int chrs = sizeof(T) * 2;
	typedef const T ConstT;
	ConstT* o = vec.Begin();
	for(int i = 0; i < vec.GetCount(); i++) {
		T mask = 0xFF << ((byts-1) * 8);
		for(int j = 0; j < byts; j++) {
			byte b = (mask & *o) >> ((byts-1-j) * 8);
			int d0 = b >> 4;
			int d1 = b & 0xF;
			s.Cat( d0 < 10 ? '0' + d0 : 'A' + d0);
			s.Cat( d1 < 10 ? '0' + d1 : 'A' + d1);
			mask >>= 8;
		}
		o++;
	}
	return s;
}

inline Color Silver() {return Color(192, 192, 192);}
inline Color Wheat() {return Color(245, 222, 179);}
inline Color LightSeaGreen() {return Color(32, 178, 170);}
inline Color YellowGreen() {return Color(154, 205, 50);}
inline Color Lime() {return Color(0, 255, 0);}
inline Color DodgerBlue() {return Color(30, 144, 255);}
inline Color SaddleBrown() {return Color(139, 69, 19);}
inline Color Pink() {return Color(255, 192, 203);}
inline Color RainbowColor(double progress) {
    int div = progress * 6;
    int ascending = (int) (progress * 255);
    int descending = 255 - ascending;

    switch (div)
    {
        case 0:
            return Color(255, ascending, 0);
        case 1:
            return Color(descending, 255, 0);
        case 2:
            return Color(0, 255, ascending);
        case 3:
            return Color(0, descending, 255);
        case 4:
            return Color(ascending, 0, 255);
        default: // case 5:
            return Color(255, 0, descending);
    }
}

inline int IncreaseMonthTS(int ts) {
	Time t(1970,1,1);
	int64 epoch = t.Get();
	t += ts;
	int year = t.year;
	int month = t.month;
	month++;
	if (month == 13) {year++; month=1;}
	return Time(year,month,1).Get() - epoch;
}

class CoreIO;

// Class for default visual settings for a single visible line of an indicator
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
	
	int GetResetEarliestWrite() {int i = earliest_write; earliest_write = INT_MAX; return i;}
	int GetCount() const {return value.GetCount();}
	bool IsEmpty() const {return value.IsEmpty();}
	double GetUnsafe(int i) const {return value[i];}
	
	
	// Some utility functions for checking that indicator values are strictly L-R
	#ifdef flagDEBUG
	CoreIO* check_cio;
	void SafetyCheck(CoreIO* io) {check_cio = io;}
	double Get(int i) const;
	void Set(int i, double value);
	void Inc(int i, double value);
	#else
	double Get(int i) const {return value[i];}
	void Set(int i, double value) {this->value[i] = value; if (i < earliest_write) earliest_write = i;}
	void Inc(int i, double value) {this->value[i] += value;}
	#endif
	
	
};

typedef const Buffer ConstBuffer;

struct Output : Moveable<Output> {
	Output() : visible(0) {}
	Vector<Buffer> buffers;
	int phase, type, visible;
};

class Core;
class System;
class CoreItem;

class Source : Moveable<Source> {
	
public:
	Source() : core(NULL), output(NULL), sym(-1), tf(-1) {}
	Source(CoreIO* c, Output* out, int s, int t) : core(c), output(out), sym(s), tf(t) {}
	Source(const Source& src) {*this = src;}
	void operator=(const Source& src) {
		core = src.core;
		output = src.output;
		sym = src.sym;
		tf = src.tf;
	}
	
	CoreIO* core;
	Output* output;
	int sym, tf;
};

class SourceDef : Moveable<SourceDef> {
	
public:
	SourceDef() : coreitem(NULL), output(-1), sym(-1), tf(-1) {}
	SourceDef(CoreItem* ci, int out, int s, int t) : coreitem(ci), output(out), sym(s), tf(t) {}
	SourceDef(const Source& src) {*this = src;}
	void operator=(const SourceDef& src) {
		coreitem = src.coreitem;
		output = src.output;
		sym = src.sym;
		tf = src.tf;
	}
	
	CoreItem* coreitem;
	int output, sym, tf;
};

typedef VectorMap<int, Source>		Input;
typedef VectorMap<int, SourceDef>	InputDef;
typedef Tuple2<int, int>			FactoryHash;

class CoreItem : Moveable<CoreItem>, public Pte<CoreItem> {
	
public:
	One<Core> core;
	int sym, tf, priority, factory, hash;
	Vector<VectorMap<int, SourceDef> > inputs;
	Vector<int> args;
	Vector<FactoryHash> input_hashes;
	
public:
	typedef CoreItem CLASSNAME;
	CoreItem() {sym = -1; tf = -1; priority = INT_MAX; factory = -1; hash = -1;}
	~CoreItem() {}
	void operator=(const CoreItem& ci) {Panic("TODO");}
	void SetInput(int input_id, int sym_id, int tf_id, CoreItem& src, int output_id);
	
};

}

#endif
