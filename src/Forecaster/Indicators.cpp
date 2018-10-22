#include "Forecaster.h"

namespace Forecast {


SimpleHurstWindow::SimpleHurstWindow()
{
	
}

void SimpleHurstWindow::Init() {
	
}

void SimpleHurstWindow::Start() {
	int bars = GetBars();
	int prev_counted = GetCounted();
	
	Buffer& hurstbuf = GetBuffer(0);
	LabelSignal& ls = GetLabelBuffer(0, 0);
	
	if (prev_counted < period + 3)
		prev_counted = period + 3;
	
	for(int i = prev_counted; i < bars; i++) {
		double hurst = GetSimpleHurst(i, period);
		
		hurstbuf.Set(i, hurst);
		
		double o1 = Open(i-1);
		double o0 = Open(i);
		bool sig = o0 < o1;
		if (hurst < 0.5) sig = !sig;
		ls.signal.Set(i, sig);
	}
	
}

double SimpleHurstWindow::GetSimpleHurst(int i, int period) {
	int pos = 0, neg = 0;
	for(int j = 0; j < period; j++) {
		double o2 = Open(i-2-j);
		double o1 = Open(i-1-j);
		double o0 = Open(i-j);
		
		double d0 = o0-o1;
		double d1 = o1-o2;
		double d = d0 * d1;
		if (d >= 0) pos++;
		else neg++;
	}
	double hurst = (double)pos / (double) period;
	return hurst;
}
















Momentum::Momentum()
{
	period = 32;
	shift = -7;
}

void Momentum::Init() {
	if ( period <= 0 )
		throw ConfExc();

}

void Momentum::Start() {
	Buffer& buffer = GetBuffer(0);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period || period <= 0 )
		return;

	if ( counted <= 0 ) {
		for (int i = 0; i < period; i++) {
			buffer.Set(i, 0.0);
		}
		counted = period;
	}
	else if (counted > period)
		counted--;
	
	for (int i = counted; i < bars; i++) {
		double close1 = Open( i );
		double close2 = Open( i - period );
		double value = close1 * 100 / close2 - 100;
		buffer.Set(i, value);
		
		sig.signal.Set(i, value < 0);
	}
}


























MovingAverage::MovingAverage()
{
	ma_period = 13;
	ma_method = 0;
	ma_counted = 0;
}

void MovingAverage::Init() {
	
}

void MovingAverage::Start() {
	int bars = GetBars();
	if ( bars <= ma_period )
		return;
	ma_counted = GetCounted();
	if ( ma_counted < 0 )
		throw DataExc();
	if ( ma_counted > 0 )
		ma_counted--;
	int begin = ma_counted;
	switch ( ma_method )
	{
		case MODE_SIMPLE:
			Simple();
			break;
		case MODE_EXPONENTIAL:
			Exponential();
			break;
		case MODE_SMOOTHED:
			Smoothed();
			break;
		case MODE_LINWEIGHT:
			LinearlyWeighted();
	}
	
	Buffer& buffer = GetBuffer(0);
	VectorBool& signal = GetLabelBuffer(0, 0).signal;
	double prev = ma_counted > 0 ? buffer.Get(ma_counted-1) : 0.0;
	for(int i = ma_counted; i < bars; i++) {
		double cur = buffer.Get(i);
		bool label_value = cur < prev;
		signal.Set(i, label_value);
		prev = cur;
	}
}

void MovingAverage::Simple()
{
	Buffer& buffer = GetBuffer(0);
	double sum = 0;
	int bars = GetBars();
	int pos = ma_counted;
	if (pos < ma_period) pos = ma_period;
	for (int i = 1; i < ma_period; i++)
		sum += Open(pos - i);
	while (pos < bars) {
		sum += Open( pos );
		buffer.Set(pos, sum / ma_period);
		sum -= Open( pos - ma_period + 1 );
		pos++;
	}
	if (ma_counted < 1)
		for (int i = 0; i < ma_period; i++)
			buffer.Set(i, Open(i));
}

