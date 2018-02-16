#include "Overlook.h"

namespace Overlook {

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
	if ( prev_calculated == 0 ) {
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











MovingAverage::MovingAverage()
{
	ma_period = 13;
	ma_shift = -6;
	ma_method = 0;
	ma_counted = 0;
}

void MovingAverage::Init() {
	int draw_begin;
	if (ma_period < 2)
		ma_period = 13;
	draw_begin = ma_period - 1;
	
	SetBufferColor(0, Red());
	SetBufferShift(0, ma_shift);
	SetBufferBegin(0, draw_begin );
}

void MovingAverage::Assist(int cursor, VectorBool& vec) {
	double open = Open(cursor);
	double ma = GetBuffer(0).Get(cursor);
	if (open > ma)	vec.Set(MA_OVERAV, true);
	else			vec.Set(MA_BELOWAV, true);
	if (cursor > 0) {
		double prev = GetBuffer(0).Get(cursor - 1);
		if (ma > prev)	vec.Set(MA_TRENDUP, true);
		else			vec.Set(MA_TRENDDOWN, true);
	}
}

void MovingAverage::Start() {
	int bars = GetBars();
	if ( bars <= ma_period )
		throw DataExc();
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
	VectorBool& label = outputs[0].label;
	label.SetCount(bars);
	SetSafetyLimit(ma_counted-1);
	double prev = ma_counted > 0 ? buffer.Get(ma_counted-1) : 0.0;
	for(int i = ma_counted; i < bars; i++) {
		SetSafetyLimit(i);
		double cur = buffer.Get(i);
		bool label_value = cur < prev;
		label.Set(i, label_value);
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
	SetSafetyLimit(pos);
	for (int i = 1; i < ma_period; i++)
		sum += Open(pos - i);
	while (pos < bars) {
		SetSafetyLimit(pos);
		sum += Open( pos );
		buffer.Set(pos, sum / ma_period);
		sum -= Open( pos - ma_period + 1 );
		pos++;
	}
	if (ma_counted < 1)
		for (int i = 0; i < ma_period; i++)
			buffer.Set(i, 0);
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
		SetSafetyLimit(pos);
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
		SetSafetyLimit(pos);
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
		SetSafetyLimit(pos);
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
		SetSafetyLimit(pos);
		value = Open(pos);
		sum = sum - lsum + value * ma_period;
		lsum -= Open(i);
		lsum += value;
	}
	if ( ma_counted < 1 )
		for (i = 0; i < ma_period; i++)
			buffer.Set(i, 0);
}













MovingAverageConvergenceDivergence::MovingAverageConvergenceDivergence() {
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
}

void MovingAverageConvergenceDivergence::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Silver);
	SetBufferColor(1, Red);
	SetBufferLineWidth(0, 2);
	
	SetBufferBegin ( 1, signal_sma_period );
	
	SetBufferStyle(0, DRAW_HISTOGRAM);
	SetBufferStyle(1, DRAW_LINE);
	
	SetBufferLabel(0,"MACD");
	SetBufferLabel(1,"Signal");
	
	AddSubCore<MovingAverage>().Set("period", fast_ema_period);
	AddSubCore<MovingAverage>().Set("period", slow_ema_period);
}

void MovingAverageConvergenceDivergence::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& signal_buffer = GetBuffer(1);
	
	int bars = GetBars();
	if ( bars <= signal_sma_period )
		throw DataExc();

	int counted = GetCounted();
	if ( counted > 0 )
		counted--;

	const Core& a_ind = At(0);
	const Core& b_ind = At(1);
	ConstBuffer& a_buf = a_ind.GetBuffer(0);
	ConstBuffer& b_buf = b_ind.GetBuffer(0);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double a_value = a_buf.Get(i);
		double b_value = b_buf.Get(i);
		double diff = a_value - b_value;
		buffer.Set(i, diff);
	}

	SimpleMAOnBuffer( bars, GetCounted(), 0, signal_sma_period, buffer, signal_buffer );
}

void MovingAverageConvergenceDivergence::Assist(int cursor, VectorBool& vec) {
	double open = Open(cursor);
	double macd = GetBuffer(0).Get(cursor);
	if (macd > 0.0)		vec.Set(MACD_OVERZERO, true);
	else				vec.Set(MACD_BELOWZERO, true);
	if (cursor > 0) {
		double prev = GetBuffer(0).Get(cursor - 1);
		if (macd > prev)	vec.Set(MACD_TRENDUP, true);
		else				vec.Set(MACD_TRENDDOWN, true);
	}
}






AverageDirectionalMovement::AverageDirectionalMovement() {
	period_adx = 14;
}

void AverageDirectionalMovement::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1,  YellowGreen);
	SetBufferColor(2,  Wheat);
	
	SetBufferLabel(0,"ADX");
	SetBufferLabel(1,"+DI");
	SetBufferLabel(2,"-DI");
	
	if ( period_adx >= 128 || period_adx <= 1 )
		throw DataExc();
	
	SetBufferBegin ( 0, period_adx );
	SetBufferBegin ( 1, period_adx );
	SetBufferBegin ( 2, period_adx );
}

void AverageDirectionalMovement::Start() {
	Buffer& adx_buffer = GetBuffer(0);
	Buffer& pdi_buffer = GetBuffer(1);
	Buffer& ndi_buffer = GetBuffer(2);
	Buffer& pd_buffer  = GetBuffer(3);
	Buffer& nd_buffer  = GetBuffer(4);
	Buffer& tmp_buffer = GetBuffer(5);
	
	int bars = GetBars();
	if ( bars < period_adx )
		throw DataExc();

	int counted = GetCounted();

	if ( counted > 0 )
		counted--;
	else {
		counted = 2;
		for(int i = 0; i < 2; i++) {
			SetSafetyLimit(i);
			pdi_buffer.Set(i, 0.0);
			ndi_buffer.Set(i, 0.0);
			adx_buffer.Set(i, 0.0);
		}
	}
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double Hi		= High(i - 1);
		double prev_hi	= High(i - 2);
		double Lo		= Low(i - 1);
		double prev_lo	= Low(i - 2);
		double prev_cl	= Open(i);

		double tmp_p = Hi - prev_hi;
		double tmp_n = prev_lo - Lo;

		if ( tmp_p < 0.0 )
			tmp_p = 0.0;

		if ( tmp_n < 0.0 )
			tmp_n = 0.0;

		if ( tmp_p > tmp_n )
			tmp_n = 0.0;
		else
		{
			if ( tmp_p < tmp_n )
				tmp_p = 0.0;
			else
			{
				tmp_p = 0.0;
				tmp_n = 0.0;
			}
		}

		double tr = Upp::max( Upp::max( fabs ( Hi - Lo ), fabs ( Hi - prev_cl ) ), fabs ( Lo - prev_cl ) );

		if ( tr != 0.0 )
		{
			pd_buffer.Set(i, 100.0 * tmp_p / tr);
			nd_buffer.Set(i, 100.0 * tmp_n / tr);
		}

		else
		{
			pd_buffer.Set(i, 0);
			nd_buffer.Set(i, 0);
		}

		pdi_buffer.Set(i, ExponentialMA ( i, period_adx, pdi_buffer.Get( i - 1 ), pd_buffer ));
		ndi_buffer.Set(i, ExponentialMA ( i, period_adx, ndi_buffer.Get( i - 1 ), nd_buffer ));
		

		double tmp = pdi_buffer.Get(i) + ndi_buffer.Get(i);

		if ( tmp != 0.0 )
			tmp = 100.0 * fabs ( ( pdi_buffer.Get(i) - ndi_buffer.Get(i) ) / tmp );
		else
			tmp = 0.0;

		tmp_buffer.Set(i, tmp);
		adx_buffer.Set(i, ExponentialMA ( i, period_adx, adx_buffer.Get ( i - 1 ), tmp_buffer ));
	}
}












BollingerBands::BollingerBands()
{
	bands_period = 20;
	bands_shift = 0;
	bands_deviation = 2.0;
	deviation = 20;
	
	plot_begin = 0;
}

void BollingerBands::Init() {
	SetCoreChartWindow();
	
	bands_deviation = deviation * 0.1;
	
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1,  LightSeaGreen);
	SetBufferColor(2,  LightSeaGreen);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferShift(0,bands_shift);
	SetBufferLabel(0,"Bands SMA");
	
	SetBufferStyle(1,DRAW_LINE);
	SetBufferShift(1,bands_shift);
	SetBufferLabel(1,"Bands Upper");
	
	SetBufferStyle(2,DRAW_LINE);
	SetBufferShift(2,bands_shift);
	SetBufferLabel(2,"Bands Lower");
	
	if ( bands_period < 2 )
		throw DataExc();
	
	if ( bands_deviation == 0.0 )
		throw DataExc();
	
	plot_begin = bands_period - 1;
	SetBufferBegin ( 0, bands_period );
	SetBufferBegin ( 1, bands_period );
	SetBufferBegin ( 2, bands_period );
	
	SetBufferShift ( 0, bands_shift );
	SetBufferShift ( 1, bands_shift );
	SetBufferShift ( 2, bands_shift );
}

void BollingerBands::Start() {
	Buffer& ml_buffer = GetBuffer(0);
	Buffer& tl_buffer = GetBuffer(1);
	Buffer& bl_buffer = GetBuffer(2);
	Buffer& stddev_buffer = GetBuffer(3);
	
	int bars = GetBars();
	int pos;
	int counted = GetCounted();
	
	if ( bars < plot_begin )
		throw DataExc();
	
	if ( counted > 1 )
		pos = counted - 1;
	else
		pos = 0;
	
	ConstBuffer& open = GetInputBuffer(0, 0);
	
	for ( int i = pos; i < bars; i++) {
		SetSafetyLimit(i);
		ml_buffer.Set(i, SimpleMA( i, bands_period, open ));
		stddev_buffer.Set(i, StdDev_Func ( i, ml_buffer, bands_period ));
		tl_buffer.Set(i, ml_buffer.Get(i) + bands_deviation * stddev_buffer.Get(i));
		bl_buffer.Set(i, ml_buffer.Get(i) - bands_deviation * stddev_buffer.Get(i));
	}
}

void BollingerBands::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double high = High(cursor - 1);
		double low  = Low(cursor - 1);
		double top = GetBuffer(1).Get(cursor);
		double bot = GetBuffer(2).Get(cursor);
		if (high >= top)		vec.Set(BB_HIGHBAND, true);
		if (low  <= bot)		vec.Set(BB_LOWBAND, true);
	}
}

double BollingerBands::StdDev_Func ( int position, const Buffer& MAvalue, int period ) {
	double tmp = 0.0;

	if ( position < period )
		return tmp;
	
	int bars = GetBars();
	for (int i = 0; i < period; i++) {
		double value = Open (position - i );
		tmp += pow ( value - MAvalue.Get( position ), 2 );
	}

	tmp = sqrt( tmp / period );

	return tmp;
}





Envelopes::Envelopes() {
	ma_period = 14;
	ma_shift = 0;
	ma_method = MODE_SMA;
	deviation = 0.1;
	dev = 10;
}

void Envelopes::Init() {
	SetCoreChartWindow();
	
	deviation = dev * 0.1;
	
	SetBufferColor(0, Blue);
	SetBufferColor(1, Red);
	
	SetBufferBegin ( 0, ma_period - 1 );
	SetBufferShift ( 0, ma_shift );
	SetBufferShift ( 1, ma_shift );
	
	SetBufferStyle(0, DRAW_LINE);
	SetBufferStyle(1, DRAW_LINE);
	
	AddSubCore<MovingAverage>().Set("period", ma_period).Set("offset", 0).Set("method", ma_method);
}


void Envelopes::Start() {
	Buffer& up_buffer = GetBuffer(0);
	Buffer& down_buffer = GetBuffer(1);
	Buffer& ma_buffer = GetBuffer(2);
	
	int bars = GetBars();

	if ( bars < ma_period )
		throw DataExc();

	int counted = GetCounted();
	if (counted) counted--;
	else counted = ma_period;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double value = Open(i);
		ASSERT(value != 0);
		ma_buffer.Set(i, value);
		up_buffer.Set(i, ( 1 + deviation / 100.0 ) * ma_buffer.Get(i));
		down_buffer.Set(i, ( 1 - deviation / 100.0 ) * ma_buffer.Get(i));
	}
}








ParabolicSAR::ParabolicSAR() {
	sar_step = 0.02;
	sar_maximum = 0.2;
	step = 20;
	maximum = 20;
}

void ParabolicSAR::Init() {
	SetCoreChartWindow();
	
	sar_step = step * 0.001;
	sar_maximum = maximum * 0.01;
	
	SetBufferColor(0, Lime);
	
	if (sar_step <= 0.0)
		throw DataExc();
	
	if (sar_maximum <= 0.0)
		throw DataExc();
	
	last_rev_pos = 0;
	direction_long = false;
	
	SetBufferStyle(0, DRAW_ARROW);
	SetBufferArrow(0, 159);
}

void ParabolicSAR::Start () {
	Buffer& sar_buffer = GetBuffer(0);
	Buffer& ep_buffer  = GetBuffer(1);
	Buffer& af_buffer  = GetBuffer(2);
	
	int counted = GetCounted();
	int bars = GetBars();

	if ( bars < 3 )
		throw DataExc();
	
	int pos = counted - 1;

	if ( pos < 1 ) {
		SetSafetyLimit(1);
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
	
	SetSafetyLimit(prev_pos+1);
	double prev_high = High( prev_pos );
	double prev_low  = Low( prev_pos );
	
	//bars--;
	
	for (int i = pos; i < bars; i++) {
		SetSafetyLimit(i);
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

void ParabolicSAR::Assist(int cursor, VectorBool& vec) {
	double open = Open(cursor);
	double sar = GetBuffer(0).Get(cursor);
	if (open > sar)		vec.Set(PSAR_TRENDUP, true);
	else				vec.Set(PSAR_TRENDDOWN, true);
}









StandardDeviation::StandardDeviation() {
	period = 20;
	ma_method = 0;
}

void StandardDeviation::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(0);
	
	SetBufferColor(0, Blue);
	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "StdDev");
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", ma_method);
}

void StandardDeviation::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > std1)	vec.Set(STDDEV_INC, true);
		else				vec.Set(STDDEV_DEC, true);
	}
}

void StandardDeviation::Start() {
	Buffer& stddev_buffer = GetBuffer(0);
	double value, amount, ma;

	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();

	int counted = GetCounted();

	if ( counted > 0 )
		counted--;
	else
		counted = period;
	
	ConstBuffer& ma_buf = At(0).GetBuffer(0);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		amount = 0.0;
		ma = ma_buf.Get(i);

		for ( int j = 0; j < period; j++ ) {
			value = Open( i - j );
			amount += ( value - ma ) * ( value - ma );
		}

		stddev_buffer.Set(i, sqrt ( amount / period ));
	}
}










AverageTrueRange::AverageTrueRange() {
	period = 14;
}

void AverageTrueRange::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "ATR");
	
	if ( period <= 0 )
		throw DataExc();
	
	SetBufferBegin ( 0, period );
}

