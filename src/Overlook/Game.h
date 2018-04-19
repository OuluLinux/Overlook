#ifndef _Overlook_Game_h_
#define _Overlook_Game_h_

namespace Overlook {

struct GameOpportunity {
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

class Game {
	
public:
	SimBroker sb;
	double free_margin_level = 0.8;
	double tplimit = DEF_TP, sllimit = DEF_SL;
	int ma1 = 240, ma2 = 480;
	int free_margin_scale = 3;
	int timelimit = DEF_TIMELIMIT;
	int max_symbols = USEDSYMBOL_COUNT*2;
	int spread_limit = USEDSYMBOL_COUNT;
	bool autostart = true;
	bool inversesig = false;
	
	OnlineAverageWindow1 ma1_av, ma2_av;
	Array<GameOpportunity> opps;
	Vector<double> sb_equity, sb_ma1, sb_ma2;
	int prev_max_level = 0;
	int signal[USEDSYMBOL_COUNT];
	bool allow_real = false;
	Mutex lock;
	
public:
	typedef Game CLASSNAME;
	Game();
	
	void Refresh();
	
};

inline Game& GetGame() {return Single<Game>();}

}

#endif
