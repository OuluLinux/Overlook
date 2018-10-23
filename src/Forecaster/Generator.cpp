#include "Forecaster.h"

namespace Forecast {

Generator::Generator() {
	
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
		
		AsmData& ad = a.Get(d);
		ad.pres += pres;
	}
	
}

void Generator::ResetGenerate() {
	a.Reset();
	price = 1.0;
	err = 0;
	iter = 0;
	
}

void Generator::RealPrice(const Vector<double>& real_data) {
	if (iter > 0) {
		double prev = real_data[iter - 1];
		double cur = real_data[iter];
		double real_diff = cur - prev;
		double gen_diff = price - prev_price;
		double abs_err = real_diff - gen_diff;
		if (abs_err < 0) abs_err = -abs_err;
		err += abs_err;
	}
	
	price = real_data[iter];
}

void Generator::ApplyIndicatorPressures(BitStream& stream, const Vector<double>& params) {
	int readiter = stream.GetBit() / stream.GetColumnCount();
	//ASSERT(iter == readiter);
	int j = 0;
	for(int i = 0; i < stream.GetColumnCount(); i++) {
		bool value = stream.Read();
		double len = fabs(params[j++] * 100) * step;
		double inc = params[j++];
		double dec = params[j++];
		if (value) Swap(inc, dec);
		for (double d = 0; d <= len; d += step) {
			a.Get(price + d).pres += inc;
			a.Get(price - d).pres += dec;
		}
	}
}

void Generator::GenerateTest(BitStream& stream, const Vector<double>& real_data, const Vector<double>& params) {
	
	for(; iter < data_count; iter++) {
		ApplyPressureChanges();
		ApplyIndicatorPressures(stream, params);
		RefreshPrice();
		RealPrice(real_data);
	}
}

void Generator::Iteration(BitStream& stream, int i, const Vector<double>& params) {
	ApplyPressureChanges();
	ApplyIndicatorPressures(stream, params);
	RefreshPrice();
}

void Generator::RefreshPrice() {
	double max_pres = -DBL_MAX;
	prev_price = this->price;
	for (int i = -3; i <= 3; i++) {
		double price = prev_price + i * step;
		double pres = a.Get(price).pres;
		if (pres > max_pres) {
			this->price = price;
			max_pres = pres;
		}
	}
}

void Generator::ApplyPressureChanges() {
	if (prev_price <= price) {
		for (double d = prev_price; d <= price; d += step) {
			a.Get(d).pres += -40;
			a.Get(d + step).pres += -20;
			a.Get(d - step).pres += -20;
		}
	} else {
		for (double d = prev_price; d >= price; d -= step) {
			a.Get(d).pres += -40;
			a.Get(d + step).pres += -20;
			a.Get(d - step).pres += -20;
		}
	}
}

}
