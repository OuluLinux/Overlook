#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

/*
	Inspect OsMA to see how SubCores and inputs with custom arguments compares.
	It has custom argument implementation, with SubCore implementation as commented.
*/

namespace Overlook {
using namespace Upp;

double SimpleMA ( const int position, const int period, ConstBuffer& value );
double ExponentialMA ( const int position, const int period, const double prev_value, ConstBuffer& value );
double SmoothedMA ( const int position, const int period, const double prev_value, ConstBuffer& value );
double LinearWeightedMA ( const int position, const int period, ConstBuffer& value );
int SimpleMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin, const int period, ConstBuffer& value, Buffer& buffer );
int ExponentialMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin, const int period, ConstBuffer& value, Buffer& buffer );
int LinearWeightedMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin, const int period, ConstBuffer& value, Buffer& buffer, int &weightsum );
int SmoothedMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin, const int period, ConstBuffer& value, Buffer& buffer );


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};


class MovingAverage : public Core {
	int ma_period;
	int ma_shift;
	int ma_method;
	int ma_counted;
	
protected:
	virtual void Start();
	
	void Simple();
	void Exponential();
	void Smoothed();
	void LinearlyWeighted();
	
public:
	MovingAverage();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", ma_period, 2)
			% Arg("offset", ma_shift, -10000)
			% Arg("method", ma_method, 0, 3);
	}
};


class MovingAverageConvergenceDivergence : public Core {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	
public:
	MovingAverageConvergenceDivergence();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Arg("fast_ema", fast_ema_period, 2, 127)
			% Arg("slow_ema", slow_ema_period, 2, 127)
			% Arg("signal_sma", signal_sma_period, 2, 127);
	}
};


class AverageDirectionalMovement : public Core {
	int period_adx;
	
public:
	AverageDirectionalMovement();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(6, 3)
			% Arg("period", period_adx, 2, 127);
	}
};


class BollingerBands : public Core {
	int           bands_period;
	int           bands_shift;
	double        bands_deviation;
	int           plot_begin;
	int           deviation;
	
	double StdDev_Func(int position, const Buffer& MAvalue, int period);
	
public:
	BollingerBands();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 3)
			% Arg("period", bands_period, 2, 127)
			% Arg("shift", bands_shift, 0, 0)
			% Arg("deviation", deviation, 2, 127);
	}
};


class Envelopes : public Core {
	int                ma_period;
	int                ma_shift;
	int                ma_method;
	double             deviation;
	int                dev;
	
public:
	Envelopes();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 2)
			% Arg("period", ma_period, 2, 127)
			% Arg("shift", ma_shift, 0, 0)
			% Arg("deviation", dev, 2, 127)
			% Arg("method", ma_method, 0, 3);
	}
};


class ParabolicSAR : public Core {
	double		sar_step;
	double		sar_maximum;
	int			last_rev_pos;
	int			step, maximum;
	bool		direction_long;
	
	double GetHigh( int pos, int start_period );
	double GetLow( int pos, int start_period );
	
public:
	ParabolicSAR();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("step", step, 2, 127)
			% Arg("maximum", maximum, 2, 127)
			% Mem(last_rev_pos)
			% Mem(direction_long);
	}
};


class StandardDeviation : public Core {
	int period;
	int ma_method;
	
public:
	StandardDeviation();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127)
			% Arg("ma_method", ma_method, 0, 3);
	}
};


class AverageTrueRange : public Core {
	int period;
	
public:
	AverageTrueRange();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 1)
			% Arg("period", period, 2, 127);
	}
};


class BearsPower : public Core {
	int period;
	
public:
	BearsPower();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127);
	}
};


class BullsPower : public Core {
	int period;
	
public:
	BullsPower();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127);
	}
};


class CommodityChannelIndex : public Core {
	int period;
	
public:
	CommodityChannelIndex();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("period", period, 2, 127);
	}
};


class DeMarker : public Core {
	int period;
	
public:
	DeMarker();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("period", period, 2, 127);
	}
};


class ForceIndex : public Core {
	int period;
	int ma_method;
	
public:
	ForceIndex();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127)
			% Arg("ma_method", ma_method, 0, 3);
	}
};


class Momentum : public Core {
	int period, shift;
	
public:
	Momentum();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2,2)
			% Arg("period", period, 2)
			% Arg("shift", shift, -10000);
	}
};


