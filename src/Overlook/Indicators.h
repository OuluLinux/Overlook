#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

namespace Overlook {
using namespace Upp;

enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};



class DataSource {
	
public:
	DataSource();
	
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .Out(5, 3, 1);
	}
};


class ValueChange {
	
	OnlineAverage1 change_av;
	
public:
	typedef ValueChange CLASSNAME;
	ValueChange();
	
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(3, 2, 1)
			.Mem(change_av);
	}
	
};

class MovingAverage {
	int ma_period;
	int ma_shift;
	int ma_method;
	
protected:
	
	void Simple(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Exponential(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Smoothed(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void LinearlyWeighted(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
public:
	MovingAverage();
	
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 3)
			.Arg(ma_period, 2)
			.Arg(ma_shift, -10000)
			.Arg(ma_method, 0, 3);
	}
};


class MovingAverageConvergenceDivergence {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	
public:
	MovingAverageConvergenceDivergence();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 12)
			.In(FACTORY_MovingAverage, 26)
			.Out(2, 2, 4)
			.Arg(fast_ema_period, 2, 127)
			.Arg(slow_ema_period, 2, 127)
			.Arg(signal_sma_period, 2, 127);
	}
};


class AverageDirectionalMovement {
	int period_adx;
	
public:
	AverageDirectionalMovement();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(6, 3, 2)
			.Arg(period_adx, 2, 127);
	}
};


class BollingerBands {
	int           bands_period;
	int           bands_shift;
	double        bands_deviation;
	int           plot_begin;
	int           deviation;
	
	
public:
	BollingerBands();
	
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(4, 3, 4)
			.Arg(bands_period, 2, 127)
			.Arg(bands_shift, 0, 0)
			.Arg(deviation, 2, 127);
	}
};


class Envelopes {
	double             deviation;
	int                dev;
	
public:
	Envelopes();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 14, 0, MODE_SMA)
			.Out(3, 2, 0)
			.Arg(dev, 2, 127);
	}
};


class ParabolicSAR {
	double		sar_step;
	double		sar_maximum;
	int			last_rev_pos;
	int			step, maximum;
	bool		direction_long;
	
	double GetHigh( int pos, int start_period, SourceImage& si, ChartImage& ci, GraphImage& gi );
	double GetLow( int pos, int start_period, SourceImage& si, ChartImage& ci, GraphImage& gi );
	
public:
	ParabolicSAR();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(3, 1, 1)
			.Arg(step, 2, 127)
			.Arg(maximum, 2, 127)
			.Mem(last_rev_pos)
			.Mem(direction_long);
	}
};


class StandardDeviation {
	int period;
	int ma_method;
	
public:
	StandardDeviation();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 20, 0, 0)
			.Out(1, 1, 1)
			.Arg(period, 2, 127)
			.Arg(ma_method, 0, 3);
	}
};


class AverageTrueRange {
	int period;
	
public:
	AverageTrueRange();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(2, 1, 1)
			.Arg(period, 2, 127);
	}
};


class BearsPower {
	int period;
	
public:
	BearsPower();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 13, 0, 0)
			.Out(1, 1, 2)
			.Arg(period, 2, 127);
	}
};


class BullsPower {
	int period;
	
public:
	BullsPower();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 13, 0, 0)
			.Out(1, 1, 2)
			.Arg(period, 2, 127);
	}
};


class CommodityChannelIndex {
	int period;
	
public:
	CommodityChannelIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(3, 1, 4)
			.Arg(period, 2, 127);
	}
};


class DeMarker {
	int period;
	
public:
	DeMarker();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(3, 1, 4)
			.Arg(period, 2, 127);
	}
};


class ForceIndex {
	int period;
	int ma_method;
	
public:
	ForceIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 13, 0, 0)
			.Out(1, 1, 2)
			.Arg(period, 2, 127)
			.Arg(ma_method, 0, 3);
	}
};


class Momentum {
	int period, shift;
	
public:
	Momentum();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(2, 2, 2)
			.Arg(period, 2)
			.Arg(shift, -10000);
	}
};


class OsMA {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	double value_mean;
	double diff_mean;
	int value_count;
	int diff_count;
	
public:
	OsMA();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 12) // fast
			.In(FACTORY_MovingAverage, 26) // slow
			.Out(4, 2, 0)
			.Arg(fast_ema_period, 2)
			.Arg(slow_ema_period, 2)
			.Arg(signal_sma_period, 2)
			.Mem(value_mean) .Mem(value_count)
			.Mem(diff_mean) .Mem(diff_count);
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


class RelativeStrengthIndex {
	int period;
	
public:
	RelativeStrengthIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(4, 4, 2)
			.Arg(period, 2);
	}
};


