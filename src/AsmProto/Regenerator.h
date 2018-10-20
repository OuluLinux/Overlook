#ifndef _AsmProto_Regenerator_h_
#define _AsmProto_Regenerator_h_





static const int CUDA_CORES = 6;



struct Regenerator {
	
	
	// Persistent
	Generator gen[CUDA_CORES];
	int trains_total = 0;
	Optimizer<ARG_COUNT, CUDA_CORES> opt;
	double last_energy = 0;
	
	
	// Temp
	double* real_data;
	
	Regenerator();
	void Iterate();
	
};



#endif
