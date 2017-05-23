#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

namespace Overlook {
using namespace Upp;


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};


class MovingAverage : public Core {
	int ma_period;
	int ma_shift;
	int ma_method;
	int ma_counted;
	int event_up, event_down;
	
protected:
	virtual void Start();
	
	void Simple();
	void Exponential();
	void Smoothed();
	void LinearlyWeighted();
	
public:
	MovingAverage();
	
	//virtual void Serialize(Stream& s) {Core::Serialize(s); s % buffer;}
	
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
	Vector<double>    buffer;
	Vector<double>    signal_buffer;
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	bool params;
	
	
public:
	MovingAverageConvergenceDivergence();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf);
		reg % Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
		reg % Arg("fast_ema", fast_ema_period);
		reg % Arg("slow_ema", slow_ema_period);
		reg % Arg("signal_sma", signal_sma_period);
	}
};


class AverageDirectionalMovement : public Core {
	Vector<double>    adx_buffer;
	Vector<double>    pdi_buffer;
	Vector<double>    ndi_buffer;
	Vector<double>    pd_buffer;
	Vector<double>    nd_buffer;
	Vector<double>    tmp_buffer;
	int period_adx;
	
	
public:
	AverageDirectionalMovement();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period_adx);
	}
};


class BollingerBands : public Core {
	Vector<double>    ml_buffer;
	Vector<double>    tl_buffer;
	Vector<double>    bl_buffer;
	Vector<double>    stddev_buffer;
	
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
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", bands_period);
		reg % Arg("shift", bands_shift);
		reg % Arg("deviation", bands_deviation);
	}
};


class Envelopes : public Core {
	
	int                ma_period;
	int                ma_shift;
	int                ma_method;
	double             deviation;
	
	Vector<double>                   up_buffer;
	Vector<double>                   down_buffer;
	Vector<double>                   ma_buffer;
	
	
public:
	Envelopes();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", ma_period);
		reg % Arg("shift", ma_shift);
		reg % Arg("deviation", deviation);
		reg % Arg("method", ma_method);
	}
};


class ParabolicSAR : public Core {
	double         sar_step;
	double         sar_maximum;
	
	Vector<double>           sar_buffer;
	Vector<double>           ep_buffer;
	Vector<double>           af_buffer;
	
	int                  last_rev_pos;
	bool                 direction_long;
	
	
	double GetHigh( int pos, int start_period );
	double GetLow( int pos, int start_period );
	
public:
	ParabolicSAR();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("step", sar_step);
		reg % Arg("maximum", sar_maximum);
	}
};


class StandardDeviation : public Core {
	int period;
	int ma_method;
	int applied_value;
	int shift;
	