class RelativeVigorIndex {
	int period;
	
public:
	RelativeVigorIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(2, 2, 2)
			.Arg(period, 2, 127);
	}
};


class StochasticOscillator {
	int k_period;
	int d_period;
	int slowing;
	
public:
	StochasticOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(4, 2, 3)
			.Arg(k_period, 2)
			.Arg(d_period, 2)
			.Arg(slowing, 2);
	}
};



class WilliamsPercentRange {
	int period;
	
	bool CompareDouble(double Number1, double Number2);
	
public:
	WilliamsPercentRange();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(2, 2, 0)
			.Arg(period, 2);
	}
};


class AccumulationDistribution {
	
public:
	AccumulationDistribution();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0);
	}
};

class MoneyFlowIndex {
	int period;
	
public:
	MoneyFlowIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0)
			.Arg(period, 2, 127);
	}
};

class ValueAndVolumeTrend {
	int applied_value;
	
public:
	ValueAndVolumeTrend();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0)
			.Arg(applied_value, 0, 0);
	}
};

class OnBalanceVolume {
	int applied_value;
	
public:
	OnBalanceVolume();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0)
			.Arg(applied_value, 0, 0);
	}
};


class Volumes {
	
public:
	Volumes();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0);
	}
};


class AcceleratorOscillator {
	
public:
	AcceleratorOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 5, 0, MODE_SMA)
			.In(FACTORY_MovingAverage, 34, 0, MODE_SMA)
			.Out(5, 3, 2);
	}
};





class AwesomeOscillator {
	
public:
	AwesomeOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, 5, 0, MODE_SMA)
			.In(FACTORY_MovingAverage, 34, 0, MODE_SMA)
			.Out(3, 3, 2);
	}
};


class Fractals {
	int left_bars;
	int right_bars;
	
	double IsFractalUp(int index, int left, int right, int maxind, SourceImage& si, ChartImage& ci, GraphImage& gi);
	double IsFractalDown(int index, int left, int right, int maxind, SourceImage& si, ChartImage& ci, GraphImage& gi);
	
public:
	Fractals();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(6, 6, 0)
			.Arg(left_bars, 2, 20)
			.Arg(right_bars, 0, 0);
	}
};


class FractalOsc {
	int left_bars;
	int right_bars;
	int smoothing_period;
	
public:
	FractalOsc();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_Fractals, 3, 0)
			.Out(2, 2, 0)
			.Arg(smoothing_period, 2, 127);
	}
};


class MarketFacilitationIndex {
	
public:
	MarketFacilitationIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(5, 4, 0);
	}
};


class ZigZag {
	int input_depth;
	int input_deviation;
	int input_backstep;
	int extremum_level;
	
public:
	ZigZag();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(4, 2, 1)
			.Arg(input_depth, 1)
			.Arg(input_depth, 1)
			.Arg(input_backstep, 1)
			.Arg(extremum_level, 2);
	}
};

class ZigZagOsc {
	int depth;
	int deviation;
	int backstep;
	
public:
	ZigZagOsc();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0)
			.In(FACTORY_ZigZag, 12, 5, 3)
			.Arg(depth, 2, 16)
			.Arg(deviation, 2, 16)
			.Arg(backstep, 2, 16);
	}
};




class LinearTimeFrames {
	
public:
	LinearTimeFrames();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(4, 4, 0);
	}
};




class LinearWeekTime {
	
public:
	LinearWeekTime();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0);
	}
};




class SupportResistance {
	int period, max_crosses, max_radius;
	
public:
	SupportResistance();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(2, 2, 0)
			.Arg(period, 300, 300)
			.Arg(max_crosses, 100, 100)
			.Arg(max_radius, 100, 100);
	}
};


class SupportResistanceOscillator {
	int period, max_crosses, max_radius, smoothing_period;
	
public:
	SupportResistanceOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_SupportResistance) // Set("period", period).Set("max_crosses", max_crosses).Set("max_radius", max_radius);
			.Out(2, 2, 0)
			.Arg(period, 2, 127)
			.Arg(max_crosses, 300, 300)
			.Arg(max_radius, 100, 100)
			.Arg(smoothing_period, 100, 100);
	}
};


class ChannelOscillator {
	int period;
	ExtremumCache ec;
	
public:
	ChannelOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 2)
			.Arg(period, 2)
			.Mem(ec);
	}
};


class ScissorChannelOscillator {
	int period;
	ExtremumCache ec;
	
public:
	ScissorChannelOscillator();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 2)
			.Arg(period, 2)
			.Mem(ec);
	}
};