void MovingAverage::Exponential()
{
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	double pr = 2.0 / ( ma_period + 1 );
	int pos = 1;
	if ( ma_counted > 2 )
		pos = ma_counted + 1;
	while (pos < bars) {
		if (pos == 1)
			buffer.Set(pos-1, Open(pos - 1));
		buffer.Set(pos, Open(pos) * pr + buffer.Get(pos-1) * ( 1 - pr ));
		pos++;
	}
}

void MovingAverage::Smoothed()
{
	Buffer& buffer = GetBuffer(0);
	double sum = 0;
	int bars = GetBars();
	int pos = ma_period;
	if (pos < ma_counted)
		pos = ma_counted;
	while (pos < bars) {
		if (pos == ma_period) {
			for (int i = 0, k = pos; i < ma_period; i++, k--) {
				sum += Open(k);
				buffer.Set(k, 0);
			}
		}
		else
			sum = buffer.Get(pos-1) * ( ma_period - 1 ) + Open(pos);
		buffer.Set(pos, sum / ma_period);
		pos++;
	}
}

void MovingAverage::LinearlyWeighted()
{
	Buffer& buffer = GetBuffer(0);
	double sum = 0.0, lsum = 0.0;
	double value;
	int bars = GetBars();
	int weight = 0, pos = ma_counted + 1;
	if (pos > bars - ma_period)
		pos = bars - ma_period;
	for (int i = 1; i <= ma_period; i++, pos++) {
		value = Open( pos );
		sum += value * i;
		lsum += value;
		weight += i;
	}
	pos--;
	int i = pos - ma_period;
	while (pos < bars) {
		buffer.Set(pos, sum / weight);
		pos++;
		i++;
		if ( pos == bars )
			break;
		value = Open(pos);
		sum = sum - lsum + value * ma_period;
		lsum -= Open(i);
		lsum += value;
	}
	if ( ma_counted < 1 )
		for (i = 0; i < ma_period; i++)
			buffer.Set(i, Open(i));
}












ParabolicSAR::ParabolicSAR() {
	sar_step = 0.02;
	sar_maximum = 0.2;
	step = 20;
	maximum = 20;
}

void ParabolicSAR::Init() {
	
	sar_step = step * 0.001;
	sar_maximum = maximum * 0.01;
	
	if (sar_step <= 0.0)
		throw ConfExc();
	
	if (sar_maximum <= 0.0)
		throw ConfExc();
	
	last_rev_pos = 0;
	direction_long = false;
}

void ParabolicSAR::Start () {
	Buffer& sar_buffer = GetBuffer(0);
	Buffer& ep_buffer  = GetBuffer(1);
	Buffer& af_buffer  = GetBuffer(2);
	
	int counted = GetCounted();
	int bars = GetBars();

	if ( bars < 3 )
		return;
	
	int pos = counted - 1;

	if ( pos < 1 ) {
		pos = 1;
		af_buffer.Set(0, sar_step);
		af_buffer.Set(1, sar_step);
		sar_buffer.Set(0, High( 0 ));
		last_rev_pos = 0;
		direction_long = false;
		sar_buffer.Set(1, GetHigh( pos-1, last_rev_pos ));
		double low = Low( 0 );
		ep_buffer.Set(0, low);
		ep_buffer.Set(1, low);
	}
	
	if (pos == 0) pos++;
	
	int prev_pos = pos - 1;
	if (prev_pos < 0) prev_pos = 0;
	
	double prev_high = High( prev_pos );
	double prev_low  = Low( prev_pos );
	
	//bars--;
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	for (int i = pos; i < bars; i++) {
		double low  = Low(i-1);
		double high = High(i-1);

		if ( direction_long ) {
			if ( sar_buffer.Get(i-1) > low ) {
				direction_long = false;
				sar_buffer.Set(i-1, GetHigh( i-1, last_rev_pos ));
				ep_buffer.Set(i, low);
				last_rev_pos = i;
				af_buffer.Set(i, sar_step);
			}
		}
		else {
			if ( sar_buffer.Get(i-1) < high ) {
				direction_long = true;
				sar_buffer.Set(i-1, GetLow( i-1, last_rev_pos ));
				ep_buffer.Set(i, high);
				last_rev_pos = i;
				af_buffer.Set(i, sar_step);
			}
		}

		if ( direction_long ) {
			if ( high > ep_buffer.Get(i-1) && i != last_rev_pos ) {
				ep_buffer.Set(i, high);
				af_buffer.Set(i, af_buffer.Get(i-1) + sar_step);

				if ( af_buffer.Get(i) > sar_maximum )
					af_buffer.Set(i, sar_maximum);
			}

			else {
				if ( i != last_rev_pos ) {
					af_buffer.Set(i, af_buffer.Get (i-1));
					ep_buffer.Set(i, ep_buffer.Get (i-1));
				}
			}

			double value = sar_buffer.Get(i-1) + af_buffer.Get(i) * ( ep_buffer.Get(i) - sar_buffer.Get(i-1) );
			sar_buffer.Set(i, value);

			if ( value > low || value > prev_low )
				sar_buffer.Set(i, Upp::min( low, prev_low ));
		}

		else {
			if ( low < ep_buffer.Get(i-1) && i != last_rev_pos ) {
				ep_buffer.Set(i, low);
				af_buffer.Set(i, af_buffer.Get(i-1) + sar_step);

				if ( af_buffer.Get(i) > sar_maximum )
					af_buffer.Set(i, sar_maximum);
			}

			else {
				if ( i != last_rev_pos ) {
					af_buffer.Set(i, af_buffer.Get(i-1));
					ep_buffer.Set(i, ep_buffer.Get(i-1));
				}
			}

			double value = sar_buffer.Get(i-1) + af_buffer.Get(i) * ( ep_buffer.Get(i) - sar_buffer.Get(i-1) );
			sar_buffer.Set(i, value);

			if ( value < high || value < prev_high )
				sar_buffer.Set(i, Upp::max( high, prev_high ));
		}
		
		sig.signal.Set(i, !direction_long);
		
		prev_high = high;
		prev_low  = low;
	}
}

