#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

namespace Overlook {
using namespace Upp;


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};


class MovingAverage : public Core {
	Vector<double> buffer;
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
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % buffer;}
	
	virtual void Init();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		reg.AddIn(SourcePhase, RealValue, SymTf);
		reg.AddOut(IndiPhase, RealIndicatorValue, SymTf, 1, 1);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class AverageTrueRange : public Core {
	int period;
	
	Vector<double> atr_buffer;
	Vector<double> tr_buffer;
	
	
public:
	AverageTrueRange();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class BearsPower : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	BearsPower();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class BullsPower : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	BullsPower();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class CommodityChannelIndex : public Core {
	int period;
	
	Vector<double> cci_buffer;
	Vector<double> value_buffer;
	Vector<double> mov_buffer;

public:
	CommodityChannelIndex();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class DeMarker : public Core {
	int period;
	
	Vector<double> buffer;
	Vector<double> max_buffer;
	Vector<double> min_buffer;
	
public:
	DeMarker();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class ForceIndex : public Core {
	int period;
	int ma_method;
	int applied_value;
	
	Vector<double> buffer;
	
public:
	ForceIndex();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class Momentum : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	Momentum();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};


class RelativeStrengthIndex : public Core {
	int period;
	
	Vector<double>    buffer;
	Vector<double>    pos_buffer;
	Vector<double>    neg_buffer;
	
public:
	RelativeStrengthIndex();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class RelativeVigorIndex : public Core {
	int period;
	
	Vector<double>     buffer;
	Vector<double>     signal_buffer;
	
public:
	RelativeVigorIndex();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class WilliamsPercentRange : public Core {
	int period;
	
	Vector<double> buffer;
	
	bool CompareDouble(double Number1, double Number2);
	
public:
	WilliamsPercentRange();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class AccumulationDistribution : public Core {
	Vector<double> buffer;
	
public:
	AccumulationDistribution();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class MoneyFlowIndex : public Core {
	int period;
	
	Vector<double> buffer;
	
public:
	MoneyFlowIndex();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class ValueAndVolumeTrend : public Core {
	int applied_value;
	Vector<double> buffer;
	
public:
	ValueAndVolumeTrend();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class OnBalanceVolume : public Core {
	int applied_value;
	Vector<double> buffer;
	
public:
	OnBalanceVolume();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class Volumes : public Core {
	Vector<double> buf;
	
public:
	Volumes();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class AwesomeOscillator : public Core {
	Vector<double>     buffer;
	Vector<double>     up_buffer;
	Vector<double>     down_buffer;
	
public:
	AwesomeOscillator();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class FractalOsc : public Core {
	int left_bars;
	int right_bars;
	int smoothing_period;
	Vector<double> buf, av;
	
public:
	FractalOsc();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class ZigZagOsc : public Core {
	Vector<double> osc;
	
	int depth;
	int deviation;
	int backstep;
	
public:
	ZigZagOsc();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};




class LinearTimeFrames : public Core {
	Vector<double> day, month, year, week;
	
public:
	LinearTimeFrames();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class RepeatingAverage : public Core {
	int period, smoothing_period, applied_value;
	Vector<double> average1, average2, average3, average4;
	
public:
	RepeatingAverage();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class RepeatingAverageOscillator : public Core {
	int period, smoothing_period, applied_value;
	Vector<double> diff1, diff2, maindiff, avdiff, main_av_diff;
	
public:
	RepeatingAverageOscillator();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class SupportResistance : public Core {
	int period, max_crosses, max_radius;
	Vector<double> support, resistance;
	
public:
	SupportResistance();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class SupportResistanceOscillator : public Core {
	int period, max_crosses, max_radius, smoothing_period;
	Vector<double> osc, osc_av;
	
public:
	SupportResistanceOscillator();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class Psychological : public Core {
	int period;
	Vector<double> buf;
	
public:
	Psychological();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
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
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

}

#endif
