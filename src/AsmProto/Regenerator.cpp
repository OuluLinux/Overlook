#include "AsmProto.h"

Regenerator::Regenerator() {
	opt.min_values.SetCount(dim);
	opt.max_values.SetCount(dim);
	int row = 0;
	for(int i = 0; i < pp_count; i++) {
		int j = i * PP_COUNT;
		opt.min_values[j + PP_LOW] = -gen.step*100;		opt.max_values[j + PP_LOW] = 0;
		opt.min_values[j + PP_HIGH] = 0;				opt.max_values[j + PP_HIGH] = +gen.step*100;
		opt.min_values[j + PP_MIN] = 0;					opt.max_values[j + PP_MIN] = 100;
		opt.min_values[j + PP_MAX] = 0;					opt.max_values[j + PP_MAX] = 200;
		opt.min_values[j + PP_SIZE] = 10;				opt.max_values[j + PP_SIZE] = 1000;
		opt.min_values[j + PP_ACTION] = 0;				opt.max_values[j + PP_ACTION] = 2;
		//opt.min_values[j + PP_ITER] = 0;				opt.max_values[j + PP_ITER] = Generator::data_count;
	}
	
	
}

void Regenerator::Iterate() {
	trains_total++;
	
	opt.Init(dim, 100);
	
	double pre_err = GetDataError();
	
	int a_begin = gen.a.src.GetCount();
	gen.a.src.SetCount(a_begin + pp_count);
	
	while (!opt.IsEnd()) {
		opt.Start();
		
		const Vector<double>& trial = opt.GetTrialSolution();
		
		
		
		for(int i = 0; i < pp_count; i++) {
			int j = i * PP_COUNT;
			PricePressure& pp = gen.a.src[a_begin + i];
			//int iter = trial[j + PP_ITER];
			//if (iter < 0) iter = 0;
			//if (iter >= generated.GetCount()) iter = generated.GetCount() - 1;
			int iter = begin;
			double price = (*real_data)[iter];
			pp.low  = price + trial[j + PP_LOW];
			pp.high = price + trial[j + PP_HIGH];
			pp.min = trial[j + PP_MIN];
			pp.max = trial[j + PP_MAX];
			pp.size = trial[j + PP_SIZE];
			pp.action = trial[j + PP_ACTION];
			pp.iter = iter;
		}
		gen.a.Sort();
		
		
		
		double err = GetDataError();
		
		double value = pre_err - err;
		
		opt.Stop(value);
	}
	
	
	const Vector<double>& trial = opt.GetBestSolution();
	for(int i = 0; i < pp_count; i++) {
		int j = i * PP_COUNT;
		PricePressure& pp = gen.a.src[a_begin + i];
		//int iter = trial[j + PP_ITER];
		//if (iter < 0) iter = 0;
		//if (iter >= generated.GetCount()) iter = generated.GetCount() - 1;
		int iter = begin;
		double price = (*real_data)[iter];
		pp.low  = price + trial[j + PP_LOW];
		pp.high = price + trial[j + PP_HIGH];
		pp.min = trial[j + PP_MIN];
		pp.max = trial[j + PP_MAX];
		pp.size = trial[j + PP_SIZE];
		pp.action = trial[j + PP_ACTION];
		pp.iter = iter;
	}
	gen.a.Sort();
	double err = GetDataError();
}

double Regenerator::GetDataError() {
	
	gen.GenerateData(generated, false, end + 100);
	Vector<double>& real = *real_data;

	ASSERT(generated.GetCount() == real.GetCount());

	double mse = 0;
	//for(int i = 0; i < generated.GetCount(); i++) {
	for(int i = begin; i < end; i++) {
		double g = generated[i];
		double r = real[i];
		double diff = g - r;
		//mse += diff * diff;
		mse += fabs(diff);
	}
	//mse /= generated.GetCount();
	mse /= end - begin;
	return mse;
}