class OsMA : public Core {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	double value_mean;
	double diff_mean;
	int value_count;
	int diff_count;
	
public:
	OsMA();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<MovingAverage>(&Args) // fast
			% In<MovingAverage>(&Args) // slow
			% Out(4, 2)
			% Arg("fast_ema_period", fast_ema_period, 2)
			% Arg("slow_ema_period", slow_ema_period, 2)
			% Arg("signal_sma", signal_sma_period, 2)
			% Mem(value_mean) % Mem(value_count)
			% Mem(diff_mean) % Mem(diff_count);
	}
	
	static void Args(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		// Note: in case you need to fill some values in between some custom values,
		// default arguments can be read from here:
		// const FactoryRegister& reg = System::GetRegs()[decl.factory];
		// const ArgType& arg = reg.args[i];
		// int default_value = arg.def;
		
		int fast_ema_period =	args[0];
		int slow_ema_period =	args[1];
		int signal_sma =		args[2];
		if		(input == 1)	decl.AddArg(fast_ema_period);
		else if	(input == 2)	decl.AddArg(slow_ema_period);
		else Panic("Unexpected input");
		decl.AddArg(0);
		decl.AddArg(MODE_EMA);
	}
	
	void AddValue(double a) {
		if (value_count == 0) {
			value_mean = a;
		} else {
			double delta = a - value_mean;
			value_mean += delta / value_count;
		}
		value_count++;
	}
	void AddDiff(double a) {
		if (diff_count == 0) {
			diff_mean = a;
		} else {
			double delta = a - diff_mean;
			diff_mean += delta / diff_count;
		}
		diff_count++;
	}
};


class RelativeStrengthIndex : public Core {
	int period;
	
public:
	RelativeStrengthIndex();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 4)
			% Arg("period", period, 2);
	}
};


class RelativeVigorIndex : public Core {
	int period;
	
public:
	RelativeVigorIndex();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Arg("period", period, 2, 127);
	}
};


class StochasticOscillator : public Core {
	int k_period;
	int d_period;
	int slowing;
	
public:
	StochasticOscillator();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 2)
			% Arg("k_period", k_period, 2)
			% Arg("d_period", d_period, 2)
			% Arg("slowing", slowing, 2);
	}
};



class WilliamsPercentRange : public Core {
	int period;
	
	bool CompareDouble(double Number1, double Number2);
	
public:
	WilliamsPercentRange();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2,2)
			% Arg("period", period, 2);
	}
};


class AccumulationDistribution : public Core {
	
public:
	AccumulationDistribution();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1);
	}
};

class MoneyFlowIndex : public Core {
	int period;
	
public:
	MoneyFlowIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127);
	}
};

class ValueAndVolumeTrend : public Core {
	int applied_value;
	
public:
	ValueAndVolumeTrend();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("applied_value", applied_value, 0, 0);
	}
};

class OnBalanceVolume : public Core {
	int applied_value;
	
public:
	OnBalanceVolume();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("applied_value", applied_value, 0, 0);
	}
};


class Volumes : public Core {
	
public:
	Volumes();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1);
	}
};


class AcceleratorOscillator : public Core {
	
public:
	AcceleratorOscillator();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(5, 3);
	}
};


class GatorOscillator : public Core {
	int jaws_period;
	int jaws_shift;
	int teeth_period;
	int teeth_shift;
	int lips_period;
	int lips_shift;
	int ma_method;
	int applied_value;
	
public:
	GatorOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(6, 6)
			% Arg("jaws_period", jaws_period, 2, 127)
			% Arg("teeth_period", teeth_period, 2, 127)
			% Arg("lips_period", lips_period, 2, 127)
			% Arg("jaws_shift", jaws_shift, 2, 127)
			% Arg("teeth_shift", teeth_shift, 2, 127)
			% Arg("lips_shift", lips_shift, 2, 127)
			% Arg("ma_method", ma_method, 0, 3)
			% Arg("applied_value", applied_value, 0, 0);
	}
};


class AwesomeOscillator : public Core {
	
public:
	AwesomeOscillator();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 3);
	}
};


class Fractals : public Core {
	int left_bars;
	int right_bars;
	
	double IsFractalUp(int index, int left, int right, int maxind);
	double IsFractalDown(int index, int left, int right, int maxind);
	
public:
	Fractals();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(6, 6)
			% Arg("left_bars", left_bars, 2, 20)
			% Arg("right_bars", right_bars, 0, 0);
	}
};


class FractalOsc : public Core {
	int left_bars;
	int right_bars;
	int smoothing_period;
	
public:
	FractalOsc();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Arg("left_bars", left_bars, 2, 20)
			% Arg("right_bars", right_bars, 0, 0)
			% Arg("smoothing", smoothing_period, 2, 127);
	}
};


class MarketFacilitationIndex : public Core {
	
public:
	MarketFacilitationIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(5, 4);
	}
};


class ZigZag : public Core {
	int input_depth;
	int input_deviation;
	int input_backstep;
	int extremum_level;
	
protected:
	virtual void Start();
	
public:
	ZigZag();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 2)
			% Arg("depth", input_depth, 1)
			% Arg("deviation", input_depth, 1)
			% Arg("backstep", input_backstep, 1)
			% Arg("level", extremum_level, 2);
	}
};

class ZigZagOsc : public Core {
	int depth;
	int deviation;
	int backstep;
	
public:
	ZigZagOsc();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("depth", depth, 2, 16)
			% Arg("deviation", deviation, 2, 16)
			% Arg("backstep", backstep, 2, 16);
	}
};




class LinearTimeFrames : public Core {
	
public:
	LinearTimeFrames();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 4);
	}
};




