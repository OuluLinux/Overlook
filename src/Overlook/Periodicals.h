#ifndef _Overlook_Periodicals_h_
#define _Overlook_Periodicals_h_


namespace Overlook {


class WdayHourChanges : public Core {
	
protected:
	Vector<OnlineVariance> wdayhour;
	
	Vector<double> stddev, mean;
	
public:
	WdayHourChanges();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	const OnlineVariance& GetOnlineVariance(int shift);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class WdayHourStats : public Core {
	
protected:
	int var_period;
	MovingOnlineVariance total;
	Vector<MovingOnlineVariance> wdayhour, wday, hour;
	
	Vector<double> t_pre_stddev, t_pre_mean;
	Vector<double> h_pre_stddev, h_pre_mean;
	Vector<double> d_pre_stddev, d_pre_mean;
	Vector<double> dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourStats();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class WdayHourDiff : public Core {
	
protected:
	
	int var_period_fast, var_period_diff;
	Vector<double> t_pre_stddev, t_pre_mean;
	Vector<double> h_pre_stddev, h_pre_mean;
	Vector<double> d_pre_stddev, d_pre_mean;
	Vector<double> dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourDiff();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class WdayHourForecastErrors : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> total, wdayhour, wday, hour;
	
	Vector<double> t_pre_stddev, t_pre_mean;
	Vector<double> h_pre_stddev, h_pre_mean;
	Vector<double> d_pre_stddev, d_pre_mean;
	Vector<double> dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourForecastErrors();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class WdayHourErrorAdjusted : public Core {
	
protected:
	Vector<double> t_pre_stddev, t_pre_mean;
	Vector<double> h_pre_stddev, h_pre_mean;
	Vector<double> d_pre_stddev, d_pre_mean;
	Vector<double> dh_pre_stddev, dh_pre_mean;
	
	
public:
	WdayHourErrorAdjusted();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class WeekStats : public Core {
	
protected:
	int var_period;
	MovingOnlineVariance week;
	Vector<MovingOnlineVariance> month, quarter, year;
	
	Vector<double> w_pre_stddev, w_pre_mean;
	Vector<double> m_pre_stddev, m_pre_mean;
	Vector<double> q_pre_stddev, q_pre_mean;
	Vector<double> y_pre_stddev, y_pre_mean;
	
	
public:
	WeekStats();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class WeekStatsForecastErrors : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> week, month, quarter, year;
	
	Vector<double> w_pre_stddev, w_pre_mean;
	Vector<double> m_pre_stddev, m_pre_mean;
	Vector<double> q_pre_stddev, q_pre_mean;
	Vector<double> y_pre_stddev, y_pre_mean;
	
public:
	WeekStatsForecastErrors();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class WeekStatsErrorAdjusted : public Core {
	
protected:
	Vector<double> w_pre_stddev, w_pre_mean;
	Vector<double> m_pre_stddev, m_pre_mean;
	Vector<double> q_pre_stddev, q_pre_mean;
	Vector<double> y_pre_stddev, y_pre_mean;
	
public:
	WeekStatsErrorAdjusted();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class WdayHourDiffWeek : public Core {
	
protected:
	Vector<double> mean, cdf;
	
public:
	WdayHourDiffWeek();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class WdayHourWeekAdjusted : public Core {
	
protected:
	Vector<double> t_pre_stddev, t_pre_mean;
	Vector<double> h_pre_stddev, h_pre_mean;
	Vector<double> d_pre_stddev, d_pre_mean;
	Vector<double> dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourWeekAdjusted();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class SubTfChanges : public Core {
	
protected:
	int other_counted;
	Vector<double> stddev, mean, absstddev, absmean;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	/*Indicator* tf_week_shift;
	Core* other_values;
	*/
	
public:
	SubTfChanges();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		reg.AddOut(IndiPhase, SubTfValue, SymTf, 4, 4);
	}
};


class MetaTfChanges : public Core {
	
protected:
	int offset;
	Vector<double> stddev, mean, absstddev, absmean;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfChanges();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class HourHeat : public Core {
	
protected:
	Vector<double> sub, day, sum;
	Core *subtf, *metatf;
	
public:
	HourHeat();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class MetaTfCDF : public Core {
	
protected:
	int offset;
	Vector<double> value;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfCDF();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class WdayHourTrending : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> s1, s2, s3; // Shifts
	
	Vector<double> shift1_mean, shift2_mean, shift3_mean, mean_av;
	Vector<double> shift1_stddev, shift2_stddev, shift3_stddev, stddev_av;
	
public:
	WdayHourTrending();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

class WdayHourTrendSuccess : public Core {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> vars;
	
	Vector<double> dir, mean, stddev, success, erradj;
	
public:
	WdayHourTrendSuccess();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class OpportinityQuality : public Core {
	
protected:
	Array<Vector<double>> buffers;
	Vector<double> quality;
	Vector<double> weights;
	
public:
	OpportinityQuality();
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};


class EdgeStatistics : public Core {
	
protected:
	Vector<double> mean, dev;
	int period, method, slowing;
	//DataVar buf;
	int es_counted;
	Vector<OnlineVariance> vars;
	int event_probable_edge;
	
protected:
	virtual void Start();
	
public:
	EdgeStatistics();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % mean % dev % es_counted % vars;}
	
	virtual void Init();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

/*
	TideStatistics is simplified EdgeStatistics.
	The lowest periodical value means changing trend.
*/
class TideStatistics : public Core {
	
protected:
	Vector<double> mean, dev;
	int period, ts_counted;
	Vector<OnlineVariance> vars;
	int event_probable_edge;
	
protected:
	virtual void Start();
	
public:
	TideStatistics();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % mean % dev % ts_counted % vars;}
	
	virtual void Init();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};

/*
	Disconnections is simplified version of EdgeStatistics and TideStatistics,
	which exports probable changing trend positions as disconnection positions.
*/
class Disconnections : public Core {
	
protected:
	Vector<double> discbuf;
	int period;
	//DataVar buf1;
	//DataVar buf2;
	int dis_counted;
	int event_disconnection;
	int prev_disc_pos;
	
protected:
	virtual void Start();
	
public:
	Disconnections();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % discbuf % dis_counted % prev_disc_pos;}
	
	virtual void Init();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class EventOsc : public Core {
	Vector<double> info, low, med, high;
	double mul;
	Index<String> keys;
	EventManager* emgr;
	int counted_events;
	
public:
	EventOsc();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % info % low % med % high % counted_events;}
	
	virtual void Init();
	virtual void Start();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class GroupEventOsc : public Core {
	Vector<double> info, low, med, high;
	int feat_count;
	double mul, ownmul;
	
public:
	GroupEventOsc();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % info % low % med % high;}
	
	virtual void Init();
	virtual void Start();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};




class FeatureOsc : public Core {
	Vector<double> info, low, med, high;
	double mul;
	FeatureDetector* feat;
	int counted_features;
	
public:
	FeatureOsc();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % info % low % med % high % counted_features;}
	
	virtual void Init();
	virtual void Start();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};



class GroupFeatureOsc : public Core {
	Vector<double> info, low, med, high;
	int feat_count;
	double mul, ownmul;
	
public:
	GroupFeatureOsc();
	
	virtual void Serialize(Stream& s) {Core::Serialize(s); s % info % low % med % high;}
	
	virtual void Init();
	virtual void Start();
	virtual void Arguments(ArgumentBase& args);
	
	virtual void GetIO(ValueRegister& reg) {
		//reg.AddIn(, , SymTf);
		//reg.AddOut(, , SymTf);
	}
};





class ChannelPredicter : public Core {
	
protected:
	
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetStyle() const;
	virtual void GetIO(ValueRegister& reg) {
		/*reg.AddIn(IndiPhase, ForecastChangeValue, SymTf);
		reg.AddIn(IndiPhase, ForecastChannelValue, SymTf);*/
		reg.AddOut(IndiPhase, ForecastChannelValue, SymTf);
	}
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	virtual void Start();
	
};



}

#endif