void AverageTrueRange::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > std1)	vec.Set(ATR_INC, true);
		else				vec.Set(ATR_DEC, true);
	}
}

void AverageTrueRange::Start() {
	Buffer& atr_buffer = GetBuffer(0);
	Buffer& tr_buffer = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period || period <= 0 )
		throw DataExc();
	
	if ( counted == 0 ) {
		tr_buffer.Set(0, 0.0);
		atr_buffer.Set(0, 0.0);
		
		for (int i = 1; i < bars; i++) {
			SetSafetyLimit(i);
			double h = High(i-1);
			double l = Low(i-1);
			double c = Open(i);

			tr_buffer.Set(i, Upp::max( h, c ) - Upp::min( l, c ));
		}

		double first_value = 0.0;

		for (int i = 1; i <= period; i++) {
			atr_buffer.Set(i, 0.0);
			first_value += tr_buffer.Get(i);
		}

		first_value /= period;

		atr_buffer.Set(period, first_value);

		counted = period + 1;
	}
	else
		counted--;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double h = High(i-1);
		double l = Low(i-1);
		double c = Open(i);

		tr_buffer.Set(i, Upp::max( h, c ) - Upp::min( l, c ));
		atr_buffer.Set(i, atr_buffer.Get( i - 1 ) + ( tr_buffer.Get(i) - tr_buffer.Get( i - period ) ) / period);
	}
}













BearsPower::BearsPower() {
	period = 13;
}

void BearsPower::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Silver);
	SetBufferStyle(0,DRAW_HISTOGRAM);
	SetBufferLabel(0,"ATR");
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", MODE_EMA);
}

void BearsPower::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(BEAR_OVERZERO, true);
		else				vec.Set(BEAR_BELOWZERO, true);
		if (std0 > std1)	vec.Set(BEAR_INC, true);
		else				vec.Set(BEAR_DEC, true);
	}
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
	
	ConstBuffer& ma_buf = At(0).GetBuffer(0);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		buffer.Set(i, Low(i-1) - ma_buf.Get(i));
	}
}









BullsPower::BullsPower() {
	period = 13;
}

void BullsPower::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Silver);
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	SetBufferLabel(0,"Bulls");
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", MODE_EMA);
}

void BullsPower::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(BULL_OVERZERO, true);
		else				vec.Set(BULL_BELOWZERO, true);
		if (std0 > std1)	vec.Set(BULL_INC, true);
		else				vec.Set(BULL_DEC, true);
	}
}

void BullsPower::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	else
		counted++;
	
	ConstBuffer& ma_buf = At(0).GetBuffer(0);
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		buffer.Set(i, High(i-1) - ma_buf.Get(i) );
	}
}













CommodityChannelIndex::CommodityChannelIndex() {
	period = 14;
}

void CommodityChannelIndex::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, LightSeaGreen);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -100.0);
	SetCoreLevel(1,  100.0);
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"CCI");
	
	if ( period <= 1 )
		throw DataExc();
	
	SetBufferBegin ( 0, period );
}

void CommodityChannelIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(CCI_OVERZERO, true);
		else				vec.Set(CCI_BELOWZERO, true);
		if (std0 > +100.0)	vec.Set(CCI_OVERHIGH, true);
		if (std0 < -100.0)	vec.Set(CCI_BELOWLOW, true);
		if (std0 > std1)	vec.Set(CCI_INC, true);
		else				vec.Set(CCI_DEC, true);
	}
}

void CommodityChannelIndex::Start() {
	Buffer& cci_buffer = GetBuffer(0);
	Buffer& value_buffer = GetBuffer(1);
	Buffer& mov_buffer = GetBuffer(2);
	
	int i, k, pos;
	double sum, mul;
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period || period <= 1 )
		throw DataExc();
	
	if ( counted < 1 ) {
		for ( i = 1; i < period; i++) {
			SetSafetyLimit(i);
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
		SetSafetyLimit(i);
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
		SetSafetyLimit(i);
		sum = 0.0;
		k = i + 1 - period;
		SafetyInspect(k);
		
		while ( k <= i ) {
			SafetyInspect(k);
			sum += fabs ( value_buffer.Get(k) - mov_buffer.Get(i) );
			k++;
		}

		sum *= mul;

		if ( sum == 0.0 )
			cci_buffer.Set(i, 0.0);
		else
			cci_buffer.Set(i, ( value_buffer.Get(i) - mov_buffer.Get(i) ) / sum);

		i++;
	}
}







DeMarker::DeMarker() {
	period = 14;
}

void DeMarker::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-0.5);  // normalized
	SetCoreMaximum(0.5);   // normalized
	
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -0.2); // normalized
	SetCoreLevel(1, +0.2); // normalized
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"DeMarker");
	
	SetBufferBegin ( 0, period );
}

void DeMarker::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = GetBuffer(0).Get(cursor);
		double std1 = GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(DEM_OVERZERO, true);
		else				vec.Set(DEM_BELOWZERO, true);
		if (std0 > +0.2)	vec.Set(DEM_OVERHIGH, true);
		if (std0 < -0.2)	vec.Set(DEM_BELOWLOW, true);
		if (std0 > std1)	vec.Set(DEM_INC, true);
		else				vec.Set(DEM_DEC, true);
	}
}

void DeMarker::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& max_buffer = GetBuffer(1);
	Buffer& min_buffer = GetBuffer(2);
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
	
	SetSafetyLimit(counted);
	double prev_high = counted >= 2 ? High( counted - 1 ) : 0;
	double prev_low  = counted >= 2 ? Low( counted - 1 ) : 0;

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
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
		SetSafetyLimit(i);
		double maxvalue = SimpleMA ( i, period, max_buffer );
		num = maxvalue + SimpleMA ( i, period, min_buffer );

		if ( num != 0.0 )
			buffer.Set(i, maxvalue / num - 0.5);  // normalized
		else
			buffer.Set(i, 0.0);
	}
}










ForceIndex::ForceIndex() {
	period = 13;
	ma_method = 0;
}

void ForceIndex::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferBegin(0,period);
	
	SetBufferBegin ( 0, period );
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", ma_method);
}

void ForceIndex::Assist(int cursor, VectorBool& vec) {
	/*if (cursor > 0) {
		double force0 = GetBuffer(0).Get(cursor);
		double force1 = GetBuffer(0).Get(cursor-1);
		if (force0 > 0.0)		vec.Set(FORCE_OVERZERO, true);
		else					vec.Set(FORCE_BELOWZERO, true);
		if (force0 > force1)	vec.Set(FORCE_INC, true);
		else					vec.Set(FORCE_DEC, true);
	}*/
}

void ForceIndex::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (bars <= period)
		throw DataExc();
	
	if (counted > 0)
		counted--;
	
	if (counted == 0)
		counted++;
	
	ConstBuffer& ma_buf = At(0).GetBuffer(0);
	
	for ( int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double volume = Volume(i-1);
		double ma1 = ma_buf.Get(i);
		double ma2 = ma_buf.Get( i - 1 );
		buffer.Set(i, volume * (ma1 - ma2));
	}
}











Momentum::Momentum()
{
	period = 32;
	shift = -7;
}

void Momentum::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	SetBufferColor(1, Color(28, 255, 200));
	
	if ( period <= 0 )
		throw DataExc();

	SetBufferBegin(0, period);
	SetBufferShift(0, shift);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"Momentum");
}

void Momentum::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(MOM_OVERZERO, true);
		else					vec.Set(MOM_BELOWZERO, true);
		if (value0 > value1)	vec.Set(MOM_INC, true);
		else					vec.Set(MOM_DEC, true);
	}
}

void Momentum::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& change_buf = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period || period <= 0 )
		throw DataExc();

	if ( counted <= 0 ) {
		for (int i = 0; i < period; i++) {
			SetSafetyLimit(i);
			buffer.Set(i, 0.0);
		}
		counted = period;
	}
	else
		counted--;
	
	SetSafetyLimit(counted-1);
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double close1 = Open( i );
		double close2 = Open( i - period );
		double value = close1 * 100 / close2 - 100;
		buffer.Set(i, value);
		
		double prev = buffer.Get(i-1);
		double change = (value - prev) * 10.0;
		change_buf.Set(i, change);
	}
}








OsMA::OsMA() {
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
	value_mean = 0.0;
	value_count = 0;
	diff_mean = 0.0;
	diff_count = 0;
}

void OsMA::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Silver);
	SetBufferLineWidth(0, 2);
	SetBufferColor(1, Red);
	SetBufferLineWidth(1, 1);
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	SetBufferStyle(1,DRAW_LINE);
	
	SetBufferBegin ( 0, signal_sma_period );

	//	AddSubCore<MovingAverage>().Set("period", fast_ema_period).Set("offset", 0).Set("method", MODE_EMA);
	//	AddSubCore<MovingAverage>().Set("period", slow_ema_period).Set("offset", 0).Set("method", MODE_EMA);
}



	
void OsMA::Start() {
	Buffer& osma_buffer = GetBuffer(0);
	Buffer& diff_buffer = GetBuffer(1);
	Buffer& buffer = GetBuffer(2);
	Buffer& signal_buffer = GetBuffer(3);
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= signal_sma_period )
		return;

	if ( counted > 0 )
		counted--;
	
	//	ConstBuffer& ma1_buf = At(0).GetBuffer(0);
	//	ConstBuffer& ma2_buf = At(1).GetBuffer(0);
	ConstBuffer& ma1_buf = GetInputBuffer(1, 0);
	ConstBuffer& ma2_buf = GetInputBuffer(2, 0);
	ASSERT(ma1_buf.GetCount() >= bars);
	ASSERT(ma2_buf.GetCount() >= bars);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double ma1 = ma1_buf.GetUnsafe(i);
		double ma2 = ma2_buf.GetUnsafe(i);
		double d = ma1 - ma2;
		buffer.Set(i, d);
	}
	
	SetSafetyLimit(bars);
	SimpleMAOnBuffer( bars, counted, 0, signal_sma_period, buffer, signal_buffer );
	
	for (int i = counted; i < bars; i++) {
		double d = buffer.Get(i) - signal_buffer.Get(i);
		if (d != 0.0)
			AddValue(fabs(d));
		osma_buffer.Set(i, d);
	}
	
	double value_mul = 1.0 / (value_mean * 3.0);
	for (int i = counted; i < bars; i++) {
		double d = osma_buffer.Get(i) * value_mul;
		osma_buffer.Set(i, d);
		if (i == 0) continue;
		double prev = osma_buffer.Get(i-1);
		double diff = d - prev;
		if (diff != 0.0)
			AddDiff(fabs(diff));
		diff_buffer.Set(i, diff);
	}
	
	double diff_mul = 1.0 / (diff_mean * 3.0);
	for (int i = counted; i < bars; i++) {
		double d = diff_buffer.Get(i) * diff_mul;
		diff_buffer.Set(i, d);
	}
	
	for (int i = counted; i < bars; i++) {
		osma_buffer.Set(i, Upp::max(-1.0, Upp::min(+1.0, osma_buffer.Get(i))));
		diff_buffer.Set(i, Upp::max(-1.0, Upp::min(+1.0, diff_buffer.Get(i))));
	}
}














RelativeStrengthIndex::RelativeStrengthIndex()
{
	period = 32;
}

void RelativeStrengthIndex::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1); // normalized
	SetCoreMaximum(+1);  // normalized
	
	SetBufferColor(0, DodgerBlue);
	SetBufferColor(1, White());
	SetBufferColor(2, White());
	SetBufferColor(3, Color(28, 255, 200));
	SetCoreLevelCount(2);
	SetCoreLevel(0, +0.4); // normalized
	SetCoreLevel(1, -0.4); // normalized
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	if ( period < 1 )
		throw DataExc();
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "RSI");
}

void RelativeStrengthIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(RSI_OVERZERO, true);
		else					vec.Set(RSI_BELOWZERO, true);
		if (value0 > value1)	vec.Set(RSI_INC, true);
		else					vec.Set(RSI_DEC, true);
	}
}

void RelativeStrengthIndex::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& pos_buffer = GetBuffer(1);
	Buffer& neg_buffer = GetBuffer(2);
	Buffer& change_buf = GetBuffer(3);
	
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
			SetSafetyLimit(i);
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
		buffer.Set(period, (1.0 - ( 1.0 / ( 1.0 + pos_buffer.Get(period) / neg_buffer.Get(period) ) )) * 2.0 - 1.0);
		pos = period + 1;
	}
	
	SetSafetyLimit(pos-1);
	double prev_value  = Open(pos-1);

	for (int i = pos; i < bars; i++) {
		SetSafetyLimit(i);
		double value  = Open(i);
		diff = value - prev_value;
		pos_buffer.Set(i, ( pos_buffer.Get(i - 1) * ( period - 1 ) + ( diff > 0.0 ? diff : 0.0 ) ) / period);
		neg_buffer.Set(i, ( neg_buffer.Get(i - 1) * ( period - 1 ) + ( diff < 0.0 ? -diff : 0.0 ) ) / period);
		double rsi = (1.0 - 1.0 / ( 1.0 + pos_buffer.Get(i) / neg_buffer.Get(i) )) * 2.0 - 1.0;
		buffer.Set(i, rsi);
		prev_value = value;
		
		double prev = buffer.Get(i-1);
		double change = (rsi - prev) * 10.0;
		change_buf.Set(i, change);
	}
}






RelativeVigorIndex::RelativeVigorIndex() {
	period = 10;
}

void RelativeVigorIndex::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Green);
	SetBufferColor(1, Red);
	
	SetBufferBegin ( 0, period + 3 );
	SetBufferBegin ( 1, period + 7 );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferStyle(1,DRAW_LINE);
	SetBufferLabel(0,"RVI");
	SetBufferLabel(1,"RVIS");
}

void RelativeVigorIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		double value2 = GetBuffer(1).Get(cursor);
		if (value0 > 0.0)		vec.Set(RVI_OVERZERO, true);
		else					vec.Set(RVI_BELOWZERO, true);
		if (value0 > value1)	vec.Set(RVI_INC, true);
		else					vec.Set(RVI_DEC, true);
		if (value0 > value2)	vec.Set(RVI_INCDIFF, true);
		else					vec.Set(RVI_DECDIFF, true);
	}
}

void RelativeVigorIndex::Start()
{
	Buffer& buffer = GetBuffer(0);
	Buffer& signal_buffer = GetBuffer(1);
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
		SetSafetyLimit(i);
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

	for ( int i = counted; i < bars; i++)
		signal_buffer.Set(i, ( buffer.Get(i) + 2 * buffer.Get(i-1) + 2 * buffer.Get(i-2) + buffer.Get(i-3) ) / 6);
}







StochasticOscillator::StochasticOscillator() {
	k_period = 5;
	d_period = 3;
	slowing = 3;
}