double ParabolicSAR::GetHigh( int pos, int start_pos )
{
	ASSERT(pos >= start_pos);
	int bars = GetBars();
	
	double result = High( start_pos );

	for ( int i = start_pos+1; i <= pos; i++)
	{
		double high = High(i);

		if ( result < high )
			result = high;
	}

	return ( result );
}

double ParabolicSAR::GetLow( int pos, int start_pos )
{
	ASSERT(pos >= start_pos);
	int bars = GetBars();
	
	double result = Low( start_pos );

	for ( int i = start_pos+1; i <= pos; i++)
	{
		double low = Low(i);

		if ( result > low )
			result = low;
	}

	return ( result );
}














Pattern::Pattern() {
	pattern.SetCount(32);
	
}

void Pattern::RandomizePattern() {
	for(int j = 0; j < 32; j++) {
		pattern[j].x = Random(period);
		pattern[j].y = Random(period);
	}
}

int32 Pattern::GetDescriptor(int pos) {
	int data_begin = pos - period + 1;
	int32 out = 0;
	for(int i = 0; i < 32; i++) {
		const Point& pt = pattern[i];
		int pos_a = data_begin + pt.x;
		int pos_b = data_begin + pt.y;
		if (pos_a < 0) pos_a = 0;
		if (pos_b < 0) pos_b = 0;
		AMPASSERT(pos_a <= pos && pos_b <= pos);
		double a = Open(pos_a);
		double b = Open(pos_b);
		bool value = a < b;
		if (value)		out |= 1 << i;
	}
	return out;
}

void Pattern::Init() {
	
	RandomizePattern();
}

void Pattern::Start() {
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	descriptors.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		int32 desc = GetDescriptor(i);
		descriptors[i] = desc;
		
		int min_dist = INT_MAX;
		int min_pos = 0;
		for(int j = 0; j < i; j++) {
			int32 descb = descriptors[j];
			int dist = PopCount32(desc ^ descb);
			if (dist < min_dist) {
				min_dist = dist;
				min_pos = j;
			}
		}
		
		int prev_pos = min_pos;
		int cur_pos = min(i, prev_pos + period);
		double prev = Open(prev_pos);
		double cur = Open(cur_pos);
		double diff = cur - prev;
		
		bool value = diff < 0.0;
		sig.signal.Set(i, value);
	}
}

}
