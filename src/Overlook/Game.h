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
	bool slow_signal;
	bool is_active = false;
	
	double GetAverageIndex() {return (vol_idx + spread_idx + trend_idx + opp_idx + bcase_idx) / 5.0;}
};

class Game {
	
public:
	double free_margin_level = 0.9;
	double tplimit = 0.5, sllimit = -0.8;
	int free_margin_scale = 3;
	int timelimit = 10;
	int max_symbols = 4;
	int spread_limit = 5;
	bool autostart = true;
	
	Array<GameOpportunity> opps;
	int prev_max_level = 0;
	int signal[USEDSYMBOL_COUNT];
	Mutex lock;
	
public:
	typedef Game CLASSNAME;
	Game();
	
	void Refresh();
	
};

inline Game& GetGame() {return Single<Game>();}

}

#endif
