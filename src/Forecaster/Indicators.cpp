#include "Forecaster.h"

namespace Forecast {


double SimpleMA ( const int position, const int period, ConstBuffer& value ) {
	double result = 0.0;
	if ( position >= period && period > 0 ) {
		for ( int i = 0; i < period; i++)
			result += value.Get( position - i );
		result /= period;
	}
	return ( result );
}

double ExponentialMA ( const int position, const int period, const double prev_value, ConstBuffer& value ) {
	double result = 0.0;
	if ( period > 0 ) {
		double pr = 2.0 / ( period + 1.0 );
		result = value.Get( position ) * pr + prev_value * ( 1 - pr );
	}
	return ( result );
}

double SmoothedMA ( const int position, const int period, const double prev_value, ConstBuffer& value ) {
	double result = 0.0;
	if ( period > 0 ) {
		if ( position == period - 1 ) {
			for ( int i = 0;i < period;i++ )
				result += value.Get( position - i );

			result /= period;
		}
		if ( position >= period )
			result = ( prev_value * ( period - 1 ) + value.Get( position ) ) / period;
	}
	return ( result );
}

double LinearWeightedMA ( const int position, const int period, ConstBuffer& value ) {
	double result = 0.0, sum = 0.0;
	int    i, wsum = 0;
	if ( position >= period - 1 && period > 0 ) {
		for ( i = period;i > 0;i-- ) {
			wsum += i;
			sum += value.Get( position - i + 1 ) * ( period - i + 1 );
		}
		result = sum / wsum;
	}
	return ( result );
}


int SimpleMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin,
		const int period, ConstBuffer& value, Buffer& buffer ) {
	int i, limit;
	if ( period <= 1 || rates_total - begin < period )
		return ( 0 );
	if ( prev_calculated <= 1 ) {
		limit = period + begin;
		for ( i = 0;i < limit - 1;i++ )
			buffer.Set( i, 0.0 );
		double first_value = 0;
		for ( i = begin; i < limit;i++ )
			first_value += value.Get(i);
		first_value /= period;
		buffer.Set( limit - 1, first_value);
	}
	else
		limit = prev_calculated - 1;
	
	for ( i = limit; i < rates_total; i++)
		buffer.Set( i, buffer.Get( i - 1 ) + ( value.Get(i) - value.Get( i - period ) ) / period );
	
	return ( rates_total );
}


int ExponentialMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin,
		const int period, ConstBuffer& value, Buffer& buffer ) {
	int    i, limit;
	if ( period <= 1 || rates_total - begin < period )
		return ( 0 );
	double dSmoothFactor = 2.0 / ( 1.0 + period );
	if ( prev_calculated == 0 )
	{
		limit = period + begin;
		for ( i = 0;i < begin;i++ )
			buffer.Set( i, 0.0 );
		buffer.Set( begin, value.Get( begin ) );
		for ( i = begin + 1;i < limit;i++ )
			buffer.Set( i, value.Get(i) * dSmoothFactor + buffer.Get( i - 1 ) * ( 1.0 - dSmoothFactor ) );
	}
	else
		limit = prev_calculated - 1;
	for ( i = limit;i < rates_total;i++ )
		buffer.Set( i, value.Get(i) * dSmoothFactor + buffer.Get( i - 1 ) * ( 1.0 - dSmoothFactor ) );
	return ( rates_total );
}


int LinearWeightedMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin,
	const int period, ConstBuffer& value, Buffer& buffer, int &weightsum ) {
	int        i, limit;
	double     sum;
	if ( period <= 1 || rates_total - begin < period )
		return ( 0 );
	if ( prev_calculated == 0 )
	{
		weightsum = 0;
		limit = period + begin;
		for ( i = 0;i < limit;i++ )
			buffer.Set( i, 0.0 );
		double first_value = 0;
		for ( i = begin;i < limit; i++) {
			int k = i - begin + 1;
			weightsum += k;
			first_value += k * value.Get(i);
		}
		first_value /= ( double ) weightsum;
		buffer.Set( limit - 1, first_value );
	}
	else
		limit = prev_calculated - 1;
	for ( i = limit;i < rates_total; i++)
	{
		sum = 0;
		for ( int j = 0;j < period;j++ )
			sum += ( period - j ) * value.Get( i - j );
		buffer.Set( i, sum / weightsum );
	}
	return ( rates_total );
}


