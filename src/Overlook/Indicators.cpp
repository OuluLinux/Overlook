#include "Overlook.h"

namespace Overlook {

void InitFactory(ConstFactoryDeclaration& id, SourceImage& si, ChartImage& ci, GraphImage& gi) {
	
	#define FITEM(x) if (id.factory == FACTORY_##x) {x().Init(si, ci, gi); return;}
	
	FACTORY_LIST
	
}

void StartFactory(ConstFactoryDeclaration& id, SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ValueRegister reg(id);
	ASSERT(reg.arg_cursor == 0);
	ASSERT(reg.is_loading);
	
	#define FITEM(x) if (id.factory == FACTORY_##x) {\
		x obj; obj.Conf(reg); obj.Start(si, ci, gi); return;\
	}
	
	FACTORY_LIST
	
}

void ConfFactory(ConstFactoryDeclaration& id, ValueRegister& reg) {
	
	#define FITEM(x) if (id.factory == FACTORY_##x) {x().Conf(reg); return;}
	
	FACTORY_LIST
	
}








double SimpleMA ( const int position, const int period, ConstBufferImage& value ) {
	double result = 0.0;
	if ( position >= period && period > 0 ) {
		for ( int i = 0; i < period; i++)
			result += value.Get( position - i );
		result /= period;
	}
	return ( result );
}

double ExponentialMA ( const int position, const int period, const double prev_value, ConstBufferImage& value ) {
	double result = 0.0;
	if ( period > 0 ) {
		double pr = 2.0 / ( period + 1.0 );
		result = value.Get( position ) * pr + prev_value * ( 1 - pr );
	}
	return ( result );
}

double SmoothedMA ( const int position, const int period, const double prev_value, ConstBufferImage& value ) {
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

double LinearWeightedMA ( const int position, const int period, ConstBufferImage& value ) {
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
		const int period, ConstBufferImage& value, BufferImage& buffer ) {
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
		const int period, ConstBufferImage& value, BufferImage& buffer ) {
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
	const int period, ConstBufferImage& value, BufferImage& buffer, int &weightsum ) {
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
		const int period, ConstBufferImage& value, BufferImage& buffer )
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














DataSource::DataSource() {
	
}

void DataSource::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	Panic("TODO");
}
void DataSource::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	Panic("TODO");
}












ValueChange::ValueChange() {
	
}

void ValueChange::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Color(128,0,0));
	gi.SetBufferLineWidth(0, 2);
	gi.SetBufferColor(1, Color(0,128,0));
	gi.SetBufferLineWidth(1, 2);
	gi.SetBufferColor(2, Color(0,0,128));
	gi.SetBufferLineWidth(2, 2);
}

void ValueChange::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int tf = ci.GetTf();
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	double point = ci.GetPoint();
	
	ConstBufferImage& open				= ci.GetInputBuffer(0, 0);
	ConstBufferImage& low				= ci.GetInputBuffer(0, 1);
	ConstBufferImage& high				= ci.GetInputBuffer(0, 2);
	BufferImage& low_change				= gi.GetBuffer(0);
	BufferImage& high_change			= gi.GetBuffer(1);
	BufferImage& value_change			= gi.GetBuffer(2);
	
	if (!begin) begin = 1;
	
	for(int i = begin; i < end; i++) {
		double open_value = open.Get(i-1);
		double change_value = open.Get(i) / open_value - 1.0;
		double low_change_ = low.Get(i-1) / open_value - 1.0;
		double high_change_ = high.Get(i-1) / open_value - 1.0;
		change_av.Add(fabs(change_value));
		value_change.Set(i, change_value);
		low_change.Set(i, low_change_);
		high_change.Set(i, high_change_);
	}
	
	for(int i = begin; i < end; i++) {
		value_change	.Set(i, value_change	.Get(i) / (3 * change_av.mean));
		low_change		.Set(i, low_change		.Get(i) / (3 * change_av.mean));
		high_change		.Set(i, high_change		.Get(i) / (3 * change_av.mean));
	}
}













MovingAverage::MovingAverage()
{
	ma_period = 13;
	ma_shift  = -6;
	ma_method  = 0;
}

void MovingAverage::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int draw_begin;
	if (ma_period < 2)
		ma_period = 13;
	draw_begin = ma_period - 1;
	
	gi.SetBufferColor(0, Red());
	gi.SetBufferShift(0, ma_shift);
	gi.SetBufferBegin(0, draw_begin );
}
/*
void MovingAverage::Assist(int cursor, VectorBool& vec) {
	double open = si.Open(cursor);
	double ma = gi.GetBuffer(0).Get(cursor);
	if (open > ma)	vec.Set(MA_OVERAV, true);
	else			vec.Set(MA_BELOWAV, true);
	if (cursor > 0) {
		double prev = gi.GetBuffer(0).Get(cursor - 1);
		if (ma > prev)	vec.Set(MA_TRENDUP, true);
		else			vec.Set(MA_TRENDDOWN, true);
	}
}
*/
void MovingAverage::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	
	switch (ma_method) {
		case MODE_SIMPLE:
			Simple(si, ci, gi);
			break;
		case MODE_EXPONENTIAL:
			Exponential(si, ci, gi);
			break;
		case MODE_SMOOTHED:
			Smoothed(si, ci, gi);
			break;
		case MODE_LINWEIGHT:
			LinearlyWeighted(si, ci, gi);
	}
	
	BufferImage& buffer = gi.GetBuffer(0);
	VectorBool& label = gi.GetSignal();
	label.SetCount(end);
	
	double prev = begin > 0 ? buffer.Get(begin-1) : 0.0;
	for(int i = begin; i < end; i++) {
		double cur = buffer.Get(i);
		bool label_value = cur < prev;
		label.Set(i, label_value);
		prev = cur;
	}
}

void MovingAverage::Simple(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	double sum = 0;
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	int pos = begin;
	if (pos < ma_period) pos = ma_period;
	
	for (int i = 1; i < ma_period; i++)
		sum += si.Open(pos - i);
	while (pos < end) {
		sum += si.Open( pos );
		buffer.Set(pos, sum / ma_period);
		sum -= si.Open( pos - ma_period + 1 );
		pos++;
	}
	if (begin < 1)
		for (int i = 0; i < ma_period; i++)
			buffer.Set(i, 0);
}

void MovingAverage::Exponential(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	double pr = 2.0 / ( ma_period + 1 );
	int pos = 1;
	if ( begin > 2 )
		pos = begin + 1;
	while (pos < end) {
		if (pos == 1)
			buffer.Set(pos-1, si.Open(pos - 1));
		buffer.Set(pos, si.Open(pos) * pr + buffer.Get(pos-1) * ( 1 - pr ));
		pos++;
	}
}

void MovingAverage::Smoothed(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	double sum = 0;
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	int pos = ma_period;
	if (pos < begin)
		pos = begin;
	while (pos < end) {
		if (pos == ma_period) {
			for (int i = 0, k = pos; i < ma_period; i++, k--) {
				sum += si.Open(k);
				buffer.Set(k, 0);
			}
		}
		else
			sum = buffer.Get(pos-1) * ( ma_period - 1 ) + si.Open(pos);
		buffer.Set(pos, sum / ma_period);
		pos++;
	}
}

void MovingAverage::LinearlyWeighted(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	double sum = 0.0, lsum = 0.0;
	double value;
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	int weight = 0, pos = begin + 1;
	if (pos > end - ma_period)
		pos = end - ma_period;
	for (int i = 1; i <= ma_period; i++, pos++) {
		value = si.Open( pos );
		sum += value * i;
		lsum += value;
		weight += i;
	}
	pos--;
	int i = pos - ma_period;
	while (pos < end) {
		buffer.Set(pos, sum / weight);
		pos++;
		i++;
		if ( pos == end )
			break;
		value = si.Open(pos);
		sum = sum - lsum + value * ma_period;
		lsum -= si.Open(i);
		lsum += value;
	}
	if ( begin < 1 )
		for (i = 0; i < ma_period; i++)
			buffer.Set(i, 0);
}













MovingAverageConvergenceDivergence::MovingAverageConvergenceDivergence() {
	fast_ema_period = 12;
	slow_ema_period = 26;
	signal_sma_period = 9;
}

void MovingAverageConvergenceDivergence::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Silver);
	gi.SetBufferColor(1, Red);
	gi.SetBufferLineWidth(0, 2);
	gi.SetBufferBegin ( 1, signal_sma_period );
	gi.SetBufferStyle(0, DRAW_HISTOGRAM);
	gi.SetBufferStyle(1, DRAW_LINE);
	//gi.SetBufferLabel(0,"MACD");
	//gi.SetBufferLabel(1,"Signal");
}

void MovingAverageConvergenceDivergence::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& signal_buffer = gi.GetBuffer(1);
	
	int end = ci.GetEnd();
	if ( end <= signal_sma_period )
		throw DataExc();

	int begin = ci.GetBegin();
	if ( begin > 0 )
		begin--;

	ConstBufferImage& a_buf = ci.GetInputBuffer(1, 0);
	ConstBufferImage& b_buf = ci.GetInputBuffer(2, 0);
	
	for (int i = begin; i < end; i++) {
		double a_value = a_buf.Get(i);
		double b_value = b_buf.Get(i);
		double diff = a_value - b_value;
		buffer.Set(i, diff);
	}

	SimpleMAOnBuffer( end, ci.GetBegin(), 0, signal_sma_period, buffer, signal_buffer );
}
/*
void MovingAverageConvergenceDivergence::Assist(int cursor, VectorBool& vec) {
	double open = si.Open(cursor);
	double macd = gi.GetBuffer(0).Get(cursor);
	if (macd > 0.0)		vec.Set(MACD_OVERZERO, true);
	else				vec.Set(MACD_BELOWZERO, true);
	if (cursor > 0) {
		double prev = gi.GetBuffer(0).Get(cursor - 1);
		if (macd > prev)	vec.Set(MACD_TRENDUP, true);
		else				vec.Set(MACD_TRENDDOWN, true);
	}
}
*/





AverageDirectionalMovement::AverageDirectionalMovement() {
	period_adx = 14;
}

void AverageDirectionalMovement::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, LightSeaGreen);
	gi.SetBufferColor(1,  YellowGreen);
	gi.SetBufferColor(2,  Wheat);
	
	//gi.SetBufferLabel(0,"ADX");
	//gi.SetBufferLabel(1,"+DI");
	//gi.SetBufferLabel(2,"-DI");
	
	if ( period_adx >= 128 || period_adx <= 1 )
		throw DataExc();
	
	gi.SetBufferBegin ( 0, period_adx );
	gi.SetBufferBegin ( 1, period_adx );
	gi.SetBufferBegin ( 2, period_adx );
}

void AverageDirectionalMovement::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& adx_buffer = gi.GetBuffer(0);
	BufferImage& pdi_buffer = gi.GetBuffer(1);
	BufferImage& ndi_buffer = gi.GetBuffer(2);
	BufferImage& pd_buffer  = gi.GetBuffer(3);
	BufferImage& nd_buffer  = gi.GetBuffer(4);
	BufferImage& tmp_buffer = gi.GetBuffer(5);
	
	int end = ci.GetEnd();
	if ( end < period_adx )
		throw DataExc();

	int begin = ci.GetBegin();

	if ( begin > 0 )
		begin--;
	else {
		begin = 2;
		for(int i = 0; i < 2; i++) {
			pdi_buffer.Set(i, 0.0);
			ndi_buffer.Set(i, 0.0);
			adx_buffer.Set(i, 0.0);
		}
	}
	
	for ( int i = begin; i < end; i++) {
		double Hi		= si.High(i - 1);
		double prev_hi	= si.High(i - 2);
		double Lo		= si.Low(i - 1);
		double prev_lo	= si.Low(i - 2);
		double prev_cl	= si.Open(i);

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

void BollingerBands::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	
	bands_deviation = deviation * 0.1;
	
	gi.SetBufferColor(0, LightSeaGreen);
	gi.SetBufferColor(1,  LightSeaGreen);
	gi.SetBufferColor(2,  LightSeaGreen);
	
	gi.SetBufferStyle(0,DRAW_LINE);
	gi.SetBufferShift(0,bands_shift);
	//gi.SetBufferLabel(0,"Bands SMA");
	
	gi.SetBufferStyle(1,DRAW_LINE);
	gi.SetBufferShift(1,bands_shift);
	//gi.SetBufferLabel(1,"Bands Upper");
	
	gi.SetBufferStyle(2,DRAW_LINE);
	gi.SetBufferShift(2,bands_shift);
	//gi.SetBufferLabel(2,"Bands Lower");
	
	if ( bands_period < 2 )
		throw DataExc();
	
	if ( bands_deviation == 0.0 )
		throw DataExc();
	
	plot_begin = bands_period - 1;
	gi.SetBufferBegin ( 0, bands_period );
	gi.SetBufferBegin ( 1, bands_period );
	gi.SetBufferBegin ( 2, bands_period );
	
	gi.SetBufferShift ( 0, bands_shift );
	gi.SetBufferShift ( 1, bands_shift );
	gi.SetBufferShift ( 2, bands_shift );
}

