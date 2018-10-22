#ifndef _Forecaster_Indicators_h_
#define _Forecaster_Indicators_h_

namespace Forecast {



class SimpleHurstWindow : public Core {
	int period = 10;
	
	static constexpr double koef = 1.253314;
	
protected:
	virtual void Start();
	
	double GetSimpleHurst(int i, int period);
	
public:
	SimpleHurstWindow();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 1);
	}
};


class Momentum : public Core {
	int period, shift;
	
public:
	Momentum();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 2);
	}
};


enum {MODE_SMA, MODE_EMA, MODE_SMMA, MODE_LWMA};
enum {MODE_SIMPLE, MODE_EXPONENTIAL, MODE_SMOOTHED, MODE_LINWEIGHT};

class MovingAverage : public Core {
	int ma_period;
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
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", ma_period, 2)
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
		reg % Out(3, 1)
			% Lbl(1)
			% Arg("step", step, 2, 127)
			% Arg("maximum", maximum, 2, 127)
			% Mem(last_rev_pos)
			% Mem(direction_long);
	}
};

class Pattern : public Core {
	int period;
	
	static const int pattern_count = 6;
	Vector<Point> pattern;
	
	Vector<int32> descriptors;
	
	void RandomizePattern();
	int32 GetDescriptor(int pos);
public:
	Pattern();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 2)
			% Mem(pattern);
	}
};

}

#endif
