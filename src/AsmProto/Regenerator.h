#ifndef _AsmProto_Regenerator_h_
#define _AsmProto_Regenerator_h_







struct Regenerator {
	
	static const int pp_count = 1;
	static const int dim = pp_count * PP_COUNT;
	
	// Persistent
	Generator gen;
	int trains_total = 0;
	Optimizer opt;
	
	
	
	// Temp
	Vector<double> generated;
	Vector<double>* real_data;
	int begin = 0, end = 0;
	
	Regenerator();
	void Iterate();
	double GetDataError();
	
};



#endif
