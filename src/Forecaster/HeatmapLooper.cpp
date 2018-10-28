#include "Forecaster.h"

namespace Forecast {

HeatmapLooper::HeatmapLooper() {
	
}

void HeatmapLooper::Init(double point, const Vector<double>& real_data) {
	this->real_data = &real_data;
	
	price = real_data[0];
	err = 0;
	iter = 0;
	
	descriptors.SetCount(real_data.GetCount());
	
	double low = +DBL_MAX;
	double high = -DBL_MAX;
	for(int i = 0; i < real_data.GetCount(); i++) {
		double o = real_data[i];
		if (o < low) low = o;
		if (o > high) high = o;
	}
	high *= 1.01;
	low *= 0.99;
	a.Init(low, high, point);
	image.Init(a.low, a.high, a.step, a.step*3, real_data.GetCount(), 5);
}

void HeatmapLooper::CalculateError() {
	err = 0.0;
	forecast_read_pos = 0;
	
	int size;
	size = real_data->GetCount();
	total = size;
	
	for(int i = 0; i < a.data.GetCount(); i++)
		a.data[i].pres = 0.0;
	image.Init(a.low, a.high, a.step, a.step*3, real_data->GetCount(), 5);
	
	for(iter = 0; iter < size; iter++) {
		actual = iter;
		
		RefreshForecast();
		RealPrice();
		ApplyIndicatorPressures();
		ApplyPressureChanges();
		RefreshDescriptor();
	}
}

void HeatmapLooper::GetSampleInput(NNSample& s, int result_id) {
	ASSERT(result_id >= 0 && result_id < NNSample::single_count);
	double scale = min(0.01, pow(10, -3 + params[0]));
	double begin = (*real_data)[0] * (1.0 - scale);
	double end = (*real_data)[0] * (1.0 + scale);
	double step = (end - begin) / NNSample::single_size;
	double it = begin;
	double absmax = 0.0;
	for(int i = 0; i < NNSample::single_size; i++) {
		double d = a.Get(it).pres;
		double absd = fabs(d);
		if (absd > absmax) absmax = absd;
		s.input[result_id][i] = d;
		it += step;
	}
	
	// Probably bad idea...
	/*if (absmax > 0.0) {
		for(int i = 0; i < NNSample::single_size; i++) {
			s.input[result_id][i] = s.input[result_id][i] / absmax * 10.0;
		}
	}*/
}

void HeatmapLooper::AddRandomPressure() {
	double low  = price - (1 + Random(10)) * a.step;
	double high = price + (1 + Random(10)) * a.step;
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
		
		AsmData& ad = a.Get( d);
		ad.pres += pres;
	}
	
}

void HeatmapLooper::ResetPattern() {
	double scale = min(0.01, pow(10, -3 + params[0]));
	double begin = (*real_data)[0] * (1.0 - scale);
	double end = (*real_data)[0] * (1.0 + scale);
	int steps = (end - begin) / a.step;
	if (steps < 3) steps = 3;
	int total = PressureDescriptor::size * 64;
	pattern.SetCount(total);
	for(int i = 0; i < total; i++) {
		pattern[i].x = Random(steps);
		pattern[i].y = Random(steps);
	}
}

void HeatmapLooper::RefreshDescriptor() {
	double scale = min(0.01, pow(10, -3 + params[0]));
	double begin = price * (1.0 - scale);
	PressureDescriptor& desc = descriptors[iter];
	for(int i = 0; i < PressureDescriptor::size; i++) {
		int64& i64 = desc.descriptor[i];
		i64 = 0;
		for(int j = 0; j < 64; j++) {
			const Point& pt = pattern[i * 64 + j];
			double k0 = begin + pt.x * a.step;
			double k1 = begin + pt.y * a.step;
			int l0 = a.Find(k0);
			int l1 = a.Find(k1);
			if (l0 >= 0 && l1 >= 0) {
				double pres0 = a.data[l0].pres;
				double pres1 = a.data[l1].pres;
				bool value = pres0 < pres1;
				if (value)
					i64 |= 1 << j;
			}
		}
		//LOG(iter << "\t" << i64);
	}
	
}

