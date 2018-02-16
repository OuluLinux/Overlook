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
	int cost_level = 0;
	
protected:
	virtual void Start();
	
public:
	MinimalLabel();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
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
			% Out(0, 0)
			% Out(0, 0)
			% Arg("idx_limit", idx_limit, 0, 100)
			% Arg("cost_level", cost_level, 0);
	}
};

class ReactionContext : public Core {
	int length = 3;
	
public:
	ReactionContext();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("length", length, 1);
	}
	
	enum {UNKNOWN, UPTREND, DOWNTREND, HIGHBREAK, LOWBREAK, REVERSALUP, REVERSALDOWN, COUNT};
	
};

class VolatilityContext : public Core {
	VectorMap<int,int> median_map;
	Vector<double> volat_divs;
	int div = DEFAULT_DIV;
	
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
	
	static const int DEFAULT_DIV = 6;
	
};



class Obviousness : public Core {
	
	struct Snap : Moveable<Snap> {
		uint64 data[2] = {0,0};
		
		void Copy(uint64* src) {data[0] = src[0]; data[1] = src[1];}
		void Set(int bit, bool b) {int s = bit/64; if (b) data[s] |= 1 << (bit-s*64); else data[s] &= ~(1 << (bit-s*64));}
		bool Get(int bit) const {int s = bit/64; return data[s] & (1 << (bit-s*64));}
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(data, sizeof(data)); else  s.Put(data, sizeof(data));}
		int GetDistance(const Snap& s) const {return PopCount64(data[0]) + PopCount64(data[1]);}
		
	};
	struct Order : Moveable<Order> {
		bool label;
		int start, stop, len;
		double av_change, err;
		double av_idx, err_idx, len_idx;
		double idx, idx_norm;
		void Serialize(Stream& s) {s % label % start % stop % len % av_change % err % av_idx % err_idx % len_idx % idx % idx_norm;}
	};
	struct BitComboStat {
		int enabled_count = 0;
		int total_count = 0;
		int true_count = 0;
	};
	
	static const int buffer_count = 2;
	static const int max_bit_ids = 4;
	static const int max_bit_values = 2*2*2*2;
	struct BitMatcher : Moveable<BitMatcher> {
		uint8 bit_count = 0;
		uint8 bit_ids[max_bit_ids] = {0,0,0,0};
		BitComboStat bit_stats[max_bit_values];
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(BitMatcher)); else s.Put(this, sizeof(BitMatcher));}
	};
	typedef Vector<Snap> VectorSnap;
	
	const int period_count = 6;
	enum {ONLINEMINLAB, TRENDINDEX,VOLATCTX0, VOLATCTX1, VOLATCTX2, VOLATCTX3, VOLATCTX4, VOLATCTX5, MA, MOM, OTREND, HTREND, LTREND, RTRENDUP, RTRENDDOWN,INPUT_COUNT};
	
	enum {RT_SIGNAL, RT_ENABLED, rt_row_size};
	const int row_size = period_count * INPUT_COUNT;
	const int volat_div = 6;
	const double idx_limit_f = 0.75;
	
	VectorSnap data_in, data_out;
	Vector<OnlineAverageWindow1> av_wins;
	Vector<Vector<double> > volat_divs;
	Vector<VectorMap<int,int> > median_maps;
	Vector<BitMatcher> bitmatches;
	double max_res_enabled_treshold;
	double max_res_signal_treshold;
	
	void RefreshInput();
	void RefreshInitialOutput();
	void RefreshOutput();
	void RefreshIOStats();
	
	
public:
	typedef Obviousness CLASSNAME;
	
	Obviousness();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(buffer_count, buffer_count)
			% Out(0, 0)
			% Mem(data_in)
			% Mem(data_out)
			% Mem(av_wins)
			% Mem(volat_divs)
			% Mem(median_maps)
			% Mem(bitmatches)
			% Mem(max_res_enabled_treshold)
			% Mem(max_res_signal_treshold)
			;
	}
	
};


class VolatilityContextReversal : public Core {
	
public:
	typedef VolatilityContextReversal CLASSNAME;
	
	VolatilityContextReversal();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<VolatilityContext>()
			% Out(0, 0);
	}
	
};


class ObviousTarget : public Core {
	struct BitComboStat {
		int total_count = 0;
		double sum = 0.0;
	};
	
