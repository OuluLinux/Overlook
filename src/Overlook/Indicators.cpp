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
	for (int i = 1; i < ma_period; i++)
		sum += Open(pos - i);
	while (pos < bars)
	{
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
			buffer.Set(i, 0);
}







MovingAverageConvergenceDivergence::MovingAverageConvergenceDivergence()
{
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
	
	params = false;
}

void MovingAverageConvergenceDivergence::Init()
{
	SetCoreSeparateWindow();
	//SetBufferCount(2, 2);
	SetBufferColor(0, Silver);
	SetBufferColor(1, Red);
	SetBufferLineWidth(0, 2);
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, signal_buffer );
	SetBufferBegin ( 1, signal_sma_period );
	
	//SetCoreDigits(GetDigits()+1);
	
	SetBufferStyle(0, DRAW_HISTOGRAM);
	SetBufferStyle(1, DRAW_LINE);
	
	SetBufferLabel(0,"MACD");
	SetBufferLabel(1,"Signal");
	
	
	if ( fast_ema_period <= 1 || slow_ema_period <= 1 || signal_sma_period <= 1 || fast_ema_period >= slow_ema_period )
		throw DataExc();
	else
		params = true;
	
	//if (RequireIndicator("ma", "period", fast_ema_period, "offset", 0, "method", MODE_EMA)) throw DataExc();
	//if (RequireIndicator("ma", "period", slow_ema_period, "offset", 0, "method", MODE_EMA)) throw DataExc();
}

void MovingAverageConvergenceDivergence::Start()
{
	/*int bars = GetBars();
	if ( bars <= signal_sma_period || !params )
		throw DataExc();

	int counted = GetCounted();
	if ( counted > 0 )
		counted--;

	Core& a_ind = At(0);
	Core& b_ind = At(1);
	
	for (int i = counted; i < bars; i++ )
	{
		double a_value = a_ind.GetIndexValue(i);
		double b_value = b_ind.GetIndexValue(i);
		double diff = a_value - b_value;
		buffer.Set(i, diff);
	}

	SimpleMAOnBuffer( bars, GetCounted(), 0, signal_sma_period, buffer, signal_buffer );
	*/
}






AverageDirectionalMovement::AverageDirectionalMovement()
{
	period_adx = 14;
}

void AverageDirectionalMovement::Init()
{
	/*
	SetCoreSeparateWindow();
	
	//SetBufferCount(6, 3);
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1,  YellowGreen);
	SetBufferColor(2,  Wheat);
	
	
	SetBufferLabel(0,"ADX");
	SetBufferLabel(1,"+DI");
	SetBufferLabel(2,"-DI");
	
	if ( period_adx >= 100 || period_adx <= 0 )
		throw DataExc();
	
	//SetIndexBuffer ( 0, adx_buffer );
	//SetIndexBuffer ( 1, pdi_buffer );
	//SetIndexBuffer ( 2, ndi_buffer );
	//SetIndexBuffer ( 3, pd_buffer);
	//SetIndexBuffer ( 4, nd_buffer);
	//SetIndexBuffer ( 5, tmp_buffer);
	SetBufferBegin ( 0, period_adx );
	SetBufferBegin ( 1, period_adx );
	SetBufferBegin ( 2, period_adx );
	*/
}

void AverageDirectionalMovement::Start()
{
	/*
	int bars = GetBars();
	if ( bars < period_adx )
		throw DataExc();

	int counted = GetCounted();

	if ( counted > 0 )
		counted--;
	else
	{
		counted = 1;
		pdi_buffer.Set(0, 0.0);
		ndi_buffer.Set(0, 0.0);
		adx_buffer.Set(0, 0.0);
	}
	
	for ( int i = counted; i < bars;i++ )
	{
		double Hi    = High( i );
		double prev_hi = High( i-1 );
		double Lo    = Low( i );
		double prev_lo = Low( i-1 );
		double prev_cl = Open( i );

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

		double tr = max ( max ( fabs ( Hi - Lo ), fabs ( Hi - prev_cl ) ), fabs ( Lo - prev_cl ) );

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
	*/
}












BollingerBands::BollingerBands()
{
	bands_period = 20;
	bands_shift = 0;
	bands_deviation = 2.0;
	
	plot_begin = 0;
}

void BollingerBands::Init()
{
	/*
	SetCoreChartWindow();
	//SetBufferCount(4, 3);
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1,  LightSeaGreen);
	SetBufferColor(2,  LightSeaGreen);
	
	//SetCoreDigits(GetDigits());
	
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
	
	//SetIndexBuffer ( 0, ml_buffer );
	//SetIndexBuffer ( 1, tl_buffer );
	//SetIndexBuffer ( 2, bl_buffer );
	//SetIndexBuffer ( 3, stddev_buffer );


	plot_begin = bands_period - 1;
	SetBufferBegin ( 0, bands_period );
	SetBufferBegin ( 1, bands_period );
	SetBufferBegin ( 2, bands_period );

	SetBufferShift ( 0, bands_shift );
	SetBufferShift ( 1, bands_shift );
	SetBufferShift ( 2, bands_shift );
	*/
}

void BollingerBands::Start()
{
	/*
	int bars = GetBars();
	int pos;
	int counted = GetCounted();


	if ( bars < plot_begin )
		throw DataExc();

	if ( counted > 1 )
		pos = counted - 1;
	else
		pos = 0;

	Core& ts = *GetSource().Get<Core>();
	for ( int i = pos; i < bars; i++ )
	{
		ml_buffer.Set(i, SimpleMAOpenValueAsc( ts, i, bands_period ));
		stddev_buffer.Set(i, StdDev_Func ( i, ml_buffer, bands_period ));
		tl_buffer.Set(i, ml_buffer.Get(i) + bands_deviation * stddev_buffer.Get(i));
		bl_buffer.Set(i, ml_buffer.Get(i) - bands_deviation * stddev_buffer.Get(i));
	}
	*/
}