void BollingerBands::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& ml_buffer = gi.GetBuffer(0);
	BufferImage& tl_buffer = gi.GetBuffer(1);
	BufferImage& bl_buffer = gi.GetBuffer(2);
	BufferImage& stddev_buffer = gi.GetBuffer(3);
	
	int end = ci.GetEnd();
	int pos;
	int begin = ci.GetBegin();
	
	if ( end < plot_begin )
		throw DataExc();
	
	if ( begin > 1 )
		pos = begin - 1;
	else
		pos = 0;
	
	ConstBufferImage& open = ci.GetInputBuffer(0, 0);
	
	for ( int i = pos; i < end; i++) {
		ml_buffer.Set(i, SimpleMA( i, bands_period, open ));
		
		double tmp = 0.0;
	
		if ( i >= bands_period ) {
			int end = ci.GetEnd();
			for (int j = 0; j < bands_period; j++) {
				double value =  si.Open(i - j );
				tmp += pow ( value - ml_buffer.Get( i ), 2 );
			}
		
			tmp = sqrt( tmp / bands_period );
		}
		stddev_buffer.Set(i, tmp);
		
		tl_buffer.Set(i, ml_buffer.Get(i) + bands_deviation * stddev_buffer.Get(i));
		bl_buffer.Set(i, ml_buffer.Get(i) - bands_deviation * stddev_buffer.Get(i));
	}
}
/*
void BollingerBands::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double high = si.High(cursor - 1);
		double low  = si.Low(cursor - 1);
		double top = gi.GetBuffer(1).Get(cursor);
		double bot = gi.GetBuffer(2).Get(cursor);
		if (high >= top)		vec.Set(BB_HIGHBAND, true);
		if (low  <= bot)		vec.Set(BB_LOWBAND, true);
	}
}
*/





Envelopes::Envelopes() {
	deviation = 0.1;
	dev = 10;
}

void Envelopes::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	
	deviation = dev * 0.1;
	
	gi.SetBufferColor(0, Blue);
	gi.SetBufferColor(1, Red);
	
	gi.SetBufferStyle(0, DRAW_LINE);
	gi.SetBufferStyle(1, DRAW_LINE);
}


void Envelopes::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& up_buffer = gi.GetBuffer(0);
	BufferImage& down_buffer = gi.GetBuffer(1);
	BufferImage& ma_buffer = gi.GetBuffer(2);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin) begin--;
	else begin = 0;
	
	for (int i = begin; i < end; i++) {
		double value = si.Open(i);
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

void ParabolicSAR::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	
	sar_step = step * 0.001;
	sar_maximum = maximum * 0.01;
	
	gi.SetBufferColor(0, Lime);
	
	if (sar_step <= 0.0)
		throw DataExc();
	
	if (sar_maximum <= 0.0)
		throw DataExc();
	
	last_rev_pos = 0;
	direction_long = false;
	
	gi.SetBufferStyle(0, DRAW_ARROW);
	gi.SetBufferArrow(0, 159);
}

void ParabolicSAR::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& sar_buffer = gi.GetBuffer(0);
	BufferImage& ep_buffer  = gi.GetBuffer(1);
	BufferImage& af_buffer  = gi.GetBuffer(2);
	
	int begin = ci.GetBegin();
	int end = ci.GetEnd();

	if ( end < 3 )
		throw DataExc();
	
	int pos = begin - 1;

	if ( pos < 1 ) {
		pos = 1;
		af_buffer.Set(0, sar_step);
		af_buffer.Set(1, sar_step);
		sar_buffer.Set(0, si.High( 0 ));
		last_rev_pos = 0;
		direction_long = false;
		sar_buffer.Set(1, GetHigh( pos-1, last_rev_pos, si, ci, gi ));
		double low = si.Low( 0 );
		ep_buffer.Set(0, low);
		ep_buffer.Set(1, low);
	}
	
	if (pos == 0) pos++;
	
	int prev_pos = pos - 1;
	if (prev_pos < 0) prev_pos = 0;
	
	double prev_high = si.High( prev_pos );
	double prev_low  = si.Low( prev_pos );
	
	//end--;
	
	for (int i = pos; i < end; i++) {
		double low  = si.Low(i-1);
		double high = si.High(i-1);

		if ( direction_long ) {
			if ( sar_buffer.Get(i-1) > low ) {
				direction_long = false;
				sar_buffer.Set(i-1, GetHigh( i-1, last_rev_pos, si, ci, gi ));
				ep_buffer.Set(i, low);
				last_rev_pos = i;
				af_buffer.Set(i, sar_step);
			}
		}
		else {
			if ( sar_buffer.Get(i-1) < high ) {
				direction_long = true;
				sar_buffer.Set(i-1, GetLow( i-1, last_rev_pos, si, ci, gi ));
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

double ParabolicSAR::GetHigh( int pos, int start_pos, SourceImage& si, ChartImage& ci, GraphImage& gi )
{
	ASSERT(pos >= start_pos);
	int end = ci.GetEnd();
	
	double result = si.High( start_pos );

	for ( int i = start_pos+1; i <= pos; i++)
	{
		double high = si.High(i);

		if ( result < high )
			result = high;
	}

	return ( result );
}

double ParabolicSAR::GetLow( int pos, int start_pos, SourceImage& si, ChartImage& ci, GraphImage& gi )
{
	ASSERT(pos >= start_pos);
	int end = ci.GetEnd();
	
	double result = si.Low( start_pos );

	for ( int i = start_pos+1; i <= pos; i++)
	{
		double low = si.Low(i);

		if ( result > low )
			result = low;
	}

	return ( result );
}
/*
void ParabolicSAR::Assist(int cursor, VectorBool& vec) {
	double open = si.Open(cursor);
	double sar = gi.GetBuffer(0).Get(cursor);
	if (open > sar)		vec.Set(PSAR_TRENDUP, true);
	else				vec.Set(PSAR_TRENDDOWN, true);
}
*/








StandardDeviation::StandardDeviation() {
	period = 20;
	ma_method = 0;
}

void StandardDeviation::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(0);
	
	gi.SetBufferColor(0, Blue);
	gi.SetBufferBegin ( 0, period );
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "StdDev");
}
/*
void StandardDeviation::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > std1)	vec.Set(STDDEV_INC, true);
		else				vec.Set(STDDEV_DEC, true);
	}
}
*/
void StandardDeviation::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& stddev_buffer = gi.GetBuffer(0);
	double value, amount, ma;

	int end = ci.GetEnd();
	if ( end <= period )
		throw DataExc();

	int begin = ci.GetBegin();

	if ( begin > 0 )
		begin--;
	else
		begin = period;
	
	ConstBufferImage& ma_buf = ci.GetInputBuffer(1, 0);
	
	for (int i = begin; i < end; i++) {
		amount = 0.0;
		ma = ma_buf.Get(i);

		for ( int j = 0; j < period; j++ ) {
			value = si.Open( i - j );
			amount += ( value - ma ) * ( value - ma );
		}

		stddev_buffer.Set(i, sqrt ( amount / period ));
	}
}










AverageTrueRange::AverageTrueRange() {
	period = 14;
}

void AverageTrueRange::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "ATR");
	
	if ( period <= 0 )
		throw DataExc();
	
	gi.SetBufferBegin ( 0, period );
}
/*
void AverageTrueRange::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > std1)	vec.Set(ATR_INC, true);
		else				vec.Set(ATR_DEC, true);
	}
}
*/
void AverageTrueRange::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& atr_buffer = gi.GetBuffer(0);
	BufferImage& tr_buffer = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if ( end <= period || period <= 0 )
		throw DataExc();
	
	if ( begin == 0 ) {
		tr_buffer.Set(0, 0.0);
		atr_buffer.Set(0, 0.0);
		
		for (int i = 1; i < end; i++) {
			double h = si.High(i-1);
			double l = si.Low(i-1);
			double c = si.Open(i);

			tr_buffer.Set(i, Upp::max( h, c ) - Upp::min( l, c ));
		}

		double first_value = 0.0;

		for (int i = 1; i <= period; i++) {
			atr_buffer.Set(i, 0.0);
			first_value += tr_buffer.Get(i);
		}

		first_value /= period;

		atr_buffer.Set(period, first_value);

		begin = period + 1;
	}
	else
		begin--;
	
	for (int i = begin; i < end; i++) {
		double h = si.High(i-1);
		double l = si.Low(i-1);
		double c = si.Open(i);

		tr_buffer.Set(i, Upp::max( h, c ) - Upp::min( l, c ));
		atr_buffer.Set(i, atr_buffer.Get( i - 1 ) + ( tr_buffer.Get(i) - tr_buffer.Get( i - period ) ) / period);
	}
}













BearsPower::BearsPower() {
	period = 13;
}

void BearsPower::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Silver);
	gi.SetBufferStyle(0,DRAW_HISTOGRAM);
	//gi.SetBufferLabel(0,"ATR");
}
/*
void BearsPower::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(BEAR_OVERZERO, true);
		else				vec.Set(BEAR_BELOWZERO, true);
		if (std0 > std1)	vec.Set(BEAR_INC, true);
		else				vec.Set(BEAR_DEC, true);
	}
}*/

void BearsPower::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if ( end <= period )
		throw DataExc();

	if ( begin > 0 )
		begin--;
	else
		begin++;
	
	ConstBufferImage& ma_buf = ci.GetInputBuffer(1, 0);
	
	for (int i = begin; i < end; i++) {
		buffer.Set(i, si.Low(i-1) - ma_buf.Get(i));
	}
}









BullsPower::BullsPower() {
	period = 13;
}

void BullsPower::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Silver);
	
	gi.SetBufferStyle(0,DRAW_HISTOGRAM);
	//gi.SetBufferLabel(0,"Bulls");
}
/*
void BullsPower::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(BULL_OVERZERO, true);
		else				vec.Set(BULL_BELOWZERO, true);
		if (std0 > std1)	vec.Set(BULL_INC, true);
		else				vec.Set(BULL_DEC, true);
	}
}
*/
void BullsPower::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();

	if ( begin > 0 )
		begin--;
	else
		begin++;
	
	ConstBufferImage& ma_buf = ci.GetInputBuffer(1, 0);
	
	for ( int i = begin; i < end; i++) {
		buffer.Set(i, si.High(i-1) - ma_buf.Get(i) );
	}
}













CommodityChannelIndex::CommodityChannelIndex() {
	period = 14;
}

void CommodityChannelIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, LightSeaGreen);
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, -100.0);
	gi.SetCoreLevel(1,  100.0);
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0,"CCI");
	
	if ( period <= 1 )
		throw DataExc();
	
	gi.SetBufferBegin ( 0, period );
}
/*
void CommodityChannelIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(CCI_OVERZERO, true);
		else				vec.Set(CCI_BELOWZERO, true);
		if (std0 > +100.0)	vec.Set(CCI_OVERHIGH, true);
		if (std0 < -100.0)	vec.Set(CCI_BELOWLOW, true);
		if (std0 > std1)	vec.Set(CCI_INC, true);
		else				vec.Set(CCI_DEC, true);
	}
}
*/
void CommodityChannelIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& cci_buffer = gi.GetBuffer(0);
	BufferImage& value_buffer = gi.GetBuffer(1);
	BufferImage& mov_buffer = gi.GetBuffer(2);
	
	int i, k, pos;
	double sum, mul;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if ( end <= period || period <= 1 )
		throw DataExc();
	
	if ( begin < 1 ) {
		for ( i = 1; i < period; i++) {
			double h = si.High(i-1);
			double l = si.Low(i-1);
			double c = si.Open(i);

			cci_buffer.Set(i, 0.0);
			value_buffer.Set(i, ( h + l + c ) / 3);
			mov_buffer.Set(i, 0.0);
		}
	}

	pos = begin - 1;

	if ( pos < period )
		pos = period;
	
	if (pos == 0)
		pos++;
	
	//end--;
	for ( i = pos; i < end; i++) {
		double h = si.High(i-1);
		double l = si.Low(i-1);
		double c = si.Open(i);
		value_buffer.Set(i, ( h + l + c ) / 3);
		mov_buffer.Set(i, SimpleMA ( i, period, value_buffer ));
	}

	mul = 0.015 / period;
	pos = period - 1;

	if ( pos < begin - 1 )
		pos = begin - 2;

	i = pos;

	while ( i < end ) {
		sum = 0.0;
		k = i + 1 - period;
		
		while ( k <= i ) {
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

void DeMarker::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-0.5);  // normalized
	gi.SetCoreMaximum(0.5);   // normalized
	
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, -0.2); // normalized
	gi.SetCoreLevel(1, +0.2); // normalized
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0,"DeMarker");
	
	gi.SetBufferBegin ( 0, period );
}
/*
void DeMarker::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double std0 = gi.GetBuffer(0).Get(cursor);
		double std1 = gi.GetBuffer(0).Get(cursor-1);
		if (std0 > 0.0)		vec.Set(DEM_OVERZERO, true);
		else				vec.Set(DEM_BELOWZERO, true);
		if (std0 > +0.2)	vec.Set(DEM_OVERHIGH, true);
		if (std0 < -0.2)	vec.Set(DEM_BELOWLOW, true);
		if (std0 > std1)	vec.Set(DEM_INC, true);
		else				vec.Set(DEM_DEC, true);
	}
}
*/
void DeMarker::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& max_buffer = gi.GetBuffer(1);
	BufferImage& min_buffer = gi.GetBuffer(2);
	double num;
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();
	
	max_buffer.Set(0, 0.0);
	min_buffer.Set(0, 0.0);

	if ( begin > 2 )
		begin--;
	else
		begin = 2;
	
	double prev_high = begin >= 2 ? si.High( begin - 1 ) : 0;
	double prev_low  = begin >= 2 ? si.Low( begin - 1 ) : 0;

	for (int i = begin; i < end; i++) {
		double high = si.High(i-1);
		double low  = si.Low(i-1);

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

	if ( begin == 0 )
		for (int i = 0; i < period; i++)
			buffer.Set(end - i, 0.0);

	
	for (int i = begin; i < end; i++)
	{
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

void ForceIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, DodgerBlue);
	
	gi.SetBufferStyle(0,DRAW_LINE);
	gi.SetBufferBegin(0,period);
	
	gi.SetBufferBegin ( 0, period );
}
/*
void ForceIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double force0 = gi.GetBuffer(0).Get(cursor);
		double force1 = gi.GetBuffer(0).Get(cursor-1);
		if (force0 > 0.0)		vec.Set(FORCE_OVERZERO, true);
		else					vec.Set(FORCE_BELOWZERO, true);
		if (force0 > force1)	vec.Set(FORCE_INC, true);
		else					vec.Set(FORCE_DEC, true);
	}
}
*/
void ForceIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (end <= period)
		throw DataExc();
	
	if (begin > 0)
		begin--;
	
	if (begin == 0)
		begin++;
	
	ConstBufferImage& ma_buf = ci.GetInputBuffer(1, 0);
	
	for ( int i = begin; i < end; i++) {
		double volume = si.Volume(i-1);
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

void Momentum::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferColor(1, Color(28, 255, 200));
	
	if ( period <= 0 )
		throw DataExc();

	gi.SetBufferBegin(0, period);
	gi.SetBufferShift(0, shift);
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0,"Momentum");
}
/*
void Momentum::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(MOM_OVERZERO, true);
		else					vec.Set(MOM_BELOWZERO, true);
		if (value0 > value1)	vec.Set(MOM_INC, true);
		else					vec.Set(MOM_DEC, true);
	}
}
*/
void Momentum::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& change_buf = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if ( end <= period || period <= 0 )
		throw DataExc();

	if ( begin <= 0 ) {
		for (int i = 0; i < period; i++) {
			buffer.Set(i, 0.0);
		}
		begin = period;
	}
	else
		begin--;
	
	for (int i = begin; i < end; i++) {
		double close1 = si.Open( i );
		double close2 = si.Open( i - period );
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

void OsMA::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Silver);
	gi.SetBufferLineWidth(0, 2);
	gi.SetBufferColor(1, Red);
	gi.SetBufferLineWidth(1, 1);
	
	gi.SetBufferStyle(0,DRAW_HISTOGRAM);
	gi.SetBufferStyle(1,DRAW_LINE);
	
	gi.SetBufferBegin ( 0, signal_sma_period );
}



	
void OsMA::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osma_buffer = gi.GetBuffer(0);
	BufferImage& diff_buffer = gi.GetBuffer(1);
	BufferImage& buffer = gi.GetBuffer(2);
	BufferImage& signal_buffer = gi.GetBuffer(3);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= signal_sma_period )
		return;

	if ( begin > 0 )
		begin--;
	
	ConstBufferImage& ma1_buf = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ma2_buf = ci.GetInputBuffer(2, 0);
	ASSERT(ma1_buf.GetCount() >= end);
	ASSERT(ma2_buf.GetCount() >= end);
	
	for (int i = begin; i < end; i++) {
		double ma1 = ma1_buf.Get(i);
		double ma2 = ma2_buf.Get(i);
		double d = ma1 - ma2;
		buffer.Set(i, d);
	}
	
	SimpleMAOnBuffer( end, begin, 0, signal_sma_period, buffer, signal_buffer );
	
	for (int i = begin; i < end; i++) {
		double d = buffer.Get(i) - signal_buffer.Get(i);
		if (d != 0.0)
			AddValue(fabs(d));
		osma_buffer.Set(i, d);
	}
	
	double value_mul = 1.0 / (value_mean * 3.0);
	for (int i = begin; i < end; i++) {
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
	for (int i = begin; i < end; i++) {
		double d = diff_buffer.Get(i) * diff_mul;
		diff_buffer.Set(i, d);
	}
	
	for (int i = begin; i < end; i++) {
		osma_buffer.Set(i, Upp::max(-1.0, Upp::min(+1.0, osma_buffer.Get(i))));
		diff_buffer.Set(i, Upp::max(-1.0, Upp::min(+1.0, diff_buffer.Get(i))));
	}
}














RelativeStrengthIndex::RelativeStrengthIndex()
{
	period = 32;
}

void RelativeStrengthIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-1); // normalized
	gi.SetCoreMaximum(+1);  // normalized
	
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferColor(1, White());
	gi.SetBufferColor(2, White());
	gi.SetBufferColor(3, Color(28, 255, 200));
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, +0.4); // normalized
	gi.SetCoreLevel(1, -0.4); // normalized
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
	
	if ( period < 1 )
		throw DataExc();
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "RSI");
}
/*
void RelativeStrengthIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(RSI_OVERZERO, true);
		else					vec.Set(RSI_BELOWZERO, true);
		if (value0 > value1)	vec.Set(RSI_INC, true);
		else					vec.Set(RSI_DEC, true);
	}
}
*/
void RelativeStrengthIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& pos_buffer = gi.GetBuffer(1);
	BufferImage& neg_buffer = gi.GetBuffer(2);
	BufferImage& change_buf = gi.GetBuffer(3);
	
	double diff;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();

	int pos = begin - 1;

	if ( pos <= period ) {
		buffer.Set(0, 0.0);
		pos_buffer.Set(0, 0.0);
		neg_buffer.Set(0, 0.0);
		double sum_p = 0.0;
		double sum_n = 0.0;


		double prev_value  = si.Open( 0 );

		for (int i = 1; i <= period; i++) {
			double value  = si.Open(i);

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
	
	double prev_value  = si.Open(pos-1);

	for (int i = pos; i < end; i++) {
		double value  = si.Open(i);
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

void RelativeVigorIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Green);
	gi.SetBufferColor(1, Red);
	
	gi.SetBufferBegin ( 0, period + 3 );
	gi.SetBufferBegin ( 1, period + 7 );
	
	gi.SetBufferStyle(0,DRAW_LINE);
	gi.SetBufferStyle(1,DRAW_LINE);
	//gi.SetBufferLabel(0,"RVI");
	//gi.SetBufferLabel(1,"RVIS");
}
/*
void RelativeVigorIndex::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		double value2 = gi.GetBuffer(1).Get(cursor);
		if (value0 > 0.0)		vec.Set(RVI_OVERZERO, true);
		else					vec.Set(RVI_BELOWZERO, true);
		if (value0 > value1)	vec.Set(RVI_INC, true);
		else					vec.Set(RVI_DEC, true);
	}
}
*/
void RelativeVigorIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi)
{
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& signal_buffer = gi.GetBuffer(1);
	double value_up, value_down, num, denum;

	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period + 8 )
		throw DataExc();

	if ( begin < 0 )
		throw DataExc();

	if (begin < 4+period) begin = 4+period;
	
	//end--;
	
	for (int i = begin; i < end; i++) {
		num = 0.0;
		denum = 0.0;

		for ( int j = i; j > i - period && j >= 0; j-- ) {
			double close0 = si.Open( j ); // peek is fixed at for loop
			double close1 = si.Open( j - 1 );
			double close2 = si.Open( j - 2 );
			double close3 = si.Open( j - 3 );

			double open0 =  si.Open( j - 1 );
			double open1 =  si.Open( j - 2 );
			double open2 =  si.Open( j - 3 );
			double open3 =  si.Open( j - 4 );

			double high0 = si.High( j - 1 );
			double high1 = si.High( j - 2 );
			double high2 = si.High( j - 3 );
			double high3 = si.High( j - 4 );

			double low0 = si.Low( j - 1 );
			double low1 = si.Low( j - 2 );
			double low2 = si.Low( j - 3 );
			double low3 = si.Low( j - 4 );

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

	if ( begin < 8 )
		begin = 8;

	for ( int i = begin; i < end; i++)
		signal_buffer.Set(i, ( buffer.Get(i) + 2 * buffer.Get(i-1) + 2 * buffer.Get(i-2) + buffer.Get(i-3) ) / 6);
}







StochasticOscillator::StochasticOscillator() {
	k_period = 5;
	d_period = 3;
	slowing = 3;
}

void StochasticOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-1.0); // normalized
	gi.SetCoreMaximum(+1.0);  // normalized
	gi.SetBufferColor(0, LightSeaGreen);
	gi.SetBufferColor(1, Red);
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, -0.60); // normalized
	gi.SetCoreLevel(1, +0.60); // normalized
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
	
	//gi.SetBufferLabel( 0, "Main");
	//gi.SetBufferLabel( 1, "Signal" );
	gi.SetBufferBegin(0, k_period + slowing - 2 );
	gi.SetBufferBegin(1, k_period + d_period );

	gi.SetBufferStyle(0, DRAW_LINE);
	gi.SetBufferStyle(1, DRAW_LINE);
	gi.SetBufferType(1, STYLE_DOT);
	
}
/*
void StochasticOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(STOCH_OVERZERO, true);
		else					vec.Set(STOCH_BELOWZERO, true);
		if (value0 > value1)	vec.Set(STOCH_INC, true);
		else					vec.Set(STOCH_DEC, true);
		if (value0 > +0.60)		vec.Set(STOCH_OVERHIGH, true);
		if (value0 < -0.60)		vec.Set(STOCH_BELOWLOW, true);
	}
}
*/
void StochasticOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& signal_buffer = gi.GetBuffer(1);
	BufferImage& high_buffer = gi.GetBuffer(2);
	BufferImage& low_buffer = gi.GetBuffer(3);
	int start;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= k_period + d_period + slowing )
		throw DataExc();

	start = k_period;

	if ( start + 1 < begin )
		start = begin - 2;
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
	for (int i = start; i < end; i++) {
		
		if (low_length >= k_period) {
			dmin = 1000000.0;
			for (int k = i - k_period; k < i; k++) {
				double low = si.Low( k );
				if ( dmin > low )
					dmin = low;
			}
			low_length = 0;
		} else {
			low_length++;
			double low = si.Low( i - 1 );
			if ( dmin > low )
				dmin = low;
		}
		
		if (high_length >= k_period) {
			dmax = -1000000.0;
			for (int k = i - k_period; k < i; k++) {
				double high = si.High( k );
				if ( dmax < high )
					dmax = high;
			}
			high_length = 0;
		} else {
			high_length++;
			double high = si.High( i - 1 );
			if ( dmax < high )
				dmax = high;
		}

		low_buffer.Set(i, dmin);
		high_buffer.Set(i, dmax);
	}

	start = k_period - 1 + slowing - 1;

	if ( start + 1 < begin )
		start = begin - 2;
	else {
		for (int i = 0; i < start; i++)
			buffer.Set(i, 0.0);
	}

	//end--;
	double sumlow = 0.0;
	double sumhigh = 0.0;
	for(int i = Upp::max(1, start - slowing); i < start; i++) {
		if (i <= 0) continue;
		double close = si.Open(i);
		double low = Upp::min(close, low_buffer.Get(i-1));
		double high = Upp::max(close, high_buffer.Get(i-1));
		sumlow  += close - low;
		sumhigh += high - low;
	}
	
	for (int i = Upp::max(1, start); i < end; i++) {
		
		int j = i - slowing;
		if (j > 0) {
			double close = si.Open(j);
			double low = Upp::min(close, low_buffer.Get(j-1));
			double high = Upp::max(close, high_buffer.Get(j-1));
			sumlow  -= close - low;
			sumhigh -= high - low;
		}
		
		
		double close = si.Open(i);
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

	if ( start + 1 < begin )
		start = begin - 2;
	else {
		for (int i = 0;i < start; i++)
			signal_buffer.Set(i, 0.0);
	}

	for (int i = start; i < end; i++) {
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

void WilliamsPercentRange::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-1); // normalized
	gi.SetCoreMaximum(+1);  // normalized
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferColor(1, Color(28, 255, 200));
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, -20 + 50); // normalized
	gi.SetCoreLevel(1, -80 + 50); // normalized
	gi.SetBufferBegin ( 0, period );
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0,"WPR");
}

