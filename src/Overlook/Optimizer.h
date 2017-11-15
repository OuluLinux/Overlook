#ifndef _Overlook_Optimizer_h_
#define _Overlook_Optimizer_h_

namespace Overlook {


class MixerOptimizer {
	int round = 0;
	int arraycount = 0, count = 0;
	int population = 1000;
	int generations = 1000;
	int max_rounds = 1000000;
	
public:
	typedef MixerOptimizer CLASSNAME;
	MixerOptimizer() {}
	
	bool IsEnd() const {return round >= max_rounds;}
	
	void SetArrayCount(int i);
	void SetCount(int i);
	void SetPopulation(int i);
	void SetMaxGenerations(int i);
	void UseLimits();
	void Set(int col, double min, double max, double step, const char* desc);
	
	void Serialize(Stream& s) {
		s % round % arraycount % count % population % generations % max_rounds;
	}
};


}

#endif