int SmoothedMAOnBuffer ( const int rates_total, const int prev_calculated, const int begin,
		const int period, ConstBuffer& value, Buffer& buffer )
{
	int i, limit;
	if ( period <= 1 || rates_total - begin < period )
		return ( 0 );
	if ( prev_calculated == 0 ) {
		limit = period + begin;
		for ( i = 0;i < limit - 1;i++ )
			buffer.Set( i, 0.0 );
		double first_value = 0;
		for ( i = begin;i < limit;i++ )
			first_value += value.Get(i);
		first_value /= period;
		buffer.Set( limit - 1, first_value );
	}
	else
		limit = prev_calculated - 1;
	for ( i = limit;i < rates_total; i++)
		buffer.Set( i, ( buffer.Get( i - 1 ) * ( period - 1 ) + value.Get(i) ) / period );
	return rates_total;
}

























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




















BearsPower::BearsPower() {
	period = 13;
}

void BearsPower::Init() {
	
	av.SetPeriod(period);
}

void BearsPower::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	else
		counted++;
	
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	for (int i = counted; i < bars; i++) {
		double open = Open(i);
		av.Add(open);
		double value = Low(i-1) - av.GetMean();
		buffer.Set(i, value);
		sig.signal.Set(i, value < 0);
		sig.enabled.Set(i, value < 0);
	}
}


















CommodityChannelIndex::CommodityChannelIndex() {
	period = 14;
}

void CommodityChannelIndex::Init() {
	
}

void CommodityChannelIndex::Start() {
	Buffer& cci_buffer = GetBuffer(0);
	Buffer& value_buffer = GetBuffer(1);
	Buffer& mov_buffer = GetBuffer(2);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	int i, k, pos;
	double sum, mul;
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period || period <= 1 )
		throw ConfExc();
	
	if ( counted < 1 ) {
		for ( i = 1; i < period; i++) {
			double h = High(i-1);
			double l = Low(i-1);
			double c = Open(i);

			cci_buffer.Set(i, 0.0);
			value_buffer.Set(i, ( h + l + c ) / 3);
			mov_buffer.Set(i, 0.0);
		}
	}

	pos = counted - 1;

	if ( pos < period )
		pos = period;
	
	if (pos == 0)
		pos++;
	
	//bars--;
	for ( i = pos; i < bars; i++) {
		double h = High(i-1);
		double l = Low(i-1);
		double c = Open(i);
		value_buffer.Set(i, ( h + l + c ) / 3);
		mov_buffer.Set(i, SimpleMA ( i, period, value_buffer ));
	}

	mul = 0.015 / period;
	pos = period - 1;

	if ( pos < counted - 1 )
		pos = counted - 2;

	i = pos;

	while ( i < bars ) {
		sum = 0.0;
		k = i + 1 - period;
		
		while ( k <= i ) {
			sum += fabs ( value_buffer.Get(k) - mov_buffer.Get(i) );
			k++;
		}

		sum *= mul;

		double value = 0;
		if ( sum != 0.0 )
			value = ( value_buffer.Get(i) - mov_buffer.Get(i) ) / sum;
		
		cci_buffer.Set(i, value);
		sig.signal.Set(i, value < 0);
		
		i++;
	}
}







DeMarker::DeMarker() {
	period = 14;
}

void DeMarker::Init() {
	
}

