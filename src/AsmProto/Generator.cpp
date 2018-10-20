#include "AsmProto.h"


Generator::Generator() {
	a.Init(data_count, 0.0, 2.0, step);
	InitMA();
	RandomizePatterns();
}

void Generator::InitMA() {
	ma.SetCount(12);
	prev_ma_mean.SetCount(ma.GetCount(), 1.0);
	for(int i = 0; i < ma.GetCount(); i++) {
		ma[i].SetPeriod(2 << i);
	}
}

void Generator::RandomizePatterns() {
	pattern.SetCount(pattern_count);
	for(int i = 0; i < pattern_count; i++) {
		int period = 4 << i;
		Vector<Point>& pattern = this->pattern[i];
		pattern.SetCount(32);
		for(int j = 0; j < 32; j++) {
			pattern[j].x = Random(period);
			pattern[j].y = Random(period);
		}
	}
}

void Generator::AddRandomPressure() {
	double low  = price - (1 + Random(10)) * step;
	double high = price + (1 + Random(10)) * step;
	double min = -1 - (int)Random(10);
	double max =  1 + (int)Random(10);
	bool action = Random(2);
	
	if (low < a.low) low = a.low;
	if (low >= a.high) low = a.high - a.step;
	if (high < a.low) high = a.low;
	if (high >= a.high) high = a.high - a.step;
	
	for (double d = low; d < high; d += a.step) {
		double range = high - low;
		double diff = d - low;
		double factor = diff / range;
		if (action) factor = 1.0 - factor;
		double prange = max - min;
		double pres = max - factor * prange;
		
		AsmData& ad = a.Get(iter, d);
		ad.pres += pres;
	}
	
}

void Generator::AddMomentumPressure() {
	for(int i = 0; i < ma.GetCount(); i++) {
		int period = ma[i].GetPeriod();
		double prev = ma[i].Top();
		double cur = price;
		double diff = cur - prev;
		if (diff == 0.0) continue;
		
		int steps = abs(diff / step) * period;
		double step = this->step * (diff < 0 ? -1.0 : +1.0);
		double d0 = price;
		double d1 = price;
		for (int j = 0; j < steps; j++) {
			AsmData& ad0 = a.Get(iter, d0);
			ad0.pres -= 0.1 * (1.0 / period);
			d0 += step;
			if (d0 < a.low) break;
			if (d0 >= a.high) break;
			
			AsmData& ad1 = a.Get(iter, d1);
			ad1.pres += 0.1 * (1.0 / period);
			d1 -= step;
			if (d1 < a.low) break;
			if (d1 >= a.high) break;
		}
	}
}

void Generator::AddMaPressure() {
	for(int i = 0; i < ma.GetCount(); i++) {
		int period = ma[i].GetPeriod();
		double prev = prev_ma_mean[i];
		double cur = ma[i].GetMean();
		double diff = cur - prev;
		if (diff == 0.0) continue;
		
		int steps = abs(diff / step) * ma[i].GetPeriod();
		double step = this->step * (diff < 0 ? -1.0 : +1.0);
		double d0 = price;
		double d1 = price;
		for (int j = 0; j < steps; j++) {
			AsmData& ad0 = a.Get(iter, d0);
			ad0.pres -= 0.1 * (1.0 / period);
			d0 += step;
			if (d0 < a.low) break;
			if (d0 >= a.high) break;
			
			AsmData& ad1 = a.Get(iter, d1);
			ad1.pres += 0.1 * (1.0 / period);
			d1 -= step;
			if (d1 < a.low) break;
			if (d1 >= a.high) break;
		}
	}
}

