#ifndef _Overlook_Game_h_
#define _Overlook_Game_h_

namespace Overlook {

struct GameOpportunity {
	Time created;
	int vol_idx = -1, spread_idx = -1, trend_idx = -1, opp_idx = -1, bcase_idx = -1;
	int level = 0;
	bool signal;
	bool is_active = false;
	
	int GetElapsedMins() {return (GetUtcTime().Get() - created.Get()) / 60;}
	double GetAverageIndex() {return (vol_idx + spread_idx + trend_idx + opp_idx + bcase_idx) / 5.0;}
};

class Game {
	
public:
	Array<GameOpportunity> opps;
	GameOpportunity used;
	int prev_max_level = 0;
	int free_margin_scale = 1;
	int signal[USEDSYMBOL_COUNT];
	double free_margin_level = 0.6;
	
public:
	typedef Game CLASSNAME;
	Game();
	
	void Refresh();
	
};

inline Game& GetGame() {return Single<Game>();}

}

#endif
