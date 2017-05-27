#include "Overlook.h"

namespace Overlook {

inline Color Silver() {return Color(192, 192, 192);}
inline Color Wheat() {return Color(245, 222, 179);}
inline Color LightSeaGreen() {return Color(32, 178, 170);}
inline Color YellowGreen() {return Color(154, 205, 50);}
inline Color Lime() {return Color(0, 255, 0);}
inline Color DodgerBlue() {return Color(30, 144, 255);}
inline Color SaddleBrown() {return Color(139, 69, 19);}
inline Color Pink() {return Color(255, 192, 203);}

Color RainbowColor(double progress)
{
    int div = progress * 6;
    int ascending = (int) (progress * 255);
    int descending = 255 - ascending;

    switch (div)
    {
        case 0:
            return Color(255, ascending, 0);
        case 1:
            return Color(descending, 255, 0);
        case 2:
            return Color(0, 255, ascending);
        case 3:
            return Color(0, descending, 255);
        case 4:
            return Color(ascending, 0, 255);
        default: // case 5:
            return Color(255, 0, descending);
    }
}
















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
	ma_shift = 0;
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
		SetSafetyLimit(pos);
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
	
	if ( period_adx >= 100 || period_adx <= 0 )
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
	
	plot_begin = 0;
}

void BollingerBands::Init() {
	SetCoreChartWindow();
	
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
}

void Envelopes::Init() {
	SetCoreChartWindow();
	
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
}

void ParabolicSAR::Init() {
	SetCoreChartWindow();
	
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







StandardDeviation::StandardDeviation() {
	period = 20;
	ma_method = 0;
	applied_value = 0;
	shift = 0;
}

void StandardDeviation::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(0);
	
	SetBufferColor(0, Blue);
	
	SetBufferShift ( 0, shift );
	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "StdDev");
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", ma_method);
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
		SafetyCheck(k);
		
		while ( k <= i ) {
			SafetyCheck(k);
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
	SetCoreLevel(0, 0.3 - 0.5); // normalized
	SetCoreLevel(1, 0.7 - 0.5); // normalized
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"DeMarker");
	
	SetBufferBegin ( 0, period );
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
	applied_value = 0;
}

void ForceIndex::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferBegin(0,period);
	
	SetBufferBegin ( 0, period );
	
	AddSubCore<MovingAverage>().Set("period", period).Set("offset", 0).Set("method", ma_method);
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











Momentum::Momentum() {
	period = 14;
}

void Momentum::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, DodgerBlue);
	
	if ( period <= 0 )
		throw DataExc();

	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"Momentum");
}

void Momentum::Start() {
	Buffer& buffer = GetBuffer(0);
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
	
	//bars--;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double close1 = Open( i );
		double close2 = Open( i - period );
		buffer.Set(i, close1 * 100 / close2 - 100);
	}
}








OsMA::OsMA() {
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
	params = false;
}

void OsMA::Init() {
	SetCoreSeparateWindow();
	
	SetBufferColor(0, Silver);
	SetBufferLineWidth(0, 2);
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	
	SetBufferBegin ( 0, signal_sma_period );

	if ( fast_ema_period <= 1 || slow_ema_period <= 1 || signal_sma_period <= 1 || fast_ema_period >= slow_ema_period ) {
		Panic ( "Wrong input parameters" );
	}
	else
		params = true;
	
	AddSubCore<MovingAverage>().Set("period", fast_ema_period).Set("offset", 0).Set("method", MODE_EMA);
	AddSubCore<MovingAverage>().Set("period", slow_ema_period).Set("offset", 0).Set("method", MODE_EMA);
}



	
void OsMA::Start() {
	Buffer& osma_buffer = GetBuffer(0);
	Buffer& buffer = GetBuffer(1);
	Buffer& signal_buffer = GetBuffer(2);
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= signal_sma_period || !params )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	
	ConstBuffer& ma1_buf = At(0).GetBuffer(0);
	ConstBuffer& ma2_buf = At(1).GetBuffer(0);
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double ma1 = ma1_buf.Get(i);
		double ma2 = ma2_buf.Get(i);
		buffer.Set(i, ma1 - ma2);
	}

	SimpleMAOnBuffer( bars, counted, 0, signal_sma_period, buffer, signal_buffer );
	
	for (int i = counted; i < bars; i++)
		osma_buffer.Set(i, buffer.Get(i) - signal_buffer.Get(i));
}




RelativeStrengthIndex::RelativeStrengthIndex() {
	period = 14;
}

void RelativeStrengthIndex::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, 30.0 - 50); // normalized
	SetCoreLevel(1, 70.0 - 50); // normalized
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	if ( period < 1 )
		throw DataExc();
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "RSI");
}


