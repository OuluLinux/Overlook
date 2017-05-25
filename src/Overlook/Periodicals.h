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
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8)
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

class WdayHourForecastErrors : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> total, wdayhour, wday, hour;
	
public:
	WdayHourForecastErrors();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8)
			% Arg("period", var_period);
	}
};



class WdayHourErrorAdjusted : public Core {
	
public:
	WdayHourErrorAdjusted();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8);
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

class WeekStatsForecastErrors : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> week, month, quarter, year;
	
public:
	WeekStatsForecastErrors();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8)
			% Arg("period", var_period);
	}
};



class WeekStatsErrorAdjusted : public Core {
	bool skip;
	
public:
	WeekStatsErrorAdjusted();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, WeekStatsErrorAdjustedValue, SymTf, 8, 8);
	}
};


class WdayHourDiffWeek : public Core {
	
public:
	WdayHourDiffWeek();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InDynamic(IndiPhase, WeekStatsErrorAdjustedValue, &FilterFunction)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 2, 2);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym != -1)
			return in_sym == out_sym;
		BaseSystem& bs = *(BaseSystem*)basesystem;
		return bs.GetPeriod(out_tf) * bs.GetBasePeriod() == 7*24*60*60; // 1 week timeframe
	}
};



class WdayHourWeekAdjusted : public Core {
	
public:
	WdayHourWeekAdjusted();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% InDynamic(IndiPhase, WeekStatsErrorAdjustedValue, &FilterFunction)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 8, 8);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym != -1)
			return in_sym == out_sym;
		BaseSystem& bs = *(BaseSystem*)basesystem;
		return bs.GetPeriod(out_tf) * bs.GetBasePeriod() == 7*24*60*60; // 1 week timeframe
	}
};


class SubTfChanges : public Core {
	
protected:
	int var_period;
	int other_counted;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	
public:
	SubTfChanges();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction)
			% Out(IndiPhase, SubTfValue, SymTf, 4, 4)
			% Arg("period", var_period);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym != -1)
			return in_sym == out_sym;
		else
			return out_tf == in_tf + 1 || out_tf == in_tf;
	}
};


class MetaTfChanges : public Core {
	
protected:
	int offset;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfChanges();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, MetaTfValue, SymTf, 4, 4)
			% Arg("period", var_period);
	}
};



class HourHeat : public Core {
	
protected:
	Core *subtf, *metatf;
	
public:
	HourHeat();
	
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(SourcePhase, SubTfValue, SymTf)
			% In(SourcePhase, MetaTfValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 3, 3);
	}
};


class MetaTfCDF : public Core {
	
protected:
	int offset;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfCDF();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("offset", offset)
			% Arg("prev_open_time", prev_open_time)
			% Arg("prev_open", prev_open)
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
			% In(SourcePhase, WdayHourTrendingValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 5, 5)
			% Arg("period", var_period);
	}
};


class EventOsc : public Core {
	double mul;
	Index<String> keys;
	EventManager* emgr;
	int counted_events;
	
public:
	EventOsc();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 1, 1)
			% Arg("mul", mul);
	}
};



class GroupEventOsc : public Core {
	int feat_count;
	double mul, ownmul;
	
public:
	GroupEventOsc();
	
	virtual void Init();
	virtual void Start();
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 4)
			% Arg("mul", mul)
			% Arg("ownmul", ownmul);
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
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 4)
			% Arg("mul", mul);
	}
};



class GroupFeatureOsc : public Core {
	int feat_count;
	double mul, ownmul;
	
public:
	GroupFeatureOsc();
	
	virtual void Init();
	virtual void Start();
	
	
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, RealIndicatorValue, SymTf, 4, 4)
			% Arg("mul", mul)
			% Arg("ownmul", ownmul);
	}
};





class ChannelPredicter : public Core {
	
protected:
	
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetStyle() const;
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% Out(IndiPhase, ForecastChannelValue, SymTf)
			% Arg("length", length);
	}
	
	virtual void Init();
	virtual void Start();
	
};



}

#endif
