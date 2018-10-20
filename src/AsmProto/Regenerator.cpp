#include "AsmProto.h"

Regenerator::Regenerator() {
	/*opt.min_values.SetCount(dim);
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
	*/
	
}

void Regenerator::Iterate() {
	/*trains_total++;
	
	bool is_fail = false;
	double pre_err = GetDataError(begin);
	
	gen.GenerateData(generated, false, begin);
	Vector<int> active_ids, sizes;
	for(int i = 0; i < gen.active_pressures.GetCount(); i++) {
		const PricePressure& pp = gen.active_pressures[i];
		active_ids.Add(pp.id);
		sizes.Add(pp.size);
	}
	
	int a_begin = gen.a.src.GetCount();
	for(int i = 0; i < pp_count; i++)
		gen.a.Add();
	
	
	opt.Init(dim, 100);
	
	while (!opt.IsEnd()) {
		opt.Start();
		
		ReadTrial(opt.GetTrialSolution(), a_begin);
		gen.a.Sort();
		
		double err = GetDataError(end);
		double value = pre_err - err;
		opt.Stop(value);
	}
	
	
	
	if (!gen.a.src.IsEmpty()) {
		
		for(int i = 0; i < pp_count; i++)
			gen.a.src.Pop();
		
		int size_dim = active_ids.GetCount();
		sizeopt.min_values.SetCount(size_dim);
		sizeopt.max_values.SetCount(size_dim);
		for(int i = 0; i < active_ids.GetCount(); i++) {
			sizeopt.min_values[i] = 10;
			sizeopt.max_values[i] = 1000;
		}
		sizeopt.Init(size_dim, 100);
		
		while (!sizeopt.IsEnd()) {
			sizeopt.Start();
			
			ReadTrialSize(sizeopt.GetTrialSolution(), active_ids);
			gen.a.Sort();
			
			double err = GetDataError(end);
			double value = pre_err - err;
			sizeopt.Stop(value);
		}
		
		if (sizeopt.GetBestEnergy() >= opt.GetBestEnergy()) {
			ReadTrialSize(sizeopt.GetBestSolution(), active_ids);
			if (sizeopt.GetBestEnergy() < 0) {
				
				// Restore sizes
				for(int i = 0; i < active_ids.GetCount(); i++)
					gen.a.src.Get(active_ids[i]).size = sizes[i];
				
				//is_fail = true;
			}
		}
		else {
			for(int i = 0; i < pp_count; i++)
				gen.a.Add();
			// Restore sizes
			for(int i = 0; i < active_ids.GetCount(); i++)
				gen.a.src.Get(active_ids[i]).size = sizes[i];
			ReadTrial(opt.GetBestSolution(), a_begin);
			if (opt.GetBestEnergy() < 0) is_fail = true;
		}
	}
	else {
		ReadTrial(opt.GetBestSolution(), a_begin);
		if (opt.GetBestEnergy() < 0) is_fail = true;
	}
	if (is_fail)
		for(int i = 0; i < pp_count; i++)
			gen.a.src.Pop();
	gen.a.Sort();
	double err = GetDataError(end);*/
}

void Regenerator::ReadTrial(const Vector<double>& trial, int a_begin) {
	/*for(int i = 0; i < pp_count; i++) {
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
	}*/
}

void Regenerator::ReadTrialSize(const Vector<double>& trial, const Vector<int>& ids) {
	/*for(int i = 0; i < trial.GetCount(); i++) {
		PricePressure& pp1 = gen.a.src.Get(ids[i]);
		pp1.size = trial[i];
	}*/
}

double Regenerator::GetDataError(int end) {
	/*int begin = 0; 
	
	gen.GenerateData(generated, false, end);
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
	return mse;*/
	return 0;
}
