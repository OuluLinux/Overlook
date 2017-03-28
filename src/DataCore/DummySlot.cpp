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



}