void RelativeStrengthIndex::Start() {
	Buffer& buffer = GetBuffer(0);
	Buffer& pos_buffer = GetBuffer(1);
	Buffer& neg_buffer = GetBuffer(2);
	
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
		buffer.Set(period, 100.0 - ( 100.0 / ( 1.0 + pos_buffer.Get(period) / neg_buffer.Get(period) ) ));
		pos = period + 1;
	}

	double prev_value  = Open(pos-1);

	for (int i = pos; i < bars; i++) {
		SetSafetyLimit(i);
		double value  = Open(i);
		diff = value - prev_value;
		pos_buffer.Set(i, ( pos_buffer.Get(i - 1) * ( period - 1 ) + ( diff > 0.0 ? diff : 0.0 ) ) / period);
		neg_buffer.Set(i, ( neg_buffer.Get(i - 1) * ( period - 1 ) + ( diff < 0.0 ? -diff : 0.0 ) ) / period);
		double rsi = 100.0 - 100.0 / ( 1 + pos_buffer.Get(i) / neg_buffer.Get(i) );
		buffer.Set(i, rsi - 50); // normalized
		prev_value = value;
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
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1, Red);
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20.0 - 50); // normalized
	SetCoreLevel(1, 80.0 - 50); // normalized
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

	
	for (int i = start; i < bars; i++) {
		SetSafetyLimit(i);
		double dmin = 1000000.0;
		double dmax = -1000000.0;

		for (int k = i - k_period; k < i; k++) {
			double low = Low( k );
			double high = High( k );

			if ( dmin > low )
				dmin = low;

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
	
	for (int i = start; i < bars; i++) {
		SetSafetyLimit(i);
		double sumlow = 0.0;
		double sumhigh = 0.0;

		for (int k = ( i - slowing + 1 ); k <= i; k++) {
			double close = Open( k );
			sumlow += ( close - low_buffer.Get(k-1) );
			sumhigh += ( high_buffer.Get(k) - low_buffer.Get(k-1) );
		}

		if ( sumhigh == 0.0 )
			buffer.Set(i, 100.0 - 50); // normalized
		else
			buffer.Set(i, sumlow / sumhigh * 100 - 50); // normalized
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






WilliamsPercentRange::WilliamsPercentRange() {
	period = 14;
}

void WilliamsPercentRange::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -20 + 50); // normalized
	SetCoreLevel(1, -80 + 50); // normalized
	SetBufferBegin ( 0, period );
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"WPR");
}

void WilliamsPercentRange::Start() {
	Buffer& buffer = GetBuffer(0);
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
		buffer.Set(i, -100 * ( max_high - close ) / ( max_high - min_low ) + 50); // normalized
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
		buffer.Set(i, Volume(i-1));
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
	
	if (counted) counted--;
	
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
	
	if (counted) counted--;
	
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
	double min = Low(index), test;
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

	if ( bars < input_depth || input_backstep >= input_depth )
		throw DataExc();

	if ( counted == 0 )
		counted = input_backstep;
	else {
		int i = bars-1;
		counter_z = 0;

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
	
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1); // This indicator peeks anyway
		
		int lowest = LowestLow(input_depth, i);
		extremum = Low(lowest);
		double low = Low(i);

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

		int highest = HighestHigh(input_depth, i);
		extremum = High(highest);
		double high = High(i);
	
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
	Buffer& day = GetBuffer(0);
	Buffer& month = GetBuffer(1);
	Buffer& year = GetBuffer(2);
	Buffer& week = GetBuffer(3);
	int bars = GetBars();
	int counted =  GetCounted();
	BaseSystem& base = GetBaseSystem();
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		Time t = base.GetTimeTf(GetTf(), i);
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
	
	Vector<double> crosses;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		int highest = HighestHigh(period, i-1);
		int lowest  = LowestLow(period, i-1);
		double highesthigh = High(highest);
		double lowestlow   = Low(lowest);
		
		int count = (highesthigh - lowestlow) / point + 1;
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
			
			int low_pos  = (low  - lowestlow)   / inc;
			int high_pos = (high - lowestlow) / inc;
			if (high_pos >= count) high_pos = count - 1;
			
			for(int k = low_pos; k <= high_pos; k++) {
				crosses[k]++;
			}
		}
		
		double close = Open(i);
		int value_pos = (close  - lowestlow)  / inc;
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







SupportResistanceOscillator::SupportResistanceOscillator() {
	period = 300;
	max_crosses = 100;
	max_radius = 100;
	smoothing_period = 30;
}

void SupportResistanceOscillator::Init() {
	SetCoreSeparateWindow();
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	SetCoreMinimum(-1);
	SetCoreMaximum(1);
	AddSubCore<SupportResistance>().Set("period", period).Set("max_crosses", max_crosses).Set("max_radius", max_radius);
}