double BollingerBands::StdDev_Func ( int position, const Buffer& MAvalue, int period )
{
	double tmp = 0.0;

	if ( position < period )
		return tmp;
	
	int bars = GetBars();
	for (int i = 0; i < period; i++ )
	{
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

void Envelopes::Init()
{
	/*
	SetCoreChartWindow();
	//SetBufferCount(3, 2);
	SetBufferColor(0, Blue);
	SetBufferColor(1, Red);
	
	//SetIndexBuffer ( 0, up_buffer );
	//SetIndexBuffer ( 1, down_buffer );
	//SetIndexBuffer ( 2, ma_buffer );

	SetBufferBegin ( 0, ma_period - 1 );
	SetBufferShift ( 0, ma_shift );
	SetBufferShift ( 1, ma_shift );
	
	SetBufferStyle(0, DRAW_LINE);
	SetBufferStyle(1, DRAW_LINE);
	//SetCoreDigits(GetDigits());
	
	if (RequireIndicator("ma", "period", ma_period, "offset", 0, "method", ma_method)) throw DataExc();
	*/
}


void Envelopes::Start() {
	/*
	int bars = GetBars();

	if ( bars < ma_period )
		throw DataExc();

	int counted = GetCounted();
	if (counted) counted--;
	else counted = ma_period;
	
	Core& ind0 = At(0);
	
	for (int i = counted; i < bars; i++) {
		double value = ind0.GetIndexValue( i );
		ASSERT(value != 0);
		ma_buffer.Set(i, value);
		up_buffer.Set(i, ( 1 + deviation / 100.0 ) * ma_buffer.Get(i));
		down_buffer.Set(i, ( 1 - deviation / 100.0 ) * ma_buffer.Get(i));
	}
	*/
}




ParabolicSAR::ParabolicSAR()
{
	sar_step = 0.02;
	sar_maximum = 0.2;
}

void ParabolicSAR::Init() {
/*
	SetCoreChartWindow();
	//SetBufferCount(3, 1);
	SetBufferColor(0, Lime);
	
	if (sar_step <= 0.0)
		throw DataExc();
	
	if (sar_maximum <= 0.0)
		throw DataExc();
	
	//SetIndexBuffer ( 0, sar_buffer );
	//SetIndexBuffer ( 1, ep_buffer );
	//SetIndexBuffer ( 2, af_buffer );

	last_rev_pos = 0;
	direction_long = false;
	
	//SetCoreDigits(GetDigits());
	SetBufferStyle(0, DRAW_ARROW);
	SetBufferArrow(0, 159);
	*/
}

void ParabolicSAR::Start ()
{
	/*
	int counted = GetCounted();
	int bars = GetBars();

	if ( bars < 3 )
		throw DataExc();
	
	int pos = counted - 1;

	if ( pos < 1 )
	{
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
		double low  = Low( i );
		double high = High( i );

		if ( direction_long )
		{
			if ( sar_buffer.Get(i-1) > low )
			{
				direction_long = false;
				sar_buffer.Set(i-1, GetHigh( i, last_rev_pos ));
				ep_buffer.Set(i, low);
				last_rev_pos = i;
				af_buffer.Set(i, sar_step);
			}
		}

		else
		{
			if ( sar_buffer.Get(i-1) < high )
			{
				direction_long = true;
				sar_buffer.Set(i-1, GetLow( i, last_rev_pos ));
				ep_buffer.Set(i, high);
				last_rev_pos = i;
				af_buffer.Set(i, sar_step);
			}
		}

		if ( direction_long )
		{
			if ( high > ep_buffer.Get(i-1) && i != last_rev_pos )
			{
				ep_buffer.Set(i, high);
				af_buffer.Set(i, af_buffer.Get(i-1) + sar_step);

				if ( af_buffer.Get(i) > sar_maximum )
					af_buffer.Set(i, sar_maximum);
			}

			else
			{
				if ( i != last_rev_pos )
				{
					af_buffer.Set(i, af_buffer.Get (i-1));
					ep_buffer.Set(i, ep_buffer.Get (i-1));
				}
			}

			double value = sar_buffer.Get(i-1) + af_buffer.Get(i) * ( ep_buffer.Get(i) - sar_buffer.Get(i-1) );
			sar_buffer.Set(i, value);

			if ( value > low || value > prev_low )
				sar_buffer.Set(i+1, min ( low, prev_low ));
		}

		else
		{
			if ( low < ep_buffer.Get(i-1) && i != last_rev_pos )
			{
				ep_buffer.Set(i, low);
				af_buffer.Set(i, af_buffer.Get(i-1) + sar_step);

				if ( af_buffer.Get(i) > sar_maximum )
					af_buffer.Set(i, sar_maximum);
			}

			else
			{
				if ( i != last_rev_pos )
				{
					af_buffer.Set(i, af_buffer.Get(i-1));
					ep_buffer.Set(i, ep_buffer.Get(i-1));
				}
			}

			double value = sar_buffer.Get(i-1) + af_buffer.Get(i) * ( ep_buffer.Get(i) - sar_buffer.Get(i-1) );
			sar_buffer.Set(i, value);

			if ( value < high || value < prev_high )
				sar_buffer.Set(i, max ( high, prev_high ));
		}

		prev_high = high;

		prev_low  = low;
	}
	*/
}

double ParabolicSAR::GetHigh( int pos, int start_pos )
{
	ASSERT(pos >= start_pos);
	int bars = GetBars();
	
	double result = High( start_pos );

	for ( int i = start_pos+1; i <= pos; i++ )
	{
		double high = High( i );

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

	for ( int i = start_pos+1; i <= pos; i++ )
	{
		double low = Low( i );

		if ( result > low )
			result = low;
	}

	return ( result );
}







StandardDeviation::StandardDeviation()
{
	period = 20;
	ma_method = 0;
	applied_value = 0;
	shift = 0;
	
	
}

void StandardDeviation::Init()
{
/*
	SetCoreSeparateWindow();
	SetCoreMinimum(0);
	//SetBufferCount(1, 1);
	SetBufferColor(0, Blue);
	
	//SetIndexBuffer ( 0, stddev_buffer );
	SetBufferShift ( 0, shift );
	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "StdDev");
	
	if (RequireIndicator("ma", "period", period, "offset", 0, "method", ma_method)) throw DataExc();
	*/
}

void StandardDeviation::Start()
{
	/*
	double value, amount, ma;

	int bars = GetBars();
	if ( bars <= period )
		throw DataExc();

	int counted = GetCounted();

	if ( counted > 0 )
		counted--;
	else
		counted = period;
	
	Core& ind0 = At(0);
	
	for (int i = counted; i < bars; i++)
	{
		amount = 0.0;
		ma = ind0.GetIndexValue( i );

		for ( int j = 0; j < period; j++ )
		{
			value = Open( i - j );
			amount += ( value - ma ) * ( value - ma );
		}

		stddev_buffer.Set(i, sqrt ( amount / period ));
	}
	*/
}










AverageTrueRange::AverageTrueRange()
{
	period = 14;
	
}

void AverageTrueRange::Init() {
	/*
	SetCoreSeparateWindow();
	//SetBufferCount(2, 1);
	SetBufferColor(0, DodgerBlue);
	
	//SetCoreDigits(GetDigits());
	SetBufferStyle(0,DRAW_LINE);
	
	//SetIndexBuffer(0,atr_buffer);
	//SetIndexBuffer(1,tr_buffer);
	SetBufferLabel(0, "ATR");
	
	//SetIndexBuffer ( 0, atr_buffer );
	//SetIndexBuffer ( 1, tr_buffer );

	if ( period <= 0 )
		throw DataExc();
	
	SetBufferBegin ( 0, period );
	*/
}

void AverageTrueRange::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();


	if ( bars <= period || period <= 0 )
		throw DataExc();

	//bars--;
	if ( counted == 0 )
	{
		tr_buffer.Set(0, 0.0);
		atr_buffer.Set(0, 0.0);
		
		for (int i = 1; i < bars; i++ )
		{
			double h = High( i );
			double l = Low( i );
			double c = Open( i+1 );

			tr_buffer.Set(i, max ( h, c ) - min ( l, c ));
		}

		double first_value = 0.0;

		for (int i = 1; i <= period; i++ )
		{
			atr_buffer.Set(i, 0.0);
			first_value += tr_buffer.Get(i);
		}

		first_value /= period;

		atr_buffer.Set(period, first_value);

		counted = period + 1;
	}

	else
		counted--;
	
	for (int i = counted; i < bars; i++ )
	{
		double h = High( i );
		double l = Low( i );
		double c = Open( i+1 );

		tr_buffer.Set(i, max ( h, c ) - min ( l, c ));
		atr_buffer.Set(i, atr_buffer.Get( i - 1 ) + ( tr_buffer.Get(i) - tr_buffer.Get( i - period ) ) / period);
	}
	*/
}




BearsPower::BearsPower()
{
	period = 13;
	
	
}

void BearsPower::Init() {
/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, Silver);
	
	//SetCoreDigits(GetDigits());
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	
	SetBufferLabel(0,"ATR");
	
	//SetIndexBuffer ( 0, buffer );
	
	if (RequireIndicator("ma", "period", period, "offset", 0, "method", MODE_EMA)) throw DataExc();
	*/
}

void BearsPower::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	if ( bars <= period )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	
	Core& ind0 = At(0);
	
	for (int i = counted; i < bars; i++) {
		buffer.Set(i, Low( i ) - ind0.GetIndexValue( i ));
	}
	*/
}









BullsPower::BullsPower()
{
	period = 13;
	
}

void BullsPower::Init()
{
	/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, Silver);
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	SetBufferLabel(0,"Bulls");
	
	//SetIndexBuffer ( 0, buffer );
	
	if (RequireIndicator("ma", "period", period, "offset", 0, "method", MODE_EMA)) throw DataExc();
	*/
}

void BullsPower::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	
	Core& ind0 = At(0);
	
	for ( int i = counted; i < bars; i++ ) {
		buffer.Set(i, High( i ) - ind0.GetIndexValue(i) );
	}
	*/
}







CommodityChannelIndex::CommodityChannelIndex()
{
	period = 14;
	
	
}

void CommodityChannelIndex::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(3, 1);
	SetBufferColor(0, LightSeaGreen);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -100.0);
	SetCoreLevel(1,  100.0);
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"CCI");
	
	//SetIndexBuffer ( 1, value_buffer );
	//SetIndexBuffer ( 2, mov_buffer );
	//SetIndexBuffer ( 0, cci_buffer );

	if ( period <= 1 )
		throw DataExc();
	
	SetBufferBegin ( 0, period );
	*/
}