class Psychological {
	int period;
	
public:
	Psychological();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 2)
			.Arg(period, 2, 127);
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
class TrendChange {
	static const int period = 13;
	
public:
	TrendChange();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, period, 0)
			.Out(1, 1, 0);
	}
};

/*
	TrendChangeEdge does the edge filtering to the TrendChange.
	The edge filter for data is the same that for images.
	Positive peak values are when trend is changing.
*/
class TrendChangeEdge {
	static const int period = 13;
	
public:
	TrendChangeEdge();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.In(FACTORY_MovingAverage, period, 0)
			.In(FACTORY_MovingAverage, 54, 0, -27)
			.Out(3, 1, 0);
	}
};



class PeriodicalChange {
	Vector<double> means;
	Vector<int> counts;
	int split_type, tfmin;
	
protected:
	
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
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 2)
			.Mem(means)
			.Mem(counts);
	}
};



class VolatilityAverage {
	int period;
	VectorMap<int, int> stats;
	Vector<double> stats_limit;
	
public:
	VolatilityAverage();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 2)
			.Arg(period, 2)
			.Mem(stats);
	}
};


class MinimalLabel {
	int cost_level = 0;
	
public:
	MinimalLabel();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 1)
			.Arg(cost_level, 0);
	}
};

class VolatilitySlots {
	
protected:
	friend class WeekSlotAdvisor;
	Vector<OnlineAverage1> stats;
	OnlineAverage1 total;
	
	int slot_count = 0;
	
	
public:
	VolatilitySlots();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 4)
			.Mem(stats)
			.Mem(total);
	}
	
};


class VolumeSlots {
	
protected:
	Vector<OnlineAverage1> stats;
	OnlineAverage1 total;
	
	int slot_count = 0;
	
	
public:
	VolumeSlots();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
		
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 4)
			.Mem(stats)
			.Mem(total);
	}
	
};



class TrendIndex {
	int period = 6;
	int err_div = 3;
public:
	TrendIndex();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(3, 3, 1)
			.Arg(period, 2)
			.Arg(err_div, 0);
	}
	
	static void Process(const Vector<double>& open_buf, int i, int period, int err_div, double& err, double& buf_value, double& av_change, bool& bit_value);
	
};

class OnlineMinimalLabel {
	int prev_counted = 0;
	int cost_level = 0;
	
public:
	OnlineMinimalLabel();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 1)
			.Mem(prev_counted)
			.Arg(cost_level, 0);
	}
	
	static void GetMinimalSignal(double cost, const Vector<double>& open_buf, int begin, int end, bool* sigbuf, int sigbuf_size);
	
};


class ReactionContext {
	int length = 3;
	
public:
	ReactionContext();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg	.Out(1, 1, 0)
			.Arg(length, 1);
	}
	
	enum {UNKNOWN, UPTREND, DOWNTREND, HIGHBREAK, LOWBREAK, REVERSALUP, REVERSALDOWN, COUNT};
	
};

class VolatilityContext {
	VectorMap<int,int> median_map;
	Vector<double> volat_divs;
	int div = DEFAULT_DIV;
	
public:
	VolatilityContext();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .Out(1, 1, 0)
			.Arg(div, 1)
			.Mem(median_map);
	}
	
	static const int DEFAULT_DIV = 6;
	
};

class ChannelContext {
	VectorMap<int,int> median_map;
	Vector<double> volat_divs;
	ExtremumCache channel;
	int div = DEFAULT_DIV;
	int useable_div = DEFAULT_DIV * 2 / 3;
	int period = 30;
	
public:
	ChannelContext();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .Out(1, 1, 1)
			.Arg(period, 1)
			.Arg(div, 1)
			.Arg(useable_div, 1)
			.Mem(median_map)
			.Mem(channel);
	}
	
	static const int DEFAULT_DIV = 6;
	
};