	Vector<double> stddev_buffer;

	
public:
	StandardDeviation();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class AverageTrueRange : public Core {
	int period;
	
	Vector<double> atr_buffer;
	Vector<double> tr_buffer;
	
	
public:
	AverageTrueRange();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class BearsPower : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	BearsPower();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class BullsPower : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	BullsPower();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class CommodityChannelIndex : public Core {
	int period;
	
	Vector<double> cci_buffer;
	Vector<double> value_buffer;
	Vector<double> mov_buffer;

public:
	CommodityChannelIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class DeMarker : public Core {
	int period;
	
	Vector<double> buffer;
	Vector<double> max_buffer;
	Vector<double> min_buffer;
	
public:
	DeMarker();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class ForceIndex : public Core {
	int period;
	int ma_method;
	int applied_value;
	
	Vector<double> buffer;
	
public:
	ForceIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class Momentum : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	Momentum();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class OsMA : public Core {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	
	Vector<double> osma_buffer;
	Vector<double> buffer;
	Vector<double> signal_buffer;
	
	bool   params;
	
public:
	OsMA();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("slow_ema", slow_ema_period);
		reg % Arg("fast_ema", fast_ema_period);
		reg % Arg("signal_sma", signal_sma_period);
	}
};


class RelativeStrengthIndex : public Core {
	int period;
	
	Vector<double>    buffer;
	Vector<double>    pos_buffer;
	Vector<double>    neg_buffer;
	
public:
	RelativeStrengthIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class RelativeVigorIndex : public Core {
	int period;
	
	Vector<double>     buffer;
	Vector<double>     signal_buffer;
	
public:
	RelativeVigorIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class StochasticOscillator : public Core {
	int k_period;
	int d_period;
	int slowing;
	
	Vector<double>    buffer;
	Vector<double>    signal_buffer;
	Vector<double>    high_buffer;
	Vector<double>    low_buffer;
		
public:
	StochasticOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("k_period", k_period);
		reg % Arg("d_period", d_period);
		reg % Arg("slowing", slowing);
	}
};



class WilliamsPercentRange : public Core {
	int period;
	
	Vector<double> buffer;
	
	bool CompareDouble(double Number1, double Number2);
	
public:
	WilliamsPercentRange();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class AccumulationDistribution : public Core {
	Vector<double> buffer;
	
public:
	AccumulationDistribution();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
	}
};

class MoneyFlowIndex : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	MoneyFlowIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};

class ValueAndVolumeTrend : public Core {
	int applied_value;
	Vector<double> buffer;
	
public:
	ValueAndVolumeTrend();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("applied_value", applied_value);
	}
};

class OnBalanceVolume : public Core {
	int applied_value;
	Vector<double> buffer;
	
public:
	OnBalanceVolume();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("applied_value", applied_value);
	}
};


class Volumes : public Core {
	Vector<double> buf;
	
public:
	Volumes();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
	}
};


class AcceleratorOscillator : public Core {
	Vector<double>     buffer;
	Vector<double>     up_buffer;
	Vector<double>     down_buffer;
	Vector<double>     macd_buffer;
	Vector<double>     signal_buffer;
	
public:
	AcceleratorOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
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
	
	Vector<double> up_buffer;
	Vector<double> up_red_buffer;
	Vector<double> up_green_buffer;
	Vector<double> down_buffer;
	Vector<double> down_red_buffer;
	Vector<double> down_green_buffer;
	
public:
	GatorOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("jaws_period", jaws_period);
		reg % Arg("jaws_shift", jaws_shift);
		reg % Arg("teeth_period", teeth_period);
		reg % Arg("teeth_shift", teeth_shift);
		reg % Arg("lips_period", lips_period);
		reg % Arg("lips_shift", lips_shift);
		reg % Arg("ma_method", ma_method);
		reg % Arg("applied_value", applied_value);
	}
};


class AwesomeOscillator : public Core {
	Vector<double>     buffer;
	Vector<double>     up_buffer;
	Vector<double>     down_buffer;
	
public:
	AwesomeOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
	}
};


class Fractals : public Core {
	int left_bars;
	int right_bars;
	int levels;
	
	Vector<double> line_up_buf1;
	Vector<double> line_up_buf2;
	Vector<double> arrow_up_buf;
	Vector<double> arrow_down_buf;
	Vector<double> arrow_breakup_buf;
	Vector<double> arrow_breakdown_buf;
	
	double IsFractalUp(int index, int left, int right, int maxind);
	double IsFractalDown(int index, int left, int right, int maxind);
	
public:
	Fractals();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("left_bars", left_bars);
		reg % Arg("right_bars", right_bars);
	}
};

class FractalOsc : public Core {
	int left_bars;
	int right_bars;
	int smoothing_period;
	Vector<double> buf, av;
	
public:
	FractalOsc();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("left_bars", left_bars);
		reg % Arg("right_bars", right_bars);
		reg % Arg("smoothing", smoothing_period);
	}
};


class MarketFacilitationIndex : public Core {
	Vector<double> buffer;
	Vector<double> up_up_buffer;
	Vector<double> down_down_buffer;
	Vector<double> up_down_buffer;
	Vector<double> down_up_buffer;
	
public:
	MarketFacilitationIndex();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
	}
};


class ZigZag : public Core {
	Vector<double> keypoint_buffer;
	Vector<double> high_buffer;
	Vector<double> low_buffer;
	Vector<double> osc;
	
	int input_depth;
	int input_deviation;
	int input_backstep;
	int extremum_level;
	
protected:
	virtual void Start();
	
public:
	ZigZag();
	
