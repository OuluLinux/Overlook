#ifndef _Overlook_Indicators_h_
#define _Overlook_Indicators_h_

namespace Overlook {
using namespace Upp;


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};



class MovingAverage : public Pipe {
	FloatVector buffer;
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
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % buffer;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class MovingAverageConvergenceDivergence : public Pipe {
	FloatVector    buffer;
	FloatVector    signal_buffer;
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	bool params;
	
	
public:
	MovingAverageConvergenceDivergence();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class AverageDirectionalMovement : public Pipe {
	FloatVector    adx_buffer;
	FloatVector    pdi_buffer;
	FloatVector    ndi_buffer;
	FloatVector    pd_buffer;
	FloatVector    nd_buffer;
	FloatVector    tmp_buffer;
	int period_adx;
	
	
public:
	AverageDirectionalMovement();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class BollingerBands : public Pipe {
	FloatVector    ml_buffer;
	FloatVector    tl_buffer;
	FloatVector    bl_buffer;
	FloatVector    stddev_buffer;
	
	int           bands_period;
	int           bands_shift;
	double        bands_deviation;
	int           plot_begin;
	
	double StdDev_Func(int position, const FloatVector& MAvalue, int period);
	
public:
	BollingerBands();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class Envelopes : public Pipe {
	
	int                ma_period;
	int                ma_shift;
	int                ma_method;
	double             deviation;
	
	FloatVector                   up_buffer;
	FloatVector                   down_buffer;
	FloatVector                   ma_buffer;
	
	
public:
	Envelopes();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class ParabolicSAR : public Pipe {
	double         sar_step;
	double         sar_maximum;
	
	FloatVector           sar_buffer;
	FloatVector           ep_buffer;
	FloatVector           af_buffer;
	
	int                  last_rev_pos;
	bool                 direction_long;
	
	
	double GetHigh( int pos, int start_period );
	double GetLow( int pos, int start_period );
	
public:
	ParabolicSAR();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class StandardDeviation : public Pipe {
	int period;
	int ma_method;
	int applied_value;
	int shift;
	
	FloatVector stddev_buffer;

	
public:
	StandardDeviation();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class AverageTrueRange : public Pipe {
	int period;
	
	FloatVector atr_buffer;
	FloatVector tr_buffer;
	
	
public:
	AverageTrueRange();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class BearsPower : public Pipe {
	int period;
	
	FloatVector buffer;
	
public:
	BearsPower();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class BullsPower : public Pipe {
	int period;
	
	FloatVector buffer;
	
public:
	BullsPower();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class CommodityChannelIndex : public Pipe {
	int period;
	
	FloatVector cci_buffer;
	FloatVector value_buffer;
	FloatVector mov_buffer;

public:
	CommodityChannelIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class DeMarker : public Pipe {
	int period;
	
	FloatVector buffer;
	FloatVector max_buffer;
	FloatVector min_buffer;
	
public:
	DeMarker();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class ForceIndex : public Pipe {
	int period;
	int ma_method;
	int applied_value;
	
	FloatVector buffer;
	
public:
	ForceIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class Momentum : public Pipe {
	int period;
	
	FloatVector buffer;
	
public:
	Momentum();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class OsMA : public Pipe {
	int fast_ema_period;
	int slow_ema_period;
	int signal_sma_period;
	
	FloatVector osma_buffer;
	FloatVector buffer;
	FloatVector signal_buffer;
	
	bool   params;
	
public:
	OsMA();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
};


class RelativeStrengthIndex : public Pipe {
	int period;
	
	FloatVector    buffer;
	FloatVector    pos_buffer;
	FloatVector    neg_buffer;
	
public:
	RelativeStrengthIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class RelativeVigorIndex : public Pipe {
	int period;
	
	FloatVector     buffer;
	FloatVector     signal_buffer;
	
public:
	RelativeVigorIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class StochasticOscillator : public Pipe {
	int k_period;
	int d_period;
	int slowing;
	
	FloatVector    buffer;
	FloatVector    signal_buffer;
	FloatVector    high_buffer;
	FloatVector    low_buffer;
		
public:
	StochasticOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class WilliamsPercentRange : public Pipe {
	int period;
	
	FloatVector buffer;
	
	bool CompareDouble(double Number1, double Number2);
	
public:
	WilliamsPercentRange();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class AccumulationDistribution : public Pipe {
	FloatVector buffer;
	
public:
	AccumulationDistribution();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class MoneyFlowIndex : public Pipe {
	int period;
	
	FloatVector buffer;
	
public:
	MoneyFlowIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class ValueAndVolumeTrend : public Pipe {
	int applied_value;
	FloatVector buffer;
	
public:
	ValueAndVolumeTrend();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class OnBalanceVolume : public Pipe {
	int applied_value;
	FloatVector buffer;
	
public:
	OnBalanceVolume();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class Volumes : public Pipe {
	FloatVector buf;
	
public:
	Volumes();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class AcceleratorOscillator : public Pipe {
	FloatVector     buffer;
	FloatVector     up_buffer;
	FloatVector     down_buffer;
	FloatVector     macd_buffer;
	FloatVector     signal_buffer;
	
public:
	AcceleratorOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class GatorOscillator : public Pipe {
	int jaws_period;
	int jaws_shift;
	int teeth_period;
	int teeth_shift;
	int lips_period;
	int lips_shift;
	int ma_method;
	int applied_value;
	
	FloatVector up_buffer;
	FloatVector up_red_buffer;
	FloatVector up_green_buffer;
	FloatVector down_buffer;
	FloatVector down_red_buffer;
	FloatVector down_green_buffer;
	
public:
	GatorOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class AwesomeOscillator : public Pipe {
	FloatVector     buffer;
	FloatVector     up_buffer;
	FloatVector     down_buffer;
	
public:
	AwesomeOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class Fractals : public Pipe {
	int left_bars;
	int right_bars;
	int levels;
	
	FloatVector line_up_buf1;
	FloatVector line_up_buf2;
	FloatVector arrow_up_buf;
	FloatVector arrow_down_buf;
	FloatVector arrow_breakup_buf;
	FloatVector arrow_breakdown_buf;
	
	double IsFractalUp(int index, int left, int right, int maxind);
	double IsFractalDown(int index, int left, int right, int maxind);
	
public:
	Fractals();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class FractalOsc : public Pipe {
	int left_bars;
	int right_bars;
	int smoothing_period;
	FloatVector buf, av;
	
public:
	FractalOsc();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class MarketFacilitationIndex : public Pipe {
	FloatVector buffer;
	FloatVector up_up_buffer;
	FloatVector down_down_buffer;
	FloatVector up_down_buffer;
	FloatVector down_up_buffer;
	
public:
	MarketFacilitationIndex();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class ZigZag : public Pipe {
	FloatVector keypoint_buffer;
	FloatVector high_buffer;
	FloatVector low_buffer;
	FloatVector osc;
	
	int input_depth;
	int input_deviation;
	int input_backstep;
	int extremum_level;
	
protected:
	virtual void Start();
	
public:
	ZigZag();
	
	virtual void Serialize(Stream& s) {
		Pipe::Serialize(s);
		s % keypoint_buffer % high_buffer % low_buffer % osc;
		s % input_depth % input_deviation % input_backstep % extremum_level;
	}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class ZigZagOsc : public Pipe {
	FloatVector osc;
	
	int depth;
	int deviation;
	int backstep;
	
public:
	ZigZagOsc();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};




class LinearTimeFrames : public Pipe {
	FloatVector day, month, year, week;
	
public:
	LinearTimeFrames();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class RepeatingAverage : public Pipe {
	int period, smoothing_period, applied_value;
	FloatVector average1, average2, average3, average4;
	
public:
	RepeatingAverage();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class RepeatingAverageOscillator : public Pipe {
	int period, smoothing_period, applied_value;
	FloatVector diff1, diff2, maindiff, avdiff, main_av_diff;
	
public:
	RepeatingAverageOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class SupportResistance : public Pipe {
	int period, max_crosses, max_radius;
	FloatVector support, resistance;
	
public:
	SupportResistance();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class SupportResistanceOscillator : public Pipe {
	int period, max_crosses, max_radius, smoothing_period;
	FloatVector osc, osc_av;
	
public:
	SupportResistanceOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class Psychological : public Pipe {
	int period;
	FloatVector buf;
	
public:
	Psychological();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class CorrelationOscillator : public Pipe {
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
	
	typedef const FloatVector ConstFloatVector;
	
	int period;
	Array<FloatVector> buf;
	int sym_count;
	Vector<Pipe*> sources;
	Vector<ConstFloatVector*> opens;
	Vector<OnlineAverage> averages;
	//PathLink* ids;
	//ConstFloatVector* open;
	
	void Process(int id);
	
public:
	CorrelationOscillator();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
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
class ParallelSymLR : public Pipe {
	
public:
	FloatVector buffer;
	int period;
	int method;
	//DataVar buf;
	int event_up, event_down, event_plus, event_minus;
	
protected:
	virtual void Start();
	
public:
	ParallelSymLR();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % buffer;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

/*
	ParallelSymLREdge does the edge filtering to the ParallelSymLR.
	The edge filter for data is the same that for images.
	Positive peak values are when trend is changing.
*/
class ParallelSymLREdge : public Pipe {
	
public:
	FloatVector symlr;
	FloatVector edge;
	FloatVector buffer;
	int period;
	int method;
	int slowing;
	int event_trendchange;
	
protected:
	virtual void Start();
	
public:
	ParallelSymLREdge();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % symlr % edge % buffer;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

}

#endif
