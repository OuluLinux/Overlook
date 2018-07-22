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


class Normalized : public Core {
	int ma_period = 5;
	OnlineVariance var;
	
protected:
	virtual void Start();
	
	
public:
	Normalized();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 2)
			% Mem(var);
	}
};


class HurstWindow : public Core {
	int period = 10;
	OnlineAverageWindow1 avwin;
	
	static constexpr double koef = 1.253314;
	
protected:
	virtual void Start();
	
	
public:
	HurstWindow();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 1);
	}
};



class SimpleHurstWindow : public Core {
	int period = 10;
	
	static constexpr double koef = 1.253314;
	
protected:
	virtual void Start();
	
	
public:
	SimpleHurstWindow();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 1);
	}
};

double GetSimpleHurst(ConstBuffer& openbuf, int i, int period);


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
			% Lbl(1)
			% Arg("period", ma_period, 2)
			% Arg("offset", ma_shift, -10000)
			% Arg("method", ma_method, 0, 3);
	}
};


class MovingAverageConvergenceDivergence : public Core {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	int method = 0;
	
public:
	MovingAverageConvergenceDivergence();
	
	virtual void Init();
	virtual void Start();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Lbl(4)
			% Arg("fast_ema", fast_ema_period, 2, 127)
			% Arg("slow_ema", slow_ema_period, 2, 127)
			% Arg("signal_sma", signal_sma_period, 2, 127)
			% Arg("method", method, 0, 3);
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
			% Lbl(1)
			% Arg("period", bands_period, 2, 10000)
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


class Channel : public Core {
	int                period = 30;
	ExtremumCache		ec;
public:
	Channel();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(2, 2)
			% Arg("period", period, 2, 10000)
			% Mem(ec);
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Out(1, 1)
			% Lbl(1)
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
			% Out(4, 2)
			% Arg("fast_ema_period", fast_ema_period, 2)
			% Arg("slow_ema_period", slow_ema_period, 2)
			% Arg("signal_sma", signal_sma_period, 2)
			% Mem(value_mean) % Mem(value_count)
			% Mem(diff_mean) % Mem(diff_count);
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
			% Out(3, 1)
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Out(1,1)
			% Lbl(1)
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
			% Lbl(1)
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
			% Out(5, 3)
			% Lbl(1);
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
			% Out(3, 3)
			% Lbl(1);
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
			% Lbl(1)
			% Arg("left_bars", left_bars, 2, 200)
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
			% Lbl(1)
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
			% Arg("period", period, 2, 3000)
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
			% Lbl(1)
			% Arg("period", period, 2, 1000)
			% Arg("max_crosses", max_crosses, 100, 100)
			% Arg("max_radius", max_radius, 100, 100)
			% Arg("smoothing", smoothing_period, 2, 1000);
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
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
			% Lbl(1)
			% Arg("period", period, 2)
			% Mem(stats);
	}
};


class MinimalLabel : public Core {
	int prev_counted = 0;
	int cost_level = 0;
	
protected:
	virtual void Start();
	
public:
	MinimalLabel();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Mem(prev_counted)
			% Arg("cost_level", cost_level, 0);
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



class TrendIndex : public Core {
	int period = 6;
	int err_div = 3;
public:
	TrendIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 3)
			% Lbl(1)
			% Arg("period", period, 2)
			% Arg("err_div", err_div, 0);
	}
	
	static void Process(ConstBuffer& open_buf, int i, int period, int err_div, double& err, double& buf_value, double& av_change, bool& bit_value);
	
};

class OnlineMinimalLabel : public Core {
	int prev_counted = 0;
	int cost_level = 0;
	
	
protected:
	virtual void Start();
	
public:
	OnlineMinimalLabel();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(0, 0)
			% Lbl(1)
			% Mem(prev_counted)
			% Arg("cost_level", cost_level, 0);
	}
	
	static void GetMinimalSignal(double cost, ConstBuffer& open_buf, int begin, int end, bool* sigbuf, int sigbuf_size);
	
};

class SelectiveMinimalLabel : public Core {
	struct Order : Moveable<Order> {
		bool label;
		int start, stop, len;
		double av_change, err;
		double av_idx, err_idx, len_idx;
		double idx, idx_norm;
		void Serialize(Stream& s) {s % label % start % stop % len % av_change % err % av_idx % err_idx % len_idx % idx % idx_norm;}
	};
	
	
	int idx_limit = 75;
	int cost_level = 0;
	
protected:
	virtual void Start();
	
public:
	SelectiveMinimalLabel();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("idx_limit", idx_limit, 0, 100)
			% Arg("cost_level", cost_level, 0);
	}
};