class LinearWeekTime : public Core {
	
public:
	LinearWeekTime();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1);
	}
};




class SupportResistance : public Core {
	int period, max_crosses, max_radius;
	
public:
	SupportResistance();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Arg("period", period, 300, 300)
			% Arg("max_crosses", max_crosses, 100, 100)
			% Arg("max_radius", max_radius, 100, 100);
	}
};


class SupportResistanceOscillator : public Core {
	int period, max_crosses, max_radius, smoothing_period;
	
public:
	SupportResistanceOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2,2)
			% Arg("period", period, 2, 127)
			% Arg("max_crosses", max_crosses, 300, 300)
			% Arg("max_radius", max_radius, 100, 100)
			% Arg("smoothing", smoothing_period, 100, 100);
	}
};


class ChannelOscillator : public Core {
	int period;
	ExtremumCache ec;
	
public:
	ChannelOscillator();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2)
			% Mem(ec);
	}
};


class ScissorChannelOscillator : public Core {
	int period;
	ExtremumCache ec;
	
public:
	ScissorChannelOscillator();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2)
			% Mem(ec);
	}
};


class Psychological : public Core {
	int period;
	
public:
	Psychological();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127);
	}
};


struct Average : Moveable<Average> {
	double mean;
	int count;
	Average() : mean(0.0), count(0) {}
	void Add(double v) {
		double delta = v - mean;
		mean += delta / count;
		count++;
	}
};

class CorrelationOscillator : public Core {
	
protected:
	typedef const Vector<double> ConstVector;
	
	int period;
	ConstBuffer* this_open;
	Vector<int> sym_ids;
	Vector<ConstBuffer*> opens;
	Vector<OnlineAverage2> averages;
	
	void Process(int id, int output);
	
public:
	CorrelationOscillator();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(SYM_COUNT-1, SYM_COUNT-1)
			% Arg("period", period, 2, 16);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		int sym_prio = ::Overlook::GetSystem().GetSymbolPriority(out_sym);
		return (sym_prio >= 0 && sym_prio < SYM_COUNT) || out_sym == in_sym;
	}
};



/*
	TrendChange takes two box-averages, one symmetric (peeks future) and one moving average.
	The it takes the change of average values and multiplies them:
	 >= 0 both averages are going same direction
	  < 0 the symmetric is changing direction, but lagging moving average is not yet
	This data can be statistically evaluated, and periodical time of change of direction can
	be estimated.
*/
class TrendChange : public Core {
	int period;
	int method;
	
protected:
	virtual void Start();
	
public:
	TrendChange();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 16)
			% Arg("method", method, 0, 3);
	}
};

/*
	TrendChangeEdge does the edge filtering to the TrendChange.
	The edge filter for data is the same that for images.
	Positive peak values are when trend is changing.
*/
class TrendChangeEdge : public Core {
	int period;
	int method;
	int slowing;
	
protected:
	virtual void Start();
	
public:
	TrendChangeEdge();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("period", period, 2, 16)
			% Arg("method", method, 0, 3)
			% Arg("slowing", slowing, 2, 127);
	}
};



class PeriodicalChange : public Core {
	Vector<double> means;
	Vector<int> counts;
	int split_type, tfmin;
	
protected:
	virtual void Start();
	
	void Add(int i, double d) {
		double& mean = means[i];
		int& count = counts[i];
		if (!count) {
			mean = d;
			count = 1;
		} else {
			double delta = d - mean;
			mean += delta / count;
			count++;
		}
	}
	double Get(int i) {return means[i];}
	
public:
	PeriodicalChange();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Mem(means)
			% Mem(counts);
	}
};



class VolatilityAverage : public Core {
	int period;
	VectorMap<int, int> stats;
	Vector<double> stats_limit;
	
protected:
	virtual void Start();
	
public:
	VolatilityAverage();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2)
			% Mem(stats);
	}
};


class MinimalLabel : public Core {
	int prev_counted = 0;
	
	
protected:
	virtual void Start();
	
public:
	MinimalLabel();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Mem(prev_counted);
	}
};


class StrongForce : public Core {
	double max_diff = 0.0;
	
public:
	StrongForce();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(1, 1)
			% Mem(max_diff);
	}
	
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		
		if (out_sym == in_sym) return true;
		if (out_sym == ::Overlook::GetSystem().GetStrongSymbol()) return true;
		
		return false;
	}
};

class VolatilitySlots : public Core {
	
protected:
	friend class WeekSlotAdvisor;
	Vector<OnlineAverage1> stats;
	OnlineAverage1 total;
	
	int slot_count = 0;
	
	
public:
	VolatilitySlots();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Mem(stats)
			% Mem(total);
	}
	
};


class VolumeSlots : public Core {
	
protected:
	Vector<OnlineAverage1> stats;
	OnlineAverage1 total;
	
	int slot_count = 0;
	
	
public:
	VolumeSlots();
	
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Mem(stats)
			% Mem(total);
	}
	
};





}

#endif