class Obviousness {
	
protected:
	friend class ObviousAdvisor;
	friend class LessObviousAdvisor;
	
	
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
		uint8 bit_ids[max_bit_ids] = {0,0,0};
		BitComboStat bit_stats[max_bit_values];
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(BitMatcher)); else s.Put(this, sizeof(BitMatcher));}
	};
	typedef Vector<Snap> VectorSnap;
	
	static const int period_count = 6;
	enum {ONLINEMINLAB, TRENDINDEX,VOLATCTX0, VOLATCTX1, VOLATCTX2, VOLATCTX3, VOLATCTX4, VOLATCTX5, MA, MOM, OTREND, HTREND, LTREND, RTRENDUP, RTRENDDOWN,INPUT_COUNT};
	
	enum {RT_SIGNAL, RT_ENABLED, rt_row_size};
	static const int row_size = period_count * INPUT_COUNT;
	const int volat_div = 6;
	const double idx_limit_f = 0.75;
	
	VectorSnap data_in, data_out;
	Vector<OnlineAverageWindow1> av_wins;
	Vector<Vector<double> > volat_divs;
	Vector<VectorMap<int,int> > median_maps;
	Vector<BitMatcher> bitmatches;
	
	void RefreshInput(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void RefreshInitialOutput(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void RefreshOutput(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void RefreshIOStats(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	
public:
	typedef Obviousness CLASSNAME;
	
	Obviousness();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .Out(buffer_count, buffer_count, 2)
			.Mem(data_in)
			.Mem(data_out)
			.Mem(av_wins)
			.Mem(volat_divs)
			.Mem(median_maps)
			.Mem(bitmatches)
			;
	}
	
};


class VolatilityContextReversal {
	
public:
	typedef VolatilityContextReversal CLASSNAME;
	
	VolatilityContextReversal();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .In(FACTORY_VolatilityContext)
			.Out(1, 1, 1);
	}
	
};


#if 0
class ObviousTargetValue {
	struct BitComboStat {
		int total_count = 0;
		double sum = 0.0;
	};
	
	const int row_size = 4;
	static const int max_bit_ids = 4;
	static const int max_bit_values = 2*2*2*2;
	struct BitMatcher : Moveable<BitMatcher> {
		uint8 bit_count = 0;
		uint8 bit_ids[max_bit_ids] = {0,0,0};
		BitComboStat bit_stats[max_bit_values];
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(BitMatcher)); else s.Put(this, sizeof(BitMatcher));}
	};
	Vector<BitMatcher> bitmatches;
	
	Vector<ConstVectorBool*> bufs;
	ConstVectorBool* outbuf = NULL;
	
	void RefreshIOStats(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void RefreshTargetValues(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void RefreshOutput(SourceImage& si, ChartImage& ci, GraphImage& gi);
	bool GetInput(int i, int j);
	
public:
	typedef ObviousTargetValue CLASSNAME;
	
	ObviousTargetValue();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .In(FACTORY_TrendIndex)
			.In(FACTORY_Obviousness)
			.In(FACTORY_OnlineMinimalLabel)
			.In(FACTORY_VolatilityContextReversal)
			.In(FACTORY_MinimalLabel, 1)
			.Out(2, 1)
			.Mem(bitmatches);
	}
	
};

class BasicSignal {
	
	
	Vector<ConstVectorBool*> bufs;
	
	
public:
	typedef BasicSignal CLASSNAME;
	
	BasicSignal();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .In(FACTORY_MinimalLabel, 10)
			.In(FACTORY_VolatilityContextReversal)
			.In(FACTORY_Obviousness)
			.In(FACTORY_ObviousTargetValue)
			.In(FACTORY_ChannelContext)
			.Out(1, 1);
	}
	
	
};



class ObviousAdvisor {
	
	
	const int row_size = 4;
	static const int max_bit_ids = 4;
	static const int max_bit_values = 2*2*2*2;
	struct BitComboStat {
		int total_positive = 0;
		int total = 0;
		int outcome_positive = 0;
		int outcome_total = 0;
	};
	struct BitMatcher : Moveable<BitMatcher> {
		uint8 bit_count = 0;
		uint8 bit_ids[max_bit_ids] = {0,0,0,0};
		BitComboStat bit_stats[max_bit_values];
		void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(BitMatcher)); else s.Put(this, sizeof(BitMatcher));}
	};
	Vector<BitMatcher> bitmatches;
	
	
	
	Index<int> begins;
	int begin_cursor = 0;
	
	Vector<ConstVectorBool*> bufs;
	BasicSignal* bs = NULL;
	Obviousness* obv = NULL;
	
	void RefreshIOStats();
	void RefreshOutput();
	void TestAdvisor();
	
	
public:
	typedef ObviousAdvisor CLASSNAME;
	
	ObviousAdvisor();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .In(FACTORY_BasicSignal)
			.In(FACTORY_Obviousness)
			.Out(1, 1)
			.Mem(begins)
			.Mem(begin_cursor);
	}
	
};


class ExampleAdvisor {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		void Paint(Draw& w);
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
	
public:
	typedef ExampleAdvisor CLASSNAME;
	ExampleAdvisor();
	
	void Init(SourceImage& si, ChartImage& ci, GraphImage& gi);
	void Start(SourceImage& si, ChartImage& ci, GraphImage& gi);
	
	void Conf(ValueRegister& reg) {
		reg .Out(1, 1)
			.Mem(training_pts)
			.Mem(prev_counted);
	}
};


#endif


}

#endif