void StochasticOscillator::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1.0); // normalized
	SetCoreMaximum(+1.0);  // normalized
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1, Red);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -0.60); // normalized
	SetCoreLevel(1, +0.60); // normalized
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	SetBufferLabel( 0, "Main");
	SetBufferLabel( 1, "Signal" );
	SetBufferBegin(0, k_period + slowing - 2 );
	SetBufferBegin(1, k_period + d_period );

	SetBufferStyle(0, DRAW_LINE);
	SetBufferStyle(1, DRAW_LINE);
	SetBufferType(1, STYLE_DOT);
	
}

void StochasticOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(STOCH_OVERZERO, true);
		else					vec.Set(STOCH_BELOWZERO, true);
		if (value0 > value1)	vec.Set(STOCH_INC, true);
		else					vec.Set(STOCH_DEC, true);
		if (value0 > +0.60)		vec.Set(STOCH_OVERHIGH, true);
		if (value0 < -0.60)		vec.Set(STOCH_BELOWLOW, true);
	}
}

void StochasticOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& signal_buffer = GetBuffer(1);
	Buffer& high_buffer = GetBuffer(2);
	Buffer& low_buffer = GetBuffer(3);
	int start;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= k_period + d_period + slowing )
		throw DataExc();

	start = k_period;

	if ( start + 1 < counted )
		start = counted - 2;
	else {
		for (int i = 0; i < start; i++) {
			SetSafetyLimit(i);
			low_buffer.Set(i, 0.0);
			high_buffer.Set(i, 0.0);
		}
	}

	int low_length = k_period;
	int high_length = k_period;
	double dmin = 1000000.0;
	double dmax = -1000000.0;
	for (int i = start; i < bars; i++) {
		SetSafetyLimit(i);

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
			SetSafetyLimit(i);
		double close = Open(i);
		double low = Upp::min(close, low_buffer.Get(i-1));
		double high = Upp::max(close, high_buffer.Get(i-1));
		sumlow  += close - low;
		sumhigh += high - low;
	}
	
	for (int i = Upp::max(1, start); i < bars; i++) {
		SetSafetyLimit(i);
		
		
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
		

		if ( sumhigh == 0.0 )
			buffer.Set(i, 1.0); // normalized
		else
			buffer.Set(i, sumlow / sumhigh * 2.0 - 1.0); // normalized
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






WilliamsPercentRange::WilliamsPercentRange()
{
	period = 14;
}

void WilliamsPercentRange::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1); // normalized
	SetCoreMaximum(+1);  // normalized
	SetBufferColor(0, DodgerBlue);
	SetBufferColor(1, Color(28, 255, 200));
	SetCoreLevelCount(2);
	SetCoreLevel(0, -20 + 50); // normalized
	SetCoreLevel(1, -80 + 50); // normalized
	SetBufferBegin ( 0, period );
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"WPR");
}

void WilliamsPercentRange::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& change_buf = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	if (!counted) {
		counted++;
		buffer.Set(0, 0);
	}
	
	//bars--;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		int highest = HighestHigh( period, i-1 );
		int lowest = LowestLow( period, i-1 );
		double max_high = High( highest );
		double min_low = Low( lowest );
		double close = Open( i );
		double cur = -2 * ( max_high - close ) / ( max_high - min_low ) + 1;
		buffer.Set(i, cur); // normalized
		
		double prev = buffer.Get(i-1);
		double change = (cur - prev) * 10.0;
		change_buf.Set(i, change);
	}
}








AccumulationDistribution::AccumulationDistribution() {
	
}

void AccumulationDistribution::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, LightSeaGreen);
	SetBufferStyle(0,DRAW_LINE);
}

bool IsEqualDoubles ( double d1, double d2, double epsilon = 0.00001 ) {
	if ( epsilon < 0.0 )
		epsilon = -epsilon;

	if ( epsilon > 0.1 )
		epsilon = 0.00001;

	double diff = d1 - d2;

	if ( diff > epsilon || diff < -epsilon )
		return ( false );

	return ( true );
}

void AccumulationDistribution::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (!counted) {
		counted++;
		buffer.Set(0, 0);
	}
	
	//bars--;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double close = Open(i);
		buffer.Set(i, (close - Low(i-1)) - (High(i-1) - close));
		
		if (buffer.Get(i) != 0.0) {
			double diff = High(i-1) - Low(i-1);
			if (diff < 0.000000001)
				buffer.Set(i, 0.0);
			else {
				buffer.Set(i, buffer.Get(i) / diff);
				buffer.Set(i, buffer.Get(i) * (double)Volume(i-1));
			}
		}
		
		buffer.Set(i, buffer.Get(i) + buffer.Get(i-1));
	}
	
}









MoneyFlowIndex::MoneyFlowIndex() {
	period = 14;
}

void MoneyFlowIndex::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20 - 50); // normalized
	SetCoreLevel(1, 80 - 50); // normalized
	SetBufferColor(0, Blue);
	SetBufferBegin ( 0, period );
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"MFI");
}

void MoneyFlowIndex::Start() {
	Buffer& buffer = GetBuffer(0);
	double pos_mf, neg_mf, cur_tp, prev_tp;
	
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();
	
	if (counted < period + 2)
		counted = period + 2;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		pos_mf = 0.0;
		neg_mf = 0.0;
		cur_tp = (
			High(i-1) +
			Low(i-1) +
			Open(i) ) / 3;

		for (int j = 0; j < period; j++) {
			prev_tp = (
				High(i-j-2) +
				Low(i-j-2) +
				Open(i-j-1) ) / 3;

			if ( cur_tp > prev_tp )
				pos_mf += Volume(i-j-1) * cur_tp;
			else {
				if ( cur_tp < prev_tp )
					neg_mf += Volume(i-j-1) * cur_tp;
			}

			cur_tp = prev_tp;
		}
		
		if ( neg_mf != 0.0 )
			buffer.Set(i, 100 - 100 / ( 1 + pos_mf / neg_mf ) - 50); // normalized
		else
			buffer.Set(i, 100 - 50); // normalized
	}
}








ValueAndVolumeTrend::ValueAndVolumeTrend() {
	applied_value = 5;
}

void ValueAndVolumeTrend::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, DodgerBlue);
	SetBufferStyle(0, DRAW_LINE);
}

void ValueAndVolumeTrend::Start() {
	Buffer& buffer = GetBuffer(0);
	double cur_value, prev_value;
	
	int bars = GetBars();
	int counted = GetCounted();

	if ( counted > 0 )
		counted--;

	if (counted < 2)
		counted = 2;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double volume = Volume(i-1);
		cur_value = GetAppliedValue ( applied_value, i - 1);
		prev_value = GetAppliedValue ( applied_value, i - 2 );
		buffer.Set(i,
			buffer.Get(i-1) + volume * ( cur_value -
			prev_value ) / prev_value);
	}
}






OnBalanceVolume::OnBalanceVolume() {
	applied_value = 5;
}

void OnBalanceVolume::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "OBV");
}

void OnBalanceVolume::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();

	if ( counted > 0 )
		counted--;

	if (counted < 2)
		counted = 2;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double cur_value = GetAppliedValue ( applied_value, i - 1);
		double prev_value = GetAppliedValue ( applied_value, i - 2);

		if ( cur_value == prev_value )
			buffer.Set(i, buffer.Get(i));
		else {
			if ( cur_value < prev_value )
				buffer.Set(i, buffer.Get(i) - Volume(i - 1));
			else
				buffer.Set(i, buffer.Get(i) + Volume(i - 1));
		}
	}
}











Volumes::Volumes() {
	
}

void Volumes::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Green);
	SetBufferStyle(0, DRAW_HISTOGRAM);
	SetBufferLabel(0,"Volume");
}

void Volumes::Start() {
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	if (!counted) counted++;
	else counted--;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		buffer.Set(i, Volume(i));
	}
}














#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#undef DATA_LIMIT
#define DATA_LIMIT  3


AcceleratorOscillator::AcceleratorOscillator() {
	
}

void AcceleratorOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Black);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	SetBufferStyle ( 0, DRAW_NONE );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	SetBufferBegin ( 0, DATA_LIMIT );
	SetBufferBegin ( 1, DATA_LIMIT );
	SetBufferBegin ( 2, DATA_LIMIT );
	
	AddSubCore<MovingAverage>().Set("period", PERIOD_FAST).Set("offset", 0).Set("method", MODE_SMA);
	AddSubCore<MovingAverage>().Set("period", PERIOD_SLOW).Set("offset", 0).Set("method", MODE_SMA);
}

void AcceleratorOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(ACC_OVERZERO, true);
		else					vec.Set(ACC_BELOWZERO, true);
		if (value0 > value1)	vec.Set(ACC_INC, true);
		else					vec.Set(ACC_DEC, true);
	}
}

void AcceleratorOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& up_buffer = GetBuffer(1);
	Buffer& down_buffer = GetBuffer(2);
	Buffer& macd_buffer = GetBuffer(3);
	Buffer& signal_buffer = GetBuffer(4);
	double prev = 0.0, current;
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= DATA_LIMIT )
		throw DataExc();
	
	Buffer& ind1 = At(0).GetBuffer(0);
	Buffer& ind2 = At(1).GetBuffer(0);
	
	if ( counted > 0 ) {
		SetSafetyLimit(counted);
		counted--;
		prev = macd_buffer.Get(counted) - signal_buffer.Get(counted);
	}

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		macd_buffer.Set(i,
			ind1.Get(i) -
			ind2.Get(i));
	}

	SimpleMAOnBuffer ( bars, counted, 0, 5, macd_buffer, signal_buffer );
	
	bool up = true;

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
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

		prev = current;
	}
}





GatorOscillator::GatorOscillator() {
	jaws_period = 13;
	jaws_shift = 8;
	teeth_period = 8;
	teeth_shift = 5;
	lips_period = 5;
	lips_shift = 3;
	ma_method = 2;
	applied_value = PRICE_MEDIAN;
}

void GatorOscillator::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Black);
	SetBufferColor(1, Red);
	SetBufferColor(2, Green);
	SetBufferColor(3, Black);
	SetBufferColor(4, Red);
	SetBufferColor(5, Green);
	
	SetBufferBegin ( 1, jaws_period + jaws_shift - teeth_shift );
	SetBufferBegin ( 2, jaws_period + jaws_shift - teeth_shift );
	SetBufferBegin ( 4, teeth_period + teeth_shift - lips_shift );
	SetBufferBegin ( 5, teeth_period + teeth_shift - lips_shift );

	SetBufferShift ( 0, teeth_shift );
	SetBufferShift ( 1, teeth_shift );
	SetBufferShift ( 2, teeth_shift );
	SetBufferShift ( 3, lips_shift );
	SetBufferShift ( 4, lips_shift );
	SetBufferShift ( 5, lips_shift );
	
	AddSubCore<MovingAverage>().Set("period", teeth_period).Set("offset", teeth_shift - lips_shift).Set("method", ma_method);
	AddSubCore<MovingAverage>().Set("period", lips_period).Set("offset", 0).Set("method", ma_method);
	AddSubCore<MovingAverage>().Set("period", jaws_period).Set("offset", jaws_shift - teeth_shift).Set("method", ma_method);
	AddSubCore<MovingAverage>().Set("period", teeth_period).Set("offset", 0).Set("method", ma_method);
	
	SetBufferStyle(0,DRAW_NONE);
	SetBufferStyle(1,DRAW_HISTOGRAM);
	SetBufferStyle(2,DRAW_HISTOGRAM);
	SetBufferStyle(3,DRAW_NONE);
	SetBufferStyle(4,DRAW_HISTOGRAM);
	SetBufferStyle(5,DRAW_HISTOGRAM);
	
	SetBufferLabel(0,"GatorUp");
	SetBufferLabel(1,NULL);
	SetBufferLabel(2,NULL);
	SetBufferLabel(3,"GatorDown");
	SetBufferLabel(4,NULL);
	SetBufferLabel(5,NULL);
}

void GatorOscillator::Start() {
	Buffer& up_buffer = GetBuffer(0);
	Buffer& up_red_buffer = GetBuffer(1);
	Buffer& up_green_buffer = GetBuffer(2);
	Buffer& down_buffer = GetBuffer(3);
	Buffer& down_red_buffer = GetBuffer(4);
	Buffer& down_green_buffer = GetBuffer(5);
	
	double prev, current;
	int    bars    = GetBars();
	int    counted = GetCounted();
	
	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(1).GetBuffer(0);
	ConstBuffer& ind3 = At(2).GetBuffer(0);
	ConstBuffer& ind4 = At(3).GetBuffer(0);
	
	if ( counted <= teeth_period + teeth_shift - lips_shift )
		counted = ( teeth_period + teeth_shift - lips_shift );
	
	if (counted > 0) counted--;
	else counted++;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		current =
			ind1.Get(i) -
			ind2.Get(i);

		if ( current <= 0.0 )
			down_buffer.Set(i, current);
		else
			down_buffer.Set(i, -current);
	}


	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		prev = down_buffer.Get(i-1);
		current = down_buffer.Get(i);

		if ( current < prev ) {
			down_red_buffer.Set(i, 0.0);
			down_green_buffer.Set(i, current);
		}

		if ( current > prev ) {
			down_red_buffer.Set(i, current);
			down_green_buffer.Set(i, 0.0);
		}

		if ( current == prev ) {
			if ( down_red_buffer.Get(i-1) < 0.0 )
				down_red_buffer.Set(i, current);
			else
				down_green_buffer.Set(i, current);
		}
	}
	
	counted = GetCounted();
	if ( counted <= jaws_period + jaws_shift - teeth_shift )
		counted = ( jaws_period + jaws_shift - teeth_shift );
	
	if (counted > 1) counted--;
	else counted++;
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		current =
			ind3.Get(i) -
			ind4.Get(i);

		if ( current >= 0.0 )
			up_buffer.Set(i, current);
		else
			up_buffer.Set(i, -current);
	}
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		prev = up_buffer.Get(i-1);
		current = up_buffer.Get(i);

		if ( current > prev ) {
			up_red_buffer.Set(i, 0.0);
			up_green_buffer.Set(i, current);
		}

		if ( current < prev ) {
			up_red_buffer.Set(i, current);
			up_green_buffer.Set(i, 0.0);
		}

		if ( current == prev ) {
			if ( up_green_buffer.Get(i-1) > 0.0 )
				up_green_buffer.Set(i, current);
			else
				up_red_buffer.Set(i, current);
		}
	}
}







#undef DATA_LIMIT
#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#define DATA_LIMIT  34

AwesomeOscillator::AwesomeOscillator() {
	
}

void AwesomeOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Black);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	SetBufferStyle ( 0, DRAW_NONE );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	
	SetBufferBegin ( 0, DATA_LIMIT );
	SetBufferBegin ( 1, DATA_LIMIT );
	SetBufferBegin ( 2, DATA_LIMIT );
	
	AddSubCore<MovingAverage>().Set("period", PERIOD_FAST).Set("offset", 0).Set("method", MODE_SMA);
	AddSubCore<MovingAverage>().Set("period", PERIOD_SLOW).Set("offset", 0).Set("method", MODE_SMA);
}

void AwesomeOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = GetBuffer(0).Get(cursor);
		double value1 = GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(AWE_OVERZERO, true);
		else					vec.Set(AWE_BELOWZERO, true);
		if (value0 > value1)	vec.Set(AWE_INC, true);
		else					vec.Set(AWE_DEC, true);
	}
}

void AwesomeOscillator::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& up_buffer = GetBuffer(1);
	Buffer& down_buffer = GetBuffer(2);
	
	int bars = GetBars();
	int counted = GetCounted();
	double prev = 0.0, current;
	
	if ( bars <= DATA_LIMIT )
		throw DataExc();

	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(1).GetBuffer(0);
	
	if (counted > 0) {
		counted--;
		SetSafetyLimit(counted);
		if (counted)
			prev = buffer.Get(counted - 1);
	}

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		buffer.Set(i,
			ind1.Get(i) - ind2.Get(i));
	}

	bool up = true;

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
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

		prev = current;
	}
}















Fractals::Fractals() {
	left_bars  = 3;
	right_bars = 0;
}

void Fractals::Init() {
	SetCoreChartWindow();
	SetBufferColor(0, Blue);
	SetBufferColor(1, Blue);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Blue);
	SetBufferColor(4, Blue);
	SetBufferColor(5, Blue);
	SetBufferStyle(0, DRAW_LINE);
	SetBufferArrow(0, 158);
	SetBufferStyle(1, DRAW_LINE);
	SetBufferArrow(1, 158);
	SetBufferStyle(2, DRAW_ARROW);
	SetBufferArrow(2, 119);
	SetBufferStyle(3, DRAW_ARROW);
	SetBufferArrow(3, 119);
	SetBufferStyle(4, DRAW_ARROW);
	SetBufferArrow(4, 119);
	SetBufferStyle(5, DRAW_ARROW);
	SetBufferArrow(5, 119);
}

void Fractals::Start() {
	Buffer& line_up_buf1 = GetBuffer(0);
	Buffer& line_up_buf2 = GetBuffer(1);
	Buffer& arrow_up_buf = GetBuffer(2);
	Buffer& arrow_down_buf = GetBuffer(3);
	Buffer& arrow_breakup_buf = GetBuffer(4);
	Buffer& arrow_breakdown_buf = GetBuffer(5);
	
	int bars = GetBars();
	int counted = GetCounted();
	
	if(counted==0)
		counted = 1 + Upp::max(left_bars, right_bars);
	else
		counted--;
	
	SetSafetyLimit(counted);
	double prev_close = Open(counted);
	
	//bars--;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		double close = Open(i);
		
		line_up_buf1.Set(i, IsFractalUp(i-1, left_bars, right_bars, bars));
		if (line_up_buf1.Get(i) == 0)
			line_up_buf1.Set(i, line_up_buf1.Get(i-1));
		else
			arrow_up_buf.Set(i, line_up_buf1.Get(i));
		
		line_up_buf2.Set(i, IsFractalDown(i-1, left_bars, right_bars, bars));
		
		if (line_up_buf2.Get(i) == 0)
			line_up_buf2.Set(i, line_up_buf2.Get(i-1));
		else
			arrow_down_buf.Set(i, line_up_buf2.Get(i));
		
		if (close < line_up_buf2.Get(i) && prev_close >= line_up_buf2.Get(i-1))
		
		arrow_breakdown_buf.Set(i, close);
		
		prev_close = close;
	}
}


double Fractals::IsFractalUp(int index, int left, int right, int maxind) {
	double max = High(index);
	for(int i = index - left; i <= (index + right); i++) {
		if (i < 0 || i > maxind)
			return(0);
		double high =  High(i);
		if(!(high > 0.0))
			return(0);
		if (max < high && i != index) {
			if(max < high)  return(0);
			if(abs(i - index) > 1) return(0);
		}
	}
	return(max);
}




double Fractals::IsFractalDown(int index, int left, int right, int maxind)
{
	double min = Low(index);
	for(int i = index - left; i <= (index + right); i++) {
		if (i < 0 || i > maxind)
			return(0);
		double low =  Low(i);
		if (!(low > 0.0))
			return(0);
		if (min > low && i != index) {
			if(min > low)
				return(0);
			if(abs(i - index) > 1)
				return(0);
		}
	}
	return(min);
}
















FractalOsc::FractalOsc() {
	left_bars  = 3;
	right_bars = 0;
	smoothing_period = 12;
}

void FractalOsc::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	SetBufferColor(1, Blue);
	SetBufferStyle(0, DRAW_LINE);
	SetBufferStyle(1, DRAW_LINE);
	AddSubCore<Fractals>().Set("left_bars", left_bars).Set("right_bars", right_bars);
}


void FractalOsc::Start() {
	Buffer& buf = GetBuffer(0);
	Buffer& av  = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	else
		counted = 1 + Upp::max(left_bars,right_bars);
	
	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(0).GetBuffer(1);
	
	//bars--;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double close = Open(i);
		double buf1 = ind1.Get(i);
		double buf2 = ind2.Get(i);
		double v = (close - buf2) / (buf1 - buf2);
		if ( v > 1) v = 1;
		else if ( v < 0) v = 0;
		buf.Set(i, v - 0.5); // normalized
		av.Set(i, SimpleMA( i, smoothing_period, buf ));
	}
}






MarketFacilitationIndex::MarketFacilitationIndex() {
	
}

void MarketFacilitationIndex::Init() {
	SetCoreSeparateWindow();
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20);
	SetCoreLevel(1, 80);
	SetBufferColor(0, Lime);
	SetBufferColor(1, SaddleBrown);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Pink);
	SetBufferStyle ( 0, DRAW_HISTOGRAM );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	SetBufferStyle ( 3, DRAW_HISTOGRAM );
	SetBufferLabel ( 0, "MFI Up, Volume Up" );
	SetBufferLabel ( 1, "MFI Down, Volume Down" );
	SetBufferLabel ( 2, "MFI Up, Volume Down" );
	SetBufferLabel ( 3, "MFI Down, Volume Up" );
}


void MarketFacilitationIndex::Start() {
	Buffer& up_up_buffer = GetBuffer(0);
	Buffer& down_down_buffer = GetBuffer(1);
	Buffer& up_down_buffer = GetBuffer(2);
	Buffer& down_up_buffer = GetBuffer(3);
	Buffer& buffer = GetBuffer(4);
	
	bool mfi_up = true, vol_up = true;

	int bars    = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	else
		counted++;
	
	double point = 0.00001;
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double volume = Volume(i-1);
		if ( IsEqualDoubles ( volume, 0.0 ) ) {
			if ( i == 0 )
				buffer.Set(i, 0.0);
			else
				buffer.Set(i, buffer.Get(i-1));
		}
		else
			buffer.Set(i,
				( High(i-1) - Low(i-1) ) /
				( volume * point ));
	}
	
	if ( counted > 1 ) {

		int i = counted -1;

		if ( up_up_buffer.Get(i) != 0.0 ) {
			mfi_up = true;
			vol_up = true;
		}

		if ( down_down_buffer.Get(i) != 0.0 ) {
			mfi_up = false;
			vol_up = false;
		}

		if ( up_down_buffer.Get(i) != 0.0 ) {
			mfi_up = true;
			vol_up = false;
		}

		if ( down_up_buffer.Get(i) != 0.0 ) {
			mfi_up = false;
			vol_up = true;
		}
	}
	
	double volume_prev;
	SetSafetyLimit(counted);
	if (counted > 0)
		volume_prev = Volume(counted-1);
	else {
		volume_prev = Volume(counted-1);
		counted++;
	}
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double volume = Volume(i-1);
		if ( i < bars - 1 ) {
			if ( buffer.Get(i) > buffer.Get(i-1) )
				mfi_up = true;

			if ( buffer.Get(i) < buffer.Get(i-1) )
				mfi_up = false;

			if ( volume > volume_prev )
				vol_up = true;

			if ( volume < volume_prev )
				vol_up = false;
		}

		if ( mfi_up && vol_up ) {
			up_up_buffer.Set(i, buffer.Get(i));
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( !mfi_up && !vol_up ) {
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, buffer.Get(i));
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( mfi_up && !vol_up ) {
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, buffer.Get(i));
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( !mfi_up && vol_up ) {
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, buffer.Get(i));
			volume_prev = volume;
			continue;
		}
		
		volume_prev = volume;
	}
}







ZigZag::ZigZag()
{
	input_depth		= 12; // Depth
	input_deviation	= 5;  // Deviation
	input_backstep	= 3;  // Backstep
	extremum_level	= 3;  // recounting's depth of extremums
}

void ZigZag::Init() {
	SetCoreChartWindow();
	SetBufferColor(0, Red());
	SetBufferColor(1, Red());
	
	if (input_backstep >= input_depth) {
		input_backstep = input_depth-1;
	}
	
	SetBufferStyle(0, DRAW_NONE);
	SetBufferStyle(1, DRAW_SECTION);
}

void ZigZag::Start() {
	Buffer& osc = GetBuffer(0);
	Buffer& keypoint_buffer = GetBuffer(1);
	Buffer& high_buffer = GetBuffer(2);
	Buffer& low_buffer = GetBuffer(3);
	int    counter_z, whatlookfor = 0;
	int    back, pos, lasthighpos = 0, lastlowpos = 0;
	double extremum;
	double curlow = 0.0, curhigh = 0.0, lasthigh = 0.0, lastlow = 0.0;
	
	int bars = GetBars();
	int counted = GetCounted();
	
	VectorBool& label = outputs[0].label;
	label.SetCount(bars);
	
	if ( bars < input_depth || input_backstep >= input_depth )
		throw DataExc();

	if ( counted == 0 )
		counted = input_backstep;
	else {
		int i = bars-1;
		counter_z = 0;
		SetSafetyLimit(i);

		while ( counter_z < extremum_level && i >= bars-100 && i >= 0 ) {
			if ( keypoint_buffer.Get(i) != 0.0 )
				counter_z++;
			i--;
		}
		
		if (i < 0)
			i = 0;
		
		if ( counter_z == 0 )
			counted = input_backstep;
		else {
			counted = i+1;

			if ( low_buffer.Get(i) != 0.0 )
			{
				curlow = low_buffer.Get(i);
				whatlookfor = 1;
			} else {
				curhigh = high_buffer.Get(i);
				whatlookfor = -1;
			}

			for (int i = counted; i < bars; i++) {
				keypoint_buffer.Set(i, 0.0);
				low_buffer.Set(i, 0.0);
				high_buffer.Set(i, 0.0);
			}
		}
	}

	double point = 0.00001;
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
	ExtremumCache ec(input_depth);
	int begin = Upp::max(0, counted - input_depth);
	ec.pos = begin - 1;
	for(int i = begin; i < counted; i++) {
		SetSafetyLimit(i+1);
		double low = Low(i);
		double high = High(i);
		ec.Add(low, high);
	}
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1); // This indicator peeks anyway
		
		double low = Low(i);
		double high = High(i);
		ec.Add(low, high);
		
		int lowest = ec.GetLowest();
		extremum = Low(lowest);
		
		if ( extremum == lastlow )
			extremum = 0.0;
		else
		{
			lastlow = extremum;

			if ( low - extremum > input_deviation*point )
				extremum = 0.0;
			else
			{
				for ( back = 1; back <= input_backstep; back++ )
				{
					pos = i - back;
					if (pos < 0) pos = 0;
					if ( low_buffer.Get(pos) != 0 && low_buffer.Get(pos) > extremum )
						low_buffer.Set(pos, 0.0);
				}
			}
		}

		if ( low == extremum )
			low_buffer.Set(i, extremum);
		else
			low_buffer.Set(i, 0.0);

		int highest = ec.GetHighest();
		extremum = High(highest);
	
		if ( extremum == lasthigh )
			extremum = 0.0;
		else
		{
			lasthigh = extremum;

			if ( extremum - high > input_deviation*point )
				extremum = 0.0;
			else
			{
				for ( back = 1; back <= input_backstep; back++ )
				{
					pos = i - back;
					if (pos < 0) pos = 0;
					if ( high_buffer.Get(pos) != 0 && high_buffer.Get(pos) < extremum )
						high_buffer.Set(pos, 0.0);
				}
			}
		}

		if ( high == extremum )
			high_buffer.Set(i, extremum);
		else
			high_buffer.Set(i, 0.0);
	}

	if ( whatlookfor == 0 )
	{
		lastlow = 0.0;
		lasthigh = 0.0;
	} else {
		lastlow = curlow;
		lasthigh = curhigh;
	}

	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1); // This indicator peeks anyway
		
		double low = Low(i);
		double high = High(i);
		
		osc.Set(i, whatlookfor);
		
		switch ( whatlookfor ) {
			
		case 0: // look for peak or lawn
			
			if ( lastlow == 0.0 && lasthigh == 0.0 ) {
				if ( high_buffer.Get(i) != 0.0 ) {
					lasthighpos = i;
					lasthigh = high_buffer.Get(lasthighpos);
					whatlookfor = -1;
					keypoint_buffer.Set(lasthighpos, lasthigh);
				}
				if ( low_buffer.Get(i) != 0.0 ) {
					lastlowpos = i;
					lastlow = low_buffer.Get(lastlowpos);
					whatlookfor = 1;
					keypoint_buffer.Set(lastlowpos, lastlow);
				}
			}
			break;

		case 1: // look for peak
			
			if ( low_buffer.Get(i) != 0.0 && low_buffer.Get(i) < lastlow && high_buffer.Get(i) == 0.0 )
			{
				keypoint_buffer.Set(lastlowpos, 0.0);
				lastlowpos = i;
				lastlow = low_buffer.Get(lastlowpos);
				keypoint_buffer.Set(lastlowpos, lastlow);
			}

			if ( high_buffer.Get(i) != 0.0 && low_buffer.Get(i) == 0.0 )
			{
				lasthighpos = i;
				lasthigh = high_buffer.Get(lasthighpos);
				keypoint_buffer.Set(lasthighpos, lasthigh);
				whatlookfor = -1;
			}

			break;

		case - 1: // look for lawn
			
			if ( high_buffer.Get(i) != 0.0 && high_buffer.Get(i) > lasthigh && low_buffer.Get(i) == 0.0 )
			{
				keypoint_buffer.Set(lasthighpos, 0.0);
				lasthighpos = i;
				lasthigh = high_buffer.Get(lasthighpos);
				keypoint_buffer.Set(lasthighpos, lasthigh);
			}

			if ( low_buffer.Get(i) != 0.0 && high_buffer.Get(i) == 0.0 )
			{
				lastlowpos = i;
				lastlow = low_buffer.Get(lastlowpos);
				keypoint_buffer.Set(lastlowpos, lastlow);
				whatlookfor = 1;
			}

			break;
		}
	}
	
	
	bool current = false;
	for (int i = counted-1; i >= 0; i--) {
		if (keypoint_buffer.Get(i) != 0.0) {
			current = high_buffer.Get(i) != 0.0; // going down after high
			break;
		}
	}
	if (!counted) counted++;
	for (int i = counted; i < bars; i++) {
		label.Set(i-1, current);
		if (keypoint_buffer.Get(i) != 0.0) {
			current = high_buffer.Get(i) != 0.0; // going down after high
		}
	}
}