	const int row_size = 4;
	static const int max_bit_ids = 4;
	static const int max_bit_values = 2*2*2*2;
	struct BitMatcher : Moveable<BitMatcher> {
		uint8 bit_count = 0;
		uint8 bit_ids[max_bit_ids] = {0,0,0,0};
		BitComboStat bit_stats[max_bit_values];
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(BitMatcher)); else s.Put(this, sizeof(BitMatcher));}
	};
	Vector<BitMatcher> bitmatches;
	
	Vector<ConstVectorBool*> bufs;
	ConstVectorBool* outbuf = NULL;
	
	void RefreshIOStats();
	void RefreshTargetValues();
	void RefreshOutput();
	bool GetInput(int i, int j);
	double GetOutput(int i);
	
public:
	typedef ObviousTarget CLASSNAME;
	
	ObviousTarget();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<TrendIndex>()
			% In<Obviousness>()
			% In<OnlineMinimalLabel>()
			% In<VolatilityContextReversal>()
			% In<MinimalLabel>(&Args0)
			% Out(2, 1)
			% Mem(bitmatches);
	}
	
	static void Args0(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		decl.args[0] = 10;
		decl.arg_count = 1;
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








class ParserAdvisor : public Core {
	
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
	GeneticOptimizer optimizer;
	Vector<double> training_pts;
	int prev_counted = 0;
	
	static const int TI_TF_COUNT = 4;
	static const int TI_PERIOD_COUNT = 3;
	static const int ML_TF_COUNT = 1;
	static const int ML_COSTLEVEL_COUNT = 3;
	
	static const int TI_TOTAL = TI_TF_COUNT * TI_PERIOD_COUNT;
	static const int ML_TOTAL = ML_TF_COUNT * ML_COSTLEVEL_COUNT;
	
	// Temporary
	ConstBuffer *open_buf = NULL, *low_buf = NULL, *high_buf = NULL, *rc_buf = NULL, *vc_buf = NULL;
	ConstBuffer* ti_buf[TI_TOTAL];
	ConstVectorBool* ti_label_buf[TI_TOTAL];
	ConstBuffer* ml_buf[ML_TOTAL];
	ConstVectorBool* ml_label_buf[ML_TOTAL];
	double spread_point = 0.0;
	int core_cursor[TI_TF_COUNT];
	int reaction_ctx_count = ReactionContext::COUNT;
	int volat_ctx_count = VolatilityContext::DEFAULT_DIV;
	int start_trigger_count = 6;
	int stop_trigger_count = 6;
	int train_bars = 0;
	int cursor = 0;
	int reaction, volatility;
	int train_end = 0;
	int round = 0;
	int max_rounds = 1000;
	bool used_signal = false;
	bool is_open = false;
	bool once = true;
	
protected:
	virtual void Start();
	
	double RunTest(int begin, int end);
	int Command();
	int ContextReaction();
	int ContextVolatility();
	int TrendIndexTrigger();
	int MinimalLabelTrigger();
	int GetInputTf(int i) const {return i < 3 ? i : i + 1;}
	
public:
	typedef ParserAdvisor CLASSNAME;
	ParserAdvisor();
	
	virtual void Init();
	
	template <int I>
	static bool FilterFunction0(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1) {
			return out_tf == I;
		}
		
		return in_sym == out_sym;
	}
	
	template <int I>
	static void Args0(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		decl.args[0] = I;
		decl.arg_count = 1;
	}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<ReactionContext>()
			% In<VolatilityContext>()
			% Out(1, 1)
			% Out(0, 0)
			% Mem(optimizer)
			% Mem(training_pts)
			% Mem(prev_counted);
		
		static FilterFunction filt[TI_TF_COUNT];
		static ArgsFn ti_period[TI_PERIOD_COUNT];
		static ArgsFn ml_costlevel[ML_COSTLEVEL_COUNT];
		
		if (filt[0] == NULL) {
			filt[0] = &FilterFunction0<0>;
			filt[1] = &FilterFunction0<1>;
			filt[2] = &FilterFunction0<2>;
			filt[3] = &FilterFunction0<4>;
			
			ti_period[0] = &Args0<3>;
			ti_period[1] = &Args0<6>;
			ti_period[2] = & Args0<12>;
			
			ml_costlevel[0] = &Args0<0>;
			ml_costlevel[1] = &Args0<3>;
			ml_costlevel[2] = &Args0<6>;
		}
		
		for(int i = 0; i < TI_TF_COUNT; i++)
			for(int j = 0; j < TI_PERIOD_COUNT; j++)
				reg % In<TrendIndex>(filt[i], ti_period[j]);
			
		
		for(int i = 0; i < ML_TF_COUNT; i++)
			for(int j = 0; j < ML_COSTLEVEL_COUNT; j++)
				reg % In<OnlineMinimalLabel>(filt[i], ml_costlevel[j]);
	}
	
};

}

#endif
