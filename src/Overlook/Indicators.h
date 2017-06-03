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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", ma_period)
			% Arg("offset", ma_shift)
			% Arg("method", ma_method);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2)
			% Arg("fast_ema", fast_ema_period)
			% Arg("slow_ema", slow_ema_period)
			% Arg("signal_sma", signal_sma_period);
	}
};


class AverageDirectionalMovement : public Core {
	int period_adx;
	
public:
	AverageDirectionalMovement();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 6, 3)
			% Arg("period", period_adx);
	}
};


class BollingerBands : public Core {
	int           bands_period;
	int           bands_shift;
	double        bands_deviation;
	int           plot_begin;
	
	double StdDev_Func(int position, const Buffer& MAvalue, int period);
	
public:
	BollingerBands();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 3)
			% Arg("period", bands_period)
			% Arg("shift", bands_shift)
			% Arg("deviation", bands_deviation);
	}
};


class Envelopes : public Core {
	int                ma_period;
	int                ma_shift;
	int                ma_method;
	double             deviation;
	
public:
	Envelopes();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 2)
			% Arg("period", ma_period)
			% Arg("shift", ma_shift)
			% Arg("deviation", deviation)
			% Arg("method", ma_method);
	}
};


class ParabolicSAR : public Core {
	double		sar_step;
	double		sar_maximum;
	int			last_rev_pos;
	bool		direction_long;
	
	double GetHigh( int pos, int start_period );
	double GetLow( int pos, int start_period );
	
public:
	ParabolicSAR();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("step", sar_step)
			% Arg("maximum", sar_maximum)
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period)
			% Arg("ma_method", ma_method);
	}
};


class AverageTrueRange : public Core {
	int period;
	
public:
	AverageTrueRange();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 1)
			% Arg("period", period);
	}
};


class BearsPower : public Core {
	int period;
	
public:
	BearsPower();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};


class BullsPower : public Core {
	int period;
	
public:
	BullsPower();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};


class CommodityChannelIndex : public Core {
	int period;
	
public:
	CommodityChannelIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("period", period);
	}
};


class DeMarker : public Core {
	int period;
	
public:
	DeMarker();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("period", period);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period)
			% Arg("ma_method", ma_method);
	}
};


class Momentum : public Core {
	int period;
	
public:
	Momentum();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};


class OsMA : public Core {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	
public:
	OsMA();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("fast_ema_period", fast_ema_period)
			% Arg("slow_ema_period", slow_ema_period)
			% Arg("signal_sma", signal_sma_period);
	}
};


class RelativeStrengthIndex : public Core {
	int period;
	
public:
	RelativeStrengthIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("period", period);
	}
};


class RelativeVigorIndex : public Core {
	int period;
	
public:
	RelativeVigorIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2)
			% Arg("period", period);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 2)
			% Arg("k_period", k_period)
			% Arg("d_period", d_period)
			% Arg("slowing", slowing);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};


class AccumulationDistribution : public Core {
	
public:
	AccumulationDistribution();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1);
	}
};

class MoneyFlowIndex : public Core {
	int period;
	
public:
	MoneyFlowIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};

class ValueAndVolumeTrend : public Core {
	int applied_value;
	
public:
	ValueAndVolumeTrend();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("applied_value", applied_value);
	}
};

class OnBalanceVolume : public Core {
	int applied_value;
	
public:
	OnBalanceVolume();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("applied_value", applied_value);
	}
};


class Volumes : public Core {
	
public:
	Volumes();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1);
	}
};


class Spreads : public Core {
	
public:
	Spreads();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1);
	}
};


class AcceleratorOscillator : public Core {
	
public:
	AcceleratorOscillator();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 5, 3);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 6, 6)
			% Arg("jaws_period", jaws_period)
			% Arg("jaws_shift", jaws_shift)
			% Arg("teeth_period", teeth_period)
			% Arg("teeth_shift", teeth_shift)
			% Arg("lips_period", lips_period)
			% Arg("lips_shift", lips_shift)
			% Arg("ma_method", ma_method)
			% Arg("applied_value", applied_value);
	}
};


class AwesomeOscillator : public Core {
	
public:
	AwesomeOscillator();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 3);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 6, 6)
			% Arg("left_bars", left_bars)
			% Arg("right_bars", right_bars);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2)
			% Arg("left_bars", left_bars)
			% Arg("right_bars", right_bars)
			% Arg("smoothing", smoothing_period);
	}
};


class MarketFacilitationIndex : public Core {
	
public:
	MarketFacilitationIndex();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 5, 4);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 2)
			% Arg("depth", input_depth)
			% Arg("deviation", input_depth)
			% Arg("backstep", input_backstep)
			% Arg("level", extremum_level);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("depth", depth)
			% Arg("deviation", deviation)
			% Arg("backstep", backstep);
	}
};




class LinearTimeFrames : public Core {
	
public:
	LinearTimeFrames();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 4);
	}
};


class SupportResistance : public Core {
	int period, max_crosses, max_radius;
	
public:
	SupportResistance();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2)
			% Arg("period", period)
			% Arg("max_crosses", max_crosses)
			% Arg("max_radius", max_radius);
	}
};


class SupportResistanceOscillator : public Core {
	int period, max_crosses, max_radius, smoothing_period;
	
public:
	SupportResistanceOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2)
			% Arg("period", period)
			% Arg("max_crosses", max_crosses)
			% Arg("max_radius", max_radius)
			% Arg("smoothing", smoothing_period);
	}
};


class Psychological : public Core {
	int period;
	
public:
	Psychological();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period);
	}
};


class CorrelationOscillator : public Core {
	
public:
	struct OnlineAverage : Moveable<OnlineAverage> {
		double mean_a, mean_b;
		int count;
		OnlineAverage() : mean_a(0), mean_b(0), count(0) {}
		void Add(double a, double b) {
			double delta_a = a - mean_a; mean_a += delta_a / count;
			double delta_b = b - mean_b; mean_b += delta_b / count;
			count++;
		}
	};
	
protected:
	typedef const Vector<double> ConstVector;
	
	int period;
	int sym_count;
	Vector<ConstBuffer*> opens;
	Vector<OnlineAverage> averages;
	
	void Process(int id, int output);
	
public:
	CorrelationOscillator();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		if (sym_count == -1 && base)
			sym_count = GetSystem().GetSymbolCount();
		reg % In(SourcePhase, RealValue, Sym)
			% Out(IndiPhase, CorrelationValue, SymTf, sym_count-1, sym_count-1)
			% Arg("period", period);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("period", period)
			% Arg("method", method);
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
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 1)
			% Arg("period", period)
			% Arg("method", method)
			% Arg("slowing", slowing);
	}
};

}

#endif