ZigZagOsc::ZigZagOsc() {
	depth = 12;
	deviation = 5;
	backstep = 3;
}

void ZigZagOsc::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, GrayColor());
	SetBufferStyle(0, DRAW_HISTOGRAM);
	AddSubCore<ZigZag>().Set("depth", depth).Set("deviation", deviation).Set("backstep", backstep);
}

void ZigZagOsc::Start() {
	Buffer& osc = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(0).GetBuffer(1);
	
	double prev, next;
	bool has_prev = false, has_next = false;;
	int prev_pos, next_pos;
	
	if (counted) {
		counted = Upp::min(counted, GetBars()-1);
		for (; counted > 0; counted--) {
			double v = ind2.Get(counted);
			if (!v) continue;
			break;
		}
	}
	
	double diff, step;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1); // This indicator peeks anyway
		
		if (has_prev) {
			if (has_next && next_pos == i) {
				prev_pos = i;
				prev = next;
				has_next = false;
			}
		} else {
			double v = ind2.Get(i);
			if (v == 0)
				continue;
			has_prev = true;
			prev = v;
			prev_pos = i;
		}
		
		if (!has_next) {
			for (int j = i+1; j < bars; j++) {
				double v = ind2.Get(j);
				if (!v) continue;
				next = v;
				has_next = true;
				next_pos = j;
				break;
			}
			if (!has_next)
				break;
			diff = next - prev;
			step = diff / (prev_pos - next_pos);
		}
		
		double v = (i - prev_pos) * step + prev;
		double value_diff = GetAppliedValue(PRICE_TYPICAL, i) - v;
		
		osc.Set(i, value_diff);
	}
}









LinearTimeFrames::LinearTimeFrames() {
	
}

void LinearTimeFrames::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Yellow);
	SetBufferLineWidth(0, 3);
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(2, 1);
	SetCoreMinimum(0);
	SetCoreMaximum(1);
}

void LinearTimeFrames::Start() {
	ConstBuffer& src_time = GetInputBuffer(0,4);
	Buffer& day = GetBuffer(0);
	Buffer& month = GetBuffer(1);
	Buffer& year = GetBuffer(2);
	Buffer& week = GetBuffer(3);
	int bars = GetBars();
	int counted =  GetCounted();
	System& base = GetSystem();
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		Time t = Time(1970,1,1) + src_time.Get(i);
		double h = t.hour;
		double t1 = ((double)t.minute + h * 60.0 ) / (24.0 * 60.0);
		day.Set(i, t1);
		double days = GetDaysOfMonth(t.month, t.year);
		double wday = (DayOfWeek(t) + t1) / 7.0;
		week.Set(i, wday);
		double d = t.day + t1;
		double t2 = d / days;
		month.Set(i, t2);
		double m = t.month + t2;
		double t3 = m / 12.0;
		year.Set(i, t3);
	}
}









LinearWeekTime::LinearWeekTime() {
	
}

void LinearWeekTime::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Green);
	SetBufferLineWidth(0, 3);
	SetCoreMinimum(-1);
	SetCoreMaximum(+1);
}

void LinearWeekTime::Start() {
	ConstBuffer& src_time = GetInputBuffer(0,4);
	Buffer& week = GetBuffer(0);
	int bars = GetBars();
	int counted =  GetCounted();
	System& base = GetSystem();
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		Time t = Time(1970,1,1) + src_time.Get(i);
		double h = t.hour;
		double t1 = ((double)t.minute + h * 60.0 ) / (24.0 * 60.0);
		double days = GetDaysOfMonth(t.month, t.year);
		double wday = (DayOfWeek(t) + t1) / 7.0;
		week.Set(i, wday);
	}
}







	




SupportResistance::SupportResistance() {
	period = 300;
	max_crosses = 100;
	max_radius = 100;
}

void SupportResistance::Init() {
	SetCoreChartWindow();
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
}

void SupportResistance::Start() {
	Buffer& support = GetBuffer(0);
	Buffer& resistance = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	double point = 0.00001;
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
	ASSERT(point > 0);
	
	if (counted) counted--;
	else counted++;
	//bars--;
	
	Vector<int> crosses;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		int highest = HighestHigh(period, i-1);
		int lowest  = LowestLow(period, i-1);
		double highesthigh = High(highest);
		double lowestlow   = Low(lowest);
		
		int count = (int)((highesthigh - lowestlow) / point + 1);
		double inc = point;
		int inc_points = 1;
		while (count > 1000) {
			count /= 10;
			inc *= 10;
			inc_points *= 10;
		}
		
		if (crosses.GetCount() < count) crosses.SetCount(count);
		
		for(int j = 0; j < count; j++)
			crosses[j] = 0;
		
		for(int j = i - period; j < i ; j++) {
			if (j < 0) continue;
			double high = High(j);
			double low  = Low(j);
			
			int low_pos  = (int)((low  - lowestlow) / inc);
			int high_pos = (int)((high - lowestlow) / inc);
			if (high_pos >= count) high_pos = count - 1;
			
			for(int k = low_pos; k <= high_pos; k++) {
				crosses[k]++;
			}
		}
		
		double close = Open(i);
		int value_pos = (int)((close  - lowestlow) / inc);
		if (value_pos >= count) value_pos = count - 1;
		if (value_pos < 0) value_pos = 0;
		
		// Find support
		int max = 0;
		int max_pos = value_pos;
		int max_cross_count = crosses[value_pos];
		for(int j = value_pos - 1; j >= 0; j--) {
			int cross_count = crosses[j];
			if (cross_count > max) {
				max = cross_count;
				max_pos = j;
				max_cross_count = cross_count;
			}
			int pos_diff = max_pos - j;
			if (pos_diff > max_radius)
				break;
			if (cross_count < max_cross_count - max_crosses)
				break;
		}
		support.Set(i, lowestlow + max_pos * inc);
		
		// Find resistance
		max = 0;
		max_pos = value_pos;
		max_cross_count = crosses[value_pos];
		for(int j = value_pos + 1; j < count; j++) {
			int cross_count = crosses[j];
			if (cross_count > max) {
				max = cross_count;
				max_pos = j;
				max_cross_count = cross_count;
			}
			int pos_diff = j - max_pos;
			if (pos_diff > max_radius)
				break;
			if (cross_count < max_cross_count - max_crosses)
				break;
		}
		resistance.Set(i, lowestlow + max_pos * inc);
	}
}







SupportResistanceOscillator::SupportResistanceOscillator()
{
	period = 300;
	max_crosses = 100;
	max_radius = 100;
	smoothing_period = 30;
}

void SupportResistanceOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Green);
	SetBufferColor(1, Red);
	SetCoreMinimum(-1);
	SetCoreMaximum(1);
	AddSubCore<SupportResistance>().Set("period", period).Set("max_crosses", max_crosses).Set("max_radius", max_radius);
}

void SupportResistanceOscillator::Start() {
	Buffer& osc_av = GetBuffer(0);
	Buffer& osc = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(0).GetBuffer(1);
	
	int prev_pos = counted ? counted-1 : 0;
	SetSafetyLimit(counted-1);
	double prev_value = counted ? osc_av.Get(counted-1) : 0;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		double applied_value = Open(i);
		double s = ind1.Get(i);
		double r = ind2.Get(i);
		double range = r - s;
		double value = (applied_value - s) / range * 2 - 1;
		if (value >  1) value = 1;
		if (value < -1) value = -1;
		osc.Set(i, value);
		value = ExponentialMA( i, smoothing_period, prev_value, osc );
		osc_av.Set(i, value);
		prev_pos = i;
		prev_value = value;
	}
}





ChannelOscillator::ChannelOscillator()
{
	period = 300;
}

void ChannelOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	SetCoreMinimum(-1);
	SetCoreMaximum(1);
}

void ChannelOscillator::Start() {
	Buffer& osc = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	ec.SetSize(period);
	
	SetSafetyLimit(counted-1);
	for (int i = ec.pos+1; i < bars; i++) {
		SetSafetyLimit(i);
		
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
	}
}

void ChannelOscillator::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	if      (value0 > +0.5)		vec.Set(CHOSC_HIGHEST, true);
	else if (value0 > +0.0)		vec.Set(CHOSC_HIGH, true);
	else if (value0 > -0.5)		vec.Set(CHOSC_LOW, true);
	else						vec.Set(CHOSC_LOWEST, true);
}









ScissorChannelOscillator::ScissorChannelOscillator()
{
	period = 30;
}

void ScissorChannelOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	SetCoreMinimum(-1);
	SetCoreMaximum(1);
}

void ScissorChannelOscillator::Start() {
	Buffer& osc = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	ec.SetSize(period);
	
	SetSafetyLimit(counted-1);
	for (int i = ec.pos+1; i < bars; i++) {
		SetSafetyLimit(i);
		
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
	}
}

void ScissorChannelOscillator::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	if (value0 > 0.0)			vec.Set(SCIS_HIGH, true);
	else						vec.Set(SCIS_LOW,  true);
}






Psychological::Psychological() {
	period = 25;
}

void Psychological::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, DodgerBlue());
	SetBufferStyle(0, DRAW_LINE);
	SetBufferBegin(0, period);
	SetBufferLabel(0, "Psychological");
}

void Psychological::Start() {
	Buffer& buf = GetBuffer(0);
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
		SetSafetyLimit(i);
		
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
		buf.Set(i, ((double)count / period) *100.0 - 50); // normalized
	}
}





















TrendChange::TrendChange() {
	period = 13;
	method = 0;
	SetCoreSeparateWindow();
}

void TrendChange::Init() {
	int draw_begin;
	if (period < 2)
		period = 13;
	draw_begin = period - 1;
	
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	
	AddSubCore<MovingAverage>().Set("period", period).Set("method", method);
	
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0);
	SetCoreLevelsColor(GrayColor(192));
	SetCoreLevelsStyle(STYLE_DOT);
}

void TrendChange::Start() {
	Buffer& buffer = GetBuffer(0);
	
	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();
	
	int counted = GetCounted();
	if ( counted < 0 )
		throw DataExc();
	
	if ( counted > 0 )
		counted--;
	
	// The newest part can't be evaluated. Only after 'shift' amount of time.
	int shift = period / 2;
	counted = Upp::max(0, counted - shift);
	//bars -= shift;
	
	// Calculate averages
	ConstBuffer& dbl = At(0).GetBuffer(0);
	
	// Prepare values for loop
	SetSafetyLimit(counted);
	double prev1 = dbl.Get(Upp::max(0, counted-1));
	double prev2 = dbl.Get(Upp::max(0, counted-1+shift));
	double prev_value = counted > 0 ? buffer.Get(counted-1) : 0;
	double prev_diff = counted > 1 ? prev_value - buffer.Get(counted-2) : 0;
	
	// Loop unprocessed range of time
	for(int i = counted; i < bars-shift; i++) {
		SetSafetyLimit(i);
		
		// Calculate values
		double d1 = dbl.Get(i);
		double d2 = dbl.Get(i+shift);
		double diff1 = d1 - prev1;
		double diff2 = d2 - prev2;
		double value = diff1 * diff2;
		double diff = value - prev_value;
		
		// Set value
		buffer.Set(i, value);
		
		// Store values for next snapation
		prev1 = d1;
		prev2 = d2;
		prev_value = value;
		prev_diff = diff;
	}
}


















TrendChangeEdge::TrendChangeEdge() {
	period = 13;
	method = 0;
	slowing = 54;
	SetCoreSeparateWindow();
}

void TrendChangeEdge::Init() {
	int draw_begin;
	if (period < 2)
		period = 13;
	draw_begin = period - 1;
	
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	
	AddSubCore<MovingAverage>().Set("period", period).Set("method", method);
	AddSubCore<MovingAverage>().Set("period", slowing).Set("method", method).Set("offset", -slowing/2);
}

void TrendChangeEdge::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& edge = GetBuffer(1);
	Buffer& symlr = GetBuffer(2);
	
	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();
	
	int counted = GetCounted();
	if ( counted < 0 )
		throw DataExc();
	
	if ( counted > 0 )
		counted-=3;
	
	// The newest part can't be evaluated. Only after 'shift' amount of time.
	int shift = period / 2;
	counted = Upp::max(0, counted - shift);
	
	// Refresh source data
	Core& cont = At(0);
	Core& slowcont = At(1);
	cont.Refresh();
	slowcont.Refresh();
	
	// Prepare values for looping
	ConstBuffer& dbl = cont.GetBuffer(0);
	ConstBuffer& slow_dbl = slowcont.GetBuffer(0);
	double prev_slow = slow_dbl.Get(Upp::max(0, counted-1));
	double prev1 = dbl.Get(Upp::max(0, counted-1)) - prev_slow;
	double prev2 = dbl.Get(Upp::max(0, counted-1+shift)) - prev_slow;
	
	// Calculate 'TrendChange' data locally. See info for that.
	for(int i = counted; i < bars-shift; i++) {
		SetSafetyLimit(i);
		
		double slow = slow_dbl.Get(i);
		double d1 = dbl.Get(i) - slow;
		double d2 = dbl.Get(i+shift) - slow;
		double diff1 = d1 - prev1;
		double diff2 = d2 - prev2;
		double r1 = diff1 * diff2;
		symlr.Set(i, r1);
		prev1 = d1;
		prev2 = d2;
	}
	
	// Loop unprocessed area
	for(int i = Upp::max(2, counted); i < bars - 2; i++) {
		SetSafetyLimit(i + 2);
		
		// Finds local maximum value for constant range. TODO: optimize
		double high = 0;
		int high_pos = -1;
		for(int j = 0; j < period; j++) {
			int pos = i - j;
			if (pos < 0) continue;
			double d = fabs(symlr.Get(pos));
			if (d > high) {
				high = d;
				high_pos = pos;
			}
		}
		double mul = high_pos == -1 ? 0 : (high - fabs(symlr.Get(i) - 0.0)) / high;
		
		// Calculates edge filter value
		double edge_value =
			 1.0 * symlr.Get(i-2) +
			 2.0 * symlr.Get(i-1) +
			-2.0 * symlr.Get(i+1) +
			-1.0 * symlr.Get(i+2);
		
		// Store value
		edge.Set(i, edge_value);
		double d = Upp::max(0.0, edge_value * mul);
		ASSERT(IsFin(d));
		buffer.Set(i, d);
	}
}
















PeriodicalChange::PeriodicalChange() {
	
	
	
}

void PeriodicalChange::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	
	tfmin = GetMinutePeriod();
	int w1 = 7 * 24 * 60;
	int count = 0;
	
	split_type = 0;
	if      (tfmin >= w1)		{count = 4;	split_type = 2;}
	else if (tfmin >= 24 * 60)	{count = 7; split_type = 1;}
	else						{count = 7 * 24 * 60 / tfmin; split_type = 0;}
	
	means.SetCount(count, 0.0);
	counts.SetCount(count, 0);
}

