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
	
	bool operator()(const RegenResult& a, const RegenResult& b) const {return a.err < b.err;}
	
};

struct ForecastResult : Moveable<ForecastResult> {
	String heatmap;
	Vector<double> data;
};

class Regenerator {
	

protected:
	friend class Session;
	friend class DrawLines;
	friend class RegeneratorCtrl;
	friend class ForecastCtrl;
	
	static const int popcount = 100;
	
	// Persistent
	Vector<ForecastResult> forecasts;
	Vector<RegenResult> results;
	Vector<double> result_errors;
	Optimizer opt;
	double last_energy = 0;
	bool is_init = false;
	
	// Temp
	Array<Generator> gen;
	Vector<double> real_data;
	Vector<double> data;
	Vector<double> err;
	Mutex result_lock;
	
	
	void RunOnce(int i);
	void ForecastOnce(int i);
	
public:
	
	typedef Regenerator CLASSNAME;
	Regenerator();
	void Init();
	void Iterate(int ms);
	void Forecast();
	
	double GetBestEnergy() {return opt.GetBestEnergy();}
	double GetLastEnergy() {return last_energy;}
	Generator& GetGenerator(int i) {return gen[i];}
	bool IsInit() {return is_init;}
	
};

}

#endif
