#ifndef _Overlook_Realtime_h_
#define _Overlook_Realtime_h_

namespace Overlook {

struct RealtimeOpportunity {
	Vector<double> open_profits;
	Time opened;
	double profit = 0.0;
	double prev_lots = 0.0;
	int vol_idx = -1, spread_idx = -1, trend_idx = -1, opp_idx = -1, bcase_idx = -1;
	int level = 0;
	bool signal;
	bool is_active = false;
	
	double GetAverageIndex() {return (vol_idx + spread_idx + trend_idx + opp_idx + bcase_idx) / 5.0;}
};

class Realtime {
	
public:
	SimBroker sb;
	double free_margin_level = 0.9;
	double tplimit = DEF_TP, sllimit = DEF_SL;
	int ma1 = 480, ma2 = 960;
	int free_margin_scale = 3;
	int timelimit = DEF_TIMELIMIT;
	int max_symbols = USEDSYMBOL_COUNT;
	int spread_limit = 2;
	bool autostart = true;
	bool inversesig = false;
	
	OnlineAverageWindow1 ma1_av, ma2_av;
	Array<RealtimeOpportunity> opps;
	Vector<double> sb_equity, sb_ma1, sb_ma2;
	int prev_max_level = 0;
	int signal[USEDSYMBOL_COUNT];
	bool allow_real = false;
	Mutex lock, ma_lock;
	
public:
	typedef Realtime CLASSNAME;
	Realtime();
	
	void Refresh();
	
};

inline Realtime& GetRealtime() {return Single<Realtime>();}

}

#endif