void DeMarker::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& max_buffer = GetBuffer(1);
	Buffer& min_buffer = GetBuffer(2);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	double num;
	
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();
	
	max_buffer.Set(0, 0.0);
	min_buffer.Set(0, 0.0);

	if ( counted > 2 )
		counted--;
	else
		counted = 2;
	
	double prev_high = counted >= 2 ? High( counted - 1 ) : 0;
	double prev_low  = counted >= 2 ? Low( counted - 1 ) : 0;

	for (int i = counted; i < bars; i++) {
		double high = High(i-1);
		double low  = Low(i-1);

		num = high - prev_high;

		if ( num < 0.0 )
			num = 0.0;

		max_buffer.Set(i, num);

		num = prev_low - low;

		if ( num < 0.0 )
			num = 0.0;

		min_buffer.Set(i, num);

		prev_high = high;
		prev_low  = low;
	}

	if ( counted == 0 )
		for (int i = 0; i < period; i++)
			buffer.Set(bars - i, 0.0);

	
	for (int i = counted; i < bars; i++)
	{
		double maxvalue = SimpleMA ( i, period, max_buffer );
		num = maxvalue + SimpleMA ( i, period, min_buffer );
		
		double value = 0.0;
		if ( num != 0.0 )
			value = maxvalue / num;
		buffer.Set(i, value);
		
		sig.signal.Set(i, value < 0.5);
	}
}



























RelativeStrengthIndex::RelativeStrengthIndex()
{
	period = 32;
}

void RelativeStrengthIndex::Init() {
	
}

void RelativeStrengthIndex::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& pos_buffer = GetBuffer(1);
	Buffer& neg_buffer = GetBuffer(2);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	double diff;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	int pos = counted - 1;

	if ( pos <= period ) {
		buffer.Set(0, 0.0);
		pos_buffer.Set(0, 0.0);
		neg_buffer.Set(0, 0.0);
		double sum_p = 0.0;
		double sum_n = 0.0;


		double prev_value  = Open( 0 );

		for (int i = 1; i <= period; i++) {
			double value  = Open(i);

			if ( prev_value == 0 ) {
				prev_value = value;
				continue;
			}

			buffer.Set(i, 0.0);

			pos_buffer.Set(i, 0.0);
			neg_buffer.Set(i, 0.0);
			diff = value - prev_value;
			sum_p += ( diff > 0 ? diff : 0 );
			sum_n += ( diff < 0 ? -diff : 0 );
			prev_value = value;
		}

		pos_buffer.Set(period, sum_p / period);
		neg_buffer.Set(period, sum_n / period);
		buffer.Set(period, (1.0 - ( 1.0 / ( 1.0 + pos_buffer.Get(period) / neg_buffer.Get(period) ) )) * 100);
		pos = period + 1;
	}
	
	double prev_value  = Open(pos-1);

	for (int i = pos; i < bars; i++) {
		double value  = Open(i);
		diff = value - prev_value;
		pos_buffer.Set(i, ( pos_buffer.Get(i - 1) * ( period - 1 ) + ( diff > 0.0 ? diff : 0.0 ) ) / period);
		neg_buffer.Set(i, ( neg_buffer.Get(i - 1) * ( period - 1 ) + ( diff < 0.0 ? -diff : 0.0 ) ) / period);
		double rsi = (1.0 - 1.0 / ( 1.0 + pos_buffer.Get(i) / neg_buffer.Get(i) )) * 100;
		buffer.Set(i, rsi);
		prev_value = value;
		
		sig.signal.Set(i, rsi < 50);
	}
}






RelativeVigorIndex::RelativeVigorIndex() {
	period = 10;
}

void RelativeVigorIndex::Init() {
	
}

