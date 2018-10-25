#include "Forecaster.h"

namespace Forecast {

Generator::Generator() {
	
}

void Generator::Serialize(Stream& s) {
	s % decl
	  % descriptors
	  % pattern
	  % stream
	  % image
	  % a
	  % real_data
	  % params
	  % price % prev_price
	  % forecast % prev_forecast
	  % err
	  % state
	  % iter;
}

void Generator::Init(double point, const Vector<double>& real_data, const Vector<double>& params) {
	state = START;
	
	this->real_data <<= real_data;
	this->params <<= params;
	
	price = real_data[0];
	err = 0;
	iter = 0;
	
	descriptors.SetCount(real_data.GetCount());
	
	double low = price * 0.5;
	double high = price * 1.5;
	a.Init(real_data.GetCount(), low, high, point);
	image.Init(a.low, a.high, a.step, a.step*3, real_data.GetCount() + 1440*2, 10);
	
	decl.SetCount(0);
	AddDefaultDeclarations(decl);
	System::GetCoreQueue(this->real_data, work_queue, decl);
	GetLabels(work_queue, lbls);
	
	stream.Clear();
	stream.SetColumnCount(lbls.GetCount());
	stream.SetCount(real_data.GetCount() + 1440*2);
	
	state = INITIALIZED;
}

bool Generator::DoNext() {
	ASSERT(state >= INITIALIZED);
	switch (state) {
		case INITIALIZED:
			RefreshIndicators();
			state++;
			break;
		
		case INDICATORS_REFRESHED:
			GetBitStream();
			state++;
			PushWarmup();
			break;
			
		case BITSTREAMED:
			ResetPattern();
			CalculateError();
			state++;
			break;
		
		case ERROR_CALCULATED:
			Forecast();
			state++;
			break;
		
		default:
			return false;
	}
	
	
	return true;
}

void Generator::GetBitStream() {
	stream.SetBit(0);
	for(int i = 0; i < real_data.GetCount(); i++) {
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
	}
}

void Generator::GetLabels(Vector<CoreItem>& work_queue, Vector<ConstLabelSignal*>& lbls) {
	lbls.SetCount(0);
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = work_queue[i];
		
		OutputCounter lc;
		ci.core->IO(lc);
		
		for(int j = 0; j < lc.lbl_counts.GetCount(); j++) {
			int count = lc.lbl_counts[j];
			for(int k = 0; k < count; k++) {
				ConstLabelSignal& buf = ci.core->GetLabelBuffer(j, k);
				lbls.Add(&buf);
			}
		}
	}
}

void Generator::PushWarmup() {
	StringStream ss;
	ss.SetStoring();
	ss % *this;
	for(int i = 0; i < work_queue.GetCount(); i++)
		ss % *work_queue[i].core;
	ss.Seek(0);
	bitstreamed = ss.Get(ss.GetSize());
}

void Generator::PopWarmup(const Vector<double>& params) {
	if (bitstreamed.IsEmpty())
		Panic("BitStream is empty");
	MemStream ms((void*)bitstreamed.Begin(), bitstreamed.GetCount());
	ms.SetLoading();
	view_lock.Enter();
	ms % *this;
	for(int i = 0; i < work_queue.GetCount(); i++)
		ms % *work_queue[i].core;
	view_lock.Leave();
	this->params <<= params;
	
	GetLabels(work_queue, lbls);
}

void Generator::RefreshIndicators() {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = work_queue[i];
		ci.core->SetBars(real_data.GetCount());
		ci.core->Refresh();
	}
}

void Generator::CalculateError() {
	err = 0.0;
	for(iter = 0; iter < real_data.GetCount(); iter++) {
		RefreshForecast();
		RealPrice();
		ApplyIndicatorPressures();
		ApplyPressureChanges();
		RefreshDescriptor();
	}
}

void Generator::Forecast() {
	int forecast_count = 1440 * 2;
	ASSERT(iter == real_data.GetCount());
	real_data.SetCount(iter + forecast_count, real_data.Top());
	descriptors.SetCount(iter + forecast_count);
	for(; iter < real_data.GetCount(); iter++) {
		RefreshForecast();
		prev_price = price;
		price = forecast;
		real_data[iter] = price;
		
		for(int i = 0; i < work_queue.GetCount(); i++) {
			CoreItem& ci = work_queue[i];
			ci.core->SetBars(iter + 1);
			ci.core->Refresh();
		}
		
		stream.SetBit(iter * stream.GetColumnCount());
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(iter);
			stream.Write(value);
		}
		
		ApplyIndicatorPressures();
		ApplyPressureChanges();
		RefreshDescriptor();
	}
}