void PeriodicalChange::Start() {
	System& sys = GetSystem();
	ConstBuffer& src = GetInputBuffer(0, 0);
	ConstBuffer& src_time = GetInputBuffer(0,4);
	Buffer& dst = GetBuffer(0);
	
	int bars = GetBars() - 1;
	int counted = GetCounted();
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1);
		
		double prev = src.Get(i);
		if (prev == 0.0)
			continue;
		double change = src.Get(i+1) / prev - 1.0;
		if (!IsFin(change))
			continue;
		
		Time t = Time(1970,1,1) + src_time.Get(i);
		
		if (split_type == 0) {
			int wday = DayOfWeek(t);
			int wdaymin = (wday * 24 + t.hour) * 60 + t.minute;
			int avpos = wdaymin / tfmin;
			Add(avpos, change);
		}
		else if (split_type == 1) {
			int wday = DayOfWeek(t);
			Add(wday, change);
		}
		else {
			int pos = (DayOfYear(t) % (7 * 4)) / 7;
			Add(pos, change);
		}
	}
	
	
	if (counted) counted--;
	bars++;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		Time t = Time(1970,1,1) + src_time.Get(i);
		double av_change = 0;
		if (split_type == 0) {
			int wday = DayOfWeek(t);
			int wdaymin = (wday * 24 + t.hour) * 60 + t.minute;
			int avpos = wdaymin / tfmin;
			av_change = means[avpos];
		}
		else if (split_type == 1) {
			int wday = DayOfWeek(t);
			av_change = means[wday];
		}
		else {
			int pos = (DayOfYear(t) % (7 * 4)) / 7;
			av_change = means[pos];
		}
		
		dst.Set(i, av_change);
	}
}

void PeriodicalChange::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	if (value0 > 0.0)		vec.Set(PC_INC, true);
	else					vec.Set(PC_DEC, true);
}














VolatilityAverage::VolatilityAverage() {
	period = 60;
}

void VolatilityAverage::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1.0);
	SetCoreMaximum(+1.0);
	SetBufferColor(0, Color(85, 255, 150));
	SetBufferLineWidth(0, 2);
	SetCoreLevelCount(2);
	SetCoreLevel(0, +0.5);
	SetCoreLevel(1, -0.5);
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
}

void VolatilityAverage::Start() {
	System& sys = GetSystem();
	ConstBuffer& src = GetInputBuffer(0, 0);
	
	int bars = GetBars();
	int counted = GetCounted();
	if (counted >= bars) return;
	if (!counted) counted++;
	
	Buffer& dst = GetBuffer(0);
	
	double sum = 0.0;
	SetSafetyLimit(counted);
	for(int i = Upp::max(1, counted - period); i < counted; i++) {
		double change = fabs(src.Get(i) / src.Get(i-1) - 1.0);
		sum += change;
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1);
		
		// Add current
		double change = fabs(src.Get(i) / src.Get(i-1) - 1.0);
		sum += change;
		
		// Subtract
		int j = i - period;
		if (j > 0) {
			double prev_change = fabs(src.Get(j) / src.Get(j-1) - 1.0);
			sum -= prev_change;
		}
		
		double average = sum / period;
		
		int av = average * 100000;
		stats.GetAdd(av,0)++;
		
		dst.Set(i, average);
	}
	
	SortByKey(stats, StdLess<int>());
	
	stats_limit.SetCount(stats.GetCount());
	int64 total = 0;
	for(int i = 0; i < stats.GetCount(); i++) {
		stats_limit[i] = total;
		total += stats[i];
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1);
		
		double average = dst.Get(i);
		int av = average * 100000;
		double j = stats_limit[stats.Find(av)];
		double sens = j / total * 2.0 - 1.0;
		dst.Set(i, sens);
	}
}

void VolatilityAverage::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	if      (value0 > +0.5)		vec.Set(VOL_HIGHEST, true);
	else if (value0 > +0.0)		vec.Set(VOL_HIGH, true);
	else if (value0 > -0.5)		vec.Set(VOL_LOW, true);
	else						vec.Set(VOL_LOWEST, true);
}




















MinimalLabel::MinimalLabel() {
	
}

void MinimalLabel::Init() {
	SetCoreChartWindow();
	SetBufferColor(0, Color(85, 255, 150));
	SetBufferStyle(0, DRAW_ARROW);
	SetBufferArrow(0, 159);
}

void MinimalLabel::Start() {
	int bars = GetBars();
	int symbol = GetSymbol();
	int tf = GetTf();
	
	DataBridge* db			= dynamic_cast<DataBridge*>(GetInputCore(0, symbol, tf));
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	double spread_point		= db->GetPoint();
	double cost				= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = GetOutput(0).label;
	labelvec.SetCount(bars);
	
	Buffer& buf = GetBuffer(0);
	
	for(int i = prev_counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		bool clean_break = false;
		for(; j < bars; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					clean_break = true;
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		
		for(int k = i; k < j; k++) {
			SetSafetyLimit(k);
			open = open_buf.GetUnsafe(k);
			labelvec.Set(k, label);
			if (label)		buf.Set(k, open - 10 * spread_point);
			else			buf.Set(k, open + 10 * spread_point);
		}
		
		if (clean_break)
			prev_counted = i;
		i = j - 1;
	}
}















VolatilitySlots::VolatilitySlots() {
	
}

void VolatilitySlots::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(113, 42, 150));
	SetBufferStyle(0, DRAW_LINE);
	
	SetCoreLevelCount(3);
	SetCoreLevel(0, 0.0002);
	SetCoreLevel(1,  0.001);
	SetCoreLevel(2,  0.010);
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	
	int tf_mins = GetMinutePeriod();
	if (tf_mins < 10080)
		slot_count = (5 * 24 * 60) / tf_mins;
	else
		slot_count = 1;
	
	stats.SetCount(slot_count);
}

void VolatilitySlots::Start() {
	Buffer& buffer = GetBuffer(0);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	
	if (counted == 0)
		counted++;
	
	int tf_mins = GetMinutePeriod();
	
	for(int i = counted; i < bars; i++) {
		double cur  = open_buf.Get(i);
		double prev = open_buf.Get(i-1);
		double change = fabs(cur / prev - 1.0);
		int slot_id = (i-1) % slot_count;
		OnlineAverage1& av = stats[slot_id];
		av.Add(change);
		total.Add(change);
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		int slot_id = i % slot_count;
		const OnlineAverage1& av = stats[slot_id];
		
		buffer.Set(i, av.mean);
	}
}

void VolatilitySlots::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	if      (value0 > total.mean * 2.000)		vec.Set(VOLSL_VERYHIGH, true);
	else if (value0 > total.mean * 1.333)		vec.Set(VOLSL_HIGH, true);
	else if (value0 > total.mean * 0.666)		vec.Set(VOLSL_MED, true);
	else										vec.Set(VOLSL_LOW, true);
	if (cursor > 0) {
		double value1 = GetBuffer(0).Get(cursor - 1);
		if (value0 > value1)		vec.Set(VOLSL_INC, true);
		else						vec.Set(VOLSL_DEC, true);
	}
}











VolumeSlots::VolumeSlots() {
	
}

void VolumeSlots::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Color(113, 42, 150));
	SetBufferStyle(0, DRAW_LINE);
	
	SetCoreLevelCount(3);
	SetCoreLevel(0, 0.0002);
	SetCoreLevel(1,  0.001);
	SetCoreLevel(2,  0.010);
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	
	int tf_mins = GetMinutePeriod();
	if (tf_mins < 10080)
		slot_count = (5 * 24 * 60) / tf_mins;
	else
		slot_count = 1;
	
	stats.SetCount(slot_count);
}

void VolumeSlots::Start() {
	Buffer& buffer = GetBuffer(0);
	ConstBuffer& vol_buf = GetInputBuffer(0, 3);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	
	if (counted == 0)
		counted++;
	
	int tf_mins = GetMinutePeriod();
	
	for(int i = counted; i < bars; i++) {
		double vol  = vol_buf.Get(i);
		if (vol == 0.0) continue;
		int slot_id = (i-1) % slot_count;
		OnlineAverage1& av = stats[slot_id];
		av.Add(vol);
		total.Add(vol);
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		int slot_id = i % slot_count;
		const OnlineAverage1& av = stats[slot_id];
		
		buffer.Set(i, av.mean);
	}
}

void VolumeSlots::Assist(int cursor, VectorBool& vec) {
	double value0 = GetBuffer(0).Get(cursor);
	ASSERT(IsFin(value0));
	if      (value0 > total.mean * 2.000)		vec.Set(VOLUME_VERYHIGH, true);
	else if (value0 > total.mean * 1.333)		vec.Set(VOLUME_HIGH, true);
	else if (value0 > total.mean * 0.666)		vec.Set(VOLUME_MED, true);
	else										vec.Set(VOLUME_LOW, true);
	if (cursor > 0) {
		double value1 = GetBuffer(0).Get(cursor - 1);
		if (value0 > value1)		vec.Set(VOLUME_INC, true);
		else						vec.Set(VOLUME_DEC, true);
	}
}









TrendIndex::TrendIndex() {
	
}

void TrendIndex::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Blue);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	
	if ( period < 1 )
		throw DataExc();
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "TrendIndex");
}

void TrendIndex::Start() {
	Buffer& buffer     = GetBuffer(0);
	Buffer& err_buffer = GetBuffer(1);
	Buffer& change_buf = GetBuffer(2);
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	
	double diff;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();
	
	VectorBool& label = GetOutput(0).label;
	label.SetCount(bars);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		bool bit_value;
		double err, av_change, buf_value;
		
		Process(open_buf, i, period, err_div, err, buf_value, av_change, bit_value);
		
		err_buffer.Set(i, err);
		buffer.Set(i, buf_value);
		change_buf.Set(i, av_change);
		label.Set(i, bit_value);
	}
	
	
	int true_count = 0;
	for (int i = 0; i < bars; i++) {
		if (label.Get(i)) true_count++;
	}
	ASSERT(true_count > 0);
}

void TrendIndex::Process(ConstBuffer& open_buf, int i, int period, int err_div, double& err, double& buf_value, double& av_change, bool& bit_value) {
	double current = open_buf.GetUnsafe(i);
	int trend_begin_pos = Upp::max(0, i - period);
	double begin = open_buf.GetUnsafe(trend_begin_pos);
	
	int len = (i - trend_begin_pos);
	if (len <= 0) return;
	av_change = fabs(current - begin) / len;
	
	err = 0;
	for(int j = trend_begin_pos; j < i; j++) {
		double change = open_buf.GetUnsafe(j+1) - open_buf.GetUnsafe(j);
		double diff = change - av_change;
		err += fabs(diff);
	}
	
	err /= len;
	err /= err_div;
	buf_value = av_change - err;
	bit_value = begin > current;
}






OnlineMinimalLabel::OnlineMinimalLabel() {
	
}

void OnlineMinimalLabel::Init() {
	SetCoreChartWindow();
}

void OnlineMinimalLabel::Start() {
	int bars = GetBars();
	int symbol = GetSymbol();
	int tf = GetTf();
	
	DataBridge* db			= dynamic_cast<DataBridge*>(GetInputCore(0, symbol, tf));
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	double spread_point		= db->GetPoint();
	double cost				= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = GetOutput(0).label;
	labelvec.SetCount(bars);
	
	for(int i = GetCounted(); i < bars; i++) {
		SetSafetyLimit(i);
		
		const int count = 1;
		bool sigbuf[count];
		int begin = Upp::max(0, i - 100);
		int end = i + 1;
		GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
		
		bool label = sigbuf[count - 1];
		labelvec.Set(i, label);
	}
}

void OnlineMinimalLabel::GetMinimalSignal(double cost, ConstBuffer& open_buf, int begin, int end, bool* sigbuf, int sigbuf_size) {
	int write_begin = end - sigbuf_size;
	
	for(int i = begin; i < end; i++) {
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < end; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		
		for(int k = i; k < j; k++) {
			int buf_pos = k - write_begin;
			if (buf_pos >= 0)
				sigbuf[buf_pos] = label;
		}
		
		i = j - 1;
	}
}

















SelectiveMinimalLabel::SelectiveMinimalLabel() {
	
}

void SelectiveMinimalLabel::Init() {
	SetCoreChartWindow();
	SetBufferColor(0, Color(85, 255, 150));
	SetBufferStyle(0, DRAW_ARROW);
	SetBufferArrow(0, 159);
}

void SelectiveMinimalLabel::Start() {
	int bars = GetBars();
	int symbol = GetSymbol();
	int tf = GetTf();
	
	// Too heavy to calculate every time and too useless
	if (GetCounted()) return;
	
	VectorMap<int, Order> orders;
	DataBridge* db			= dynamic_cast<DataBridge*>(GetInputCore(0, symbol, tf));
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	double spread_point		= db->GetPoint();
	double cost				= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = GetOutput(0).label;
	VectorBool& enabledvec = GetOutput(1).label;
	labelvec.SetCount(bars);
	enabledvec.SetCount(bars);
	
	Buffer& buf = GetBuffer(0);
	
	bars--;
	
	for(int i = 0; i < bars; i++) {
		SetSafetyLimit(i);
		
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < bars; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		for(int k = i; k < j; k++) {
			SetSafetyLimit(k);
			labelvec.Set(k, label);
		}
		
		i = j - 1;
	}
	
	bool prev_label = labelvec.Get(0);
	int prev_switch = 0;
	for(int i = 1; i < bars; i++) {
		bool label = labelvec.Get(i);
		
		int len = i - prev_switch;
		
		if (label != prev_label) {
			Order& o = orders.Add(i);
			o.label = prev_label;
			o.start = prev_switch;
			o.stop = i;
			o.len = o.stop - o.start;
			
			double open = open_buf.GetUnsafe(o.start);
			double close = open_buf.GetUnsafe(o.stop);
			o.av_change = fabs(close - open) / len;
			
			double err = 0;
			for(int k = o.start; k < o.stop; k++) {
				SetSafetyLimit(k);
				double diff = open_buf.GetUnsafe(k+1) - open_buf.GetUnsafe(k);
				err += fabs(diff);
			}
			o.err = err / len;
			
			prev_switch = i;
			prev_label = label;
		}
	}
	
	
	
	struct ChangeSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.av_change < b.av_change;
		}
	};
	Sort(orders, ChangeSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.av_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	struct LengthSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.len < b.len;
		}
	};
	Sort(orders, LengthSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.len_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	
	struct ErrorSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.err > b.err;
		}
	};
	Sort(orders, ErrorSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.err_idx = (double)i / (double)(orders.GetCount() - 1);
		o.idx = o.av_idx + o.err_idx + 2 * o.len_idx;
	}
	
	
	struct IndexSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.idx < b.idx;
		}
	};
	Sort(orders, IndexSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.idx_norm = (double)i / (double)(orders.GetCount() - 1);
	}
	
	/*
	struct PosSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.start < b.start;
		}
	};
	Sort(orders, PosSorter());
	
	if (GetCounted() == 0) {DUMPC(orders);}*/
	
	double idx_limit_f = idx_limit * 0.01;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.idx_norm < idx_limit_f) continue;
		
		ASSERT(o.start <= o.stop);
		
		for(int k = o.start; k < o.stop; k++) {
			SetSafetyLimit(k);
			enabledvec.Set(k, true);
			double open = open_buf.GetUnsafe(k);
			if (o.label)		buf.Set(k, open - 10 * spread_point);
			else				buf.Set(k, open + 10 * spread_point);
		}
	}
}






