void RelativeVigorIndex::Start()
{
	Buffer& buffer = GetBuffer(0);
	Buffer& signal_buffer = GetBuffer(1);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	double value_up, value_down, num, denum;

	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period + 8 )
		throw DataExc();

	if ( counted < 0 )
		throw DataExc();

	if (counted < 4+period) counted = 4+period;
	
	//bars--;
	
	for (int i = counted; i < bars; i++) {
		num = 0.0;
		denum = 0.0;

		for ( int j = i; j > i - period && j >= 0; j-- ) {
			double close0 = Open( j ); // peek is fixed at for loop
			double close1 = Open( j - 1 );
			double close2 = Open( j - 2 );
			double close3 = Open( j - 3 );

			double open0 = Open ( j - 1 );
			double open1 = Open ( j - 2 );
			double open2 = Open ( j - 3 );
			double open3 = Open ( j - 4 );

			double high0 = High( j - 1 );
			double high1 = High( j - 2 );
			double high2 = High( j - 3 );
			double high3 = High( j - 4 );

			double low0 = Low( j - 1 );
			double low1 = Low( j - 2 );
			double low2 = Low( j - 3 );
			double low3 = Low( j - 4 );

			value_up = ( ( close0 - open0 ) + 2 * ( close1 - open1 ) + 2 * ( close2 - open2 ) + ( close3 - open3 ) ) / 6;
			value_down = ( ( high0 - low0 ) + 2 * ( high1 - low1 ) + 2 * ( high2 - low2 ) + ( high3 - low3 ) ) / 6;

			num += value_up;
			denum += value_down;
		}

		if ( denum != 0.0 )
			buffer.Set(i, num / denum);
		else
			buffer.Set(i, num);
	}

	if ( counted < 8 )
		counted = 8;

	for ( int i = counted; i < bars; i++) {
		double bufvalue = buffer.Get(i);
		double value = ( bufvalue + 2 * buffer.Get(i-1) + 2 * buffer.Get(i-2) + buffer.Get(i-3) ) / 6;
		signal_buffer.Set(i, value);
		sig.signal.Set(i, bufvalue < value);
	}
}







StochasticOscillator::StochasticOscillator() {
	k_period = 5;
	d_period = 3;
	slowing = 3;
}

void StochasticOscillator::Init() {
	
}

void StochasticOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& signal_buffer = GetBuffer(1);
	Buffer& high_buffer = GetBuffer(2);
	Buffer& low_buffer = GetBuffer(3);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	int start;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= k_period + d_period + slowing )
		throw ConfExc();

	start = k_period;

	if ( start + 1 < counted )
		start = counted - 2;
	else {
		for (int i = 0; i < start; i++) {
			low_buffer.Set(i, 0.0);
			high_buffer.Set(i, 0.0);
		}
	}

	int low_length = k_period;
	int high_length = k_period;
	double dmin = 1000000.0;
	double dmax = -1000000.0;
	for (int i = start; i < bars; i++) {
		
		if (low_length >= k_period) {
			dmin = 1000000.0;
			for (int k = i - k_period; k < i; k++) {
				double low = Low( k );
				if ( dmin > low )
					dmin = low;
			}
			low_length = 0;
		} else {
			low_length++;
			double low = Low( i - 1 );
			if ( dmin > low )
				dmin = low;
		}
		
		if (high_length >= k_period) {
			dmax = -1000000.0;
			for (int k = i - k_period; k < i; k++) {
				double high = High( k );
				if ( dmax < high )
					dmax = high;
			}
			high_length = 0;
		} else {
			high_length++;
			double high = High( i - 1 );
			if ( dmax < high )
				dmax = high;
		}

		low_buffer.Set(i, dmin);
		high_buffer.Set(i, dmax);
	}

	start = k_period - 1 + slowing - 1;

	if ( start + 1 < counted )
		start = counted - 2;
	else {
		for (int i = 0; i < start; i++)
			buffer.Set(i, 0.0);
	}

	//bars--;
	double sumlow = 0.0;
	double sumhigh = 0.0;
	for(int i = Upp::max(1, start - slowing); i < start; i++) {
		if (i <= 0) continue;
		double close = Open(i);
		double low = Upp::min(close, low_buffer.Get(i-1));
		double high = Upp::max(close, high_buffer.Get(i-1));
		sumlow  += close - low;
		sumhigh += high - low;
	}
	
	for (int i = Upp::max(1, start); i < bars; i++) {
		int j = i - slowing;
		if (j > 0) {
			double close = Open(j);
			double low = Upp::min(close, low_buffer.Get(j-1));
			double high = Upp::max(close, high_buffer.Get(j-1));
			sumlow  -= close - low;
			sumhigh -= high - low;
		}
		
		
		double close = Open(i);
		double low = Upp::min(close, low_buffer.Get(i-1));
		double high = Upp::max(close, high_buffer.Get(i-1));
		sumlow  += close - low;
		sumhigh += high - low;
		
		
		double value = 1.0;
		if ( sumhigh != 0.0 )
			value = sumlow / sumhigh * 100;
		buffer.Set(i, value);
		sig.signal.Set(i, value < 50);
	}

	start = d_period - 1;

	if ( start + 1 < counted )
		start = counted - 2;
	else {
		for (int i = 0;i < start; i++)
			signal_buffer.Set(i, 0.0);
	}

	for (int i = start; i < bars; i++) {
		double sum = 0.0;

		for (int k = 0; k < d_period; k++ )
			sum += buffer.Get(i - k);

		signal_buffer.Set(i, sum / d_period);
	}
}



















