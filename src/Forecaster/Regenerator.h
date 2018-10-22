#ifndef _Forecaster_Regenerator_h_
#define _Forecaster_Regenerator_h_


namespace Forecast {


static const int POPCOUNT = 100;



struct Regenerator {
	
	
	// Persistent
	Generator gen[POPCOUNT];
	int trains_total = 0;
	Optimizer opt;
	double last_energy = 0;
	
	
	// Temp
	Vector<double>* real_data = NULL;
	double err[POPCOUNT];
	Vector<double> data;
	
	
	Regenerator();
	void Iterate();
	
};

}

#endif