void Generator::AddRandomPressure() {
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

void Generator::ResetPattern() {
	double scale = min(0.01, pow(10, -3 + params[0]));
	double begin = real_data[0] * (1.0 - scale);
	double end = real_data[0] * (1.0 + scale);
	int steps = (end - begin) / a.step;
	if (steps < 3) steps = 3;
	int total = PressureDescriptor::size * 64;
	pattern.SetCount(total);
	for(int i = 0; i < total; i++) {
		pattern[i].x = Random(steps);
		pattern[i].y = Random(steps);
	}
}

void Generator::RefreshDescriptor() {
	double scale = min(0.01, pow(10, -3 + params[0]));
	double begin = price * (1.0 - scale);
	double end = price * (1.0 + scale);
	int steps = (end - begin) / a.step;
	PressureDescriptor& desc = descriptors[iter];
	for(int i = 0; i < PressureDescriptor::size; i++) {
		int64& i64 = desc.descriptor[i];
		i64 = 0;
		for(int j = 0; j < 64; j++) {
			const Point& pt = pattern[i * 64 + j];
			double k0 = begin + pt.x * a.step;
			double k1 = begin + pt.y * a.step;
			double pres0 = a.Get(k0).pres;
			double pres1 = a.Get(k1).pres;
			bool value = pres0 < pres1;
			if (value)
				i64 |= 1 << j;
		}
		//LOG(iter << "\t" << i64);
	}
	
}

void Generator::RefreshForecast() {
	#if 0
	double max_pres = -DBL_MAX;
	prev_forecast = price;
	for (int i = -5; i <= 5; i++) {
		double price2 = prev_price + i * a.step;
		double pres = a.Get(price2).pres;
		if (pres > max_pres) {
			forecast = price2;
			max_pres = pres;
		}
	}
	
	#else
	
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
		
		double d0 = real_data[min_pos + 1];
		double d1 = real_data[min_pos];
		
		
		prev_forecast = price;
		forecast = prev_forecast * (d0 / d1);
		
		if (d0 == d1 && iter > 1 && real_data[iter - 1] != real_data[iter - 2]) {
			skip_pos.Add(min_pos);
			continue;
		}
		else
			break;
	}
	#endif
}

void Generator::RealPrice() {
	if (iter > 0) {
		double prev = real_data[iter - 1];
		double cur = real_data[iter];
		double real_diff = cur - prev;
		double forecast_diff = forecast - prev_forecast;
		double abs_err = fabs(real_diff - forecast_diff);
		ASSERT(IsFin(abs_err) && IsFin(err) && abs_err != -DBL_MAX);
		err += abs_err;
	}
	
	prev_price = price;
	price = real_data[iter];
}

void Generator::ApplyIndicatorPressures() {
	int readbit = iter * stream.GetColumnCount();
	stream.SetBit(readbit);
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
			if (k >= this->a.low && k <= this->a.high) {
				double& ii = a.Get(k).pres;
				ii = ii + inc;
				//ii = max(0.0, ii + inc);
			}
			k = price - d;
			if (k >= this->a.low && k <= this->a.high) {
				double& dd = a.Get(k).pres;
				dd = dd + dec;
				//dd = max(0.0, dd + dec);
			}
		}
	}
}

void Generator::ApplyPressureChanges() {
	double scale = min(0.1, pow(10, -2 + params[1]));
	double dec = 1.0 - fabs(params[2] * scale);
	double main_inc = fabs(params[3] * 10);
	double inc0 = fabs(params[4] * 10);
	double inc1 = fabs(params[5] * 10);
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
			a.data[pos].pres += -main_inc;
			a.data[pos + 1].pres += -inc0;
			a.data[pos - 1].pres += -inc1;
		}
	} else {
		for (double d = prev_price; d >= price; d -= a.step) {
			int pos = a.GetPos(d);
			a.data[pos].pres += -main_inc;
			a.data[pos + 1].pres += -inc1;
			a.data[pos - 1].pres += -inc0;
		}
	}
}

}
