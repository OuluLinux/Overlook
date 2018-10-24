#ifndef _Forecaster_Regenerator_h_
#define _Forecaster_Regenerator_h_


namespace Forecast {





struct Regenerator {
	
	
	// Persistent
	Optimizer opt;
	double last_energy = 0;
	
	
	// Temp
	Vector<double>* real_data = NULL;
	Vector<double> data;
	Array<Generator> gen;
	
	Regenerator();
	void Iterate();
	
};

}

#endif
