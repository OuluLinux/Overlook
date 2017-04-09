#include "DataCore.h"

namespace DataCore {
	
DummyValue::DummyValue() {
	AddValue<double>(); // open
	AddValue<double>(); // low
	AddValue<double>(); // high
}

bool DummyValue::Process(const SlotProcessAttributes& attr) {
	double* open = GetValue<double>(0, attr);
	double* low  = GetValue<double>(1, attr);
	double* high = GetValue<double>(2, attr);
	double counted = attr.GetCounted();
	double period = attr.GetPeriod();
	double shift = counted * period / (7*24);
	double v = sin(shift) + attr.sym_id + 2;
	*open = v;
	*low = v * (1.0 - 0.00001);
	*high = v * (1.0 + 0.00001);
	//LOG(attr.pos[attr.tf_id] << "/" << attr.bars[attr.tf_id] << ": " << v);
	return true;
}



DummyIndicator::DummyIndicator() {
	AddValue<double>();
}

void DummyIndicator::Init() {
	src = FindLinkSlot("/open");
	ASSERTEXC(src);
}

bool DummyIndicator::Process(const SlotProcessAttributes& attr) {
	double* value = GetValue<double>(0, attr);
	double sum = 0;
	int count = 0;
	for(int i = 0; i < 10; i++) {
		double* d = src->GetValue<double>(0, i, attr);
		if (!d) continue;
		sum += *d;
		count++;
	}
	double av = !count ? 0 : sum / count;
	*value = av;
	//LOG(attr.pos[attr.tf_id] << "/" << attr.bars[attr.tf_id] << ": " << av);
	return true;
}







DummyTrainer::DummyTrainer() {
	SetWithoutData(); // just a safety measure
}

bool DummyTrainer::Process(const SlotProcessAttributes& attr) {
	
	// Use only one instance
	if (attr.sym_id != 0) return true;
	
	LOG(Format("sym %d, tf %d pos %d", attr.sym_id, attr.tf_id, attr.GetCounted()));
	
	return false;
}








TestValue::TestValue() {
	AddValue<double>(); // open
	AddValue<double>(); // low
	AddValue<double>(); // high
	AddValue<double>(); // exogenous 1
	AddValue<double>(); // exogenous 2
	AddValue<double>(); // exogenous 3
	
	series_noise = 5; // %
}

bool TestValue::Process(const SlotProcessAttributes& attr) {
	
	// Get points for output values & attributes
	double* open = GetValue<double>(0, attr);
	double* low  = GetValue<double>(1, attr);
	double* high = GetValue<double>(2, attr);
	double* exo1 = GetValue<double>(3, attr);
	double* exo2 = GetValue<double>(4, attr);
	double* exo3 = GetValue<double>(5, attr);
	double counted = attr.GetCounted();
	double period = attr.GetPeriod();
	
	// Get previous open values
	double* d;
	d = GetValue<double>(0, 1, attr);
	double prev1 = d ? *d : 0.0;
	d = GetValue<double>(0, 2, attr);
	double prev2 = d ? *d : 0.0;
	d = GetValue<double>(0, 3, attr);
	double prev3 = d ? *d : 0.0;
	
	// Calculate cursor
	double cur = counted * period / (7*24);
	double cury = cur * +0.05;
	double curz = cur * -0.03;
	double prev_cur = (counted-1) * period / (7*24);
	double prev_cury = prev_cur * +0.05;
	double prev_curz = prev_cur * -0.03;
	double prev2_cur = (counted-2) * period / (7*24);
	double prev2_cury = prev_cur * +0.05;
	double prev2_curz = prev_cur * -0.03;
	
	// Set cursor value as first exogenous value
	*exo1 = cur;
	
	// Determine function and series function from symbol id
	double val;
	int series_func = attr.sym_id / 2;
	int predef = attr.sym_id % 2;
	
	switch (series_func) {
		case 0:	val = sin(cur);		break;
		case 1: val = log(cur);		break;
		case 2: val = exp(cur);		break;
		case 3: val = cur * cur;	break;
		case 4: val = 1 / cur;		break;
		case 5: val = cur;			break;
		default: val = 1.0;
	}
	
	// Functions
	if (predef == 0) {
		if (counted >= 3)			val = sin(cur + cury * prev1) * curz  + tan(prev2 - prev3);
		else if (counted == 2)		val = sin(cur + cury * prev1) * curz  + tan(prev2);
		else if (counted == 1)		val = sin(cur + cury * prev1) * curz  ;
		else						val = sin(cur + cury) * curz * curz ;
		*exo2 = cury;
		*exo3 = curz;
	} else {
		if (counted >= 1)			val = sin(cur - prev2_cury) * log(prev_cury + 1) +  log(fabs(prev1) + 1) - cury * sin(cur - prev_cury) ;
		else						val = sin(cur - prev2_cury) * log(prev_cury +  1) - cury * sin(cur - prev_cury) ;
		*exo2 = cury;
		*exo3 = 0.0;
	}
	
	// Add noise
	if (series_noise)				val += val * ((Random(2 * series_noise) - series_noise)) / 100.0 ;
	
	// Set output value
	*open	= val;
	*low	= val * (1.0 - 0.00001);
	*high	= val * (1.0 + 0.00001);
	
	return true;
}




}