void CommodityChannelIndex::Start()
{
	/*
	int    i, k, pos;
	double sum, mul;
	int bars = GetBars();
	int counted = GetCounted();


	if ( bars <= period || period <= 1 )
		throw DataExc();


	if ( counted < 1 )
	{
		for ( i = 1; i < period; i++ )
		{
			double h = High( i );
			double l = Low( i );
			double c = Open( i+1 );

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
	for ( i = pos; i < bars; i++ )
	{
		double h = High( i );
		double l = Low( i );
		double c = Open( i+1 );

		value_buffer.Set(i, ( h + l + c ) / 3);
		mov_buffer.Set(i, SimpleMA ( i, period, value_buffer ));
	}

	mul = 0.015 / period;

	pos = period - 1;

	if ( pos < counted - 1 )
		pos = counted - 2;

	i = pos;

	while ( i < bars )
	{
		sum = 0.0;
		k = i + 1 - period;

		while ( k <= i )
		{
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
	*/
}







DeMarker::DeMarker()
{
	period = 14;
	
	
}

void DeMarker::Init()
{
	/*
	SetCoreSeparateWindow();
	SetCoreMinimum(-0.5);  // normalized
	SetCoreMaximum(0.5);   // normalized
	//SetBufferCount(3, 1);
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, 0.3 - 0.5); // normalized
	SetCoreLevel(1, 0.7 - 0.5); // normalized
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"DeMarker");
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, max_buffer );
	//SetIndexBuffer ( 2, min_buffer );
	SetBufferBegin ( 0, period );
	*/
}

void DeMarker::Start()
{
	/*
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

	for (int i = counted; i < bars; i++)
	{
		double high = High(i);
		double low  = Low(i);

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

		if ( num != 0.0 )
			buffer.Set(i, maxvalue / num - 0.5);  // normalized
		else
			buffer.Set(i, 0.0);
	}
	*/
}










ForceIndex::ForceIndex()
{
	period = 13;
	ma_method = 0;
	applied_value = 0;
}

void ForceIndex::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue);
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferBegin(0,period);
	
	//SetIndexBuffer ( 0, buffer );
	SetBufferBegin ( 0, period );
	
	if (RequireIndicator("ma", "period", period, "offset", 0, "method", ma_method)) throw DataExc();
	*/
}

void ForceIndex::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	if (bars <= period)
		throw DataExc();

	if (counted > 0)
		counted--;

	if (counted == 0)
		counted++;
	
	Core& ind0 = At(0);
	
	for ( int i = counted; i < bars; i++ )
	{
		double volume = Volume( i );
		double ma1 = ind0.GetIndexValue( i );
		double ma2 = ind0.GetIndexValue( i - 1 );
		buffer.Set(i, volume * (ma1 - ma2));
	}
	*/
}











Momentum::Momentum()
{
	period = 14;
	
}

void Momentum::Init()
{
	/*

	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue);
	
	//SetIndexBuffer ( 0, buffer );

	if ( period <= 0 )
		throw DataExc();

	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"Momentum");
	*/
}

void Momentum::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();


	if ( bars <= period || period <= 0 )
		throw DataExc();

	if ( counted <= 0 )
	{
		for (int i = 0; i < period; i++ )
			buffer.Set(i, 0.0);

		counted = period;
	}

	else
		counted--;
	
	//bars--;
	for (int i = counted; i < bars; i++ )
	{
		double close1 = Open( i+1 );
		double close2 = Open( i+1 - period );
		buffer.Set(i, close1 * 100 / close2 - 100);
	}
	*/
}








OsMA::OsMA()
{
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
	params = false;
}

void OsMA::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(3, 1);
	SetBufferColor(0, Silver);
	SetBufferLineWidth(0, 2);
	
	SetBufferStyle(0,DRAW_HISTOGRAM);
	//SetCoreDigits(GetDigits() + 2);
	
	//SetIndexBuffer ( 0, osma_buffer );
	//SetIndexBuffer ( 1, buffer );
	//SetIndexBuffer ( 2, signal_buffer );
	SetBufferBegin ( 0, signal_sma_period );

	if ( fast_ema_period <= 1 || slow_ema_period <= 1 || signal_sma_period <= 1 || fast_ema_period >= slow_ema_period )
	{
		Panic ( "Wrong input parameters" );
	}

	else
		params = true;
	
	if (RequireIndicator("ma", "period", fast_ema_period, "offset", 0, "method", MODE_EMA)) throw DataExc();
	if (RequireIndicator("ma", "period", slow_ema_period, "offset", 0, "method", MODE_EMA)) throw DataExc();
	*/
}



	
void OsMA::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= signal_sma_period || !params )
		throw DataExc();

	if ( counted > 0 )
		counted--;
	
	Core& ind1 = At(0);
	Core& ind2 = At(1);
	
	for (int i = counted; i < bars; i++)
	{
		double ma1 = ind1.GetIndexValue(i);
		double ma2 = ind2.GetIndexValue(i);
		buffer.Set(i, ma1 - ma2);
	}

	SimpleMAOnBuffer( bars, counted, 0, signal_sma_period, buffer, signal_buffer );
	
	for (int i = counted; i < bars; i++)
		osma_buffer.Set(i, buffer.Get(i) - signal_buffer.Get(i));
	*/
}




RelativeStrengthIndex::RelativeStrengthIndex()
{
	period = 14;
	
}

void RelativeStrengthIndex::Init()
{
/*
	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	//SetBufferCount(3, 1);
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, 30.0 - 50); // normalized
	SetCoreLevel(1, 70.0 - 50); // normalized
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	if ( period < 1 )
		throw DataExc();
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, pos_buffer );
	//SetIndexBuffer ( 2, neg_buffer );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0, "RSI");
	*/
}


void RelativeStrengthIndex::Start()
{
	/*
	double diff;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	int pos = counted - 1;

	if ( pos <= period )
	{
		buffer.Set(0, 0.0);
		pos_buffer.Set(0, 0.0);
		neg_buffer.Set(0, 0.0);
		double sum_p = 0.0;
		double sum_n = 0.0;


		double prev_value  = Open( 0 );

		for (int i = 1; i <= period;i++ )
		{
			double value  = Open(i);

			if ( prev_value == 0 )
			{
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

	for (int i = pos; i < bars; i++ )
	{
		double value  = Open(i);
		diff = value - prev_value;
		pos_buffer.Set(i, ( pos_buffer.Get(i - 1) * ( period - 1 ) + ( diff > 0.0 ? diff : 0.0 ) ) / period);
		neg_buffer.Set(i, ( neg_buffer.Get(i - 1) * ( period - 1 ) + ( diff < 0.0 ? -diff : 0.0 ) ) / period);
		double rsi = 100.0 - 100.0 / ( 1 + pos_buffer.Get(i) / neg_buffer.Get(i) );
		buffer.Set(i, rsi - 50); // normalized
		prev_value = value;
	}
	*/
}






RelativeVigorIndex::RelativeVigorIndex()
{
	period = 10;
	
}

