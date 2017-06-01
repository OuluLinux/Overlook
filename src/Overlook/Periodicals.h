#if 0

#ifndef _Overlook_Periodicals_h_
#define _Overlook_Periodicals_h_


namespace Overlook {



class WdayHourChanges : public Core {
	
protected:
	Vector<OnlineVariance> wdayhour;
	
public:
	WdayHourChanges();
	
	virtual void Init();
	virtual void Start();
	
	const OnlineVariance& GetOnlineVariance(int shift);
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
};

class WdayHourStats : public Core {
	
protected:
	int var_period;
	MovingOnlineVariance total;
	Vector<MovingOnlineVariance> wdayhour, wday, hour;
	
public:
	WdayHourStats();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, WdayHourStatsValue, SymTf, 8, 8)
			% Arg("period", var_period);
	}
};

class WdayHourDiff : public Core {
	
protected:
	int var_period_fast, var_period_diff;
	
public:
	WdayHourDiff();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8)
			% Arg("period_fast", var_period_fast)
			% Arg("period_diff", var_period_diff);
	}
};


class WeekStats : public Core {
	
protected:
	int var_period;
	MovingOnlineVariance week;
	Vector<MovingOnlineVariance> month, quarter, year;
	
public:
	WeekStats();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8)
			% Arg("period", var_period);
	}
};


class WdayHourTrending : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> s1, s2, s3; // Shifts
	
public:
	WdayHourTrending();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, WdayHourTrendingValue, SymTf, 8, 8)
			% Arg("period", var_period);
	}
};

class WdayHourTrendSuccess : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> vars;
	
public:
	WdayHourTrendSuccess();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(IndiPhase, WdayHourTrendingValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 5, 5)
			% Arg("period", var_period);
	}
};




class FeatureOsc : public Core {
	double mul;
	FeatureDetector* feat;
	int counted_features;
	
public:
	FeatureOsc();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(IndiPhase, FeatureValue, SymTf)
			% Out(IndiPhase, FeatureOscValue, SymTf, 4, 4)
			% Arg("mul", mul);
	}
};




class ChannelPredicter : public Core {
	
public:
	ChannelPredicter();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(IndiPhase, WdayHourStatsValue, SymTf)
			% Out(IndiPhase, ForecastChannelValue, SymTf, 7, 7);
	}
	
	virtual void Init();
	virtual void Start();
	
};



}

#endif
#endif
