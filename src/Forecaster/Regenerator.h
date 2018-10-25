#ifndef _Forecaster_Regenerator_h_
#define _Forecaster_Regenerator_h_


namespace Forecast {



struct RegenResult : Moveable<RegenResult> {
	Vector<double> params;
	String heatmap;
	double err;
	int id, gen_id;
	
	
	
	void operator=(const RegenResult& s) {
		id = s.id;
		gen_id = s.gen_id;
		err = s.err;
		heatmap = s.heatmap;
		params <<= s.params;
	}
};

class Regenerator {
	

protected:
	friend class Session;
	friend class DrawLines;
	friend class RegeneratorCtrl;
	
	
	// Persistent
	Vector<RegenResult> results;
	Vector<double> result_errors;
	Optimizer opt;
	double last_energy = 0;
	
	
	// Temp
	Array<Generator> gen;
	Vector<double> real_data;
	Vector<double> data;
	Vector<double> err;
	Mutex result_lock;
	
	
	void RunOnce(int i);
	
public:
	
	typedef Regenerator CLASSNAME;
	Regenerator();
	void Iterate();
	
	double GetBestEnergy() {return opt.GetBestEnergy();}
	double GetLastEnergy() {return last_energy;}
	Generator& GetGenerator(int i) {return gen[i];}
	
};

}

#endif