void SupportResistanceOscillator::Start() {
	Buffer& osc = GetBuffer(0);
	Buffer& osc_av = GetBuffer(1);
	int bars = GetBars();
	int counted = GetCounted();
	
	ConstBuffer& ind1 = At(0).GetBuffer(0);
	ConstBuffer& ind2 = At(0).GetBuffer(1);
	
	Vector<double> crosses;
	int prev_pos = counted ? counted-1 : 0;
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








CorrelationOscillator::CorrelationOscillator() {
	period = 10;
	sym_count = -1;
}

void CorrelationOscillator::Init() {
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	sym_count = GetBaseSystem().GetSymbolCount();
	if (sym_count < 0) throw DataExc();
	
	SetCoreMaximum(+1.0);
	SetCoreMinimum(-1.0);
	SetCoreSeparateWindow();
	SetBufferBegin(0, period);
	
	for(int i = 0; i < sym_count-1; i++) {
		SetBufferColor(i, RainbowColor((double)i / sym_count));
	}
	
	opens.SetCount(sym_count, 0);
	averages.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++) {
		ConstBuffer& open = GetInputBuffer(0, i, GetTimeframe(), 0);
		opens[i] = &open;
	}
}

void CorrelationOscillator::Process(int id, int output) {
	int counted = GetCounted();
	int bars = GetBars();
	
	ConstBuffer& a = *opens[GetSymbol()];
	ConstBuffer& b = *opens[id];
	
	Buffer& buf = GetBuffer(output);
	
	OnlineAverage& s = averages[id];
	
	if (b.GetCount() == 0) {
		LOG("CorrelationOscillator error: No data for symbol " << GetBaseSystem().GetSymbol(id));
		return;
	}
	
	if (counted < period) {
		ASSERT(counted == 0);
		s.mean_a = a.Get(0);
		s.mean_b = b.Get(0);
		s.count = 1;
		for(int i = 1; i < period; i++) {
			SetSafetyLimit(i);
			s.Add(a.Get(i), b.Get(i));
		}
		counted = period;
	}
	SetSafetyLimit(counted);
	
	Vector<double> cache_a, cache_b;
	cache_a.SetCount(period);
	cache_b.SetCount(period);
	
	int cache_offset = 0;
	for(int i = 1; i < period; i++) {
		cache_a[i] = a.Get(counted - period + i);
		cache_b[i] = b.Get(counted - period + i);
	}
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		
		double da = a.Get(i);
		double db = b.Get(i);
		
		s.Add(da, db);
		cache_a[cache_offset] = da;
		cache_b[cache_offset] = db;
		cache_offset = (cache_offset + 1) % period;
		
		double avg1 = s.mean_a;
		double avg2 = s.mean_b;
		double sum1 = 0;
	    double sumSqr1 = 0;
	    double sumSqr2 = 0;
		for(int j = 0; j < period; j++) {
			double v1 = cache_a[j];
			double v2 = cache_b[j];
			sum1 += (v1 - avg1) * (v2 - avg2);
			sumSqr1 += pow(v1 - avg1, 2.0);
			sumSqr2 += pow(v2 - avg2, 2.0);
		}
    
		double mul = sumSqr1 * sumSqr2;
		double correlation_coef = mul != 0.0 ? sum1 / sqrt(mul) : 0;
		
		buf.Set(i, correlation_coef);
	}
}

void CorrelationOscillator::Start() {
	int counted = GetCounted();
	int bars = GetBars();
	
	if (counted == bars)
		return;
	
	if (counted > 0) counted = Upp::max(counted - period, 0);
	
	for(int i = 0, j = 0; i < sym_count; i++) {
		if (i == GetSymbol()) continue;
		Process(i, j);
		j++;
	}
}





















ParallelSymLR::ParallelSymLR() {
	period = 13;
	method = 0;
	SetCoreSeparateWindow();
}

void ParallelSymLR::Init() {
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

void ParallelSymLR::Start() {
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
		
		// Store values for next iteration
		prev1 = d1;
		prev2 = d2;
		prev_value = value;
		prev_diff = diff;
	}
}


















ParallelSymLREdge::ParallelSymLREdge() {
	period = 13;
	method = 0;
	slowing = 54;
	SetCoreSeparateWindow();
}

void ParallelSymLREdge::Init() {
	int draw_begin;
	if (period < 2)
		period = 13;
	draw_begin = period - 1;
	
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	
	AddSubCore<MovingAverage>().Set("period", period).Set("method", method);
	AddSubCore<MovingAverage>().Set("period", slowing).Set("method", method).Set("offset", -slowing/2);
}

void ParallelSymLREdge::Start() {
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
	
	// Calculate 'ParallelSymLR' data locally. See info for that.
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



}
