#include "AsmProto.h"

Regenerator::Regenerator() {
	
	seeker.Init(1, SSENS_COUNT, SEEKER_COUNT);
	seeker.Reset();
	adjuster.Init(1, ASENS_COUNT, ADJ_COUNT);
	adjuster.Reset();
}

void Regenerator::Train() {
	
	for(int i = 0; i < seeker_train_count; i++) {
		trains_total++;
		
		
		GetSeekerSensors(slist);
		
		int act = seeker.Act(slist);
		
		double reward = 0.0;
		double a = 0, b = 0;
		
		switch (act) {
			case SEEKER_BWD:
				seeker_state.speed--;
				break;
			case SEEKER_FWD:
				seeker_state.speed++;
				break;
			case SEEKER_BWDOBJ:
				seeker_state.snap_speed--;
				break;
			case SEEKER_FWDOBJ:
				seeker_state.snap_speed++;
				break;
			case SEEKER_OBJITER:
				ObjIterToIter();
				break;
			case SEEKER_ITEROBJ:
				IterToObjIter();
				break;
			case SEEKER_NEWOBJ:
				a = GetDataError();
				IterToObjIter();
				gen.Randomize(gen.a.src.Insert(seeker_state.obj_iter), (*real_data)[seeker_state.iter], seeker_state.iter);
				TrainAdjuster();
				b = GetDataError();
				reward = (a - b) * 100000;
				break;
			case SEEKER_EDITOBJ:
				a = GetDataError();
				TrainAdjuster();
				b = GetDataError();
				reward = (a - b) * 100000;
				break;
			case SEEKER_REMOBJ:
				a = GetDataError();
				if (seeker_state.obj_iter >= 0 && seeker_state.obj_iter < gen.a.src.GetCount())
					gen.a.src.Remove(seeker_state.obj_iter);
				b = GetDataError();
				reward = (a - b) * 100000;
				break;
			case SEEKER_RANDLOC:
				seeker_state.iter = Random(real_data->GetCount());
				break;
		}
		
		seeker_state.iter += seeker_state.speed;
		if (seeker_state.iter < 0) {
			seeker_state.iter = 0;
			seeker_state.speed = 0;
		}
		else if (seeker_state.iter >= generated.GetCount()) {
			seeker_state.iter = generated.GetCount() - 1;
			seeker_state.speed = 0;
		}
		
		seeker_state.obj_iter += seeker_state.snap_speed;
		if (seeker_state.obj_iter < 0) {
			seeker_state.obj_iter = 0;
			seeker_state.snap_speed = 0;
		}
		else if (seeker_state.obj_iter >= gen.a.src.GetCount()) {
			seeker_state.obj_iter = gen.a.src.GetCount() - 1;
			seeker_state.snap_speed = 0;
		}
		
		
		seeker.Learn(reward);
	}
	
	
	// TODO: clean completely detached pricepressures sometimes
	
}

void Regenerator::ObjIterToIter() {
	if (gen.a.src.IsEmpty())
		return;
	seeker_state.iter = gen.a.src[seeker_state.obj_iter].iter;
}

void Regenerator::IterToObjIter() {
	int min = INT_MAX;
	int pos = 0;
	for(int i = 0; i < gen.a.src.GetCount(); i++) {
		const PricePressure& pp = gen.a.src[i];
		int dist = abs(pp.iter - seeker_state.iter);
		if (dist < min) {
			min = dist;
			pos = i;
		}
	}
	seeker_state.obj_iter = pos;
}
	
void Regenerator::GetSeekerSensors(Vector<double>& slist) {
	slist.SetCount(SSENS_COUNT, 0);
	
	double r = (*real_data)[seeker_state.iter];
	double g = generated[seeker_state.iter];
	double diff = fabs(r - g);
	
	bool is_obj = false;
	if (seeker_state.obj_iter >= 0 && seeker_state.obj_iter < gen.a.src.GetCount()) {
		const PricePressure& pp = gen.a.src[seeker_state.obj_iter];
		is_obj = pp.iter == seeker_state.iter;
	}
	slist[SSENS_DIFF] = diff / gen.step;
	slist[SSENS_ISOBJ] = is_obj ? 1.0 : 0.0;
	slist[SSENS_VEL] = seeker_state.speed;
	slist[SSENS_OBJVEL] = seeker_state.snap_speed;
	slist[SSENS_ITER] = (double)seeker_state.iter / (double)generated.GetCount();
	slist[SSENS_OBJITER] = (double)seeker_state.obj_iter / (double)gen.a.src.GetCount();
}

double Regenerator::TrainAdjuster() {
	if (gen.a.src.IsEmpty())
		return 0.0;
	
	PricePressure& pp = gen.a.src[seeker_state.obj_iter];
	double price_step = gen.step;
	double pres_step = 1.0;
	double size_step = 1.0;
	
	double prev_err = GetDataError();
	double reward_sum = 0.0;
	int reward_count = 0;
	while (true) {
		
		GetAdjusterSensors(slist);
		
		int act = adjuster.Act(slist);
		bool do_exit = false;
		
		switch (act) {
			case ADJ_INCLOW:
				pp.low += price_step;
				break;
			case ADJ_DECLOW:
				pp.low -= price_step;
				break;
			case ADJ_INCHIGH:
				pp.high += price_step;
				break;
			case ADJ_DECHIGH:
				pp.high -= price_step;
				break;
			case ADJ_INCMIN:
				pp.min += pres_step;
				break;
			case ADJ_DECMIN:
				pp.min -= pres_step;
				break;
			case ADJ_INCMAX:
				pp.max += pres_step;
				break;
			case ADJ_DECMAX:
				pp.max -= pres_step;
				break;
			case ADJ_INCSIZE:
				pp.size += size_step;
				break;
			case ADJ_DECSIZE:
				pp.size -= size_step;
				break;
			case ADJ_TOGGLEACTION:
				pp.action = !pp.action;
				break;
			case ADJ_EXIT:
				do_exit = true;
				break;
		}
		
		
		
		double err = GetDataError();
		
		double reward = (prev_err - err) * 100000;
		
		adjuster.Learn(reward);
		
		reward_sum += reward;
		reward_count++;
		if (do_exit) break;
	}
	
	return reward_sum / reward_count;
}

void Regenerator::GetAdjusterSensors(Vector<double>& slist) {
	slist.SetCount(ASENS_COUNT, 0);
	
	PricePressure& pp = gen.a.src[seeker_state.obj_iter];
	double mid = (pp.low + pp.high) * 0.5;
	double midlow = mid - pp.low;
	double midhigh = pp.high - mid;
	//double price = (real_data*)[pp.iter];
	double price = (*real_data)[seeker_state.iter];
	double midpricediff = mid - price;
	slist[ASENS_MIDLOW] = midlow;
	slist[ASENS_MIDHIGH] = midhigh;
	slist[ASENS_MIDPRICEDIFF] = midpricediff;
	slist[ASENS_MIN] = pp.min;
	slist[ASENS_MAX] = pp.max;
	slist[ASENS_SIZE] = pp.size;
	slist[ASENS_ACTION] = pp.action ? 1.0 : 0.0;
}

double Regenerator::GetDataError() {
	gen.GenerateData(generated, false);
	Vector<double>& real = *real_data;
	
	ASSERT(generated.GetCount() == real.GetCount());
	
	double mse = 0;
	for(int i = 0; i < generated.GetCount(); i++) {
		double g = generated[i];
		double r = real[i];
		double diff = g - r;
		mse += diff * diff;
	}
	mse /= generated.GetCount();
	return mse;
}