void WilliamsPercentRange::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& change_buf = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();

	if (!begin) {
		begin++;
		buffer.Set(0, 0);
	}
	
	//end--;
	for (int i = begin; i < end; i++) {
		int highest = si.HighestHigh( period, i-1 );
		int lowest = si.LowestLow( period, i-1 );
		double max_high = si.High( highest );
		double min_low = si.Low( lowest );
		double close = si.Open( i );
		double cur = -2 * ( max_high - close ) / ( max_high - min_low ) + 1;
		buffer.Set(i, cur); // normalized
		
		double prev = buffer.Get(i-1);
		double change = (cur - prev) * 10.0;
		change_buf.Set(i, change);
	}
}








AccumulationDistribution::AccumulationDistribution() {
	
}

void AccumulationDistribution::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, LightSeaGreen);
	gi.SetBufferStyle(0,DRAW_LINE);
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

void AccumulationDistribution::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (!begin) {
		begin++;
		buffer.Set(0, 0);
	}
	
	//end--;
	for (int i = begin; i < end; i++) {
		double close = si.Open(i);
		buffer.Set(i, (close - si.Low(i-1)) - (si.High(i-1) - close));
		
		if (buffer.Get(i) != 0.0) {
			double diff = si.High(i-1) - si.Low(i-1);
			if (diff < 0.000000001)
				buffer.Set(i, 0.0);
			else {
				buffer.Set(i, buffer.Get(i) / diff);
				buffer.Set(i, buffer.Get(i) * (double)si.Volume(i-1));
			}
		}
		
		buffer.Set(i, buffer.Get(i) + buffer.Get(i-1));
	}
	
}









MoneyFlowIndex::MoneyFlowIndex() {
	period = 14;
}

void MoneyFlowIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-50); // normalized
	gi.SetCoreMaximum(50);  // normalized
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, 20 - 50); // normalized
	gi.SetCoreLevel(1, 80 - 50); // normalized
	gi.SetBufferColor(0, Blue);
	gi.SetBufferBegin ( 0, period );
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0,"MFI");
}

void MoneyFlowIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	double pos_mf, neg_mf, cur_tp, prev_tp;
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();
	
	if (begin < period + 2)
		begin = period + 2;
	
	for (int i = begin; i < end; i++) {
		pos_mf = 0.0;
		neg_mf = 0.0;
		cur_tp = (
			si.High(i-1) +
			si.Low(i-1) +
			si.Open(i) ) / 3;

		for (int j = 0; j < period; j++) {
			prev_tp = (
				si.High(i-j-2) +
				si.Low (i-j-2) +
				si.Open(i-j-1) ) / 3;

			if ( cur_tp > prev_tp )
				pos_mf += si.Volume(i-j-1) * cur_tp;
			else {
				if ( cur_tp < prev_tp )
					neg_mf += si.Volume(i-j-1) * cur_tp;
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

void ValueAndVolumeTrend::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferStyle(0, DRAW_LINE);
}

void ValueAndVolumeTrend::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	double cur_value, prev_value;
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( begin > 0 )
		begin--;

	if (begin < 2)
		begin = 2;
	
	for (int i = begin; i < end; i++) {
		double volume = si.Volume(i-1);
		cur_value = si.GetAppliedValue( applied_value, i - 1);
		prev_value = si.GetAppliedValue( applied_value, i - 2 );
		buffer.Set(i,
			buffer.Get(i-1) + volume * ( cur_value -
			prev_value ) / prev_value);
	}
}






OnBalanceVolume::OnBalanceVolume() {
	applied_value = 5;
}

void OnBalanceVolume::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, DodgerBlue);
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "OBV");
}

void OnBalanceVolume::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( begin > 0 )
		begin--;

	if (begin < 2)
		begin = 2;
	
	for (int i = begin; i < end; i++) {
		double cur_value = si.GetAppliedValue( applied_value, i - 1);
		double prev_value = si.GetAppliedValue( applied_value, i - 2);

		if ( cur_value == prev_value )
			buffer.Set(i, buffer.Get(i));
		else {
			if ( cur_value < prev_value )
				buffer.Set(i, buffer.Get(i) - si.Volume(i - 1));
			else
				buffer.Set(i, buffer.Get(i) + si.Volume(i - 1));
		}
	}
}











Volumes::Volumes() {
	
}

void Volumes::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Green);
	gi.SetBufferStyle(0, DRAW_HISTOGRAM);
	//gi.SetBufferLabel(0,"Volume");
}

void Volumes::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	if (!begin) begin++;
	else begin--;
	for (int i = begin; i < end; i++) {
		buffer.Set(i, si.Volume(i));
	}
}















AcceleratorOscillator::AcceleratorOscillator() {
	
}

void AcceleratorOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Black);
	gi.SetBufferColor(1, Green);
	gi.SetBufferColor(2, Red);
	gi.SetBufferStyle ( 0, DRAW_NONE );
	gi.SetBufferStyle ( 1, DRAW_HISTOGRAM );
	gi.SetBufferStyle ( 2, DRAW_HISTOGRAM );
	gi.SetBufferBegin ( 0, 3 );
	gi.SetBufferBegin ( 1, 3 );
	gi.SetBufferBegin ( 2, 3 );
}
/*
void AcceleratorOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(ACC_OVERZERO, true);
		else					vec.Set(ACC_BELOWZERO, true);
		if (value0 > value1)	vec.Set(ACC_INC, true);
		else					vec.Set(ACC_DEC, true);
	}
}
*/
void AcceleratorOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& up_buffer = gi.GetBuffer(1);
	BufferImage& down_buffer = gi.GetBuffer(2);
	BufferImage& macd_buffer = gi.GetBuffer(3);
	BufferImage& signal_buffer = gi.GetBuffer(4);
	double prev = 0.0, current;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if ( end <= 3 )
		throw DataExc();
	
	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(2, 0);
	
	if ( begin > 0 ) {
		begin--;
		prev = macd_buffer.Get(begin) - signal_buffer.Get(begin);
	}

	for (int i = begin; i < end; i++) {
		macd_buffer.Set(i,
			ind1.Get(i) -
			ind2.Get(i));
	}

	SimpleMAOnBuffer ( end, begin, 0, 5, macd_buffer, signal_buffer );
	
	bool up = true;

	for (int i = begin; i < end; i++) {
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

void GatorOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Black);
	gi.SetBufferColor(1, Red);
	gi.SetBufferColor(2, Green);
	gi.SetBufferColor(3, Black);
	gi.SetBufferColor(4, Red);
	gi.SetBufferColor(5, Green);
	
	gi.SetBufferBegin ( 1, jaws_period + jaws_shift - teeth_shift );
	gi.SetBufferBegin ( 2, jaws_period + jaws_shift - teeth_shift );
	gi.SetBufferBegin ( 4, teeth_period + teeth_shift - lips_shift );
	gi.SetBufferBegin ( 5, teeth_period + teeth_shift - lips_shift );

	gi.SetBufferShift ( 0, teeth_shift );
	gi.SetBufferShift ( 1, teeth_shift );
	gi.SetBufferShift ( 2, teeth_shift );
	gi.SetBufferShift ( 3, lips_shift );
	gi.SetBufferShift ( 4, lips_shift );
	gi.SetBufferShift ( 5, lips_shift );
	
	gi.SetBufferStyle(0,DRAW_NONE);
	gi.SetBufferStyle(1,DRAW_HISTOGRAM);
	gi.SetBufferStyle(2,DRAW_HISTOGRAM);
	gi.SetBufferStyle(3,DRAW_NONE);
	gi.SetBufferStyle(4,DRAW_HISTOGRAM);
	gi.SetBufferStyle(5,DRAW_HISTOGRAM);
	
	//gi.SetBufferLabel(0,"GatorUp");
	//gi.SetBufferLabel(1,NULL);
	//gi.SetBufferLabel(2,NULL);
	//gi.SetBufferLabel(3,"GatorDown");
	//gi.SetBufferLabel(4,NULL);
	//gi.SetBufferLabel(5,NULL);
}

void GatorOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& up_buffer = gi.GetBuffer(0);
	BufferImage& up_red_buffer = gi.GetBuffer(1);
	BufferImage& up_green_buffer = gi.GetBuffer(2);
	BufferImage& down_buffer = gi.GetBuffer(3);
	BufferImage& down_red_buffer = gi.GetBuffer(4);
	BufferImage& down_green_buffer = gi.GetBuffer(5);
	
	double prev, current;
	int    end    = ci.GetEnd();
	int    begin = ci.GetBegin();
	
	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(2, 0);
	ConstBufferImage& ind3 = ci.GetInputBuffer(3, 0);
	ConstBufferImage& ind4 = ci.GetInputBuffer(4, 0);
	
	if ( begin <= teeth_period + teeth_shift - lips_shift )
		begin = ( teeth_period + teeth_shift - lips_shift );
	
	if (begin > 0) begin--;
	else begin++;
	
	for (int i = begin; i < end; i++) {
		
		current =
			ind1.Get(i) -
			ind2.Get(i);

		if ( current <= 0.0 )
			down_buffer.Set(i, current);
		else
			down_buffer.Set(i, -current);
	}


	for (int i = begin; i < end; i++) {
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
	
	begin = ci.GetBegin();
	if ( begin <= jaws_period + jaws_shift - teeth_shift )
		begin = ( jaws_period + jaws_shift - teeth_shift );
	
	if (begin > 1) begin--;
	else begin++;
	
	for (int i = begin; i < end; i++) {
		current =
			ind3.Get(i) -
			ind4.Get(i);

		if ( current >= 0.0 )
			up_buffer.Set(i, current);
		else
			up_buffer.Set(i, -current);
	}
	
	for (int i = begin; i < end; i++) {
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











AwesomeOscillator::AwesomeOscillator() {
	
}

void AwesomeOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Black);
	gi.SetBufferColor(1, Green);
	gi.SetBufferColor(2, Red);
	gi.SetBufferStyle ( 0, DRAW_NONE );
	gi.SetBufferStyle ( 1, DRAW_HISTOGRAM );
	gi.SetBufferStyle ( 2, DRAW_HISTOGRAM );
	
	gi.SetBufferBegin ( 0, 5 );
	gi.SetBufferBegin ( 1, 5 );
	gi.SetBufferBegin ( 2, 5 );
}
/*
void AwesomeOscillator::Assist(int cursor, VectorBool& vec) {
	if (cursor > 0) {
		double value0 = gi.GetBuffer(0).Get(cursor);
		double value1 = gi.GetBuffer(0).Get(cursor-1);
		if (value0 > 0.0)		vec.Set(AWE_OVERZERO, true);
		else					vec.Set(AWE_BELOWZERO, true);
		if (value0 > value1)	vec.Set(AWE_INC, true);
		else					vec.Set(AWE_DEC, true);
	}
}
*/
void AwesomeOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& up_buffer = gi.GetBuffer(1);
	BufferImage& down_buffer = gi.GetBuffer(2);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	double prev = 0.0, current;
	
	if ( end <= 5 )
		throw DataExc();

	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(2, 0);
	
	if (begin > 0) {
		begin--;
		if (begin)
			prev = buffer.Get(begin - 1);
	}

	for (int i = begin; i < end; i++) {
		buffer.Set(i,
			ind1.Get(i) - ind2.Get(i));
	}

	bool up = true;

	for (int i = begin; i < end; i++) {
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

void Fractals::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	gi.SetBufferColor(0, Blue);
	gi.SetBufferColor(1, Blue);
	gi.SetBufferColor(2, Blue);
	gi.SetBufferColor(3, Blue);
	gi.SetBufferColor(4, Blue);
	gi.SetBufferColor(5, Blue);
	gi.SetBufferStyle(0, DRAW_LINE);
	gi.SetBufferArrow(0, 158);
	gi.SetBufferStyle(1, DRAW_LINE);
	gi.SetBufferArrow(1, 158);
	gi.SetBufferStyle(2, DRAW_ARROW);
	gi.SetBufferArrow(2, 119);
	gi.SetBufferStyle(3, DRAW_ARROW);
	gi.SetBufferArrow(3, 119);
	gi.SetBufferStyle(4, DRAW_ARROW);
	gi.SetBufferArrow(4, 119);
	gi.SetBufferStyle(5, DRAW_ARROW);
	gi.SetBufferArrow(5, 119);
}

