#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", ma_period, 2, 127)
			% Arg("offset", ma_shift, 0, 0)
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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("step", step, 2, 127)
			% Arg("maximum", maximum, 2, 127)
			% Persistent(last_rev_pos)
			% Persistent(direction_long);
	}
};


class StandardDeviation : public Core {
	int period;
	int ma_method;
	
public:
	StandardDeviation();
	
	virtual void Init();
	virtual void Start();
	
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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127)
			% Arg("ma_method", ma_method, 0, 3);
	}
};


class Momentum : public Core {
	int period;
	
public:
	Momentum();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Arg("period", period, 2, 127);
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
			% Arg("fast_ema_period", fast_ema_period, 2, 10000)
			% Arg("slow_ema_period", slow_ema_period, 2, 10000)
			% Arg("signal_sma", signal_sma_period, 2, 10000)
			% Persistent(value_mean) % Persistent(value_count)
			% Persistent(diff_mean) % Persistent(diff_count);
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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(3, 1)
			% Arg("period", period, 2, 127);
	}
};


class RelativeVigorIndex : public Core {
	int period;
	
public:
	RelativeVigorIndex();
	
	virtual void Init();
	virtual void Start();
	
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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(4, 2)
			% Arg("k_period", k_period, 2, 10000)
			% Arg("d_period", d_period, 2, 10000)
			% Arg("slowing", slowing, 2, 10000);
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
			% Out(1, 1)
			% Arg("period", period, 2, 127);
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


class Spreads : public Core {
	
public:
	Spreads();
	
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
			% Arg("depth", input_depth, 2, 16)
			% Arg("deviation", input_depth, 2, 16)
			% Arg("backstep", input_backstep, 2, 16)
			% Arg("level", extremum_level, 2, 16);
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
			% Out(2, 2)
			% Arg("period", period, 2, 127)
			% Arg("max_crosses", max_crosses, 300, 300)
			% Arg("max_radius", max_radius, 100, 100)
			% Arg("smoothing", smoothing_period, 100, 100);
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
	int sym_count;
	ConstBuffer* this_open;
	Vector<int> major_sym;
	Vector<ConstBuffer*> opens;
	Vector<OnlineAverage2> averages;
	
	void Process(int id, int output);
	
public:
	CorrelationOscillator();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		if (sym_count == -1)
			sym_count = 4 + GetMetaTrader().GetIndexCount();
		reg % In<DataBridge>(&FilterFunction)
			% Out(sym_count, sym_count)
			% Arg("period", period, 2, 16);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		static Index<int> major_sym;
		if (major_sym.IsEmpty()) {
			MetaTrader& mt = GetMetaTrader();
			
			// Add most important currencies
			for(int i = 0, j = mt.GetSymbolCount(); i < 4; i++, j++)
				major_sym.Add(j);
			
			// Add most important indices
			for(int i = 0; i < mt.GetIndexCount(); i++)
				major_sym.Add(mt.GetIndexId(i));
		}
		
		// Accept all symbols in the same timeframe
		if (in_sym == -1)
			return in_tf == out_tf;
		
		if (major_sym.Find(out_sym) != -1 || out_sym == in_sym)
			return true;
		return false;
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
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Persistent(means)
			% Persistent(counts);
	}
};

//#define LARGE_SENSOR
#define SMALL_COUNT 4

/*class Sensors : public Core {
	Vector<double> means;
	Vector<int> counts;
	double total_mean;
	int total_count;
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
		
		if (!total_count) {
			total_mean = fabs(d);
			total_count = 1;
		} else {
			double delta = fabs(d) - total_mean;
			total_mean += delta / total_count;
			total_count++;
		}
	}
	double Get(int i) {return means[i];}
	
public:
	Sensors();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
		#ifdef LARGE_SENSOR
			% Out(10, 10)
		#else
			% Out(SMALL_COUNT*2, SMALL_COUNT*2)
		#endif
			% Persistent(means)
			% Persistent(counts)
			% Persistent(total_mean)
			% Persistent(total_count);
	}
};*/



}

#endif