void Generator::AddAntiPatternPressure() {
	int last_pos = iter - 1;
	int32 last[pattern_count];
	int closest[pattern_count];
	int min_dist[pattern_count];
	for(int i = 0; i < pattern_count; i++) {
		last[i] = descriptors[i][iter - 1];
		closest[i] = -1;
		min_dist[i] = INT_MAX;
	}
	#if 1
	for(int i = 0; i < last_pos; i++) {
		for(int j = 0; j < pattern_count; j++) {
			int32 cur = descriptors[j][i];
			int dist = PopCount32(last[j] ^ cur);
			if (dist < min_dist[j]) {
				min_dist[j] = dist;
				closest[j] = i;
			}
		}
	}
	#else
	CoWork co;
	for(int i = 0; i < pattern_count; i++) {
		co & [=, &closest]()
		{
			int& cl = closest[i];
			int min_dist = INT_MAX;
			int l = last[i];
			int lock = 0;
			Vector<int>& descriptors = this->descriptors[i];
			
			array_view<int, 1> md_view(1, &min_dist);
			array_view<int, 1> cl_view(1, &cl);
			array_view<int, 1> lock_view(1, &lock);
			array_view<int, 1> descriptor_view(iter, descriptors.Begin());
			
			try {
				parallel_for_each(descriptor_view.extent, [=](index<1> idx) PARALLEL {
					int& l = lock_view[0];
					int32 cur = descriptor_view[idx];
					int32 last = l;
					int32 distbits = cur ^ last;
					int dist = 0;
					#ifdef flagFORCE_COMPAT_AMP
					dist = PopCount64(distbits);
					#else
					for(int i = 0; i < 32; i++)
						if (distbits & (1 << i))
							dist++;
					#endif
					if (dist < md_view[0]) {
						//while (AmpAtomicInc(l)) AmpAtomicDec(l);
						while (l++) l--;
						if (dist < md_view[0]) {
							md_view[0] = dist;
							cl_view[0] = idx[0];
						}
						l--;
					}
				});
				
				cl_view.synchronize();
				md_view.discard_data();
				lock_view.discard_data();
				descriptor_view.discard_data();
			}
			catch (runtime_exception& ex) {
				LOG(ex.what());
			}
		};
	}
	co.Finish();
	#endif
	
	for(int i = 0; i < pattern_count; i++) {
		int period = 4 << i;
		int prev_pos = closest[i];
		int cur_pos = prev_pos + period;
		if (cur_pos > last_pos) cur_pos = last_pos;
		if (prev_pos < 0) continue;
		
		double cur = data[cur_pos];
		double prev = data[prev_pos];
		double diff = cur - prev;
		if (diff == 0.0) continue;
		
		int steps = abs(diff / step) * ma[i].GetPeriod();
		double step = this->step * (diff < 0 ? -1.0 : +1.0);
		double d0 = price;
		double d1 = price;
		for (int j = 0; j < steps; j++) {
			AsmData& ad0 = a.Get(iter, d0);
			ad0.pres -= 3 * (1.0 / period);
			d0 += step;
			if (d0 < a.low) break;
			if (d0 >= a.high) break;
			
			AsmData& ad1 = a.Get(iter, d1);
			ad1.pres += 3 * (1.0 / period);
			d1 -= step;
			if (d1 < a.low) break;
			if (d1 >= a.high) break;
		}
	}
}

void Generator::GenerateData(bool add_random, int count) {
	if (count <= 0)
		count = data_count;
	
	int max_random_count = count - test_count;
	
	
	price = 1.0;
	
	iter = 0;
	if (add_random) {
		for(int i = 0; i < 3; i++)
			AddRandomPressure();
	}
	
	
	
	data.SetCount(data_count, price);
	descriptors.SetCount(pattern_count);
	for(int i = 0; i < descriptors.GetCount(); i++)
		descriptors[i].SetCount(data_count, 0);
	
	
	for(iter = 0; iter < count; iter++) {
		
		if (iter > 0) {
			ApplyPressureChanges();
			
			/*if (add_random && iter < max_random_count)
				if (Random(10) == 0)
					AddRandomPressure();*/
			
			AddMomentumPressure();
			AddMaPressure();
			AddAntiPatternPressure();
		}
		
		double max_pres = -DBL_MAX;
		prev_price = this->price;
		for (int i = -3; i <= 3; i++) {
			double price = prev_price + i * step;
			double pres = a.Get(iter, price).pres;
			if (pres > max_pres) {
				this->price = price;
				max_pres = pres;
			}
			/*if (iter == 500) {
				LOG(i << "\t" << price << "\t" << pres);
			}*/
		}
		
		
		// Price is known
		
		
		for(int i = 0; i < ma.GetCount(); i++) {
			double mean = ma[i].GetMean();
			if (mean == 0.0) mean = 1.0;
			prev_ma_mean[i] = mean;
			ma[i].Add(price);
		}
		data[iter] = price;
		
		for(int i = 0; i < pattern_count; i++)
			descriptors[i][iter] = GetDescriptor(iter, i);
	}
	
}

int32 Generator::GetDescriptor(int pos, int pattern_id) {
	Vector<Point>& pattern = this->pattern[pattern_id];
	int period = 4 << pattern_id;
	int data_begin = pos - period + 1;
	int32 out = 0;
	for(int i = 0; i < pattern.GetCount(); i++) {
		const Point& pt = pattern[i];
		int pos_a = data_begin + pt.x;
		int pos_b = data_begin + pt.y;
		if (pos_a < 0) pos_a = 0;
		if (pos_b < 0) pos_b = 0;
		ASSERT(pos_a <= pos && pos_b <= pos);
		double a = data[pos_a];
		double b = data[pos_b];
		bool value = a < b;
		if (value)		out |= 1 << i;
	}
	return out;
}

void Generator::ApplyPressureChanges() {
	if (iter > 0) {
		Vector<AsmData>& vec0 = a.data[iter];
		Vector<AsmData>& vec1 = a.data[iter - 1];
		for(int i = 0; i < vec0.GetCount(); i++) {
			AsmData& ad0 = vec0[i];
			AsmData& ad1 = vec1[i];
			ad0.pres = ad1.pres;
		}
		
		a.Get(iter, price).pres += -0.4;
		a.Get(iter, price + step).pres += -0.2;
		a.Get(iter, price - step).pres += -0.2;
	}
}