void Fractals::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& line_up_buf1 = gi.GetBuffer(0);
	BufferImage& line_up_buf2 = gi.GetBuffer(1);
	BufferImage& arrow_up_buf = gi.GetBuffer(2);
	BufferImage& arrow_down_buf = gi.GetBuffer(3);
	BufferImage& arrow_breakup_buf = gi.GetBuffer(4);
	BufferImage& arrow_breakdown_buf = gi.GetBuffer(5);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if(begin==0)
		begin = 1 + Upp::max(left_bars, right_bars);
	else
		begin--;
	
	double prev_close = si.Open(begin);
	
	//end--;
	
	for(int i = begin; i < end; i++) {
		double close = si.Open(i);
		
		line_up_buf1.Set(i, IsFractalUp(i-1, left_bars, right_bars, end, si, ci, gi));
		if (line_up_buf1.Get(i) == 0)
			line_up_buf1.Set(i, line_up_buf1.Get(i-1));
		else
			arrow_up_buf.Set(i, line_up_buf1.Get(i));
		
		line_up_buf2.Set(i, IsFractalDown(i-1, left_bars, right_bars, end, si, ci, gi));
		
		if (line_up_buf2.Get(i) == 0)
			line_up_buf2.Set(i, line_up_buf2.Get(i-1));
		else
			arrow_down_buf.Set(i, line_up_buf2.Get(i));
		
		if (close < line_up_buf2.Get(i) && prev_close >= line_up_buf2.Get(i-1))
		
		arrow_breakdown_buf.Set(i, close);
		
		prev_close = close;
	}
}


double Fractals::IsFractalUp(int index, int left, int right, int maxind, SourceImage& si, ChartImage& ci, GraphImage& gi) {
	double max = si.High(index);
	for(int i = index - left; i <= (index + right); i++) {
		if (i < 0 || i > maxind)
			return(0);
		double high =  si.High(i);
		if(!(high > 0.0))
			return(0);
		if (max < high && i != index) {
			if(max < high)  return(0);
			if(abs(i - index) > 1) return(0);
		}
	}
	return(max);
}




double Fractals::IsFractalDown(int index, int left, int right, int maxind, SourceImage& si, ChartImage& ci, GraphImage& gi)
{
	double min = si.Low(index);
	for(int i = index - left; i <= (index + right); i++) {
		if (i < 0 || i > maxind)
			return(0);
		double low =  si.Low(i);
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

void FractalOsc::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red);
	gi.SetBufferColor(1, Blue);
	gi.SetBufferStyle(0, DRAW_LINE);
	gi.SetBufferStyle(1, DRAW_LINE);
}


void FractalOsc::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buf = gi.GetBuffer(0);
	BufferImage& av  = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin > 0)
		begin--;
	else
		begin = 1 + Upp::max(left_bars, right_bars);
	
	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(1, 1);
	
	//end--;
	
	for(int i = begin; i < end; i++) {
		double close = si.Open(i);
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

void MarketFacilitationIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, 20);
	gi.SetCoreLevel(1, 80);
	gi.SetBufferColor(0, Lime);
	gi.SetBufferColor(1, SaddleBrown);
	gi.SetBufferColor(2, Blue);
	gi.SetBufferColor(3, Pink);
	gi.SetBufferStyle ( 0, DRAW_HISTOGRAM );
	gi.SetBufferStyle ( 1, DRAW_HISTOGRAM );
	gi.SetBufferStyle ( 2, DRAW_HISTOGRAM );
	gi.SetBufferStyle ( 3, DRAW_HISTOGRAM );
	//gi.SetBufferLabel ( 0, "MFI Up, Volume Up" );
	//gi.SetBufferLabel ( 1, "MFI Down, Volume Down" );
	//gi.SetBufferLabel ( 2, "MFI Up, Volume Down" );
	//gi.SetBufferLabel ( 3, "MFI Down, Volume Up" );
}


void MarketFacilitationIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& up_up_buffer = gi.GetBuffer(0);
	BufferImage& down_down_buffer = gi.GetBuffer(1);
	BufferImage& up_down_buffer = gi.GetBuffer(2);
	BufferImage& down_up_buffer = gi.GetBuffer(3);
	BufferImage& buffer = gi.GetBuffer(4);
	
	bool mfi_up = true, vol_up = true;

	int sym_id = ci.GetSymbol();
	int end    = ci.GetEnd();
	int begin  = ci.GetBegin();
	
	if (begin > 0)
		begin--;
	else
		begin++;
	
	double point = GetMetaTrader().GetSymbol(sym_id).point;
	
	
	for (int i = begin; i < end; i++) {
		double volume = si.Volume(i-1);
		if ( IsEqualDoubles ( volume, 0.0 ) ) {
			if ( i == 0 )
				buffer.Set(i, 0.0);
			else
				buffer.Set(i, buffer.Get(i-1));
		}
		else
			buffer.Set(i,
				( si.High(i-1) - si.Low(i-1) ) /
				( volume * point ));
	}
	
	if ( begin > 1 ) {

		int i = begin -1;

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
	if (begin > 0)
		volume_prev = si.Volume(begin-1);
	else {
		volume_prev = si.Volume(begin-1);
		begin++;
	}
	
	for (int i = begin; i < end; i++) {
		double volume = si.Volume(i-1);
		if ( i < end - 1 ) {
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

void ZigZag::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	gi.SetBufferColor(0, Red());
	gi.SetBufferColor(1, Red());
	
	if (input_backstep >= input_depth) {
		input_backstep = input_depth-1;
	}
	
	gi.SetBufferStyle(0, DRAW_NONE);
	gi.SetBufferStyle(1, DRAW_SECTION);
}

void ZigZag::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osc = gi.GetBuffer(0);
	BufferImage& keypoint_buffer = gi.GetBuffer(1);
	BufferImage& high_buffer = gi.GetBuffer(2);
	BufferImage& low_buffer = gi.GetBuffer(3);
	int    counter_z, whatlookfor = 0;
	int    back, pos, lasthighpos = 0, lastlowpos = 0;
	double extremum;
	double curlow = 0.0, curhigh = 0.0, lasthigh = 0.0, lastlow = 0.0;
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	VectorBool& label = gi.GetSignal();
	label.SetCount(end);
	
	if ( end < input_depth || input_backstep >= input_depth )
		throw DataExc();

	if ( begin == 0 )
		begin = input_backstep;
	else {
		int i = end-1;
		counter_z = 0;
		
		while ( counter_z < extremum_level && i >= end-100 && i >= 0 ) {
			if ( keypoint_buffer.Get(i) != 0.0 )
				counter_z++;
			i--;
		}
		
		if (i < 0)
			i = 0;
		
		if ( counter_z == 0 )
			begin = input_backstep;
		else {
			begin = i+1;

			if ( low_buffer.Get(i) != 0.0 )
			{
				curlow = low_buffer.Get(i);
				whatlookfor = 1;
			} else {
				curhigh = high_buffer.Get(i);
				whatlookfor = -1;
			}

			for (int i = begin; i < end; i++) {
				keypoint_buffer.Set(i, 0.0);
				low_buffer.Set(i, 0.0);
				high_buffer.Set(i, 0.0);
			}
		}
	}

	double point = ci.GetPoint();
	
	
	ExtremumCache ec(input_depth);
	int border = Upp::max(0, begin - input_depth);
	ec.pos = begin - 1;
	for(int i = border; i < begin; i++) {
		double low = si.Low(i);
		double high = si.High(i);
		ec.Add(low, high);
	}
	
	for (int i = begin; i < end; i++) {
		double low = si.Low(i);
		double high = si.High(i);
		ec.Add(low, high);
		
		int lowest = ec.GetLowest();
		extremum = si.Low(lowest);
		
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
		extremum = si.High(highest);
	
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

	for (int i = begin; i < end; i++) {
		double low = si.Low(i);
		double high = si.High(i);
		
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
	for (int i = begin-1; i >= 0; i--) {
		if (keypoint_buffer.Get(i) != 0.0) {
			current = high_buffer.Get(i) != 0.0; // going down after high
			break;
		}
	}
	if (!begin) begin++;
	for (int i = begin; i < end; i++) {
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

void ZigZagOsc::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, GrayColor());
	gi.SetBufferStyle(0, DRAW_HISTOGRAM);
}

void ZigZagOsc::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osc = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(1, 1);
	
	double prev, next;
	bool has_prev = false, has_next = false;;
	int prev_pos, next_pos;
	
	if (begin) {
		begin = Upp::min(begin, ci.GetEnd()-1);
		for (; begin > 0; begin--) {
			double v = ind2.Get(begin);
			if (!v) continue;
			break;
		}
	}
	
	double diff, step;
	for (int i = begin; i < end; i++) {
		
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
			for (int j = i+1; j < end; j++) {
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
		double value_diff = si.GetAppliedValue(PRICE_TYPICAL, i) - v;
		
		osc.Set(i, value_diff);
	}
}









LinearTimeFrames::LinearTimeFrames() {
	
}

void LinearTimeFrames::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red);
	gi.SetBufferColor(1, Green);
	gi.SetBufferColor(2, Blue);
	gi.SetBufferColor(3, Yellow);
	gi.SetBufferLineWidth(0, 3);
	gi.SetBufferLineWidth(1, 2);
	gi.SetBufferLineWidth(2, 1);
	gi.SetCoreMinimum(0);
	gi.SetCoreMaximum(1);
}

void LinearTimeFrames::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ConstBufferImage& src_time = ci.GetInputBuffer(0,4);
	BufferImage& day = gi.GetBuffer(0);
	BufferImage& month = gi.GetBuffer(1);
	BufferImage& year = gi.GetBuffer(2);
	BufferImage& week = gi.GetBuffer(3);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	System& base = GetSystem();
	for(int i = begin; i < end; i++) {
		
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

void LinearWeekTime::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Green);
	gi.SetBufferLineWidth(0, 3);
	gi.SetCoreMinimum(-1);
	gi.SetCoreMaximum(+1);
}

void LinearWeekTime::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ConstBufferImage& src_time = ci.GetInputBuffer(0,4);
	BufferImage& week = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	System& base = GetSystem();
	for(int i = begin; i < end; i++) {
		
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

void SupportResistance::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	gi.SetBufferColor(0, Red);
	gi.SetBufferColor(1, Green);
}

void SupportResistance::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& support = gi.GetBuffer(0);
	BufferImage& resistance = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	double point = ci.GetPoint();
	
	ASSERT(point > 0);
	
	if (begin) begin--;
	else begin++;
	//end--;
	
	Vector<int> crosses;
	for (int i = begin; i < end; i++) {
		
		int highest = si.HighestHigh(period, i-1);
		int lowest  = si.LowestLow(period, i-1);
		double highesthigh = si.High(highest);
		double lowestlow   = si.Low(lowest);
		
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
			double high = si.High(j);
			double low  = si.Low(j);
			
			int low_pos  = (int)((low  - lowestlow) / inc);
			int high_pos = (int)((high - lowestlow) / inc);
			if (high_pos >= count) high_pos = count - 1;
			
			for(int k = low_pos; k <= high_pos; k++) {
				crosses[k]++;
			}
		}
		
		double close = si.Open(i);
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

void SupportResistanceOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Green);
	gi.SetBufferColor(1, Red);
	gi.SetCoreMinimum(-1);
	gi.SetCoreMaximum(1);
}

void SupportResistanceOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osc_av = gi.GetBuffer(0);
	BufferImage& osc = gi.GetBuffer(1);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	ConstBufferImage& ind1 = ci.GetInputBuffer(1, 0);
	ConstBufferImage& ind2 = ci.GetInputBuffer(1, 1);
	
	int prev_pos = begin ? begin-1 : 0;
	double prev_value = begin ? osc_av.Get(begin-1) : 0;
	for (int i = begin; i < end; i++) {
		
		double applied_value = si.Open(i);
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

void ChannelOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red);
	gi.SetCoreMinimum(-1);
	gi.SetCoreMaximum(1);
}

void ChannelOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osc = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	ec.SetSize(period);
	
	for (int i = ec.pos+1; i < end; i++) {
		double open = si.Open(i);
		double low = i > 0 ? si.Low(i-1) : open;
		double high = i > 0 ? si.High(i-1) : open;
		double max = Upp::max(open, high);
		double min = Upp::min(open, low);
		ec.Add(max, min);
		
		double ch_high = si.High(ec.GetHighest());
		double ch_low = si.Low(ec.GetLowest());
		double ch_diff = ch_high - ch_low;
		
		double value = (open - ch_low) / ch_diff * 2.0 - 1.0;
		osc.Set(i, value);
	}
}
/*
void ChannelOscillator::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	if      (value0 > +0.5)		vec.Set(CHOSC_HIGHEST, true);
	else if (value0 > +0.0)		vec.Set(CHOSC_HIGH, true);
	else if (value0 > -0.5)		vec.Set(CHOSC_LOW, true);
	else						vec.Set(CHOSC_LOWEST, true);
}
*/








