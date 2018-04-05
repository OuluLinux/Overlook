#ifndef _Overlook_Game_h_
#define _Overlook_Game_h_

namespace Overlook {

struct GameOpportunity {
	Time created;
	int vol_idx = -1, spread_idx = -1, trend_idx = -1;
	int level = 0;
	bool signal;
	
	int GetElapsedMins() {return (GetUtcTime().Get() - created.Get()) / 60;}
	double GetAverageIndex() {return (vol_idx + spread_idx + trend_idx) / 3.0;}
};

class Game {
	
public:
	Array<GameOpportunity> opps;
	GameOpportunity used;
	
public:
	typedef Game CLASSNAME;
	Game();
	
	void Refresh();
	
};

inline Game& GetGame() {return Single<Game>();}

}

#endif