#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#undef DATA_LIMIT
#define DATA_LIMIT  3


AcceleratorOscillator::AcceleratorOscillator() {
	
}

void AcceleratorOscillator::Init() {
	av1.SetPeriod(PERIOD_FAST);
	av2.SetPeriod(PERIOD_SLOW);
}

void AcceleratorOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& up_buffer = GetBuffer(1);
	Buffer& down_buffer = GetBuffer(2);
	Buffer& macd_buffer = GetBuffer(3);
	Buffer& signal_buffer = GetBuffer(4);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	double prev = 0.0, current;
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= DATA_LIMIT )
		throw DataExc();
	
	if ( counted > 0 ) {
		counted--;
		prev = macd_buffer.Get(counted) - signal_buffer.Get(counted);
	}

	for (int i = counted; i < bars; i++) {
		double open = Open(i);
		av1.Add(open);
		av2.Add(open);
		macd_buffer.Set(i,
			av1.GetMean() -
			av2.GetMean());
	}

	SimpleMAOnBuffer ( bars, counted, 0, 5, macd_buffer, signal_buffer );
	
	bool up = true;

	for (int i = counted; i < bars; i++) {
		current = macd_buffer.Get(i) - signal_buffer.Get(i);

		if ( current > prev )
			up = true;

		if ( current < prev )
			up = false;

		if ( !up ) {
			up_buffer.Set(i, 0.0);
			down_buffer.Set(i, current);
		}
		else {
			up_buffer.Set(i, current);
			down_buffer.Set(i, 0.0);
		}

		buffer.Set(i, current);
		
		sig.signal.Set(i, current < 0);

		prev = current;
	}
}




















#undef DATA_LIMIT
#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#define DATA_LIMIT  34

AwesomeOscillator::AwesomeOscillator() {
	
}

void AwesomeOscillator::Init() {
	av1.SetPeriod(PERIOD_FAST);
	av2.SetPeriod(PERIOD_SLOW);
}

void AwesomeOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& up_buffer = GetBuffer(1);
	Buffer& down_buffer = GetBuffer(2);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	
	int bars = GetBars();
	int counted = GetCounted();
	double prev = 0.0, current;
	
	if ( bars <= DATA_LIMIT )
		throw DataExc();

	if (counted > 0) {
		counted--;
		if (counted)
			prev = buffer.Get(counted - 1);
	}

	for (int i = counted; i < bars; i++) {
		double open = Open(i);
		av1.Add(open);
		av2.Add(open);
		buffer.Set(i,
			av1.GetMean() -
			av2.GetMean());
	}

	bool up = true;

	for (int i = counted; i < bars; i++) {
		current = buffer.Get(i);

		if ( current > prev )
			up = true;

		if ( current < prev )
			up = false;

		if ( !up ) {
			down_buffer.Set(i, current);
			up_buffer.Set(i, 0.0);
		} else {
			up_buffer.Set(i, current);
			down_buffer.Set(i, 0.0);
		}
		
		sig.signal.Set(i, current < 0);

		prev = current;
	}
}


























Psychological::Psychological() {
	period = 25;
}

void Psychological::Init() {

}

