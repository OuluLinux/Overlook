#include "DataCore.h"

namespace DataCore {

MovingAverage::MovingAverage() {
	ma_period = 13;
	ma_shift = 0;
	ma_method = 0;
	AddValue<double>();
}

void MovingAverage::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("period");
	if (i != -1)
		ma_period = args[i];
	i = args.Find("offset");
	if (i != -1)
		ma_shift = args[i];
	i = args.Find("method");
	if (i != -1)
		ma_method = args[i];
}

void MovingAverage::Init() {
	int draw_begin;
	if (ma_period < 2)
		ma_period = 2;
	draw_begin = ma_period - 1;
	/*SetBufferCount(1);
	SetBufferColor(0, Red());
	SetBufferShift(0, ma_shift);
	SetBufferBegin(0, draw_begin );
	SetIndexCount(1);
	SetIndexBuffer(0, buffer);*/
	src = FindLinkSlot("/open");
	ASSERTEXC(src);
}

bool MovingAverage::Process(const SlotProcessAttributes& attr) {
	switch ( ma_method )
	{
		case MODE_SIMPLE:
			return Simple(attr);
		case MODE_EXPONENTIAL:
			return Exponential(attr);
		case MODE_SMOOTHED:
			return Smoothed(attr);
		case MODE_LINWEIGHT:
			return LinearlyWeighted(attr);
	}
	return false;
}

bool MovingAverage::Simple(const SlotProcessAttributes& attr) {
	int end = attr.GetCounted();
	int begin = Upp::max(0, end - ma_period);
	int period = end - begin;
	double sum = 0.0;
	for(int i = 0; i < period; i++) {
		double* d = src->GetValue<double>(0, i, attr);
		ASSERT(d);
		sum += *d;
	}
	double* value = GetValue<double>(0, attr);
	ASSERT(value);
	*value = sum / period;
	return true;
}

bool MovingAverage::Exponential(const SlotProcessAttributes& attr) {
	int end = attr.GetCounted();
	int begin = Upp::max(0, end - ma_period);
	int period = end - begin;
	double* d = src->GetValue<double>(0, attr);
	double* value = GetValue<double>(0, attr);
	ASSERT(d && value);
	if (!period) {
		*value = *d;
	} else {
		double* value1 = GetValue<double>(0, 1, attr);
		ASSERT(value1);
		double pr = 2.0 / ( period + 1 );
		*value = (*d) * pr + (*value1) * ( 1 - pr );
	}
	return true;
}

bool MovingAverage::Smoothed(const SlotProcessAttributes& attr) {
	int end = attr.GetCounted();
	int begin = Upp::max(0, end - ma_period);
	double* value = GetValue<double>(0, attr);
	ASSERT(value);
	if (end < ma_period) {
		double sum = 0.0;
		int count = (end+1);
		for (int i = 0; i < count; i++) {
			double* d = src->GetValue<double>(0, i, attr);
			ASSERT(d);
			sum += *d;
		}
		*value = sum / count;
	}
	else {
		double* d = src->GetValue<double>(0, attr);
		double* value1 = GetValue<double>(0, 1, attr);
		ASSERT(d && value1);
		double sum = (*value1) * ( ma_period - 1 ) + (*d);
		*value = sum / ma_period;
	}
	return true;
}

bool MovingAverage::LinearlyWeighted(const SlotProcessAttributes& attr) {
	int end = attr.GetCounted();
	int begin = Upp::max(0, end - ma_period);
	int period = end - begin;
	double sum = 0.0;
	int div = 0;
	for(int i = 0; i < period; i++) {
		int mul = period - i;
		double* d = src->GetValue<double>(0, i, attr);
		ASSERT(d);
		sum += *d * mul;
		div += mul;
	}
	double* value = GetValue<double>(0, attr);
	ASSERT(value);
	*value = sum / div;
	return true;
}

}
