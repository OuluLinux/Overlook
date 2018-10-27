#ifndef _Forecaster_Regenerator_h_
#define _Forecaster_Regenerator_h_


namespace Forecast {



struct RegenResult : Moveable<RegenResult> {
	Vector<double> params;
	String heatmap;
	double err;
	int id, gen_id;
	
	RegenResult() {}
	RegenResult(const RegenResult& s) {*this = s;}
	
	void operator=(const RegenResult& s) {
		id = s.id;
		gen_id = s.gen_id;
		err = s.err;
		heatmap = s.heatmap;
		params <<= s.params;
	}
	
	void Serialize(Stream& s) {s % params % heatmap % err % id % gen_id;}
	bool operator()(const RegenResult& a, const RegenResult& b) const {return a.err < b.err;}
	
};

struct ForecastResult : Moveable<ForecastResult> {
	String heatmap;
	Vector<double> data;
	int id = -1;
	
	void Serialize(Stream& s) {s % heatmap % data % id;}
	bool operator()(const ForecastResult& a, const ForecastResult& b) const {return a.id < b.id;}
};

class Regenerator {
	

public:
	
	#ifdef flagDEBUG
	static const int popcount = 10;
	static const int max_dqniters = 1000;
	#else
	static const int popcount = 100;
	static const int max_dqniters = 10000000;
	#endif
	
	// Persistent
	DQNAgent dqn;
	VectorMap<int, NNSample> nnsamples;
	Vector<ForecastResult> forecasts;
	Vector<RegenResult> results;
	Vector<double> result_errors;
	Optimizer opt;
	double last_energy = 0;
	int dqn_iters = 0;
	
	// Temp
	Vector<double> real_data;
	Array<Generator> gen;
	Vector<double> err;
	Mutex result_lock;
	bool is_init = false;
	
	
	void RunOnce(int i);
	
public:
	
	typedef Regenerator CLASSNAME;
	Regenerator();
	void Init();
	void Iterate(int ms);
	void IterateNN(int ms);
	void RefreshNNSamples();
	void GetProgress(int& actual, int& total, String& state);
	void Forecast();
	
	double GetBestEnergy() {return opt.GetBestEnergy();}
	double GetLastEnergy() {return last_energy;}
	Generator& GetGenerator(int i) {return gen[i];}
	bool IsInit() {return is_init;}
	bool HasGenerators() const {return gen.GetCount();}
	void Serialize(Stream& s) {s % dqn % nnsamples % forecasts % results % result_errors % real_data % opt % last_energy % dqn_iters;}
	
};

}

#endif