ScissorChannelOscillator::ScissorChannelOscillator()
{
	period = 30;
}

void ScissorChannelOscillator::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red);
	gi.SetCoreMinimum(-1);
	gi.SetCoreMaximum(1);
}

void ScissorChannelOscillator::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& osc = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	ec.SetSize(period);
	
	for (int i = ec.pos+1; i < end; i++) {
		double open = si.Open(i);
		double low = i > 0 ? si.Low(i-1) : open;
		double high = i > 0 ? si.High(i-1) : open;
		double max = Upp::max(open, high);
		double min = Upp::min(open, low);
		ec.Add(max, min);
		
		int high_pos = ec.GetHighest();
		int low_pos = ec.GetLowest();
		double ch_high = si.High(high_pos);
		double ch_low = si.Low(low_pos);
		double ch_diff = ch_high - ch_low;
		
		double highest_change = DBL_MAX;
		int highest_change_pos = 0;
		double lowest_change = DBL_MAX;
		int lowest_change_pos = 0;
		
		for(int j = Upp::min(high_pos+1, low_pos+1); j <= i; j++) {
			double open = si.Open(j);
			
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
/*
void ScissorChannelOscillator::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	if (value0 > 0.0)			vec.Set(SCIS_HIGH, true);
	else						vec.Set(SCIS_LOW,  true);
}
*/





Psychological::Psychological() {
	period = 25;
}

void Psychological::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, DodgerBlue());
	gi.SetBufferStyle(0, DRAW_LINE);
	gi.SetBufferBegin(0, period);
	//gi.SetBufferLabel(0, "Psychological");
}

void Psychological::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buf = gi.GetBuffer(0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if(begin < 0)
		throw DataExc();
	
	if(begin > 0)
		begin--;
	
	if (end < period)
		throw DataExc();
	
	if (begin == 0)
		begin = (1 + period + 1);
	
	//end--;
	
	for(int i = begin; i < end; i++) {
		int count=0;
		for (int j=i-period+1; j <= i; j++) {
			if (si.Open(j) > si.Open(j-1)) {
				count++;
			}
		}
		if (si.Open(i) > si.Open(i-1)) {
			count++;
		}
		if (si.Open(i-period) > si.Open(i-period-1)) {
			count--;
		}
		buf.Set(i, ((double)count / period) *100.0 - 50); // normalized
	}
}





















TrendChange::TrendChange() {
	
}

void TrendChange::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red());
	
	gi.SetCoreLevelCount(1);
	gi.SetCoreLevel(0, 0);
	gi.SetCoreLevelsColor(GrayColor(192));
	gi.SetCoreLevelsStyle(STYLE_DOT);
}

void TrendChange::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin > 0)
		begin--;
	
	// The newest part can't be evaluated. Only after 'shift' amount of time.
	int shift = period / 2;
	begin = Upp::max(0, begin - shift);
	//end -= shift;
	
	// Calculate averages
	ConstBufferImage& dbl = ci.GetInputBuffer(1, 0);
	
	// Prepare values for loop
	double prev1 = dbl.Get(Upp::max(0, begin-1));
	double prev2 = dbl.Get(Upp::max(0, begin-1+shift));
	double prev_value = begin > 0 ? buffer.Get(begin-1) : 0;
	double prev_diff = begin > 1 ? prev_value - buffer.Get(begin-2) : 0;
	
	// Loop unprocessed range of time
	for(int i = begin; i < end-shift; i++) {
		
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
	
}

void TrendChangeEdge::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red());
}

void TrendChangeEdge::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	BufferImage& edge = gi.GetBuffer(1);
	BufferImage& symlr = gi.GetBuffer(2);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin > 0) begin -= 3;
	
	
	// The newest part can't be evaluated. Only after 'shift' amount of time.
	int shift = period / 2;
	begin = Upp::max(0, begin - shift);
	
	// Prepare values for looping
	ConstBufferImage& dbl = ci.GetInputBuffer(1, 0);
	ConstBufferImage& slow_dbl = ci.GetInputBuffer(2, 0);
	double prev_slow = slow_dbl.Get(Upp::max(0, begin-1));
	double prev1 = dbl.Get(Upp::max(0, begin-1)) - prev_slow;
	double prev2 = dbl.Get(Upp::max(0, begin-1+shift)) - prev_slow;
	
	// Calculate 'TrendChange' data locally. See info for that.
	for(int i = begin; i < end-shift; i++) {
		
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
	for(int i = Upp::max(2, begin); i < end - 2; i++) {
		
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

void PeriodicalChange::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, Red);
	
	tfmin = ci.GetPeriod();
	int w1 = 7 * 24 * 60;
	int count = 0;
	
	split_type = 0;
	if      (tfmin >= w1)		{count = 4;	split_type = 2;}
	else if (tfmin >= 24 * 60)	{count = 7; split_type = 1;}
	else						{count = 7 * 24 * 60 / tfmin; split_type = 0;}
	
	means.SetCount(count, 0.0);
	counts.SetCount(count, 0);
}

void PeriodicalChange::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	System& sys = GetSystem();
	ConstBufferImage& src = ci.GetInputBuffer(0, 0);
	ConstBufferImage& src_time = ci.GetInputBuffer(0,4);
	BufferImage& dst = gi.GetBuffer(0);
	
	int end = ci.GetEnd() - 1;
	int begin = ci.GetBegin();
	
	for(int i = begin; i < end; i++) {
		
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
	
	
	if (begin) begin--;
	end++;
	
	for(int i = begin; i < end; i++) {
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
/*
void PeriodicalChange::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	if (value0 > 0.0)		vec.Set(PC_INC, true);
	else					vec.Set(PC_DEC, true);
}
*/













VolatilityAverage::VolatilityAverage() {
	period = 60;
}

void VolatilityAverage::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetCoreMinimum(-1.0);
	gi.SetCoreMaximum(+1.0);
	gi.SetBufferColor(0, Color(85, 255, 150));
	gi.SetBufferLineWidth(0, 2);
	gi.SetCoreLevelCount(2);
	gi.SetCoreLevel(0, +0.5);
	gi.SetCoreLevel(1, -0.5);
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
}

void VolatilityAverage::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	System& sys = GetSystem();
	ConstBufferImage& src = ci.GetInputBuffer(0, 0);
	
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	if (begin >= end) return;
	if (!begin) begin++;
	
	BufferImage& dst = gi.GetBuffer(0);
	
	double sum = 0.0;
	for(int i = Upp::max(1, begin - period); i < begin; i++) {
		double change = fabs(src.Get(i) / src.Get(i-1) - 1.0);
		sum += change;
	}
	
	for(int i = begin; i < end; i++) {
		
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
	
	for(int i = begin; i < end; i++) {
		double average = dst.Get(i);
		int av = average * 100000;
		double j = stats_limit[stats.Find(av)];
		double sens = j / total * 2.0 - 1.0;
		dst.Set(i, sens);
	}
}
/*
void VolatilityAverage::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	if      (value0 > +0.5)		vec.Set(VOL_HIGHEST, true);
	else if (value0 > +0.0)		vec.Set(VOL_HIGH, true);
	else if (value0 > -0.5)		vec.Set(VOL_LOW, true);
	else						vec.Set(VOL_LOWEST, true);
}
*/



















MinimalLabel::MinimalLabel() {
	
}

void MinimalLabel::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	gi.SetBufferColor(0, Color(85, 255, 150));
	gi.SetBufferStyle(0, DRAW_ARROW);
	gi.SetBufferArrow(0, 159);
}

void MinimalLabel::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	int symbol = ci.GetSymbol();
	int tf = ci.GetTf();
	
	begin = max(0, begin - 200);
	
	double spread_point		= ci.GetPoint();
	double cost				= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = gi.GetSignal();
	labelvec.SetCount(end);
	
	BufferImage& buf = gi.GetBuffer(0);
	
	for(int i = begin; i < end; i++) {
		double open = si.Open(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		bool clean_break = false;
		for(; j < end; j++) {
			close = si.Open(j);
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
			open = si.Open(k);
			labelvec.Set(k, label);
			if (label)		buf.Set(k, open - 10 * spread_point);
			else			buf.Set(k, open + 10 * spread_point);
		}
		
		i = j - 1;
	}
}















VolatilitySlots::VolatilitySlots() {
	
}

void VolatilitySlots::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Color(113, 42, 150));
	gi.SetBufferStyle(0, DRAW_LINE);
	
	gi.SetCoreLevelCount(3);
	gi.SetCoreLevel(0, 0.0002);
	gi.SetCoreLevel(1,  0.001);
	gi.SetCoreLevel(2,  0.010);
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
	
	
	int tf_mins = ci.GetPeriod();
	if (tf_mins < 10080)
		slot_count = (5 * 24 * 60) / tf_mins;
	else
		slot_count = 1;
	
	stats.SetCount(slot_count);
}

void VolatilitySlots::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	ConstBufferImage& open_buf = ci.GetInputBuffer(0, 0);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin > 0)
		begin--;
	
	if (begin == 0)
		begin++;
	
	int tf_mins = ci.GetPeriod();
	
	for(int i = begin; i < end; i++) {
		double cur  = open_buf.Get(i);
		double prev = open_buf.Get(i-1);
		double change = fabs(cur / prev - 1.0);
		int slot_id = (i-1) % slot_count;
		OnlineAverage1& av = stats[slot_id];
		av.Add(change);
		total.Add(change);
	}
	
	for(int i = begin; i < end; i++) {
		int slot_id = i % slot_count;
		const OnlineAverage1& av = stats[slot_id];
		
		buffer.Set(i, av.mean);
	}
}
/*
void VolatilitySlots::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	if      (value0 > total.mean * 2.000)		vec.Set(VOLSL_VERYHIGH, true);
	else if (value0 > total.mean * 1.333)		vec.Set(VOLSL_HIGH, true);
	else if (value0 > total.mean * 0.666)		vec.Set(VOLSL_MED, true);
	else										vec.Set(VOLSL_LOW, true);
	if (cursor > 0) {
		double value1 = gi.GetBuffer(0).Get(cursor - 1);
		if (value0 > value1)		vec.Set(VOLSL_INC, true);
		else						vec.Set(VOLSL_DEC, true);
	}
}
*/










VolumeSlots::VolumeSlots() {
	
}

void VolumeSlots::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Color(113, 42, 150));
	gi.SetBufferStyle(0, DRAW_LINE);
	
	gi.SetCoreLevelCount(3);
	gi.SetCoreLevel(0, 0.0002);
	gi.SetCoreLevel(1,  0.001);
	gi.SetCoreLevel(2,  0.010);
	gi.SetCoreLevelsColor(Silver);
	gi.SetCoreLevelsStyle(STYLE_DOT);
	
	
	int tf_mins = ci.GetPeriod();
	if (tf_mins < 10080)
		slot_count = (5 * 24 * 60) / tf_mins;
	else
		slot_count = 1;
	
	stats.SetCount(slot_count);
}

void VolumeSlots::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer = gi.GetBuffer(0);
	ConstBufferImage& vol_buf = ci.GetInputBuffer(0, 3);
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	
	if (begin > 0)
		begin--;
	
	if (begin == 0)
		begin++;
	
	int tf_mins = ci.GetPeriod();
	
	for(int i = begin; i < end; i++) {
		double vol  = vol_buf.Get(i);
		if (vol == 0.0) continue;
		int slot_id = (i-1) % slot_count;
		OnlineAverage1& av = stats[slot_id];
		av.Add(vol);
		total.Add(vol);
	}
	
	for(int i = begin; i < end; i++) {
		int slot_id = i % slot_count;
		const OnlineAverage1& av = stats[slot_id];
		
		buffer.Set(i, av.mean);
	}
}
/*
void VolumeSlots::Assist(int cursor, VectorBool& vec) {
	double value0 = gi.GetBuffer(0).Get(cursor);
	ASSERT(IsFin(value0));
	if      (value0 > total.mean * 2.000)		vec.Set(VOLUME_VERYHIGH, true);
	else if (value0 > total.mean * 1.333)		vec.Set(VOLUME_HIGH, true);
	else if (value0 > total.mean * 0.666)		vec.Set(VOLUME_MED, true);
	else										vec.Set(VOLUME_LOW, true);
	if (cursor > 0) {
		double value1 = gi.GetBuffer(0).Get(cursor - 1);
		if (value0 > value1)		vec.Set(VOLUME_INC, true);
		else						vec.Set(VOLUME_DEC, true);
	}
}
*/








TrendIndex::TrendIndex() {
	
}