void RelativeVigorIndex::Init()
{

	SetCoreSeparateWindow();
	//SetBufferCount(2, 2);
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
	double value_up, value_down, num, denum;

	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period + 8 )
		throw DataExc();

	if ( counted < 0 )
		throw DataExc();

	if (counted < 4+period) counted = 4+period;
	
	//bars--;
	
	for (int i = counted; i < bars; i++ )
	{
		num = 0.0;
		denum = 0.0;

		for ( int j = i; j > i - period && j >= 0; j-- )
		{
			double close0 = Open( j + 1 ); // peek is fixed at for loop
			double close1 = Open( j - 1 + 1 );
			double close2 = Open( j - 2 + 1 );
			double close3 = Open( j - 3 + 1 );

			double open0 = Open ( j );
			double open1 = Open ( j - 1 );
			double open2 = Open ( j - 2 );
			double open3 = Open ( j - 3 );

			double high0 = High( j );
			double high1 = High( j - 1 );
			double high2 = High( j - 2 );
			double high3 = High( j - 3 );

			double low0 = Low( j );
			double low1 = Low( j - 1 );
			double low2 = Low( j - 2 );
			double low3 = Low( j - 3 );

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

	for ( int i = counted; i < bars; i++ )
		signal_buffer.Set(i, ( buffer.Get(i) + 2 * buffer.Get(i-1) + 2 * buffer.Get(i-2) + buffer.Get(i-3) ) / 6);
	
}




StochasticOscillator::StochasticOscillator()
{
	k_period = 5;
	d_period = 3;
	slowing = 3;
	
}

void StochasticOscillator::Init()
{

	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	//SetBufferCount(4, 2);
	SetBufferColor(0, LightSeaGreen);
	SetBufferColor(1, Red);
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20.0 - 50); // normalized
	SetCoreLevel(1, 80.0 - 50); // normalized
	SetCoreLevelsColor(Silver);
	SetCoreLevelsStyle(STYLE_DOT);
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, signal_buffer );
	//SetIndexBuffer ( 2, high_buffer );
	//SetIndexBuffer ( 3, low_buffer );
	//SetCoreDigits(2);
	
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
	int i, k, start;
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= k_period + d_period + slowing )
		throw DataExc();

	start = k_period;

	if ( start + 1 < counted )
		start = counted - 2;
	else
	{
		for ( i = 0; i < start;i++ )
		{
			low_buffer.Set(i, 0.0);
			high_buffer.Set(i, 0.0);
		}
	}

	
	for ( i = start; i < bars;i++ )
	{
		double dmin = 1000000.0;
		double dmax = -1000000.0;

		for ( k = i - k_period + 1;k <= i;k++ )
		{
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
	else
	{
		for ( i = 0;i < start;i++ )
			buffer.Set(i, 0.0);
	}

	//bars--;
	
	for ( i = start; i < bars; i++ )
	{
		double sumlow = 0.0;
		double sumhigh = 0.0;

		for ( k = ( i - slowing + 1 ); k <= i; k++ )
		{
			double close = Open( k+1 );
			sumlow += ( close - low_buffer.Get(k) );
			sumhigh += ( high_buffer.Get(k) - low_buffer.Get(k) );
		}

		if ( sumhigh == 0.0 )
			buffer.Set(i, 100.0 - 50); // normalized
		else
			buffer.Set(i, sumlow / sumhigh * 100 - 50); // normalized
	}

	start = d_period - 1;

	if ( start + 1 < counted )
		start = counted - 2;
	else
	{
		for ( i = 0;i < start;i++ )
			signal_buffer.Set(i, 0.0);
	}

	for ( i = start; i < bars; i++ )
	{
		double sum = 0.0;

		for ( k = 0; k < d_period; k++ )
			sum += buffer.Get(i - k);

		signal_buffer.Set(i, sum / d_period);
	}
	
}






WilliamsPercentRange::WilliamsPercentRange()
{
	period = 14;
	
}

void WilliamsPercentRange::Init()
{

	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue);
	SetCoreLevelCount(2);
	SetCoreLevel(0, -20 + 50); // normalized
	SetCoreLevel(1, -80 + 50); // normalized
	
	//SetIndexBuffer ( 0, buffer );
	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"WPR");
	
}

void WilliamsPercentRange::Start()
{
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();

	if (!counted) {
		counted++;
		buffer.Set(0, 0);
	}
	
	//bars--;
	for (int i = counted; i < bars; i++)
	{
		int highest = HighestHigh( period, i );
		int lowest = LowestLow( period, i );
		double max_high = High( highest );
		double min_low = Low( lowest );
		double close = Open( i+1 );
		buffer.Set(i, -100 * ( max_high - close ) / ( max_high - min_low ) + 50); // normalized
	}
	
}








AccumulationDistribution::AccumulationDistribution() {
	
}

void AccumulationDistribution::Init()
{
	
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, LightSeaGreen);
	
	//SetCoreDigits(0);
	
	SetBufferStyle(0,DRAW_LINE);
	
	//SetIndexBuffer ( 0, buffer );
	
}

bool IsEqualDoubles ( double d1, double d2, double epsilon = 0.00001 )
{
	if ( epsilon < 0.0 )
		epsilon = -epsilon;

	if ( epsilon > 0.1 )
		epsilon = 0.00001;

	double diff = d1 - d2;

	if ( diff > epsilon || diff < -epsilon )
		return ( false );

	return ( true );
}

void AccumulationDistribution::Start()
{
	Buffer& buffer = GetBuffer(0);
	int bars = GetBars();
	int counted = GetCounted();
	
	if (!counted) {
		counted++;
		buffer.Set(0, 0);
	}
	
	//bars--;
	for (int i=counted; i < bars; i++)
	{
		double close = Open(i+1);
		buffer.Set(i, (close - Low(i)) - (High(i) - close));
		
		if (buffer.Get(i) != 0.0) {
			double diff = High(i) - Low(i);
			if (diff < 0.000000001)
				buffer.Set(i, 0.0);
			else
			{
				buffer.Set(i, buffer.Get(i) / diff);
				buffer.Set(i, buffer.Get(i) * (double)Volume(i));
			}
		}
		
		buffer.Set(i, buffer.Get(i) + buffer.Get(i-1));
	}
	
}






























MoneyFlowIndex::MoneyFlowIndex()
{
	period = 14;
	
}

void MoneyFlowIndex::Init()
{

	SetCoreSeparateWindow();
	SetCoreMinimum(-50); // normalized
	SetCoreMaximum(50);  // normalized
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20 - 50); // normalized
	SetCoreLevel(1, 80 - 50); // normalized
	//SetBufferCount(1, 1);
	SetBufferColor(0, Blue);
	
	//SetIndexBuffer ( 0, buffer );
	SetBufferBegin ( 0, period );
	
	SetBufferStyle(0,DRAW_LINE);
	SetBufferLabel(0,"MFI");
	
}

void MoneyFlowIndex::Start()
{
	/*
	double pos_mf, neg_mf, cur_tp, prev_tp;
	
	int bars = GetBars();
	int counted = GetCounted();

	if ( bars <= period )
		throw DataExc();
	
	if (counted < period + 2)
		counted = period + 2;
	
	for (int i=counted; i < bars; i++) {
		pos_mf = 0.0;
		neg_mf = 0.0;
		cur_tp = (
			High(i-1) +
			Low(i-1) +
			Open(i) ) / 3;

		for (int j = 0; j < period; j++)
		{
			prev_tp = (
				High(i-j-2) +
				Low(i-j-2) +
				Open(i-j-1) ) / 3;

			if ( cur_tp > prev_tp )
				pos_mf += Volume(i-j-1) * cur_tp;
			else
			{
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
	*/
}








ValueAndVolumeTrend::ValueAndVolumeTrend()
{
	applied_value = 5;
	
}

void ValueAndVolumeTrend::Init()
{
	
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue);
	
	//SetIndexBuffer ( 0, buffer );
	
	SetBufferStyle(0, DRAW_LINE);
	//SetCoreDigits(0);
	
}

void ValueAndVolumeTrend::Start()
{
	/*
	double cur_value, prev_value;
	
	int bars = GetBars();
	int counted = GetCounted();

	if ( counted > 0 )
		counted--;

	if (counted < 2)
		counted = 2;
	
	for (int i = counted; i < bars; i++ )
	{
		double volume = Volume(i-1);
		cur_value = GetAppliedValue ( applied_value, i - 1);
		prev_value = GetAppliedValue ( applied_value, i - 2 );
		buffer.Set(i,
			buffer.Get(i-1) + volume * ( cur_value -
			prev_value ) / prev_value);
	}
	*/
}






OnBalanceVolume::OnBalanceVolume()
{
	applied_value = 5;
	
}

void OnBalanceVolume::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue);
	
	//SetIndexBuffer ( 0, buffer );
	
	SetBufferStyle(0,DRAW_LINE);
	//SetCoreDigits(0);
	SetBufferLabel(0, "OBV");
	*/
}

void OnBalanceVolume::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();

	if ( counted > 0 )
		counted--;

	if (counted < 2)
		counted = 2;
	
	for (int i = counted; i < bars; i++)
	{
		double cur_value = GetAppliedValue ( applied_value, i);
		double prev_value = GetAppliedValue ( applied_value, i - 1 );

		if ( cur_value == prev_value )
			buffer.Set(i, buffer.Get(i));
		else
		{
			if ( cur_value < prev_value )
				buffer.Set(i, buffer.Get(i) - Volume(i));
			else
				buffer.Set(i, buffer.Get(i) + Volume(i));
		}
	}
	*/
}











Volumes::Volumes() {
	
}

void Volumes::Init() {
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, Green);
	
	//SetIndexBuffer ( 0, buf );
	SetBufferStyle(0, DRAW_HISTOGRAM);
	SetBufferLabel(0,"Volume");
}

void Volumes::Start() {
	int bars = GetBars();
	int counted = GetCounted();
	for (int i = counted; i < bars; i++)
		buf.Set(i, Volume(i));
}









#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#undef DATA_LIMIT
#define DATA_LIMIT  3


AcceleratorOscillator::AcceleratorOscillator() {
	
}

