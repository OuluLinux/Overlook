#ifndef _Overlook_Periodicals_h_
#define _Overlook_Periodicals_h_


namespace Overlook {


class WdayHourChanges : public Pipe {
	
protected:
	Vector<OnlineVariance> wdayhour;
	
	FloatVector stddev, mean;
	
public:
	WdayHourChanges();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	const OnlineVariance& GetOnlineVariance(int shift);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class WdayHourStats : public Pipe {
	
protected:
	int var_period;
	MovingOnlineVariance total;
	Vector<MovingOnlineVariance> wdayhour, wday, hour;
	
	FloatVector t_pre_stddev, t_pre_mean;
	FloatVector h_pre_stddev, h_pre_mean;
	FloatVector d_pre_stddev, d_pre_mean;
	FloatVector dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourStats();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class WdayHourDiff : public Pipe {
	
protected:
	
	int var_period_fast, var_period_diff;
	FloatVector t_pre_stddev, t_pre_mean;
	FloatVector h_pre_stddev, h_pre_mean;
	FloatVector d_pre_stddev, d_pre_mean;
	FloatVector dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourDiff();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class WdayHourForecastErrors : public Pipe {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> total, wdayhour, wday, hour;
	
	FloatVector t_pre_stddev, t_pre_mean;
	FloatVector h_pre_stddev, h_pre_mean;
	FloatVector d_pre_stddev, d_pre_mean;
	FloatVector dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourForecastErrors();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class WdayHourErrorAdjusted : public Pipe {
	
protected:
	FloatVector t_pre_stddev, t_pre_mean;
	FloatVector h_pre_stddev, h_pre_mean;
	FloatVector d_pre_stddev, d_pre_mean;
	FloatVector dh_pre_stddev, dh_pre_mean;
	
	
public:
	WdayHourErrorAdjusted();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class WeekStats : public Pipe {
	
protected:
	int var_period;
	MovingOnlineVariance week;
	Vector<MovingOnlineVariance> month, quarter, year;
	
	FloatVector w_pre_stddev, w_pre_mean;
	FloatVector m_pre_stddev, m_pre_mean;
	FloatVector q_pre_stddev, q_pre_mean;
	FloatVector y_pre_stddev, y_pre_mean;
	
	
public:
	WeekStats();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class WeekStatsForecastErrors : public Pipe {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> week, month, quarter, year;
	
	FloatVector w_pre_stddev, w_pre_mean;
	FloatVector m_pre_stddev, m_pre_mean;
	FloatVector q_pre_stddev, q_pre_mean;
	FloatVector y_pre_stddev, y_pre_mean;
	
public:
	WeekStatsForecastErrors();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class WeekStatsErrorAdjusted : public Pipe {
	
protected:
	FloatVector w_pre_stddev, w_pre_mean;
	FloatVector m_pre_stddev, m_pre_mean;
	FloatVector q_pre_stddev, q_pre_mean;
	FloatVector y_pre_stddev, y_pre_mean;
	
public:
	WeekStatsErrorAdjusted();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class WdayHourDiffWeek : public Pipe {
	
protected:
	FloatVector mean, cdf;
	
public:
	WdayHourDiffWeek();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class WdayHourWeekAdjusted : public Pipe {
	
protected:
	FloatVector t_pre_stddev, t_pre_mean;
	FloatVector h_pre_stddev, h_pre_mean;
	FloatVector d_pre_stddev, d_pre_mean;
	FloatVector dh_pre_stddev, dh_pre_mean;
	
public:
	WdayHourWeekAdjusted();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class SubTfChanges : public Pipe {
	
protected:
	int other_counted;
	FloatVector stddev, mean, absstddev, absmean;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	/*Indicator* tf_week_shift;
	Pipe* other_values;
	*/
	
public:
	SubTfChanges();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class MetaTfChanges : public Pipe {
	
protected:
	int offset;
	FloatVector stddev, mean, absstddev, absmean;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour, abswdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfChanges();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class HourHeat : public Pipe {
	
protected:
	FloatVector sub, day, sum;
	Pipe *subtf, *metatf;
	
public:
	HourHeat();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class MetaTfCDF : public Pipe {
	
protected:
	int offset;
	FloatVector value;
	
	int var_period;
	Vector<MovingOnlineVariance> wdayhour;
	Time prev_open_time;
	double prev_open;
	
public:
	MetaTfCDF();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class WdayHourTrending : public Pipe {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> s1, s2, s3; // Shifts
	
	FloatVector shift1_mean, shift2_mean, shift3_mean, mean_av;
	FloatVector shift1_stddev, shift2_stddev, shift3_stddev, stddev_av;
	
public:
	WdayHourTrending();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

class WdayHourTrendSuccess : public Pipe {
	
protected:
	int var_period;
	Vector<MovingOnlineVariance> vars;
	
	FloatVector dir, mean, stddev, success, erradj;
	
public:
	WdayHourTrendSuccess();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class OpportinityQuality : public Pipe {
	
protected:
	Array<FloatVector> buffers;
	FloatVector quality;
	Vector<double> weights;
	
public:
	OpportinityQuality();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual void Start();
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};


class EdgeStatistics : public Pipe {
	
protected:
	FloatVector mean, dev;
	int period, method, slowing;
	//DataVar buf;
	int es_counted;
	Vector<OnlineVariance> vars;
	int event_probable_edge;
	
protected:
	virtual void Start();
	
public:
	EdgeStatistics();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % mean % dev % es_counted % vars;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

/*
	TideStatistics is simplified EdgeStatistics.
	The lowest periodical value means changing trend.
*/
class TideStatistics : public Pipe {
	
protected:
	FloatVector mean, dev;
	int period, ts_counted;
	Vector<OnlineVariance> vars;
	int event_probable_edge;
	
protected:
	virtual void Start();
	
public:
	TideStatistics();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % mean % dev % ts_counted % vars;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};

/*
	Disconnections is simplified version of EdgeStatistics and TideStatistics,
	which exports probable changing trend positions as disconnection positions.
*/
class Disconnections : public Pipe {
	
protected:
	FloatVector discbuf;
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
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % discbuf % dis_counted % prev_disc_pos;}
	
	virtual void Init();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class EventOsc : public Pipe {
	FloatVector info, low, med, high;
	double mul;
	Index<String> keys;
	EventManager* emgr;
	int counted_events;
	
public:
	EventOsc();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % info % low % med % high % counted_events;}
	
	virtual void Init();
	virtual void Start();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class GroupEventOsc : public Pipe {
	FloatVector info, low, med, high;
	int feat_count;
	double mul, ownmul;
	
public:
	GroupEventOsc();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % info % low % med % high;}
	
	virtual void Init();
	virtual void Start();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};




class FeatureOsc : public Pipe {
	FloatVector info, low, med, high;
	double mul;
	FeatureDetector* feat;
	int counted_features;
	
public:
	FeatureOsc();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % info % low % med % high % counted_features;}
	
	virtual void Init();
	virtual void Start();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};



class GroupFeatureOsc : public Pipe {
	FloatVector info, low, med, high;
	int feat_count;
	double mul, ownmul;
	
public:
	GroupFeatureOsc();
	
	virtual void Serialize(Stream& s) {Pipe::Serialize(s); s % info % low % med % high;}
	
	virtual void Init();
	virtual void Start();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		//in.Add(ValueType(, , SymTf, 1));
		//out.Add(ValueType(, , SymTf, 1));
	}
};





class ChannelPredicter : public Pipe {
	
protected:
	
	int length;
	
public:
	ChannelPredicter();
	
	virtual String GetStyle() const;
	static void GetIO(Vector<ValueType>& in, Vector<ValueType>& in_opt, Vector<ValueType>& out) {
		in.Add(ValueType(IndiPhase, ForecastChangeValue, SymTf, 1));
		in.Add(ValueType(IndiPhase, ForecastChannelValue, SymTf, 1));
		out.Add(ValueType(IndiPhase, ForecastChannelValue, SymTf, 1));
	}
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual bool Process(const CoreProcessAttributes& attr);
	
};



}

#endif