void HeatmapLooper::RefreshForecast() {
	
	#if 0
	double max_pres = -DBL_MAX;
	prev_forecast = price;
	for (int i = -5; i <= 5; i++) {
		double price2 = prev_price + i * a.step;
		int j = a.Find(price2);
		if (j >= 0) {
			double pres = a.data[j].pres;
			if (pres > max_pres) {
				forecast = price2;
				max_pres = pres;
			}
		}
	}
	
	#else
	
	if (forecast_read_pos > 0 && forecast_read_pos < forecast_tmp.GetCount() - 1) {
		double d0 = forecast_tmp[forecast_read_pos + 1];
		double d1 = forecast_tmp[forecast_read_pos];
		if (d0 == d1) {
			forecast_read_pos++;
			RefreshForecast();
			return;
		}
		prev_forecast = price;
		forecast = prev_forecast * (d0 / d1);
		forecast_read_pos++;
		return;
	}
	forecast_read_pos = 0;
	forecast_tmp.SetCount(0);
	
	if (iter < 2)
		return;
	
	Index<int> skip_pos;
	
	while (true) {
		int min_dist = INT_MAX;
		int min_pos = -1;
		
		// Use binary descriptor, which is known from ORB image descriptor
		
		PressureDescriptor& prev = descriptors[iter - 1];
		for(int i = 0; i < iter - 1; i++) {
			PressureDescriptor& desc = descriptors[i];
			
			int dist = 0;
			for(int j = 0; j < PressureDescriptor::size; j++) {
				int64& a = prev.descriptor[j];
				int64& b = desc.descriptor[j];
				dist += PopCount64(a ^ b);
			}
			
			if (dist < min_dist) {
				if (skip_pos.Find(i) != -1)
					continue;
				min_dist = dist;
				min_pos = i;
			}
		}
		
		if (min_pos == -1)
			break;
		
		double d0 = (*real_data)[min_pos + 1];
		double d1 = (*real_data)[min_pos];
		
		
		prev_forecast = price;
		forecast = prev_forecast * (d0 / d1);
		
		if (d0 == d1 && iter > 1 && (*real_data)[iter - 1] != (*real_data)[iter - 2]) {
			skip_pos.Add(min_pos);
			continue;
		}
		else {
			int size = 10+1;
			int pos = min_pos;
			forecast_tmp.SetCount(size);
			for(int i = 0; i < size; i++) {
				if (pos >= iter) {
					forecast_tmp.SetCount(i);
					break;
				}
				forecast_tmp[i] = (*real_data)[pos];
				pos++;
			}
			forecast_read_pos = 1;
			break;
		}
	}
	#endif
}

void HeatmapLooper::RealPrice() {
	if (iter > 0) {
		double prev = (*real_data)[iter - 1];
		double cur = (*real_data)[iter];
		double real_diff = cur - prev;
		double forecast_diff = forecast - prev_forecast;
		double abs_err = fabs(real_diff - forecast_diff);
		ASSERT(IsFin(abs_err) && IsFin(err) && abs_err != -DBL_MAX);
		err += abs_err;
	}
	
	prev_price = price;
	price = (*real_data)[iter];
}

void HeatmapLooper::ApplyIndicatorPressures() {
	int readbit = iter * stream.GetColumnCount();
	stream.SetBit(readbit);
	ASSERT(iter >= 0 && iter < stream.GetCount());
	ASSERT(stream.GetColumnCount() > 0);
	int j = APPLYPRESSURE_PARAMS;
	for(int i = 0; i < stream.GetColumnCount(); i++) {
		bool value = stream.Read();
		double scale = pow(10, 2 + params[j++]);
		double len = fabs(params[j++] * scale) * a.step;
		double inc = params[j++];
		double dec = params[j++];
		if (value) Swap(inc, dec);
		for (double d = 0; d <= len; d += a.step) {
			double k = price + d;
			if (k >= this->a.low && k <= this->a.high - this->a.step) {
				double& ii = a.Get(k).pres;
				ii = ii + inc;
				//ii = max(0.0, ii + inc);
			}
			k = price - d;
			if (k >= this->a.low && k <= this->a.high - this->a.step) {
				double& dd = a.Get(k).pres;
				dd = dd + dec;
				//dd = max(0.0, dd + dec);
			}
		}
	}
}