void AcceleratorOscillator::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(5, 3);
	SetBufferColor(0, Black);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	
	//SetCoreDigits(GetDigits() + 2);
	
	SetBufferStyle ( 0, DRAW_NONE );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, up_buffer );
	//SetIndexBuffer ( 2, down_buffer );
	//SetIndexBuffer ( 3, macd_buffer );
	//SetIndexBuffer ( 4, signal_buffer);
	SetBufferBegin ( 0, DATA_LIMIT );
	SetBufferBegin ( 1, DATA_LIMIT );
	SetBufferBegin ( 2, DATA_LIMIT );
	
	if (RequireIndicator("ma", "period", PERIOD_FAST, "offset", 0, "method", MODE_SMA)) throw DataExc();
	if (RequireIndicator("ma", "period", PERIOD_SLOW, "offset", 0, "method", MODE_SMA)) throw DataExc();
	*/
}



void AcceleratorOscillator::Start()
{
	/*
	double prev = 0.0, current;
	int bars = GetBars();
	int counted = GetCounted();


	if ( bars <= DATA_LIMIT )
		throw DataExc();
	
	Core& ind1 = At(0);
	Core& ind2 = At(1);
	
	if ( counted > 0 )
	{
		counted--;
		prev = macd_buffer.Get(counted) - signal_buffer.Get(counted);
	}

	for (int i = counted; i < bars; i++ )
		macd_buffer.Set(i,
			ind1.GetIndexValue(i) -
			ind2.GetIndexValue(i));

	SimpleMAOnBuffer ( bars, counted, 0, 5, macd_buffer, signal_buffer );
	
	bool up = true;

	for (int i = counted; i < bars; i++)
	{
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
	*/
}





GatorOscillator::GatorOscillator()
{
	jaws_period = 13;
	jaws_shift = 8;
	teeth_period = 8;
	teeth_shift = 5;
	lips_period = 5;
	lips_shift = 3;
	ma_method = 2;
	applied_value = PRICE_MEDIAN;
}

void GatorOscillator::Init()
{
	/*
	SetCoreSeparateWindow();
	//SetBufferCount(6, 6);
	SetBufferColor(0, Black);
	SetBufferColor(1, Red);
	SetBufferColor(2, Green);
	SetBufferColor(3, Black);
	SetBufferColor(4, Red);
	SetBufferColor(5, Green);
	
	//SetIndexBuffer ( 0, up_buffer );
	//SetIndexBuffer ( 1, up_red_buffer );
	//SetIndexBuffer ( 2, up_green_buffer );
	//SetIndexBuffer ( 3, down_buffer );
	//SetIndexBuffer ( 4, down_red_buffer );
	//SetIndexBuffer ( 5, down_green_buffer );
	
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
	
	if (RequireIndicator("ma", "period", teeth_period, "offset", teeth_shift - lips_shift, "method", ma_method)) throw DataExc();
	if (RequireIndicator("ma", "period", lips_period, "offset", 0, "method", ma_method)) throw DataExc();
	if (RequireIndicator("ma", "period", jaws_period, "offset", jaws_shift - teeth_shift, "method", ma_method)) throw DataExc();
	if (RequireIndicator("ma", "period", teeth_period, "offset", 0, "method", ma_method)) throw DataExc();
	
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
	*/
}

void GatorOscillator::Start()
{
	/*
	double prev, current;
	int    bars    = GetBars();
	int    counted = GetCounted();
	
	Core& ind1 = At(0);
	Core& ind2 = At(1);
	Core& ind3 = At(2);
	Core& ind4 = At(3);
	

	if ( counted <= teeth_period + teeth_shift - lips_shift )
		counted = ( teeth_period + teeth_shift - lips_shift );
	
	if (counted) counted--;
	
	for (int i = counted; i < bars; i++)
	{
		current =
			ind1.GetIndexValue(i) -
			ind2.GetIndexValue(i);

		if ( current <= 0.0 )
			down_buffer.Set(i, current);
		else
			down_buffer.Set(i, -current);
	}


	for (int i = counted; i < bars; i++)
	{
		prev = down_buffer.Get(i-1);
		current = down_buffer.Get(i);

		if ( current < prev )
		{
			down_red_buffer.Set(i, 0.0);
			down_green_buffer.Set(i, current);
		}

		if ( current > prev )
		{
			down_red_buffer.Set(i, current);
			down_green_buffer.Set(i, 0.0);
		}

		if ( current == prev )
		{
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
	
	for (int i = counted; i < bars; i++ )
	{
		current =
			ind3.GetIndexValue(i) -
			ind4.GetIndexValue(i);

		if ( current >= 0.0 )
			up_buffer.Set(i, current);
		else
			up_buffer.Set(i, -current);
	}

	for (int i = counted; i < bars; i++ )
	{
		prev = up_buffer.Get(i-1);
		current = up_buffer.Get(i);

		if ( current > prev )
		{
			up_red_buffer.Set(i, 0.0);
			up_green_buffer.Set(i, current);
		}

		if ( current < prev )
		{
			up_red_buffer.Set(i, current);
			up_green_buffer.Set(i, 0.0);
		}

		if ( current == prev )
		{
			if ( up_green_buffer.Get(i-1) > 0.0 )
				up_green_buffer.Set(i, current);
			else
				up_red_buffer.Set(i, current);
		}
	}
	*/
	
}







#undef DATA_LIMIT
#define PERIOD_FAST  5
#define PERIOD_SLOW 34
#define DATA_LIMIT  34

AwesomeOscillator::AwesomeOscillator()
{
	
}

void AwesomeOscillator::Init()
{
/*
	SetCoreSeparateWindow();
	//SetBufferCount(3, 3);
	SetBufferColor(0, Black);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	
	SetBufferStyle ( 0, DRAW_NONE );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	//SetCoreDigits ( GetDigits() + 1 );
	
	//SetIndexBuffer ( 0, buffer );
	//SetIndexBuffer ( 1, up_buffer );
	//SetIndexBuffer ( 2, down_buffer );
	
	SetBufferBegin ( 0, DATA_LIMIT );
	SetBufferBegin ( 1, DATA_LIMIT );
	SetBufferBegin ( 2, DATA_LIMIT );
	
	if (RequireIndicator("ma", "period", PERIOD_FAST, "offset", 0, "method", MODE_SMA)) throw DataExc();
	if (RequireIndicator("ma", "period", PERIOD_SLOW, "offset", 0, "method", MODE_SMA)) throw DataExc();
	*/
}

void AwesomeOscillator::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();
	double prev = 0.0, current;
	
	if ( bars <= DATA_LIMIT )
		throw DataExc();

	Core& ind1 = At(0);
	Core& ind2 = At(1);
	
	if (counted > 0) {
		counted--;
		if (counted)
			prev = buffer.Get(counted - 1);
	}

	for (int i = counted; i < bars; i++ )
		buffer.Set(i,
			ind1.GetIndexValue(i) -
			ind2.GetIndexValue(i));

	bool up = true;

	for (int i = counted; i < bars; i++)
	{
		current = buffer.Get(i);

		if ( current > prev )
			up = true;

		if ( current < prev )
			up = false;

		if ( !up )
		{
			down_buffer.Set(i, current);
			up_buffer.Set(i, 0.0);
		}

		else
		{
			up_buffer.Set(i, current);
			down_buffer.Set(i, 0.0);
		}

		prev = current;
	}
	*/
}















Fractals::Fractals()
{
	left_bars  = 3;
	right_bars = 0;
	
}

void Fractals::Init()
{
/*
	SetCoreChartWindow();
	
	//SetBufferCount(6, 6);
	SetBufferColor(0, Blue);
	SetBufferColor(1, Blue);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Blue);
	SetBufferColor(4, Blue);
	SetBufferColor(5, Blue);
	
	SetBufferStyle(0, DRAW_LINE);
	SetBufferArrow(0, 158);
	//SetIndexBuffer(0, line_up_buf1);
	
	SetBufferStyle(1, DRAW_LINE);
	SetBufferArrow(1, 158);
	//SetIndexBuffer(1, line_up_buf2);
	
	SetBufferStyle(2, DRAW_ARROW);
	SetBufferArrow(2, 119);
	//SetIndexBuffer(2, arrow_up_buf);
	
	SetBufferStyle(3, DRAW_ARROW);
	SetBufferArrow(3, 119);
	//SetIndexBuffer(3, arrow_down_buf);
	
	SetBufferStyle(4, DRAW_ARROW);
	SetBufferArrow(4, 119);
	//SetIndexBuffer(4, arrow_breakup_buf);
	
	SetBufferStyle(5, DRAW_ARROW);
	SetBufferArrow(5, 119);
	//SetIndexBuffer(5, arrow_breakdown_buf);
	*/
}