ReactionContext::ReactionContext() {
	
}

void ReactionContext::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Blue);
	
	if ( period < 1 )
		throw DataExc();
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "ReactionContext");
}

void ReactionContext::Start() {
	ConstBuffer& open = GetInputBuffer(0, 0);
	ConstBuffer& low  = GetInputBuffer(0, 1);
	ConstBuffer& high = GetInputBuffer(0, 2);
	
	Buffer& buffer     = GetBuffer(0);
	
	double diff;
	int bars = GetBars();
	int counted = GetCounted();
	if (!counted) counted++;

	for (int cursor = counted; cursor < bars; cursor++) {
		SetSafetyLimit(cursor);

		
		// Open/Close trend
		{
			int dir = 0;
			int len = 0;
			for (int i = cursor-1; i >= 0; i--) {
				int idir = open.Get(i+1) > open.Get(i) ? +1 : -1;
				if (dir != 0 && idir != dir) break;
				dir = idir;
				len++;
			}
			if (len >= length) {
				if (dir == +1)
					buffer.Set(cursor, UPTREND);
				else
					buffer.Set(cursor, DOWNTREND);
				continue;
			}
		}
		
		
		// High break
		{
			int dir = 0;
			int len = 0;
			double hi = high.Get(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = hi > high.Get(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			if (len >= length) {
				buffer.Set(cursor, HIGHBREAK);
				continue;
			}
		}
		
		// Low break
		{
			int dir = 0;
			int len = 0;
			double lo = low.Get(cursor-1);
			for (int i = cursor-2; i >= 0; i--) {
				int idir = lo < low.Get(i) ? +1 : -1;
				if (dir != 0 && idir != +1) break;
				dir = idir;
				len++;
			}
			if (len >= length) {
				buffer.Set(cursor, LOWBREAK);
				continue;
			}
		}
		
		// Trend reversal
		if (cursor >= 4) {
			double t0_diff		= open.Get(cursor-0) - open.Get(cursor-1);
			double t1_diff		= open.Get(cursor-1) - open.Get(cursor-2);
			double t2_diff		= open.Get(cursor-2) - open.Get(cursor-3);
			int t0 = t0_diff > 0 ? +1 : -1;
			int t1 = t1_diff > 0 ? +1 : -1;
			int t2 = t2_diff > 0 ? +1 : -1;
			if (t0 * t1 == -1 && t1 * t2 == +1) {
				double t0_hilodiff	= high.Get(cursor-1) - low.Get(cursor-1);
				if (fabs(t0_hilodiff) >= fabs(t1_diff)) {
					if (t0 == +1)
						buffer.Set(cursor, REVERSALUP);
					else
						buffer.Set(cursor, REVERSALDOWN);
				} else {
					if (t0 == +1)
						buffer.Set(cursor, REVERSALUP);
					else
						buffer.Set(cursor, REVERSALDOWN);
				}
				continue;
			}
		}
		
	}
}









VolatilityContext::VolatilityContext() {
	
}

void VolatilityContext::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Blue);
	
	if ( period < 1 )
		throw DataExc();
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "VolatilityContext");
}

void VolatilityContext::Start() {
	ConstBuffer& open = GetInputBuffer(0, 0);
	ConstBuffer& low  = GetInputBuffer(0, 1);
	ConstBuffer& high = GetInputBuffer(0, 2);
	
	Buffer& buffer     = GetBuffer(0);
	
	double diff;
	int bars = GetBars();
	int counted = GetCounted();
	if (!counted) counted++;
	
	double point = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()))->GetPoint();

	for (int cursor = counted; cursor < bars; cursor++) {
		
		double diff = fabs(open.Get(cursor) - open.Get(cursor - 1));
		int step = (int)((diff + point * 0.5) / point);
		median_map.GetAdd(step, 0)++;
		
	}
	
	SortByKey(median_map, StdLess<int>());
	
	volat_divs.SetCount(0);
	
	int64 total = 0;
	for(int i = 0; i < median_map.GetCount(); i++)
		total += median_map[i];
	
	int64 count_div = total / div;
	total = 0;
	int64 next_div = count_div;
	volat_divs.Add(median_map.GetKey(0) * point);
	for(int i = 0; i < median_map.GetCount(); i++) {
		total += median_map[i];
		if (total >= next_div) {
			next_div += count_div;
			volat_divs.Add(median_map.GetKey(i) * point);
		}
	}
	if (volat_divs.GetCount() < div) {
		volat_divs.Add(median_map.TopKey() * point);
	}
	
	for (int cursor = counted; cursor < bars; cursor++) {
		SetSafetyLimit(cursor);
		
		double diff = fabs(open.Get(cursor) - open.Get(cursor - 1));
		
		int lvl = -1;
		for(int i = 0; i < volat_divs.GetCount(); i++) {
			if (diff < volat_divs[i]) {
				lvl = i - 1;
				break;
			}
		}
		if (lvl == -1)
			lvl = div - 1;
		
		
		buffer.Set(cursor, lvl / (double)(volat_divs.GetCount()-2));
	}
}










Obviousness::Obviousness() {
	
}

void Obviousness::Init() {
	SetCoreSeparateWindow();
	for(int i = 0; i < buffer_count; i++) {
		int j = i / 2;
		bool is_sig = i % 2;
		SetBufferColor(i, GrayColor(80 + j * 20));
		SetBufferLineWidth(i, 1 + is_sig);
	}
	
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0.5);
}

void Obviousness::Start() {
	int counted = GetCounted();
	int bars = GetBars();
	
	RefreshInput();
	if (!counted) {
		RefreshInitialOutput();
		RefreshIOStats();
	}
	
	RefreshOutput();
	
}

void Obviousness::RefreshInput() {

	// Get reference values
	System& sys = GetSystem();
	DataBridge& db = dynamic_cast<DataBridge&>(*GetInputCore(0, GetSymbol(), GetTf()));
	double spread_point		= db.GetPoint();
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int begin = data_in.GetCount();
	data_in.SetCount(data_count);
	
	// Prepare Moving average
	av_wins.SetCount(period_count);
	for(int i = 0; i < av_wins.GetCount(); i++)
		av_wins[i].SetPeriod(1 << (1+i));
	
	
	// Prepare VolatilityContext
	volat_divs.SetCount(period_count);
	median_maps.SetCount(period_count);
	for(int j = 0; j < period_count; j++) {
		int period = 1 << (1+j);
		VectorMap<int,int>& median_map = median_maps[j];
		for(int cursor = max(period, begin); cursor < data_count; cursor++) {
			double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
			int step = (int)((diff + spread_point * 0.5) / spread_point);
			median_map.GetAdd(step, 0)++;
		}
		SortByKey(median_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_map.GetCount(); i++)
			total += median_map[i];
		int64 count_div = total / volat_div;
		total = 0;
		int64 next_div = count_div;
		volat_divs[j].SetCount(0);
		volat_divs[j].Add(median_map.GetKey(0) * spread_point);
		for(int i = 0; i < median_map.GetCount(); i++) {
			total += median_map[i];
			if (total >= next_div) {
				next_div += count_div;
				volat_divs[j].Add(median_map.GetKey(i) * spread_point);
			}
		}
		if (volat_divs[j].GetCount() < volat_div)
			volat_divs[j].Add(median_map.TopKey() * spread_point);
	}
	
	
	// Run main data filler
	for(int cursor = begin; cursor < data_count; cursor++) {
		Snap& snap = data_in[cursor];
		int bit_pos = 0;
		double open1 = open_buf.Get(cursor);
		
		
		for(int k = 0; k < period_count; k++) {
			
			// OnlineMinimalLabel
			double cost	 = spread_point * (1 + k);
			const int count = 1;
			bool sigbuf[count];
			int begin = Upp::max(0, cursor - 200);
			int end = cursor + 1;
			OnlineMinimalLabel::GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
			bool label = sigbuf[count - 1];
			snap.Set(bit_pos++, label);
		
			
			// TrendIndex
			bool bit_value;
			int period = 1 << (1 + k);
			double err, av_change, buf_value;
			TrendIndex::Process(open_buf, cursor, period, 3, err, buf_value, av_change, bit_value);
			snap.Set(bit_pos++, buf_value > 0.0);
			
			
			// VolatilityContext
			int lvl = -1;
			if (cursor >= period) {
				double diff = fabs(open_buf.Get(cursor) - open_buf.Get(cursor - period));
				for(int i = 0; i < volat_divs[k].GetCount(); i++) {
					if (diff < volat_divs[k][i]) {
						lvl = i - 1;
						break;
					}
				}
			}
			for(int i = 0; i < volat_div; i++)
				snap.Set(bit_pos++,  lvl == i);
			
		
			// MovingAverage
			OnlineAverageWindow1& av_win = av_wins[k];
			double prev = av_win.GetMean();
			av_win.Add(open1);
			double curr = av_win.GetMean();
			label = open1 < prev;
			snap.Set(bit_pos++, label);
			
			
			// Momentum
			begin = Upp::max(0, cursor - period);
			double open2 = open_buf.Get(begin);
			double value = open1 / open2 - 1.0;
			label = value < 0.0;
			snap.Set(bit_pos++, label);
			
			
			// Open/Close trend
			period = 1 << k;
			int dir = 0;
			int len = 0;
			if (cursor >= period * 3) {
				for (int i = cursor-period; i >= 0; i -= period) {
					int idir = open_buf.Get(i+period) > open_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != dir) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
		
		
			// High break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double hi = high_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = hi > high_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
			
			
			// Low break
			dir = 0;
			len = 0;
			if (cursor >= period * 3) {
				double lo = low_buf.Get(cursor-period);
				for (int i = cursor-1-period; i >= 0; i -= period) {
					int idir = lo < low_buf.Get(i) ? +1 : -1;
					if (dir != 0 && idir != +1) break;
					dir = idir;
					len++;
				}
			}
			snap.Set(bit_pos++, len > 2);
			
			
			// Trend reversal
			int t0 = +1;
			int t1 = +1;
			int t2 = -1;
			if (cursor >= 4*period) {
				double t0_diff		= open_buf.Get(cursor-0*period) - open_buf.Get(cursor-1*period);
				double t1_diff		= open_buf.Get(cursor-1*period) - open_buf.Get(cursor-2*period);
				double t2_diff		= open_buf.Get(cursor-2*period) - open_buf.Get(cursor-3*period);
				t0 = t0_diff > 0 ? +1 : -1;
				t1 = t1_diff > 0 ? +1 : -1;
				t2 = t2_diff > 0 ? +1 : -1;
			}
			if (t0 * t1 == -1 && t1 * t2 == +1) {
				snap.Set(bit_pos++, t0 == +1);
				snap.Set(bit_pos++, t0 != +1);
			} else {
				snap.Set(bit_pos++, false);
				snap.Set(bit_pos++, false);
			}
		}
		
		ASSERT(bit_pos == row_size);
	}
}

void Obviousness::RefreshInitialOutput() {
	
	// Get reference values
	System& sys = GetSystem();
	DataBridge& db = dynamic_cast<DataBridge&>(*GetInputCore(0, GetSymbol(), GetTf()));
	double spread_point		= db.GetPoint();
	double cost				= spread_point * 3;
	int tf = 0;
	ASSERT(spread_point > 0.0);
	
	
	// Prepare maind data
	ConstBuffer& open_buf = db.GetBuffer(0);
	ConstBuffer& low_buf  = db.GetBuffer(1);
	ConstBuffer& high_buf = db.GetBuffer(2);
	int data_count = open_buf.GetCount();
	int begin = data_out.GetCount();
	data_out.SetCount(data_count);
	VectorMap<int, Order> orders;
	data_count--;
	
	if (begin != 0) return;
	
	for(int i = GetCounted(); i < data_count; i++) {
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < data_count; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		for(int k = i; k < j; k++) {
			data_out[k].Set(RT_SIGNAL, label);
		}
		
		i = j - 1;
	}
	
	bool prev_label = data_out[0].Get(RT_SIGNAL);
	int prev_switch = 0;
	for(int i = 1; i < data_count; i++) {
		bool label = data_out[i].Get(RT_SIGNAL);
		
		int len = i - prev_switch;
		
		if (label != prev_label) {
			Order& o = orders.Add(i);
			o.label = prev_label;
			o.start = prev_switch;
			o.stop = i;
			o.len = o.stop - o.start;
			
			double open = open_buf.GetUnsafe(o.start);
			double close = open_buf.GetUnsafe(o.stop);
			o.av_change = fabs(close - open) / len;
			
			double err = 0;
			for(int k = o.start; k < o.stop; k++) {
				double diff = open_buf.GetUnsafe(k+1) - open_buf.GetUnsafe(k);
				err += fabs(diff);
			}
			o.err = err / len;
			
			prev_switch = i;
			prev_label = label;
		}
	}
	
	
	
	struct ChangeSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.av_change < b.av_change;
		}
	};
	Sort(orders, ChangeSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.av_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	struct LengthSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.len < b.len;
		}
	};
	Sort(orders, LengthSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.len_idx = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	
	struct ErrorSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.err > b.err;
		}
	};
	Sort(orders, ErrorSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.err_idx = (double)i / (double)(orders.GetCount() - 1);
		o.idx = o.av_idx + o.err_idx + 2 * o.len_idx;
	}
	
	
	struct IndexSorter {
		bool operator ()(const Order& a, const Order& b) const {
			return a.idx < b.idx;
		}
	};
	Sort(orders, IndexSorter());
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		o.idx_norm = (double)i / (double)(orders.GetCount() - 1);
	}
	
	
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.idx_norm < idx_limit_f) continue;
		
		ASSERT(o.start <= o.stop);
		
		for(int k = o.start; k < o.stop; k++) {
			data_out[k].Set(RT_ENABLED, true);
		}
	}
}

