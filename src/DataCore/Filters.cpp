#include "DataCore.h"

namespace DataCore {

NormalizedValue::NormalizedValue() {
	AddValue<double>(); // open
	AddValue<double>(); // low
	AddValue<double>(); // high
}

void NormalizedValue::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("src");
	if (i != -1)
		src_path = args[i];
	else
		throw DataExc("NormalizedValue requires source data argument");
}

void NormalizedValue::Init() {
	src = FindLinkSlot(src_path);
	ASSERTEXC(src);
}

// Normalization is not correct, because future open values can't be seen due the design.
bool NormalizedValue::Process(const SlotProcessAttributes& attr) {
	if (!src) return false;
	
	// Get points for output values & attributes
	double* src_open = src->GetValue<double>(0, attr);
	double* open = GetValue<double>(0, attr);
	double* low  = GetValue<double>(1, attr);
	double* high = GetValue<double>(2, attr);
	double counted = attr.GetCounted();
	double period = attr.GetPeriod();
	
	double val;
	
	if (*src_open != 0.0) {
		var.AddResult(*src_open);
		
		double variance = var.GetVariance();
		if (variance != 0.0)
			val = (*src_open - var.GetMean()) / variance;
		else
			val = 0.0;
	} else {
		val = 0.0;
	}
	
	// Set output value
	*open	= val;
	*low	= val;
	*high	= val;
	
	return true;
}









DerivedValue::DerivedValue() {
	AddValue<double>(); // close
	AddValue<double>(); // low
	AddValue<double>(); // high
}

void DerivedValue::SetArguments(const VectorMap<String, Value>& args) {
	int i;
	i = args.Find("src");
	if (i != -1)
		src_path = args[i];
	else
		throw DataExc("DerivedValue requires source data argument");
}

void DerivedValue::Init() {
	src = FindLinkSlot(src_path);
	ASSERTEXC(src);
}

// Normalization is not correct, because future open values can't be seen due the design.
bool DerivedValue::Process(const SlotProcessAttributes& attr) {
	if (!src) return false;
	
	// Get points for output values & attributes
	double* src_open = src->GetValue<double>(0, attr);
	double* prev_src_open = src->GetValue<double>(0, 1, attr);
	double* prev_src_low  = src->GetValue<double>(1, 1, attr);
	double* prev_src_high = src->GetValue<double>(2, 1, attr);
	double* close = GetValue<double>(0, attr);
	double* low  = GetValue<double>(1, attr);
	double* high = GetValue<double>(2, attr);
	double counted = attr.GetCounted();
	double period = attr.GetPeriod();
	
	if (attr.GetCounted() > 0) {
		double* prev_close	= GetValue<double>(0, 1, attr);
		double* prev_low	= GetValue<double>(1, 1, attr);
		double* prev_high	= GetValue<double>(2, 1, attr);
		*prev_close			= *src_open			- *prev_src_open;
		*prev_high			= *prev_src_high	- *prev_src_open;
		*prev_low			= *prev_src_low		- *prev_src_open;
	}
	
	// Set output value
	*close	= 0;
	*low	= 0;
	*high	= 0;
	
	return true;
}

}