void Fractals::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	if(counted==0)
		counted = 1+max(left_bars, right_bars);
	else
		counted--;
	
	double prev_close = Open(counted);
	
	//bars--;
	
	for(int i = counted; i < bars; i++)
	{
		double close = Open(i+1);
		
		line_up_buf1.Set(i, IsFractalUp(i, left_bars, right_bars, bars));
		if (line_up_buf1.Get(i) == 0)
			line_up_buf1.Set(i, line_up_buf1.Get(i-1));
		else
			arrow_up_buf.Set(i, line_up_buf1.Get(i));
		
		line_up_buf2.Set(i, IsFractalDown(i, left_bars, right_bars, bars));
		
		if (line_up_buf2.Get(i) == 0)
			line_up_buf2.Set(i, line_up_buf2.Get(i-1));
		else
			arrow_down_buf.Set(i, line_up_buf2.Get(i));
		
		if (close < line_up_buf2.Get(i) && prev_close >= line_up_buf2.Get(i-1))
		
		arrow_breakdown_buf.Set(i, close);
		
		prev_close = close;
	}
	*/
}


double Fractals::IsFractalUp(int index, int left, int right, int maxind)
{
   double max = High(index);
   for(int i = index - left; i <= (index + right); i++)
   {
     if (i<0 || i>maxind) return(0);
     double high =  High(i);
      if(!(high > 0.0))return(0);
      if(max < high && i != index)
      {
         if(max < high)  return(0);
         if(abs(i - index) > 1) return(0);
      }
   }
   return(max);
}




double Fractals::IsFractalDown(int index, int left, int right, int maxind)
{
   double min = Low(index), test;
   for(int i = index - left; i <= (index + right); i++)
   {
      if (i<0 || i>maxind) return(0);
      double low =  Low(i);
      if(!(low > 0.0)) return(0);
      if(min > low && i != index)
      {
         if(min > low)
            return(0);

         if(abs(i - index) > 1)
            return(0);
      }

   }
   return(min);
}
















FractalOsc::FractalOsc()
{
	left_bars  = 3;
	right_bars = 0;
	smoothing_period = 12;
	
}

void FractalOsc::Init()
{
/*
	SetCoreSeparateWindow();
	
	//SetBufferCount(2, 2);
	SetBufferColor(0, Red);
	SetBufferColor(1, Blue);
	
	//SetIndexBuffer(0, buf);
	SetBufferStyle(0, DRAW_LINE);
	
	//SetIndexBuffer(1, av);
	SetBufferStyle(1, DRAW_LINE);
	
	if (RequireIndicator("frac", "left_bars", left_bars, "right_bars", right_bars)) throw DataExc();
	*/
}


void FractalOsc::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	else
		counted = 1+max(left_bars,right_bars);
	
	Core& ind = At(0);
	
	//bars--;
	
	for(int i = counted; i < bars; i++)
	{
		double close = Open(i+1);
		double buf1 = ind.GetIndexValue(0, i);
		double buf2 = ind.GetIndexValue(1, i);
		double v = (close - buf2) / (buf1 - buf2);
		if ( v > 1) v = 1;
		else if ( v < 0) v = 0;
		buf.Set(i, v - 0.5); // normalized
		
		av.Set(i, SimpleMA( i, smoothing_period, buf ));
	}
	*/
}






MarketFacilitationIndex::MarketFacilitationIndex()
{
	
}

void MarketFacilitationIndex::Init()
{
	/*
	SetCoreSeparateWindow();
	
	
	SetCoreLevelCount(2);
	SetCoreLevel(0, 20);
	SetCoreLevel(1, 80);
	
	//SetBufferCount(5, 4);
	SetBufferColor(0, Lime);
	SetBufferColor(1, SaddleBrown);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Pink);
	
	//SetIndexBuffer ( 0, up_up_buffer );
	//SetIndexBuffer ( 1, down_down_buffer );
	//SetIndexBuffer ( 2, up_down_buffer );
	//SetIndexBuffer ( 3, down_up_buffer );
	//SetIndexBuffer ( 4, buffer );

	SetBufferStyle ( 0, DRAW_HISTOGRAM );
	SetBufferStyle ( 1, DRAW_HISTOGRAM );
	SetBufferStyle ( 2, DRAW_HISTOGRAM );
	SetBufferStyle ( 3, DRAW_HISTOGRAM );

	SetBufferLabel ( 0, "MFI Up, Volume Up" );
	SetBufferLabel ( 1, "MFI Down, Volume Down" );
	SetBufferLabel ( 2, "MFI Up, Volume Down" );
	SetBufferLabel ( 3, "MFI Down, Volume Up" );
	*/
}


void MarketFacilitationIndex::Start()
{
	/*
	bool mfi_up = true, vol_up = true;

	int bars    = GetBars();
	int counted = GetCounted();
	
	if (counted > 0)
		counted--;
	
	Core& bd = *GetSource().Get<Core>();
	double point = bd.GetPoint();
	
	for (int i = counted; i < bars; i++ )
	{
		double volume = Volume(i);
		if ( IsEqualDoubles ( volume, 0.0 ) )
		{

			if ( i == 0 )
				buffer.Set(i, 0.0);
			else
				buffer.Set(i, buffer.Get(i-1));
		}

		else
			buffer.Set(i,
				( High(i) - Low(i) ) /
				( volume * point ));
	}



	if ( counted > 1 ) {

		int i = counted -1;

		if ( up_up_buffer.Get(i) != 0.0 )
		{
			mfi_up = true;
			vol_up = true;
		}

		if ( down_down_buffer.Get(i) != 0.0 )
		{
			mfi_up = false;
			vol_up = false;
		}

		if ( up_down_buffer.Get(i) != 0.0 )
		{
			mfi_up = true;
			vol_up = false;
		}

		if ( down_up_buffer.Get(i) != 0.0 )
		{
			mfi_up = false;
			vol_up = true;
		}
	}


	
	double volume_prev;
	if (counted > 0)
		volume_prev = Volume(counted-1);
	else
		volume_prev = Volume(counted);
	
	for (int i = counted; i < bars; i++)
	{
		double volume = Volume(i);
		if ( i < bars - 1 )
		{
			if ( buffer.Get(i) > buffer.Get(i-1) )
				mfi_up = true;

			if ( buffer.Get(i) < buffer.Get(i-1) )
				mfi_up = false;

			if ( volume > volume_prev )
				vol_up = true;

			if ( volume < volume_prev )
				vol_up = false;
		}

		if ( mfi_up && vol_up )
		{
			up_up_buffer.Set(i, buffer.Get(i));
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( !mfi_up && !vol_up )
		{
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, buffer.Get(i));
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( mfi_up && !vol_up )
		{
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, buffer.Get(i));
			down_up_buffer.Set(i, 0.0);
			volume_prev = volume;
			continue;
		}

		if ( !mfi_up && vol_up )
		{
			up_up_buffer.Set(i, 0.0);
			down_down_buffer.Set(i, 0.0);
			up_down_buffer.Set(i, 0.0);
			down_up_buffer.Set(i, buffer.Get(i));
			volume_prev = volume;
			continue;
		}
		
		volume_prev = volume;
	}
	*/
}






ZigZag::ZigZag()
{
	input_depth		= 12; // Depth
	input_deviation	= 5;  // Deviation
	input_backstep	= 3;  // Backstep
	extremum_level	= 3;  // recounting's depth of extremums
}

void ZigZag::Init()
{
	/*
	SetCoreChartWindow();
	
	//SetBufferCount(4, 2);
	SetBufferColor(0, Red());
	SetBufferColor(1, Red());
	
	if (input_backstep >= input_depth) {
		input_backstep = input_depth-1;
	}
	
	SetBufferStyle(0, DRAW_NONE);
	SetBufferStyle(1, DRAW_SECTION);
	
	//SetIndexBuffer( 0, osc );
	//SetIndexBuffer( 1, keypoint_buffer);
	//SetIndexBuffer( 2, high_buffer);
	//SetIndexBuffer( 3, low_buffer);
	*/
}