void Obviousness::RefreshIOStats() {
	int counted = GetCounted();
	int bars = GetBars();
	
	bars -= 1;
	
	// Randomize the input pattern at first call
	int c0 = Upp::min(250, row_size);
	int c1 = Upp::min(250, row_size*row_size);
	int c2 = Upp::min(250, row_size*row_size*row_size);
	int c3 = Upp::min(250, row_size*row_size*row_size*row_size);
	const int MAX_BITS = 4;
	int counts[MAX_BITS] = {c0, c1, c2, c3};
	int exp_count = c0 + c1 + c2 + c3;
	bitmatches.SetCount(exp_count);
	Index<uint32> hashes;
	
	ASSERT(row_size < 256);
	int total_count = 0;
	for(int i = 0; i < MAX_BITS; i++) {
		int count = counts[i];
		
		for(int j = 0; j < count; j++) {
			BitMatcher& bm = bitmatches[total_count++];
			bm.bit_count = i+1;
			
			uint32 hash;
			do {
				CombineHash ch;
				for(int k = 0; k < bm.bit_count; k++) {
					int v;
					bool already;
					do {
						v = Random(row_size);
						already = false;
						for (int l = 0; l < k; l++) {
							if (bm.bit_ids[l] == v) {
								already = true;
								break;
							}
						}
					}
					while (already);
					bm.bit_ids[k] = v;
					ch << v << 1;
				}
				hash = ch;
			}
			while (hashes.Find(hash) != -1);
			hashes.Add(hash);
		}
		
	}
	ASSERT(exp_count == total_count);
	
	
	// Collect stats
	for(int i = counted; i < bars; i++) {
		Snap& snap_in = data_in[i];
		
		
		for(int j = 0; j < bitmatches.GetCount(); j++) {
			BitMatcher& bm = bitmatches[j];
			uint32 value = 0;
			for(int k = 0; k < bm.bit_count; k++) {
				bool b = snap_in.Get(bm.bit_ids[k]);
				if (b)
					value |= 1 << k;
			}
			
			ASSERT(value < max_bit_values);
			
			BitComboStat& stat = bm.bit_stats[value];
			Snap& snap_out = data_out[i];
			
			bool enabled = snap_out.Get(RT_ENABLED);
			bool signal  = snap_out.Get(RT_SIGNAL);
			
			stat.total_count++;
			if (enabled) {
				stat.enabled_count++;
				if (signal) stat.true_count++;
			}
		}
	}
}


void Obviousness::RefreshOutput() {
	int counted = GetCounted();
	int bars = GetBars();
	
	VectorBool& signalbool = GetOutput(0).label;
	VectorBool& enabledbool = GetOutput(1).label;
	signalbool.SetCount(bars);
	enabledbool.SetCount(bars);
	for(int i = counted; i < bars; i++) {
		
		Snap& snap_in = data_in[i];
		
		int signal_div = 0;
		int enabled_div = 0;
		double signal_av = 0;
		double enabled_av = 0;
		
		for(int j = 0; j < bitmatches.GetCount(); j++) {
			BitMatcher& bm = bitmatches[j];
			uint32 value = 0;
			for(int k = 0; k < bm.bit_count; k++) {
				bool b = snap_in.Get(bm.bit_ids[k]);
				if (b)
					value |= 1 << k;
			}
			
			ASSERT(value < max_bit_values);
			
			BitComboStat& stat = bm.bit_stats[value];
			
			if (stat.total_count > 1) {
				enabled_div++;
				double prob = stat.total_count > 1 ? (double)stat.enabled_count / (double)(stat.total_count - 1) : 0.5;
				enabled_av += prob;
				if (prob > 0.5) {
					signal_div++;
					double sigprob = stat.enabled_count > 1 ? (double)stat.true_count / (double)(stat.enabled_count - 1) : 0.5;
					signal_av += sigprob;
				}
			}
		}
		
		enabled_av	/= max(1, enabled_div);
		signal_av	/= max(1, signal_div);
		GetBuffer(0).Set(i, enabled_av);
		GetBuffer(1).Set(i, signal_av);
		
		signalbool.Set(i, signal_av > 0.5);
		enabledbool.Set(i, enabled_av > 0.5);
	}
}
/*
void Obviousness::RefreshTreshold() {
	int counted = GetCounted();
	int bars = GetBars();
	
	
	System& sys = GetSystem();
	DataBridge& db = dynamic_cast<DataBridge&>(*GetInputCore(0, GetSymbol(), GetTf()));
	double spread_point		= db.GetPoint();
	double cost				= spread_point * 3;
	int tf = 0;
	ASSERT(spread_point > 0.0);
	ConstBuffer& open_buf = db.GetBuffer(0);

	
	// Get enabled step
	double max_enabled = 0, max_signal = 0;
	for(int i = 0; i < 15; i++) {
		int pos = bars - 1 - i;
		double enabled = fabs(GetBuffer(0).Get(pos));
		double signal = fabs(GetBuffer(1).Get(pos) - 0.5);
		if (enabled > max_enabled) max_enabled = enabled;
		if (signal  > max_signal) max_signal = signal;
	}
	double enabled_step = max_enabled * 0.1;
	double signal_step = max_signal * 0.1;
	
	
	// Loop treshold ranges
	double max_res = -DBL_MAX;
	max_res_enabled_treshold = 0;
	max_res_signal_treshold = 0;
	const int min_len = 5;
	for (int step_i = -15; step_i < 15; step_i++) {
		double enabled_treshold = step_i * enabled_step;
		
		for (int step_j = 0; step_j < 15; step_j++) {
			double signal_treshold = step_j * signal_step;
			
			double total_change = 1.0;
			double prev_open;
			bool prev_enabled = false, prev_signal;
			int open_len = 0;
			for(int i = counted; i < bars; i++) {
				
				double enabled_av = GetBuffer(0).Get(i);
				
				
				bool enabled = enabled_av >= enabled_treshold;
				bool signal;
				
				if (enabled) {
					double signal_av = GetBuffer(1).Get(i);
					signal = signal_av > 0.5;
					if ( signal && signal_av < 0.5 + signal_treshold)
						enabled = false;
					if (!signal && signal_av > 0.5 - signal_treshold)
						enabled = false;
				}
				
				bool do_close = false, do_open = false;
				if (prev_enabled) {
					if (!enabled)
						do_close = true;
					else if (signal != prev_signal && open_len >= min_len) {
						do_close = true;
						do_open = true;
					}
				}
				else if (enabled)
					do_open = true;
				
				double open = open_buf.Get(i);
				if (do_close) {
					double change;
					if (prev_signal == false) change = open / (prev_open + spread_point);
					else change = 1.0 - (open / (prev_open - spread_point) - 1.0);
					total_change *= change;
				}
				if (do_open) {
					prev_signal = signal;
					prev_open = open;
					open_len = 0;
				}
				prev_enabled = enabled;
				if (enabled) open_len++;
			}
			if (prev_enabled) {
				double open = open_buf.Get(bars-1);
				double change = open / prev_open;
				if (prev_signal == false) change = open / (prev_open + spread_point);
				else change = 1.0 - (open / (prev_open - spread_point) - 1.0);
				total_change *= change;
			}
			
			
			if (total_change > max_res) {
				max_res_enabled_treshold = enabled_treshold;
				max_res_signal_treshold = signal_treshold;
				max_res = total_change;
			}
		}
	}
	
	
	LOG("Max treshold: " << max_res_enabled_treshold << ", " << max_res_signal_treshold << " with result " << max_res);
	
}
*/







VolatilityContextReversal::VolatilityContextReversal() {
	
}

void VolatilityContextReversal::Init() {
	/*SetCoreSeparateWindow();
	SetBufferColor(0, GrayColor(80));
	SetBufferLineWidth(0, 2);
	
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0.5);*/
}

void VolatilityContextReversal::Start() {
	int counted = GetCounted();
	int bars = GetBars();
	
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	ConstBuffer& volat_ctx = GetInputBuffer(1, 0);
	
	VectorBool& signal = GetOutput(0).label;
	signal.SetCount(bars);
	
	for(int i = counted; i < bars; i++) {
		double d = volat_ctx.Get(i);
		bool is_peak = d > 0.99;
		
		if (is_peak) {
			double open = open_buf.Get(i);
			int true_count = 0;
			for(int j = max(0, i-11); j < i; j++) {
				double o = open_buf.Get(j);
				if (o > open) true_count++;
			}
			signal.Set(i, true_count >= 6);
		}
		else {
			// Copy previous
			signal.Set(i, signal.Get(max(0, i-1)));
		}
	}
}





ObviousTarget::ObviousTarget() {
	
}

bool ObviousTarget::GetInput(int i, int j) {
	return bufs[j]->Get(i);
}

double ObviousTarget::GetOutput(int i) {
	return GetBuffer(1).Get(i);
}

void ObviousTarget::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, GrayColor(80));
	SetBufferLineWidth(0, 1);
	
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0.0);
	
	bufs.SetCount(row_size);
	for(int i = 0; i < bufs.GetCount(); i++)
		bufs[i] = &GetInputCore(1 + i, GetSymbol(), GetTf())->GetOutput(0).label;
	
	outbuf = &GetInputCore(5, GetSymbol(), GetTf())->GetOutput(0).label;
}

void ObviousTarget::Start() {
	
	if (!GetCounted()) {
		RefreshTargetValues();
		RefreshIOStats();
	}
	
	RefreshOutput();
}

void ObviousTarget::RefreshTargetValues() {
	int counted = GetCounted();
	int bars = GetBars();
	
	ConstBuffer& open_buf = GetInputBuffer(0,0);
	Buffer& dst = GetBuffer(1);
	
	bool prev_value = outbuf->Get(bars-1);
	double prev_open = open_buf.Get(bars-1);
	for(int i = bars-1; i >= counted; i--) {
		bool value = outbuf->Get(i);
		double open = open_buf.Get(i);
		
		if (value != prev_value) {
			prev_open = open;
			prev_value = value;
		}
		
		double change = prev_open / open - 1.0;
		// Don't add this: if (prev_value) change *= -1.0;
		
		dst.Set(i, change);
	}
	
	
}

void ObviousTarget::RefreshIOStats() {
	int counted = GetCounted();
	int bars = GetBars();
	
	bars -= 1;
	
	// Randomize the input pattern at first call
	int c0 = Upp::min(250, row_size);
	int c1 = Upp::min(250, row_size*(row_size-1));
	int c2 = Upp::min(250, row_size*(row_size-1)*(row_size-2));
	int c3 = Upp::min(250, row_size*(row_size-1)*(row_size-2)*(row_size-3));
	const int MAX_BITS = 4;
	int counts[MAX_BITS] = {c0, c1, c2, c3};
	int exp_count = c0 + c1 + c2 + c3;
	bitmatches.SetCount(exp_count);
	Index<uint32> hashes;
	
	ASSERT(row_size < 256);
	int total_count = 0;
	for(int i = 0; i < MAX_BITS; i++) {
		int count = counts[i];
		
		for(int j = 0; j < count; j++) {
			BitMatcher& bm = bitmatches[total_count++];
			bm.bit_count = i+1;
			
			uint32 hash;
			do {
				CombineHash ch;
				for(int k = 0; k < bm.bit_count; k++) {
					int v;
					bool already;
					do {
						v = Random(row_size);
						already = false;
						for (int l = 0; l < k; l++) {
							if (bm.bit_ids[l] == v) {
								already = true;
								break;
							}
						}
					}
					while (already);
					bm.bit_ids[k] = v;
					ch << v << 1;
				}
				hash = ch;
			}
			while (hashes.Find(hash) != -1);
			hashes.Add(hash);
		}
		
	}
	ASSERT(exp_count == total_count);
	
	
	// Collect stats
	for(int i = counted; i < bars; i++) {
		for(int j = 0; j < bitmatches.GetCount(); j++) {
			BitMatcher& bm = bitmatches[j];
			uint32 value = 0;
			for(int k = 0; k < bm.bit_count; k++) {
				bool b = GetInput(i, bm.bit_ids[k]);
				if (b)
					value |= 1 << k;
			}
			
			ASSERT(value < max_bit_values);
			
			BitComboStat& stat = bm.bit_stats[value];
			
			stat.total_count++;
			stat.sum += GetOutput(i);
		}
	}
}

void ObviousTarget::RefreshOutput() {
	int counted = GetCounted();
	int bars = GetBars();
	
	for(int i = counted; i < bars; i++) {
		int enabled_div = 0;
		double enabled_av = 0;
		
		for(int j = 0; j < bitmatches.GetCount(); j++) {
			BitMatcher& bm = bitmatches[j];
			uint32 value = 0;
			for(int k = 0; k < bm.bit_count; k++) {
				bool b = GetInput(i, bm.bit_ids[k]);
				if (b)
					value |= 1 << k;
			}
			
			ASSERT(value < max_bit_values);
			
			BitComboStat& stat = bm.bit_stats[value];
			
			if (stat.total_count > 1) {
				enabled_div++;
				double av = stat.total_count > 0 ? stat.sum / stat.total_count : 0.0;
				enabled_av += av;
			}
		}
		
		enabled_av	/= max(1, enabled_div);
		GetBuffer(0).Set(i, enabled_av);
	}
}









ExampleAdvisor::ExampleAdvisor() {
	
}

void ExampleAdvisor::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, RainbowColor(Randomf()));
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	SetJob(0, tf_str + " Training")
		.SetBegin		(THISBACK(TrainingBegin))
		.SetIterator	(THISBACK(TrainingIterator))
		.SetEnd			(THISBACK(TrainingEnd))
		.SetInspect		(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void ExampleAdvisor::Start() {
	if (once) {
		if (prev_counted > 0) prev_counted--;
		once = false;
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int bars = GetBars();
		if (prev_counted < bars) {
			LOG("ExampleAdvisor::Start Refresh");
			RefreshAll();
		}
	}
}

bool ExampleAdvisor::TrainingBegin() {
	
	training_pts.SetCount(max_rounds, 0);
	
	// In case of having other advisors as dependency:
	// Don't start if jobs of dependencies are not finished
	for(int i = 0; i < inputs.GetCount(); i++) {
		Input& in = inputs[i];
		for(int j = 0; j < in.GetCount(); j++) {
			Source& src = in[j];
			if (src.core) {
				Core* core = dynamic_cast<Core*>(src.core);
				if (core && !core->IsJobsFinished())
					return false;
			}
		}
	}
	
	// Allow iterating
	return true;
}

bool ExampleAdvisor::TrainingIterator() {
	
	// Show progress
	GetCurrentJob().SetProgress(round, max_rounds);
	
	
	// ---- Do your training work here ----
	
	
	// Put some result data here for graph
	training_pts[round] = sin(round * 0.01);
	
	
	// Keep count of iterations
	round++;
	
	// Stop eventually
	if (round >= max_rounds) {
		SetJobFinished();
	}
	
	return true;
}

bool ExampleAdvisor::TrainingEnd() {
	RefreshAll();
	return true;
}

bool ExampleAdvisor::TrainingInspect() {
	bool success = false;
	
	INSPECT(success, "ok: this is an example");
	INSPECT(success, "warning: this is an example");
	INSPECT(success, "error: this is an example");
	
	// You can fail the inspection too
	//if (!success) return false;
	
	return true;
}

void ExampleAdvisor::RefreshAll() {
	RefreshSourcesOnlyDeep();
	
	
	// ---- Do your final result work here ----
	
	
	// Keep counted manually
	prev_counted = GetBars();
	
	
	// Write oscillator indicator
	Buffer& buf = GetBuffer(0);
	for(int i = 0; i < GetBars(); i++) {
		buf.Set(i, sin(i * 0.01));
	}
}

void ExampleAdvisor::TrainingCtrl::Paint(Draw& w) {
	Size sz = GetSize();
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	ExampleAdvisor* ea = dynamic_cast<ExampleAdvisor*>(&*job->core);
	ASSERT(ea);
	DrawVectorPolyline(id, sz, ea->training_pts, polyline);
	
	w.DrawImage(0, 0, id);
}








}