void TrendIndex::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Blue);
	gi.SetBufferColor(1, Green);
	gi.SetBufferColor(2, Red);
	
	if ( period < 1 )
		throw DataExc();
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "TrendIndex");
}

void TrendIndex::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	BufferImage& buffer     = gi.GetBuffer(0);
	BufferImage& err_buffer = gi.GetBuffer(1);
	BufferImage& change_buf = gi.GetBuffer(2);
	ConstBufferImage& open_buf = ci.GetInputBuffer(0, 0);
	
	double diff;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();

	if ( end <= period )
		throw DataExc();
	
	VectorBool& label = gi.GetSignal();
	label.SetCount(end);
	
	for (int i = begin; i < end; i++) {
		bool bit_value;
		double err, av_change, buf_value;
		
		Process(open_buf, i, period, err_div, err, buf_value, av_change, bit_value);
		
		err_buffer.Set(i, err);
		buffer.Set(i, buf_value);
		change_buf.Set(i, av_change);
		label.Set(i, bit_value);
	}
	
	
	int true_count = 0;
	for (int i = 0; i < end; i++) {
		if (label.Get(i)) true_count++;
	}
	ASSERT(true_count > 0);
}

void TrendIndex::Process(ConstBufferImage& open_buf, int i, int period, int err_div, double& err, double& buf_value, double& av_change, bool& bit_value) {
	double current = open_buf.Get(i);
	int trend_begin_pos = Upp::max(0, i - period);
	double begin = open_buf.Get(trend_begin_pos);
	
	int len = (i - trend_begin_pos);
	if (len <= 0) return;
	av_change = fabs(current - begin) / len;
	
	err = 0;
	for(int j = trend_begin_pos; j < i; j++) {
		double change = open_buf.Get(j+1) - open_buf.Get(j);
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

void OnlineMinimalLabel::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
}

void OnlineMinimalLabel::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int end = ci.GetEnd();
	int symbol = ci.GetSymbol();
	int tf = ci.GetTf();
	
	ConstBufferImage& open_buf	= ci.GetInputBuffer(0, 0);
	double spread_point			= ci.GetPoint();
	double cost					= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = gi.GetSignal();
	labelvec.SetCount(end);
	
	for(int i = ci.GetBegin(); i < end; i++) {
		const int count = 1;
		bool sigbuf[count];
		int begin = Upp::max(0, i - 100);
		int end = i + 1;
		GetMinimalSignal(cost, open_buf, begin, end, sigbuf, count);
		
		bool label = sigbuf[count - 1];
		labelvec.Set(i, label);
	}
}

void OnlineMinimalLabel::GetMinimalSignal(double cost, ConstBufferImage& open_buf, int begin, int end, bool* sigbuf, int sigbuf_size) {
	int write_begin = end - sigbuf_size;
	
	for(int i = begin; i < end; i++) {
		double open = open_buf.Get(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < end; j++) {
			close = open_buf.Get(j);
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

void SelectiveMinimalLabel::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreChartWindow();
	gi.SetBufferColor(0, Color(85, 255, 150));
	gi.SetBufferStyle(0, DRAW_ARROW);
	gi.SetBufferArrow(0, 159);
}

void SelectiveMinimalLabel::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int end = ci.GetEnd();
	int symbol = ci.GetSymbol();
	int tf = ci.GetTf();
	
	// Too heavy to calculate every time and too useless
	if (ci.GetBegin()) return;
	
	VectorMap<int, Order> orders;
	ConstBufferImage& open_buf	= ci.GetInputBuffer(0, 0);
	double spread_point		= ci.GetPoint();
	double cost				= spread_point * (1 + cost_level);
	ASSERT(spread_point > 0.0);
	
	VectorBool& labelvec = gi.GetSignal();
	VectorBool& enabledvec = gi.GetEnabled();
	labelvec.SetCount(end);
	enabledvec.SetCount(end);
	
	BufferImage& buf = gi.GetBuffer(0);
	
	end--;
	
	for(int i = 0; i < end; i++) {
		double open = open_buf.Get(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < end; j++) {
			close = open_buf.Get(j);
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
			labelvec.Set(k, label);
		}
		
		i = j - 1;
	}
	
	bool prev_label = labelvec.Get(0);
	int prev_switch = 0;
	for(int i = 1; i < end; i++) {
		bool label = labelvec.Get(i);
		
		int len = i - prev_switch;
		
		if (label != prev_label) {
			Order& o = orders.Add(i);
			o.label = prev_label;
			o.start = prev_switch;
			o.stop = i;
			o.len = o.stop - o.start;
			
			double open = open_buf.Get(o.start);
			double close = open_buf.Get(o.stop);
			o.av_change = fabs(close - open) / len;
			
			double err = 0;
			for(int k = o.start; k < o.stop; k++) {
				double diff = open_buf.Get(k+1) - open_buf.Get(k);
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
	
	if (GetBegin() == 0) {DUMPC(orders);}*/
	
	double idx_limit_f = idx_limit * 0.01;
	for(int i = 0; i < orders.GetCount(); i++) {
		Order& o = orders[i];
		if (o.idx_norm < idx_limit_f) continue;
		
		ASSERT(o.start <= o.stop);
		
		for(int k = o.start; k < o.stop; k++) {
			enabledvec.Set(k, true);
			double open = open_buf.Get(k);
			if (o.label)		buf.Set(k, open - 10 * spread_point);
			else				buf.Set(k, open + 10 * spread_point);
		}
	}
}






















ReactionContext::ReactionContext() {
	
}

void ReactionContext::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Blue);
	
	if ( length < 1 )
		throw DataExc();
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "ReactionContext");
}

void ReactionContext::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ConstBufferImage& open = ci.GetInputBuffer(0, 0);
	ConstBufferImage& low  = ci.GetInputBuffer(0, 1);
	ConstBufferImage& high = ci.GetInputBuffer(0, 2);
	
	BufferImage& buffer     = gi.GetBuffer(0);
	
	double diff;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	if (!begin) begin++;

	for (int cursor = begin; cursor < end; cursor++) {
		
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

void VolatilityContext::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Blue);
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "VolatilityContext");
}

void VolatilityContext::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ConstBufferImage& open = ci.GetInputBuffer(0, 0);
	ConstBufferImage& low  = ci.GetInputBuffer(0, 1);
	ConstBufferImage& high = ci.GetInputBuffer(0, 2);
	
	BufferImage& buffer     = gi.GetBuffer(0);
	
	double diff;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	if (!begin) begin++;
	
	double point = ci.GetPoint();

	for (int cursor = begin; cursor < end; cursor++) {
		
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
	
	for (int cursor = begin; cursor < end; cursor++) {
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











ChannelContext::ChannelContext() {
	
}

void ChannelContext::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, Blue);
	
	if ( period < 1 )
		throw DataExc();
	
	gi.SetBufferStyle(0,DRAW_LINE);
	//gi.SetBufferLabel(0, "ChannelContext");
}

void ChannelContext::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	ConstBufferImage& open		= ci.GetInputBuffer(0, 0);
	ConstBufferImage& low		= ci.GetInputBuffer(0, 1);
	ConstBufferImage& high		= ci.GetInputBuffer(0, 2);
	
	BufferImage& buffer			= gi.GetBuffer(0);
	VectorBool& enabled_buf	= gi.GetSignal();
	
	double diff;
	int end = ci.GetEnd();
	int begin = ci.GetBegin();
	if (!begin) begin++;
	
	enabled_buf.SetCount(end);
	
	double point = ci.GetPoint();

	channel.SetSize(period);
	for(int i = 0; i < period; i++) {
		int cursor = max(1, begin - i - 1);
		double l = low.Get(cursor - 1);
		double h = high.Get(cursor - 1);
		channel.Add(l, h);
	}
	channel.pos = begin-1;
	
	for (int cursor = begin; cursor < end; cursor++) {
		double l = low.Get(cursor - 1);
		double h = high.Get(cursor - 1);
		channel.Add(l, h);
		l = low.Get(channel.GetLowest());
		h = high.Get(channel.GetHighest());
		
		double diff = h - l;
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
	
	channel.SetSize(period);
	for(int i = 0; i < period; i++) {
		int cursor = max(1, begin - i - 1);
		double l = low.Get(cursor - 1);
		double h = high.Get(cursor - 1);
		channel.Add(l, h);
	}
	channel.pos = begin-1;
	
	for (int cursor = begin; cursor < end; cursor++) {
		double l = low.Get(cursor - 1);
		double h = high.Get(cursor - 1);
		channel.Add(l, h);
		l = low.Get(channel.GetLowest());
		h = high.Get(channel.GetHighest());
		
		double diff = h - l;
		
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
		enabled_buf.Set(cursor, lvl < useable_div);
	}
}










Obviousness::Obviousness() {
	
}

const int Obviousness::row_size;

void Obviousness::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	for(int i = 0; i < buffer_count; i++) {
		int j = i / 2;
		bool is_sig = i % 2;
		gi.SetBufferColor(i, GrayColor(80 + j * 20));
		gi.SetBufferLineWidth(i, 1 + is_sig);
	}
	
	gi.SetCoreLevelCount(1);
	gi.SetCoreLevel(0, 0.5);
}

void Obviousness::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	RefreshInput(si, ci, gi);
	if (!begin) {
		RefreshInitialOutput(si, ci, gi);
		RefreshIOStats(si, ci, gi);
	}
	
	RefreshOutput(si, ci, gi);
	
}

