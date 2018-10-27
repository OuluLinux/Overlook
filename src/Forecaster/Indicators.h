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


class BearsPower : public Core {
	int period;
	OnlineAverageWindow1 av;
	
public:
	BearsPower();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(3, 1)
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(3, 1)
			% Lbl(1)
			% Arg("period", period, 2, 127);
	}
};


class RelativeStrengthIndex : public Core {
	int period;
	
public:
	RelativeStrengthIndex();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(3, 1)
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(2, 2)
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(4, 2)
			% Lbl(1)
			% Arg("k_period", k_period, 2)
			% Arg("d_period", d_period, 2)
			% Arg("slowing", slowing, 2);
	}
};



class AcceleratorOscillator : public Core {
	OnlineAverageWindow1 av1, av2;
	
public:
	AcceleratorOscillator();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(5, 3)
			% Lbl(1);
	}
};



class AwesomeOscillator : public Core {
	OnlineAverageWindow1 av1, av2;
	
public:
	AwesomeOscillator();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(3, 3)
			% Lbl(1);
	}
};



class Psychological : public Core {
	int period;
	
public:
	Psychological();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 2, 127);
	}
};


class ChannelOscillator : public Core {
	int period;
	ExtremumCache ec;
	
public:
	ChannelOscillator();
	
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 2)
			% Mem(ec);
	}
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
		reg % Out(0, 0)
			% Lbl(1)
			% Mem(prev_counted)
			% Arg("cost_level", cost_level, 0);
	}
	
	void GetMinimalSignal(double cost, int begin, int end, bool* sigbuf, int sigbuf_size);
	
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
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



class TickBalanceOscillator : public Core {
	
protected:
	virtual void Start();
	
	int period = 10;
	
public:
	TickBalanceOscillator();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(1, 1)
			% Lbl(1)
			% Arg("period", period, 0, 1000)
			;
	}
};


















inline void AddDefaultDeclarations(Vector<FactoryDeclaration>& decl) {
	decl.Add().Set(System::Find<SimpleHurstWindow>());
	decl.Add().Set(System::Find<ParabolicSAR>());
	for(int i = 0; i < 12; i++) {
		decl.Add().Set(System::Find<Momentum>()).AddArg(2 << i);
		decl.Add().Set(System::Find<MovingAverage>()).AddArg(2 << i);
		//decl.Add().Set(System::Find<Pattern>()).AddArg(2 << i);
	}
	decl.Add().Set(System::Find<BearsPower>());
	decl.Add().Set(System::Find<CommodityChannelIndex>());
	decl.Add().Set(System::Find<RelativeStrengthIndex>());
	decl.Add().Set(System::Find<RelativeVigorIndex>());
	decl.Add().Set(System::Find<StochasticOscillator>());
	decl.Add().Set(System::Find<AcceleratorOscillator>());
	decl.Add().Set(System::Find<AwesomeOscillator>());
	decl.Add().Set(System::Find<Psychological>());
	decl.Add().Set(System::Find<ChannelOscillator>());
	decl.Add().Set(System::Find<ScissorChannelOscillator>());
	decl.Add().Set(System::Find<OnlineMinimalLabel>());
	decl.Add().Set(System::Find<Laguerre>());
	decl.Add().Set(System::Find<TickBalanceOscillator>());
}

}

#endif