class VolatilityContext : public Core {
	VectorMap<int,int> median_map;
	Vector<double> volat_divs;
	int div = 4;
	
public:
	VolatilityContext();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("div", div, 1)
			% Mem(median_map);
	}
	
};

class ChannelContext : public Core {
	VectorMap<int,int> median_map;
	Vector<double> volat_divs;
	ExtremumCache channel;
	int div = DEFAULT_DIV;
	int useable_div = DEFAULT_DIV * 2 / 3;
	int period = 30;
	
public:
	ChannelContext();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 1)
			% Arg("div", div, 1)
			% Arg("useable_div", useable_div, 1)
			% Mem(median_map)
			% Mem(channel);
	}
	
	static const int DEFAULT_DIV = 6;
	
};



class GridSignal : public Core {
	int main_interval = 1;
	int grid_interval = 60;
	
	int trend = 0;
	double line_value = 0;
	
public:
	GridSignal();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("main_interval", main_interval, 1)
			% Arg("grid_interval", grid_interval, 1)
			% Mem(trend) % Mem(line_value);
	}
	
	static const int DEFAULT_DIV = 6;
	
};




class Anomaly : public Core {
	Vector<OnlineVariance> var;
	int split_type, tfmin;
	
protected:
	virtual void Start();
	
	
public:
	Anomaly();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Mem(var);
	}
};



class VariantDifference : public Core {
	int period = 1, multiplier = 10;
	
protected:
	virtual void Start();
	
	
public:
	VariantDifference();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Out(1, 1)
			% Arg("period", period, 1)
			% Arg("multiplier", multiplier, 1)
			% Lbl(1);
	}
	
	static bool FilterFunction(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (match_tf)
			return in_tf == out_tf;
		else if (in_sym == out_sym)
			return true;
		
		System& sys = GetSystem();
		const VariantList& vl = sys.GetVariants(in_sym);
		
		return vl.dependencies.Find(out_sym) != -1;
	}
};



class ScalperSignal : public Core {
	
protected:
	virtual void Start();
	
	
public:
	ScalperSignal();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Lbl(1);
	}
};




class EasierScalperSignal : public Core {
	
protected:
	virtual void Start();
	
	
public:
	EasierScalperSignal();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Lbl(1);
	}
};






class PulseIndicator : public Core {
	int min_simultaneous = 9;
	
protected:
	virtual void Start();
	
	
public:
	PulseIndicator();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction)
			% Lbl(1)
			% Arg("Min simultaneous", min_simultaneous, 1);
	}
	
	static const int symcount = 10;
	static const char* symlist[10];
	
	static bool FilterFunction(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (match_tf)
			return in_tf == out_tf;
		else if (in_sym == out_sym)
			return true;
		
		System& sys = GetSystem();
		String sym = sys.GetSymbol(out_sym);
		for(int i = 0; i < symcount; i++) {
			if (sym == symlist[i])
				return true;
		}
		return false;
	}
};




class SimpleHeatmap : public Core {
	
protected:
	virtual void Start() {}
	
public:
	
	virtual void Init() {SetHeatmap();}
	virtual bool IsHeatmap() {return true;}
	virtual double GetHeatmapValue(int i, double price) {
		ConstBuffer& open_buf = GetInputBuffer(0, 0);
		double point = dynamic_cast<DataBridge&>(*GetInputCore(0)).GetPoint();
		double o0 = open_buf.Get(i);
		double diff = price - o0;
		return min(1.0, max(-1.0, diff / point / 4));
	}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>();
	}
};



class Avoidance : public Core {
	
protected:
	friend class ExpertAdvisor;
	
	typedef Tuple<int, int, ConstLabelSignal*> SrcPtr;
	
	struct DataPt : Moveable<DataPt> {
		char pips;
		byte len;
		bool action;
		
		void Serialize(Stream& s) {s % pips % len % action;}
	};
	
	// Persistent
	Vector<Vector<DataPt> > data;
	
	// Temporary
	VectorMap<int, Vector<SrcPtr> > sources;
	