void Obviousness::RefreshInput(SourceImage& si, ChartImage& ci, GraphImage& gi) {

	// Get reference values
	System& sys = GetSystem();
	double spread_point		= ci.GetPoint();
	
	
	// Prepare maind data
	ConstBufferImage& open_buf = ci.GetInputBuffer(0, 0);
	ConstBufferImage& low_buf  = ci.GetInputBuffer(0, 1);
	ConstBufferImage& high_buf = ci.GetInputBuffer(0, 2);
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

void Obviousness::RefreshInitialOutput(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	
	// Get reference values
	System& sys = GetSystem();
	double spread_point		= ci.GetPoint();
	double cost				= spread_point * 3;
	int tf = 0;
	ASSERT(spread_point > 0.0);
	
	
	// Prepare maind data
	ConstBufferImage& open_buf = ci.GetInputBuffer(0, 0);
	ConstBufferImage& low_buf  = ci.GetInputBuffer(0, 1);
	ConstBufferImage& high_buf = ci.GetInputBuffer(0, 2);
	int data_count = open_buf.GetCount();
	int begin = data_out.GetCount();
	data_out.SetCount(data_count);
	VectorMap<int, Order> orders;
	data_count -= 1 + TEST_SIZE;
	
	if (begin != 0) return;
	
	for(int i = ci.GetBegin(); i < data_count; i++) {
		double open = open_buf.Get(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < data_count; j++) {
			close = open_buf.Get(j);
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
			
			double open = open_buf.Get(o.start);
			double close = open_buf.Get(o.stop);
			o.av_change = fabs(close - open) / len;
			
			double err = 0;
			for(int k = o.start; k < o.stop; k++) {
				double diff = open_buf.Get(k+1) - open_buf.Get(k);
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

void Obviousness::RefreshIOStats(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	end -= 1 + TEST_SIZE;
	
	// Randomize the input pattern at first call
	int c0 = Upp::min(250, row_size);
	int c1 = Upp::min(250, row_size*(row_size-1));
	int c2 = Upp::min(250, row_size*(row_size-1)*(row_size-2));
	int c3 = Upp::min(250, row_size*(row_size-1)*(row_size-2)*(row_size-3));
	const int MAX_BITS = max_bit_ids;
	int counts[MAX_BITS] = {c0, c1, c2, c3};
	int exp_count = 0; for (int i = 0; i < MAX_BITS; i++) exp_count += counts[i];
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
	for(int i = begin; i < end; i++) {
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


void Obviousness::RefreshOutput(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	VectorBool& signalbool = gi.GetSignal();
	VectorBool& enabledbool = gi.GetEnabled();
	signalbool.SetCount(end);
	enabledbool.SetCount(end);
	for(int i = begin; i < end; i++) {
		
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
		gi.GetBuffer(0).Set(i, enabled_av);
		gi.GetBuffer(1).Set(i, signal_av);
		
		signalbool.Set(i, signal_av > 0.5);
		enabledbool.Set(i, enabled_av > 0.5);
	}
}








VolatilityContextReversal::VolatilityContextReversal() {
	
}

void VolatilityContextReversal::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	/*gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, GrayColor(80));
	gi.SetBufferLineWidth(0, 2);
	
	gi.SetCoreLevelCount(1);
	gi.SetCoreLevel(0, 0.5);*/
}

void VolatilityContextReversal::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	ConstBufferImage& open_buf = ci.GetInputBuffer(0, 0);
	ConstBufferImage& volat_ctx = ci.GetInputBuffer(1, 0);
	
	VectorBool& signal = gi.GetSignal();
	signal.SetCount(end);
	
	for(int i = begin; i < end; i++) {
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





ObviousTargetValue::ObviousTargetValue() {
	
}

bool ObviousTargetValue::GetInput(int i, int j) {
	return bufs[j]->Get(i);
}

void ObviousTargetValue::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	gi.SetBufferColor(0, GrayColor(80));
	gi.SetBufferLineWidth(0, 1);
	
	gi.SetCoreLevelCount(1);
	gi.SetCoreLevel(0, 0.0);
	
	bufs.SetCount(row_size);
	for(int i = 0; i < bufs.GetCount(); i++)
		bufs[i] = &ci.GetInputSignal(1 + i);
	
	outbuf = &ci.GetInputSignal(5);
}

void ObviousTargetValue::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	
	if (!ci.GetBegin()) {
		RefreshTargetValues(si, ci, gi);
		RefreshIOStats(si, ci, gi);
	}
	
	RefreshOutput(si, ci, gi);
}

void ObviousTargetValue::RefreshTargetValues(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	ConstBufferImage& open_buf = ci.GetInputBuffer(0,0);
	BufferImage& dst = gi.GetBuffer(1);
	
	bool prev_value = outbuf->Get(end-1);
	double prev_open = open_buf.Get(end-1);
	for(int i = end-1; i >= begin; i--) {
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

void ObviousTargetValue::RefreshIOStats(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	end -= 1 + TEST_SIZE;
	
	// Randomize the input pattern at first call
	int c0 = Upp::min(250, row_size);
	int c1 = Upp::min(250, row_size*(row_size-1));
	int c2 = Upp::min(250, row_size*(row_size-1)*(row_size-2));
	int c3 = Upp::min(250, row_size*(row_size-1)*(row_size-2)*(row_size-3));
	const int MAX_BITS = max_bit_ids;
	int counts[MAX_BITS] = {c0, c1, c2, c3};
	int exp_count = 0; for (int i = 0; i < MAX_BITS; i++) exp_count += counts[i];
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
	for(int i = begin; i < end; i++) {
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
			stat.sum += gi.GetBuffer(1).Get(i);
		}
	}
}

void ObviousTargetValue::RefreshOutput(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	VectorBool& label = gi.GetSignal();
	label.SetCount(end);
	for(int i = begin; i < end; i++) {
		int changed_div = 0;
		double change_av = 0;
		
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
				changed_div++;
				double av = stat.total_count > 0 ? stat.sum / stat.total_count : 0.0;
				change_av += av;
			}
		}
		
		change_av	/= max(1, changed_div);
		gi.GetBuffer(0).Set(i, change_av);
		label.Set(i, change_av < 0.0);
	}
}









BasicSignal::BasicSignal() {
	
}

void BasicSignal::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	bufs.SetCount(5);
	for(int i = 0; i < bufs.GetCount(); i++)
		bufs[i] = &ci.GetInputSignal(1 + i);
}

void BasicSignal::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	VectorBool& signal = gi.GetSignal();
	VectorBool& enabled = gi.GetEnabled();
	signal.SetCount(end);
	enabled.SetCount(end);
	for(int i = begin; i < end; i++) {
		
		bool mini_signal  = bufs[0]->Get(i);
		bool vola_signal  = bufs[1]->Get(i);
		bool obvs_signal  = bufs[2]->Get(i);
		bool obvt_signal  = bufs[3]->Get(i);
		
		bool chnl_signal = true;
		if (i >= 3) for(int j = 0; j < 4; j++) {
			chnl_signal &= bufs[4]->Get(i - j);
		}
		
		
		bool is_enabled = mini_signal == vola_signal && !chnl_signal;
		bool triggered = mini_signal == vola_signal && !chnl_signal && vola_signal == obvs_signal && obvs_signal == obvt_signal;
		
		bool prev_enabled = i > 0 ? enabled.Get(i-1) : false;
		bool prev_signal = i > 0 ? signal.Get(i-1) : false;
		
		if (prev_enabled) {
			if (is_enabled && prev_signal == mini_signal) {
				signal.Set(i, mini_signal);
				enabled.Set(i, true);
			}
			else {
				signal.Set(i, false);
				enabled.Set(i, false);
			}
		} else {
			if (triggered) {
				signal.Set(i, mini_signal);
				enabled.Set(i, true);
			}
		}
	}
	
	
	{
		int sig = 0;
		bool signal = gi.GetSignal().Get(end-1);
		bool enabled = gi.GetEnabled().Get(end-1);
		if (enabled)
			sig = signal ? -1.0 : +1.0;
		GetSystem().SetSignal(ci.GetSymbol(), sig);
	}
}












#if 0
ObviousAdvisor::ObviousAdvisor() {
	
}

void ObviousAdvisor::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	
	bs  = dynamic_cast<BasicSignal*>(GetInputCore(1));
	obv = dynamic_cast<Obviousness*>(GetInputCore(2));
	ASSERT(bs && obv);
}

void ObviousAdvisor::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	if (!begin) RefreshIOStats();
	
	RefreshOutput();
	
	if (!begin) TestAdvisor();
	
	
	int sig = 0;
	bool signal = GetOutput(0).label.Get(end-1);
	bool enabled = GetOutput(1).label.Get(end-1);
	if (enabled)
		sig = signal ? -1.0 : +1.0;
	
	GetSystem().SetSignal(GetSymbol(), sig);
}

void ObviousAdvisor::RefreshIOStats() {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	ConstBufferImage& open_buf = ci.GetInputBuffer(0,0);
	
	end -= 1 + TEST_SIZE;
	
	ConstVectorBool& src_signal  = bs->GetSignal();
	ConstVectorBool& src_enabled = bs->GetEnabled();
	
	bool prev_enabled = false, prev_signal;
	for(int i = 0; i < end; i++) {
		bool enabled = src_enabled.Get(i);
		if (enabled) {
			bool signal = src_signal.Get(i);
			if (!prev_enabled || prev_signal != signal) {
				begins.FindAdd(i);
			}
			prev_signal = signal;
		}
		prev_enabled = enabled;
	}
	
	// Randomize the input pattern at first call
	int c0 = Upp::min(250, row_size);
	int c1 = Upp::min(250, row_size*(row_size-1));
	int c2 = Upp::min(250, row_size*(row_size-1)*(row_size-2));
	int c3 = Upp::min(250, row_size*(row_size-1)*(row_size-2)*(row_size-3));
	const int MAX_BITS = max_bit_ids;
	int counts[MAX_BITS] = {c0, c1, c2, c3};
	int exp_count = 0; for (int i = 0; i < MAX_BITS; i++) exp_count += counts[i];
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
	for(int i = 0; i < begins.GetCount(); i++) {
		int pos = begins[i];
		Obviousness::Snap& snap_in = obv->data_in[pos];
		
		bool open_enabled = src_enabled.Get(pos);
		bool open_signal = src_signal.Get(pos);
		ASSERT(open_enabled);
		double signal_open, prev_open;
		prev_open = signal_open = open_buf.Get(pos);
		int total_positive = 0, total = 0;
		for(int j = pos + 1; j < end; j++) {
			bool enabled = src_enabled.Get(j);
			if (!enabled) break;
			bool signal = src_signal.Get(j);
			if (open_signal != signal) break;
			prev_open = open_buf.Get(j);
			bool positive = prev_open > signal_open;
			if (signal) positive = !positive;
			if (positive) total_positive++;
			total++;
		}
		bool positive_outcome = prev_open > signal_open;
		if (open_signal) positive_outcome = !positive_outcome;
		
		
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
			
			// Distance
			stat.total_positive += total_positive;
			stat.total += total;
			
			// outcome
			if (positive_outcome) stat.outcome_positive++;
			stat.outcome_total++;
		}
	}
}

void ObviousAdvisor::RefreshOutput() {
	int begin = ci.GetBegin();
	int end = ci.GetEnd();
	
	ConstVectorBool& src_signal  = bs->GetSignal();
	ConstVectorBool& src_enabled = bs->GetEnabled();
	
	bool prev_enabled = src_enabled.Get(begin - 1);
	bool prev_signal = src_signal.Get(begin - 1);
	for(int i = begin; i < end; i++) {
		bool enabled = src_enabled.Get(i);
		if (enabled) {
			bool signal = src_signal.Get(i);
			if (!prev_enabled || prev_signal != signal) {
				begins.FindAdd(i);
			}
			prev_signal = signal;
		}
		prev_enabled = enabled;
	}
	
	
	
	VectorBool& signalbool = gi.GetSignal();
	VectorBool& enabledbool = gi.GetEnabled();
	signalbool.SetCount(end);
	enabledbool.SetCount(end);
	if (begin_cursor > 0) begin_cursor--;
	for(; begin_cursor < begins.GetCount(); begin_cursor++) {
		int i = begins[begin_cursor];
		
		Obviousness::Snap& snap_in = obv->data_in[i];
		
		int total_div = 0;
		int outcome_div = 0;
		double total_av = 0;
		double outcome_av = 0;
		
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
			
			if (stat.total > 1) {
				total_div++;
				total_av += (double)stat.total_positive / (double)(stat.total - 1);
			}
			if (stat.outcome_total > 1) {
				outcome_div++;
				outcome_av += (double)stat.outcome_positive / (double)(stat.outcome_total - 1);
			}
		}
		
		total_av	/= max(1, total_div);
		outcome_av	/= max(1, outcome_div);
		
		bool enabled = outcome_av >= 0.667 && total_av >= 0.667;
		
		if (!enabled)
			continue;
		
		bool prev_enabled = src_enabled.Get(i);
		bool prev_signal = src_signal.Get(i);
		ASSERT(prev_enabled);
		for(int j = i; j < end; j++) {
			bool enabled = src_enabled.Get(j);
			if (enabled) {
				bool signal = src_signal.Get(j);
				if (prev_signal != signal)
					break;
				prev_signal = signal;
			}
			else
				break;
			prev_enabled = enabled;
			signalbool.Set(j, prev_signal);
			enabledbool.Set(j, true);
		}
	}
}

void ObviousAdvisor::TestAdvisor() {
	int end = ci.GetEnd();
	int begin = max(0, end - TEST_SIZE);
	
	
	VectorBool& signalbool = gi.GetSignal();
	VectorBool& enabledbool = gi.GetEnabled();
	
	System& sys = GetSystem();
	DataBridge& db = dynamic_cast<DataBridge&>(*GetInputCore(0));
	double spread_point		= db.GetPoint();
	double cost				= spread_point * 3;
	int tf = 0;
	ASSERT(spread_point > 0.0);
	ConstBufferImage& open_buf = db.GetBuffer(0);

	
	
	double total_change = 1.0;
	double prev_open;
	bool prev_enabled = false, prev_signal;
	int open_len = 0;
	for(int i = begin; i < end; i++) {
		bool enabled = enabledbool.Get(i);
		bool signal = signalbool.Get(i);
		
		bool do_close = false, do_open = false;
		if (prev_enabled) {
			if (!enabled)
				do_close = true;
			else if (signal != prev_signal) {
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
			LOG(Format("total_change=%f change=%f", total_change, change));
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
		double open = open_buf.Get(end-1);
		double change = open / prev_open;
		if (prev_signal == false) change = open / (prev_open + spread_point);
		else change = 1.0 - (open / (prev_open - spread_point) - 1.0);
		total_change *= change;
		LOG(Format("total_change=%f change=%f", total_change, change));
	}
	
	
	
	ReleaseLog("Result " + DblStr(total_change));
	
}















ExampleAdvisor::ExampleAdvisor() {
	
}

void ExampleAdvisor::Init(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	gi.SetCoreSeparateWindow();
	
	gi.SetBufferColor(0, RainbowColor(Randomf()));
	
	String tf_str = GetSystem().GetPeriodString(GetTf()) + " ";
	
	SetJobCount(1);
	
	SetJob(0, tf_str + " Training")
		.SetBegin		(THISBACK(TrainingBegin))
		.SetIterator	(THISBACK(TrainingIterator))
		.SetEnd			(THISBACK(TrainingEnd))
		.SetInspect		(THISBACK(TrainingInspect))
		.SetCtrl		<TrainingCtrl>();
}

void ExampleAdvisor::Start(SourceImage& si, ChartImage& ci, GraphImage& gi) {
	if (once) {
		if (prev_begin > 0) prev_begin--;
		once = false;
		RefreshSourcesOnlyDeep();
	}
	
	if (IsJobsFinished()) {
		int end = ci.GetEnd();
		if (prev_begin < end) {
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
	
	
	// Keep begin manually
	prev_begin = ci.GetEnd();
	
	
	// Write oscillator indicator
	BufferImage& buf = gi.GetBuffer(0);
	for(int i = 0; i < ci.GetEnd(); i++) {
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












#endif


}