	virtual void Serialize(Stream& s) {
		Core::Serialize(s);
		s % keypoint_buffer % high_buffer % low_buffer % osc;
		s % input_depth % input_deviation % input_backstep % extremum_level;
	}
	
	virtual void Init();
	
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("depth", input_depth);
		reg % Arg("deviation", input_depth);
		reg % Arg("backstep", input_backstep);
		reg % Arg("level", extremum_level);
	}
};

class ZigZagOsc : public Core {
	Vector<double> osc;
	
	int depth;
	int deviation;
	int backstep;
	
public:
	ZigZagOsc();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("depth", depth);
		reg % Arg("deviation", deviation);
		reg % Arg("backstep", backstep);
	}
};




class LinearTimeFrames : public Core {
	Vector<double> day, month, year, week;
	
public:
	LinearTimeFrames();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
	}
};


class RepeatingAverage : public Core {
	int period, smoothing_period, applied_value;
	Vector<double> average1, average2, average3, average4;
	
public:
	RepeatingAverage();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("smoothing", smoothing_period);
		reg % Arg("applied_value", applied_value);
	}
};

class RepeatingAverageOscillator : public Core {
	int period, smoothing_period, applied_value;
	Vector<double> diff1, diff2, maindiff, avdiff, main_av_diff;
	
public:
	RepeatingAverageOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("smoothing", smoothing_period);
		reg % Arg("applied_value", applied_value);
	}
};


class SupportResistance : public Core {
	int period, max_crosses, max_radius;
	Vector<double> support, resistance;
	
public:
	SupportResistance();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("max_crosses", max_crosses);
		reg % Arg("max_radius", max_radius);
	}
};


class SupportResistanceOscillator : public Core {
	int period, max_crosses, max_radius, smoothing_period;
	Vector<double> osc, osc_av;
	
public:
	SupportResistanceOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("max_crosses", max_crosses);
		reg % Arg("max_radius", max_radius);
		reg % Arg("smoothing", smoothing_period);
	}
};


class Psychological : public Core {
	int period;
	Vector<double> buf;
	
public:
	Psychological();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};


class CorrelationOscillator : public Core {
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
	
	typedef const Vector<double> ConstVector;
	
	int period;
	Array<Vector<double>> buf;
	int sym_count;
	Vector<Core*> sources;
	Vector<ConstVector*> opens;
	Vector<OnlineAverage> averages;
	//PathLink* ids;
	//ConstVector<double>* open;
	
	void Process(int id);
	
public:
	CorrelationOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
	}
};







/*
	ParallelSymLR takes two box-averages, one symmetric (peeks future) and one moving average.
	The it takes the change of average values and multiplies them:
	 >= 0 both averages are going same direction
	  < 0 the symmetric is changing direction, but lagging moving average is not yet
	This data can be statistically evaluated, and periodical time of change of direction can
	be estimated.
*/
class ParallelSymLR : public Core {
	
public:
	Vector<double> buffer;
	int period;
	int method;
	//DataVar buf;
	int event_up, event_down, event_plus, event_minus;
	
protected:
	virtual void Start();
	
public:
	ParallelSymLR();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % buffer;}
	
	virtual void Init();
	
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("method", method);
	}
};

/*
	ParallelSymLREdge does the edge filtering to the ParallelSymLR.
	The edge filter for data is the same that for images.
	Positive peak values are when trend is changing.
*/
class ParallelSymLREdge : public Core {
	
public:
	Vector<double> symlr;
	Vector<double> edge;
	Vector<double> buffer;
	int period;
	int method;
	int slowing;
	int event_trendchange;
	
protected:
	virtual void Start();
	
public:
	ParallelSymLREdge();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % symlr % edge % buffer;}
	
	virtual void Init();
	
	
	virtual void IO(ValueRegister& reg) {
		//reg % In(, , SymTf);
		//reg % Out(, , SymTf);
		reg % Arg("period", period);
		reg % Arg("method", method);
		reg % Arg("slowing", slowing);
	}
};

}

#endif