void HeatmapLooper::ApplyPressureChanges() {
	double scale = min(0.1, pow(10, -2 + params[1]));
	double dec = 1.0 - fabs(params[2] * scale);
	double main_inc = fabs(params[3] * 0.1);
	double inc0 = fabs(params[4] * 0.1);
	double inc1 = fabs(params[5] * 0.1);
	if (iter > 0) {
		Vector<AsmData>& cur = a.data;
		for(int i = 0; i < cur.GetCount(); i++) {
			double d = a.low + i * a.step;
			double& pres = cur[i].pres;
			pres = max(0.0, pres * dec);
			//pres = pres * dec;
			image.Add(iter, d, pres);
		}
	}
	if (prev_price <= price) {
		for (double d = prev_price; d <= price; d += a.step) {
			int pos = a.GetPos(d);
			if (pos <= 0 || pos >= a.data.GetCount()-1)return;
			a.data[pos].pres += -main_inc;
			a.data[pos + 1].pres += -inc0;
			a.data[pos - 1].pres += -inc1;
		}
	} else {
		for (double d = prev_price; d >= price; d -= a.step) {
			int pos = a.GetPos(d);
			if (pos <= 0 || pos >= a.data.GetCount()-1)return;
			a.data[pos].pres += -main_inc;
			a.data[pos + 1].pres += -inc1;
			a.data[pos - 1].pres += -inc0;
		}
	}
}




















MultiHeatmapLooper::MultiHeatmapLooper() {
	
}

void MultiHeatmapLooper::Init(double point, const Vector<double>& real_data, const Vector<Vector<double> >& params, BitStream& stream) {
	int cores = GetUsedCpuCores();
	
	this->real_data = &real_data;
	
	if (params.GetCount() < NNSample::single_count)
		Panic("MultiHeatmapLooper::Init Not enough params");
	
	loopers.SetCount(NNSample::single_count);
	for(int i = 0; i < NNSample::single_count; i++) {
		HeatmapLooper& l = loopers[i];
		l.Init(point, real_data);
		l.stream = stream;
		l.params <<= params[i];
		l.ResetPattern();
	}
}

void MultiHeatmapLooper::Run(bool get_samples) {
	for(int i = 0; i < loopers.GetCount(); i++) {
		HeatmapLooper& l = loopers[i];
		l.forecast_read_pos = 0;
		l.price = (*real_data)[0];
		for(int i = 0; i < l.a.data.GetCount(); i++)
			l.a.data[i].pres = 0.0;
	}
	
	int end = get_samples ? real_data->GetCount() - 241 : real_data->GetCount();
	
	for(int i = 0; i < end; i ++) {
		
		for(int j = 0; j < loopers.GetCount(); j++) {
			HeatmapLooper& l = loopers[j];
			l.iter = i;
			l.prev_price = l.price;
			l.price = (*real_data)[i];
			l.ApplyIndicatorPressures();
			l.ApplyPressureChanges();
		}
		
		if (get_samples && i % 10 == 0) {
			NNSample& s = nnsamples.GetAdd(i);
			
			for(int j = 0; j < loopers.GetCount(); j++)
				loopers[j].GetSampleInput(s, j);
			
			double begin = (*real_data)[i];
			for(int j = 10, k = 0; j <= 240; j += 10, k++) {
				int pos = i + j;
				double d = ((*real_data)[pos] / begin - 1.0) * 1000.0;
				s.output[k] = d;
			}
		}
	}
}

void MultiHeatmapLooper::GetSampleInput(NNSample& s) {
	for(int j = 0; j < loopers.GetCount(); j++)
		loopers[j].GetSampleInput(s, j);
}

void MultiHeatmapLooper::Clear() {
	nnsamples.Clear();
	loopers.Clear();
}

}