void Psychological::Start() {
	Buffer& buf = GetBuffer(0);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if(counted < 0)
		throw DataExc();
	
	if(counted > 0)
		counted--;
	
	if (bars < period)
		throw DataExc();
	
	if (counted == 0)
		counted = (1 + period + 1);
	
	//bars--;
	
	for(int i = counted; i < bars; i++) {
		int count=0;
		for (int j=i-period+1; j <= i; j++) {
			if (Open(j) > Open(j-1)) {
				count++;
			}
		}
		if (Open(i) > Open(i-1)) {
			count++;
		}
		if (Open(i-period) > Open(i-period-1)) {
			count--;
		}
		double value = ((double)count / period) *100.0;
		buf.Set(i, value);
		
		sig.signal.Set(i, value < 50);
	}
}













ChannelOscillator::ChannelOscillator()
{
	period = 300;
}

void ChannelOscillator::Init() {
	
}

void ChannelOscillator::Start() {
	Buffer& osc = GetBuffer(0);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	int bars = GetBars();
	int counted = GetCounted();
	
	ec.SetSize(period);
	
	for (int i = ec.pos+1; i < bars; i++) {
		double open = Open(i);
		double low = i > 0 ? Low(i-1) : open;
		double high = i > 0 ? High(i-1) : open;
		double max = Upp::max(open, high);
		double min = Upp::min(open, low);
		ec.Add(max, min);
		
		double ch_high = High(ec.GetHighest());
		double ch_low = Low(ec.GetLowest());
		double ch_diff = ch_high - ch_low;
		
		double value = (open - ch_low) / ch_diff * 2.0 - 1.0;
		osc.Set(i, value);
		
		sig.signal.Set(i, value < 0);
	}
}














ScissorChannelOscillator::ScissorChannelOscillator()
{
	period = 30;
}

void ScissorChannelOscillator::Init() {
	
}

void ScissorChannelOscillator::Start() {
	Buffer& osc = GetBuffer(0);
	LabelSignal& sig = GetLabelBuffer(0, 0);
	int bars = GetBars();
	int counted = GetCounted();
	
	ec.SetSize(period);
	
	for (int i = ec.pos+1; i < bars; i++) {
		double open = Open(i);
		double low = i > 0 ? Low(i-1) : open;
		double high = i > 0 ? High(i-1) : open;
		double max = Upp::max(open, high);
		double min = Upp::min(open, low);
		ec.Add(max, min);
		
		int high_pos = ec.GetHighest();
		int low_pos = ec.GetLowest();
		double ch_high = High(high_pos);
		double ch_low = Low(low_pos);
		double ch_diff = ch_high - ch_low;
		
		double highest_change = DBL_MAX;
		int highest_change_pos = 0;
		double lowest_change = DBL_MAX;
		int lowest_change_pos = 0;
		
		for(int j = Upp::min(high_pos+1, low_pos+1); j <= i; j++) {
			double open = Open(j);
			
			if (j >= high_pos) {
				int dist = j - high_pos;
				double change = ch_high - open;
				double av_change = change / dist;
				if (av_change < highest_change) {
					highest_change = av_change;
					highest_change_pos = j;
				}
			}
			
			if (j >= low_pos) {
				int dist = j - low_pos;
				double change = open - ch_low;
				double av_change = change / dist;
				if (av_change < lowest_change) {
					lowest_change = av_change;
					lowest_change_pos = j;
				}
			}
		}
		
		int high_dist = i - high_pos;
		double high_limit = ch_high - high_dist * highest_change;
		int low_dist = i - low_pos;
		double low_limit = ch_low + low_dist * lowest_change;
		
		double limit_diff = high_limit - low_limit;
		double value = (open - low_limit) / limit_diff * 2.0 - 1.0;
		if (!IsFin(value)) value = 0.0;
		osc.Set(i, value);
		
		sig.signal.Set(i, value < 0);
	}
}



















OnlineMinimalLabel::OnlineMinimalLabel() {
	
}

void OnlineMinimalLabel::Init() {
	
}