	virtual void Start();
	
	
public:
	Avoidance();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args)
			% In<MovingAverage>(&FilterFunction, &Args) // 7
			% In<MovingAverageConvergenceDivergence>(&FilterFunction, &Args)
			% In<MovingAverageConvergenceDivergence>(&FilterFunction, &Args)
			% In<MovingAverageConvergenceDivergence>(&FilterFunction, &Args)
			% In<ParabolicSAR>(&FilterFunction, &Args)
			% In<ParabolicSAR>(&FilterFunction, &Args)
			% In<ParabolicSAR>(&FilterFunction, &Args)
			% In<StochasticOscillator>(&FilterFunction, &Args)
			% In<StochasticOscillator>(&FilterFunction, &Args)
			% In<StochasticOscillator>(&FilterFunction, &Args)
			% Mem(data)
			% Out(2,2)
			% Lbl(1)
		;
	}
	
	static bool FilterFunction(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (match_tf)
			return in_tf == out_tf;
		System& sys = GetSystem();
		String out_symstr = sys.GetSymbol(out_sym);
		#if 0
		String out_A = out_symstr.Left(3);
		String out_B = out_symstr.Right(3);
		String in_symstr = sys.GetSymbol(in_sym);
		String in_A = in_symstr.Left(3);
		String in_B = in_symstr.Right(3);
		if (in_symstr.GetCount() != 6 || out_symstr.GetCount() != 6) return false;
		if (out_A == in_A || out_B == in_A || out_A == in_B || out_B == in_B) return true;
		return false;
		#else
		return out_symstr == "EURUSD" || out_symstr == "GBPUSD" || out_symstr == "USDJPY" || out_symstr == "EURGBP" || out_symstr == "EURJPY";
		#endif
	}
	
	static void Args(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		switch (input) {
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:		decl.AddArg(2 << (input - 1));		break;
			case 8:		decl.AddArg(12); decl.AddArg( 26);	break;
			case 9:		decl.AddArg(24); decl.AddArg( 52);	break;
			case 10:	decl.AddArg(48); decl.AddArg(104);	break;
			case 11:	decl.AddArg(5); decl.AddArg(5);	break;
			case 12:	decl.AddArg(10); decl.AddArg(10);	break;
			case 13:	decl.AddArg(20); decl.AddArg(20);	break;
			case 14:
			case 15:
			case 16:	decl.AddArg(2 << (input - 14 + 3));	break;
			default: Panic("Invalid usage");
		}
	}
};








class AvoidancePeaks : public Core {
	
protected:
	virtual void Start();
	
	ExtremumCache ec;
	int peak_period = 100;
	
	bool GetType(int pos, ConstBuffer& open_buf, ConstBuffer& avoid_buf);
	
public:
	AvoidancePeaks();
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<Avoidance>()
			% Lbl(4)
			% Arg("Peak period", peak_period, 2, 1000)
			;
	}
};








class Laguerre : public Core {
	
protected:
	virtual void Start();
	
	int gamma = 60;
	double gd_88 = 0.0;
	double gd_96 = 0.0;
	double gd_104 = 0.0;
	double gd_112 = 0.0;
	double gd_120 = 0.0;
	double gd_128 = 0.0;
	double gd_136 = 0.0;
	double gd_144 = 0.0;
	double gd_152 = 0.0;
	double gd_160 = 0.0;
	double gd_168 = 0.0;
	
public:
	Laguerre();
	
	virtual void Init();
	virtual void Assist(int cursor, VectorBool& vec);
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Lbl(1)
			% Arg("gamma", gamma, 0, 100)
			% Mem(gd_88)
			% Mem(gd_96)
			% Mem(gd_104)
			% Mem(gd_112)
			% Mem(gd_120)
			% Mem(gd_128)
			% Mem(gd_136)
			% Mem(gd_144)
			% Mem(gd_152)
			% Mem(gd_160)
			% Mem(gd_168)
			;
	}
};











class QuantitativeQualitativeEstimation : public Core {
	
protected:
	virtual void Start();
	
	int period = 10;
	
public:
	QuantitativeQualitativeEstimation();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 2)
			% Lbl(1)
			% Arg("period", period, 0, 1000)
			;
	}
};











struct CalendarStat : Moveable<CalendarStat> {
	Index<int> pos, todo_stats;
	OnlineAverage1 av_diff, pos_ac, eq_ac, neg_ac;
	
	void Serialize(Stream& s) {s % pos % todo_stats % av_diff % pos_ac % eq_ac % neg_ac;}
};

class Calendar : public Core {
	VectorMap<String, CalendarStat> stats;
	int pre_cal_cursor = 0;
	int main_cal_cursor = 0;
	
	
protected:
	virtual void Start();
	
	
public:
	Calendar();
	
	virtual void Init();
	
	double GetTopDiff();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Lbl(4)
			% Mem(stats)
			% Mem(pre_cal_cursor)
			% Mem(main_cal_cursor)
			;
	}
};








class ExampleAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	
	
	// Persistent
	Vector<double> training_pts;
	int prev_counted = 0;
	
	// Temporary
	int round = 0;
	int max_rounds = 1000;
	bool once = true;
	
protected:
	virtual void Start();
	
public:
	typedef ExampleAdvisor CLASSNAME;
	ExampleAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Mem(training_pts)
			% Mem(prev_counted);
	}
};




}

#endif
