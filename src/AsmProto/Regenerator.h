#ifndef _AsmProto_Regenerator_h_
#define _AsmProto_Regenerator_h_







struct Regenerator {
	
	static const int pp_count = 1;
	static const int dim = 1;//pp_count * PP_COUNT;
	
	// Persistent
	Generator gen;
	int trains_total = 0;
	Optimizer opt, sizeopt;
	
	
	
	// Temp
	Vector<double> generated;
	Vector<double>* real_data;
	int begin = 0, end = 0;
	
	Regenerator();
	void Iterate();
	double GetDataError(int end);
	void ReadTrial(const Vector<double>& trial, int a_begin);
	void ReadTrialSize(const Vector<double>& trial, const Vector<int>& ids);
	
};



#endif