void ZigZag::Start()
{
	/*
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
	else
	{
		int i = bars-1;
		counter_z = 0;

		while ( counter_z < extremum_level && i >= bars-100 && i >= 0 )
		{
			if ( keypoint_buffer.Get(i) != 0.0 )
				counter_z++;
			i--;
		}
		
		if (i < 0)
			i = 0;
		
		if ( counter_z == 0 )
			counted = input_backstep;
		else
		{
			counted = i+1;

			if ( low_buffer.Get(i) != 0.0 )
			{
				curlow = low_buffer.Get(i);
				whatlookfor = 1;
			} else {
				curhigh = high_buffer.Get(i);
				whatlookfor = -1;
			}

			for (int i = counted; i < bars; i++ ) {
				keypoint_buffer.Set(i, 0.0);
				low_buffer.Set(i, 0.0);
				high_buffer.Set(i, 0.0);
			}
		}
	}

	Core& bd = *GetSource().Get<Core>();
	double point = bd.GetPoint();
	
	for (int i = counted; i < bars; i++ )
	{
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

	for (int i = counted; i < bars; i++ ) {
		double low = Low(i);
		double high = High(i);
		
		osc.Set(i, whatlookfor);
		
		switch ( whatlookfor ) {
			
		case 0: // look for peak or lawn

			if ( lastlow == 0.0 && lasthigh == 0.0 )
			{
				if ( high_buffer.Get(i) != 0.0 )
				{
					lasthighpos = i;
					lasthigh = high_buffer.Get(lasthighpos);
					whatlookfor = -1;
					keypoint_buffer.Set(lasthighpos, lasthigh);
				}

				if ( low_buffer.Get(i) != 0.0 )
				{
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
	*/
}












ZigZagOsc::ZigZagOsc()
{
	depth=12;
	deviation=5;
	backstep=3;
	
}

void ZigZagOsc::Init()
{
	/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, GrayColor());
	
	SetBufferStyle(0, DRAW_HISTOGRAM);
	
	//SetIndexBuffer ( 0, osc );
	
	if (RequireIndicator("zz", "depth", depth, "deviation", deviation, "backstep", backstep)) throw DataExc();
	*/
}