void OnlineMinimalLabel::Start() {
	int bars = GetBars();
	
	double point			= 0.0001;
	double cost				= 2 * point + point * cost_level;
	if (cost <= 0.0) cost = point;
	
	VectorBool& labelvec = GetLabelBuffer(0,0).signal;
	
	for(int i = GetCounted(); i < bars; i++) {
		const int count = 1;
		bool sigbuf[count];
		int begin = Upp::max(0, i - 100);
		int end = i + 1;
		GetMinimalSignal(cost, begin, end, sigbuf, count);
		
		bool label = sigbuf[count - 1];
		labelvec.Set(i, label);
	}
}

void OnlineMinimalLabel::GetMinimalSignal(double cost, int begin, int end, bool* sigbuf, int sigbuf_size) {
	int write_begin = end - sigbuf_size;
	
	for(int i = begin; i < end; i++) {
		double open = Open(i);
		double close = open;
		
		for (int j = i-1; j >= begin; j--) {
			open = Open(j);
			double abs_diff = fabs(close - open);
			if (abs_diff >= cost) {
				break;
			}
		}
		bool label = close < open;
		
		int buf_pos = i - write_begin;
		if (buf_pos >= 0)
			sigbuf[buf_pos] = label;
	}
}




















Laguerre::Laguerre() {
	
	
	
}

void Laguerre::Init() {
	
}

void Laguerre::Start() {
	LabelSignal& sig = GetLabelBuffer(0, 0);
	Buffer& dst = GetBuffer(0);
	
	int bars = GetBars() - 1;
	int counted = GetCounted();
	
	if (counted)
		counted--;
	else
		counted++;
		
	double gammaf = gamma * 0.01;
	
	for (int i = counted; i < bars; i++) {
		gd_120 = gd_88;
		gd_128 = gd_96;
		gd_136 = gd_104;
		gd_144 = gd_112;
		gd_88 = (1 - gammaf) * Open(i) + gammaf * gd_120;
		gd_96 = (-gammaf) * gd_88 + gd_120 + gammaf * gd_128;
		gd_104 = (-gammaf) * gd_96 + gd_128 + gammaf * gd_136;
		gd_112 = (-gammaf) * gd_104 + gd_136 + gammaf * gd_144;
		gd_160 = 0;
		gd_168 = 0;
		
		if (gd_88 >= gd_96)
			gd_160 = gd_88 - gd_96;
		else
			gd_168 = gd_96 - gd_88;
			
		if (gd_96 >= gd_104)
			gd_160 = gd_160 + gd_96 - gd_104;
		else
			gd_168 = gd_168 + gd_104 - gd_96;
			
		if (gd_104 >= gd_112)
			gd_160 = gd_160 + gd_104 - gd_112;
		else
			gd_168 = gd_168 + gd_112 - gd_104;
			
		if (gd_160 + gd_168 != 0.0)
			gd_152 = gd_160 / (gd_160 + gd_168);
		
		dst.Set(i, gd_152);
		
		sig.signal.Set(i, gd_152 <= 0.5);
	}
}














TickBalanceOscillator::TickBalanceOscillator() {
	
}

void TickBalanceOscillator::Init() {
	
}

void TickBalanceOscillator::Start() {
	LabelSignal& sig = GetLabelBuffer(0, 0);
	Buffer& dst = GetBuffer(0);
	
	int bars = GetBars();
	int counted = GetCounted();
	
	if (counted)
		counted--;
	for (int i = max(period+1, counted); i < bars; i++) {
		double pos_sum = 0, neg_sum = 0;
		int pos_count = 0, neg_count = 0;
		for(int j = 0; j < period; j++) {
			double o0 = Open(i-j);
			double o1 = Open(i-j-1);
			double diff = o0 - o1;
			if (diff > 0)	{pos_sum += diff; pos_count++;}
			else			{neg_sum -= diff; neg_count++;}
		}
		double pos_av = pos_count ? pos_sum / pos_count : 0;
		double neg_av = neg_count ? neg_sum / neg_count : 0;
		double d = pos_av - neg_av;

		dst.Set(i, d);
		
		sig.signal.Set(i, d < 0);
	}
}











}
