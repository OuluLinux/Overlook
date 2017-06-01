#ifndef _Overlook_Common_h_
#define _Overlook_Common_h_

#include <PlotCtrl/PlotCtrl.h>
#include <Core/Core.h>

/*#include <ConvNet/ConvNet.h>
#include <NARX/NARX.h>
#include <Mona/Mona.h>

#include <ConvNetCtrl/ConvNetCtrl.h>
#include <GraphLib/GraphLib.h>*/

#undef ASSERTEXC

namespace Overlook {
using namespace Upp;

enum {SourcePhase, IndiPhase, ForecastPhase, ForecastCombPhase, AgentPhase, AgentCombPhase, DestPhase};

enum {
	TimeValue,
	RealValue,
	DataBridgeValue,
	VirtualNodeValue,
	RealChangeValue,
	
	// TRASH
	SpreadChangeValue,
	RealProxyChangeValue,
	SpreadProxyChangeValue,
	RealLowChangeValue,
	RealHighChangeValue,
	// TRASH END
	
	IdealOrderSignal,
	ForecastChangeValue,
	ForecastChannelValue,
	IndicatorValue,				// Any indicator value
	RealIndicatorValue,			// Indicator value in the scale of price value
	TimeOscillatorValue,
	ForecastOrderSignal,
	RealVolumeValue,
	AskBidValue,
	SpreadValue,
	SpreadOvercomeValue,
	SubTfValue,
	SpreadOvercomeDistValue,
	SpreadProbValue,
	FeatureValue,
	WeekStatsErrorAdjustedValue,
	MetaTfValue,
	WdayHourTrendingValue,
	FeatureOscValue,
	WdayHourStatsValue,
	CorrelationValue
};

enum {
	SymTf,
	Sym,
	Tf,
	All
};

struct ValueType : Moveable<ValueType> {
	ValueType() : phase(-1), type(-1), scale(-1) {}
	ValueType(const ValueType& v) : phase(v.phase), type(v.type), scale(v.scale) {}
	ValueType(int phase, int type, int scale) : phase(phase), type(type), scale(scale) {}
	bool operator==(const ValueType& vt) const {return phase == vt.phase && type == vt.type && scale == vt.scale;}
	String ToString() const {return Format("{phase=%d type=\"%s\", type-id=%d, scale=%d}", phase, TypeString(), type, scale);}
	String TypeString() const {
		switch (type) {
			case 0: return "RealValue";
			case 1: return "RealChangeValue";
			case 2: return "RealIndicatorValue";
			case 3: return "SpreadChangeValue";
			case 4: return "RealProxyChangeValue";
			case 5: return "SpreadProxyChangeValue";
			case 6: return "RealLowChangeValue";
			case 7: return "RealHighChangeValue";
			case 8: return "IdealOrderSignal";
			case 9: return "ForecastChangeValue";
			case 10: return "ForecastChannelValue";
			case 11: return "IndicatorValue";
			case 12: return "TimeOscillatorValue";
			case 13: return "ForecastOrderSignal";
			case 14: return "RealVolumeValue";
			case 15: return "AskBidValue";
			case 16: return "SpreadValue";
			case 17: return "SpreadOvercomeValue";
			case 18: return "SubTfValue";
			case 19: return "SpreadOvercomeDistValue";
			case 20: return "SpreadProbValue";
			case 21: return "FeatureValue";
			case 22: return "WeekStatsErrorAdjustedValue";
			case 23: return "MetaTfValue";
			case 24: return "WdayHourTrendingValue";
			case 25: return "FeatureOscValue";
			case 26: return "WdayHourStatsValue";
			case 27: return "CorrelationValue";
			default: return "Invalid type";
		}
	}
	
	int phase, type, scale;
};

struct DataExc : public Exc {
	DataExc() {
		#ifdef flagDEBUG
		Panic("debug DataExc");
		#endif
	}
	
	DataExc(String msg) : Exc(msg) {
		#ifdef flagDEBUG
		Panic("debug DataExc");
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

enum {SLOT_SYMTF, SLOT_SYM, SLOT_TF, SLOT_ONCE};

struct CoreProcessAttributes : Moveable<CoreProcessAttributes> {
	int sym_id;								// id of symbol
	int sym_count;							// count of symbol ids
	int tf_id;								// timeframe-id from shorter to longer
	int tf_count;							// count of timeframe ids
	int64 time;								// time of current position
	int pos[16];							// time-position for all timeframes
	int bars[16];							// count of time-positions for all timeframes
	int periods[16];						// periods for all timeframes
	int slot_bytes;							// reserved memory in bytes for current slot
	int slot_pos;							// position of the current slot in the sym/tf/pos vector
	Core* slot;								// pointer to the current slot
	
	int GetBars() const {return bars[tf_id];}
	int GetCounted() const {return pos[tf_id];}
	int GetPeriod() const {return periods[tf_id];}
};

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

}

#endif