void ZigZagOsc::Start()
{
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	Core& ind = At(0);
	
	
	double prev, next;
	bool has_prev = false, has_next = false;;
	int prev_pos, next_pos;
	
	if (counted) {
		counted = min(counted, ind.GetBars()-1);
		for (; counted > 0; counted--) {
			double v = ind.GetIndexValue(1, counted);
			if (!v) continue;
			break;
		}
	}
	
	double diff, step;
	for (int i = counted; i < bars; i++)
	{
		if (has_prev) {
			if (has_next && next_pos == i) {
				prev_pos = i;
				prev = next;
				has_next = false;
			}
		} else {
			double v = ind.GetIndexValue(1, i);
			if (v == 0)
				continue;
			has_prev = true;
			prev = v;
			prev_pos = i;
		}
		
		if (!has_next) {
			for (int j = i+1; j < bars; j++) {
				double v = ind.GetIndexValue(1, j);
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
	*/
}




LinearTimeFrames::LinearTimeFrames() {
	
}

void LinearTimeFrames::Init() {
/*
	SetCoreSeparateWindow();
	//SetBufferCount(4, 4);
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	SetBufferColor(2, Blue);
	SetBufferColor(3, Yellow);
	SetBufferLineWidth(0, 3);
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(2, 1);
	SetCoreMinimum(0);
	SetCoreMaximum(1);
	
	//SetIndexBuffer(0, day);
	//SetIndexBuffer(1, month);
	//SetIndexBuffer(2, year);
	//SetIndexBuffer(3, week);
	*/
}

void LinearTimeFrames::Start() {
	/*
	int bars = GetBars();
	int counted =  GetCounted();
	for(int i = counted; i < bars; i++) {
		Time t = GetTime().GetTime(GetPeriod(), i);
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
	*/
}








RepeatingAverage::RepeatingAverage() {
	period = 5;
	smoothing_period = 5;
	applied_value = 5;
}

void RepeatingAverage::Init() {
	/*
	SetCoreChartWindow();
	//SetBufferCount(4, 4);
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	SetBufferColor(3, Green);
	SetBufferType(2, STYLE_DOT);
	SetBufferType(3, STYLE_DOT);
	
	//SetIndexBuffer(0, average1);
	//SetIndexBuffer(1, average2);
	//SetIndexBuffer(2, average3);
	//SetIndexBuffer(3, average4);
	*/
}

void RepeatingAverage::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	int seconds;
	int tf_period = GetMinutePeriod();
	switch (tf_period) {
		case 1:
		case 5:
		case 15:
		case 30:
			seconds = 60 * 60; break;
		case 60:
		case 240:
			seconds = 60 * 60 * 24; break;
		case 1440:
			seconds = 60 * 60 * 24 * 7; break;
		case 10080:
			seconds = 60 * 60 * 24 * 7 * 4; break;
		default:
			Panic("Unknown tf-period " + IntStr(tf_period));
	}
	
	//bars--;
	int prev_pos = counted ? counted - 1 : 0;
	for (int i = counted; i < bars; i++) {
		for (int part = 0; part < 2; part++) {
			Time t = GetTime().GetTime(GetPeriod(), i);
			
			if (part == 1) {
				t += tf_period * 60;
			}
			int64 mod1 = t.Get() % seconds;
			Time time_limit = t - seconds * period;
			double close = Open(i+1);
			double sum = 0;
			int count = 0;
			for(int j = i+1; j < bars && count < period; j++) {
				Time t2 = GetTime().GetTime(GetPeriod(), j);
				if (t2 < time_limit) break;
				int64 mod2 = t2.Get() % seconds;
				if (mod1 == mod2) {
					double value = GetAppliedValue(applied_value, j);
					sum += value;
					count++;
				}
			}
			// Fill missing values
			for(int j = count; j < period; j++) {
				sum += average1.Get(prev_pos);
				count++;
			}
			double average = sum / count;
			if (part == 0) {
				average1.Set(i, average);
				average2.Set(i, SimpleMA ( i, smoothing_period, average1 ));
			} else {
				average3.Set(i, average);
				average4.Set(i, SimpleMA ( i, smoothing_period, average3 ));
			}
			prev_pos = i;
		}
	}
	*/
}

	






RepeatingAverageOscillator::RepeatingAverageOscillator() {
	period = 5;
	smoothing_period = 5;
	applied_value = 5;
}

void RepeatingAverageOscillator::Init() {
/*
	SetCoreSeparateWindow();
	//SetBufferCount(5, 5);
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	SetBufferColor(2, Red);
	SetBufferColor(3, Green);
	SetBufferColor(4, Color(255, 69, 0));
	SetBufferType(0, STYLE_DOT);
	SetBufferType(1, STYLE_DOT);
	SetBufferLineWidth(4, 2);
	
	//SetIndexBuffer(0, diff1);
	//SetIndexBuffer(1, diff2);
	//SetIndexBuffer(2, maindiff);
	//SetIndexBuffer(3, avdiff);
	//SetIndexBuffer(4, main_av_diff);
	
	if (RequireIndicator("repav", "period", period, "smoothing", smoothing_period, "applied_value", applied_value)) throw DataExc();
	*/
}

void RepeatingAverageOscillator::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	Core& ind = At(0);
	
	
	for (int i = counted; i < bars; i++) {
		double d, d1, d2;
		d = ind.GetIndexValue(0, i) - ind.GetIndexValue(1, i);
		diff1.Set(i, d);
		d = ind.GetIndexValue(2, i) - ind.GetIndexValue(3, i);
		diff2.Set(i, d);
		d = ind.GetIndexValue(2, i) - ind.GetIndexValue(0, i);
		maindiff.Set(i, d);
		d1 = d;
		d = ind.GetIndexValue(3, i) - ind.GetIndexValue(1, i);
		avdiff.Set(i, d);
		d2 = d;
		main_av_diff.Set(i, d1 - d2);
	}
	*/
}

	




SupportResistance::SupportResistance() {
	period = 300;
	max_crosses = 100;
	max_radius = 100;
}

void SupportResistance::Init() {
/*
	SetCoreChartWindow();
	//SetBufferCount(2, 2);
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	
	//SetIndexBuffer(0, support);
	//SetIndexBuffer(1, resistance);
	*/
}

void SupportResistance::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	Core& bd = *GetSource().Get<Core>();
	double point = bd.GetPoint();
	
	ASSERT(point > 0);
	
	if (counted) counted--;
	//bars--;
	
	Vector<double> crosses;
	for (int i = counted; i < bars; i++) {
		
		int highest = HighestHigh(period, i);
		int lowest  = LowestLow(period, i);
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
		
		for(int j = i - period + 1; j <= i ; j++) {
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
		
		double close = Open(i+1);
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
	*/
}







SupportResistanceOscillator::SupportResistanceOscillator() {
	period = 300;
	max_crosses = 100;
	max_radius = 100;
	smoothing_period = 30;
}

void SupportResistanceOscillator::Init() {
/*
	SetCoreSeparateWindow();
	//SetBufferCount(2, 2);
	SetBufferColor(0, Red);
	SetBufferColor(1, Green);
	
	//SetIndexBuffer(0, osc);
	//SetIndexBuffer(1, osc_av);
	
	SetCoreMinimum(-1);
	SetCoreMaximum(1);
	
	if (RequireIndicator("supres", "period", period, "max_crosses", max_crosses, "max_radius", max_radius)) throw DataExc();
	*/
}

void SupportResistanceOscillator::Start() {
	/*
	int bars = GetBars();
	int counted = GetCounted();
	
	Core& ind = At(0);
	
	Vector<double> crosses;
	int prev_pos = counted ? counted-1 : 0;
	double prev_value = counted ? osc_av.Get(counted-1) : 0;
	for (int i = counted; i < bars; i++) {
		double applied_value = GetAppliedValue(5, i);
		double s = ind.GetIndexValue(0, i);
		double r = ind.GetIndexValue(1, i);
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
	*/
}





Psychological::Psychological() {
	period = 25;
}

void Psychological::Init() {
	/*
	SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	SetBufferColor(0, DodgerBlue());
	SetBufferStyle(0, DRAW_LINE);
	//SetIndexBuffer(0, buf);
	SetBufferBegin(0, period);
	SetBufferLabel(0, "Psychological");
	*/
}

void Psychological::Start() {
	/*
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
	
	for(int i=counted; i < bars; i++)
	{
		int count=0;
		for (int j=i-period+1; j <= i; j++)
		{
			if (Open(j+1) > Open(j))
			{
				count++;
			}
		}
		if (Open(i+1) > Open(i))
		{
			count++;
		}
		if (Open(i-period+1) > Open(i-period))
		{
			count--;
		}
		
		buf.Set(i, ((double)count / period) *100.0 - 50); // normalized
	}
	*/
}








CorrelationOscillator::CorrelationOscillator() {
	period = 10;
	sym_count = -1;
	//ids = 0;
}

void CorrelationOscillator::Init() {
	/*
	ids = GetResolver().FindLinkPath("/id");
	if (!ids) throw DataExc();
	int id = GetSymbol();
	if (id == -1) throw DataExc();
	
	sym_count = ids->keys.GetCount() - 1 - id;
	
	if (sym_count < 0) throw DataExc();
	
	SetCoreMaximum(+1.0);
	SetCoreMinimum(-1.0);
	SetCoreSeparateWindow();
	//SetBufferCount(sym_count, sym_count);
	
	buf.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++) {
		//SetIndexBuffer(i, buf[i]);
		SetBufferColor(i, RainbowColor((double)i / sym_count));
	}
	
	SetBufferBegin(0, period);
	
	Core* bd = GetSource().Get<Core>();
	if (!bd) throw DataExc();
	open = &bd->GetOpen();
	
	sources.SetCount(sym_count, 0);
	opens.SetCount(sym_count, 0);
	averages.SetCount(sym_count);
	
	String tf_str = "tf" + IntStr(GetPeriod());
	
	for(int i = 0; i < sym_count; i++) {
		int j = ids->keys.Find("id" + IntStr(id + 1 + i));
		if (j == -1) throw DataExc();
		PathLink& link = ids->keys[j];
		j = link.keys.Find(tf_str);
		if (j == -1) throw DataExc();
		Core* bd = link.keys[j].link.Get<Core>();
		if (!bd) throw DataExc();
		opens[i] = &bd->GetOpen();
		sources[i] = bd;
	}
	*/
}

void CorrelationOscillator::Process(int id) {
	/*
	int counted = GetCounted();
	int bars = GetBars();
	
	const Vector<double>& a = *open;
	const Vector<double>& b = *opens[id];
	
	Vector<double>& buf = this->buf[id];
	
	OnlineAverage& s = averages[id];
	
	if (b.GetCount() == 0) {
		LOG("CorrelationOscillator error: No data at " << sources[id]->GetPath());
		return;
	}
	
	if (counted < period) {
		ASSERT(counted == 0);
		s.mean_a = a.Get(0);
		s.mean_b = b.Get(0);
		s.count = 1;
		for(int i = 1; i < period; i++)
			s.Add(a.Get(i), b.Get(i));
		counted = period;
	}
	
	Vector<double> cache_a, cache_b;
	cache_a.SetCount(period);
	cache_b.SetCount(period);
	
	int cache_offset = 0;
	for(int i = 1; i < period; i++) {
		cache_a[i] = a.Get(counted - period + i);
		cache_b[i] = b.Get(counted - period + i);
	}
	
	for(int i = counted; i < bars; i++) {
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
	*/
}

void CorrelationOscillator::Start() {
	/*
	int counted = GetCounted();
	int bars = GetBars();
	
	if (counted == bars)
		throw DataExc();
	
	if (counted > 0) counted = max(counted - period, 0);
	
	for(int i = 0; i < sym_count; i++) {
		sources[i]->Refresh();
	}

	for(int i = 0; i < sym_count; i++) {
		Process(i);
	}
	*/
}





















ParallelSymLR::ParallelSymLR()
{
	period = 13;
	method = 0;
	SetCoreSeparateWindow();
}

void ParallelSymLR::Init()
{
	/*
	int draw_begin;
	if (period < 2)
		period = 13;
	draw_begin = period - 1;
	//SetBufferCount(1, 1);
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	
	//SetIndexBuffer(0, buffer);
	
	if (RequireIndicator("ma", "period", period, "method", method)) throw DataExc();
	
	SetCoreLevelCount(1);
	SetCoreLevel(0, 0);
	SetCoreLevelsColor(GrayColor(192));
	SetCoreLevelsStyle(STYLE_DOT);
	*/
}

void ParallelSymLR::Start()
{
	/*
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
	counted = max(0, counted - shift);
	bars -= shift;
	
	// Calculate averages
	Core& cont = At(0);
	cont.Refresh();
	
	// Prepare values for loop
	Vector<double>& dbl = cont.GetIndex(0);
	double prev1 = dbl.Get(max(0, counted-1));
	double prev2 = dbl.Get(max(0, counted-1+shift));
	double prev_value = counted > 0 ? buffer.Get(counted-1) : 0;
	double prev_diff = counted > 1 ? prev_value - buffer.Get(counted-2) : 0;
	
	// Loop unprocessed range of time
	for(int i = counted; i < bars; i++) {
		
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
	*/
}


















ParallelSymLREdge::ParallelSymLREdge()
{
	period = 13;
	method = 0;
	slowing = 54;
	SetCoreSeparateWindow();
}

void ParallelSymLREdge::Init()
{
	/*
	int draw_begin;
	if (period < 2)
		period = 13;
	draw_begin = period - 1;
	//SetBufferCount(3, 1);
	SetBufferColor(0, Red());
	SetBufferBegin(0, draw_begin );
	
	//SetIndexBuffer(0, buffer);
	//SetIndexBuffer(1, edge);
	//SetIndexBuffer(2, symlr);
	
	if (RequireIndicator("ma", "period", period, "method", method)) throw DataExc();
	if (RequireIndicator("ma", "period", slowing, "method", method, "offset", IntStr(-slowing/2))) throw DataExc();
	*/
}

void ParallelSymLREdge::Start()
{
	/*
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
	counted = max(0, counted - shift);
	bars -= shift;
	
	// Refresh source data
	Core& cont = At(0);
	Core& slowcont = At(1);
	cont.Refresh();
	slowcont.Refresh();
	
	// Prepare values for looping
	Vector<double>& dbl = cont.GetIndex(0);
	Vector<double>& slow_dbl = slowcont.GetIndex(0);
	double prev_slow = slow_dbl.Get(max(0, counted-1));
	double prev1 = dbl.Get(max(0, counted-1)) - prev_slow;
	double prev2 = dbl.Get(max(0, counted-1+shift)) - prev_slow;
	
	// Calculate 'ParallelSymLR' data locally. See info for that.
	for(int i = counted; i < bars; i++) {
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
	for(int i = max(2, counted); i < bars - 1; i++) {
		
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
		double edge =
			 1.0 * symlr.Get(i-2) +
			 2.0 * symlr.Get(i-1) +
			-2.0 * symlr.Get(i+1) +
			-1.0 * symlr.Get(i+2);
		
		// Store value
		this->edge.Set(i, edge);
		double d = max(0.0, edge * mul);
		ASSERT(IsFin(d));
		this->buffer.Set(i, d);
	}
	*/
}



}
