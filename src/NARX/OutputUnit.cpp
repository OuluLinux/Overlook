#include "NARX.h"

OutputUnit::OutputUnit(): Unit() {
	target = 0;
}


OutputUnit::~OutputUnit() {
}


void OutputUnit::SetTarget(double target) {
	this->target = target;
}

double OutputUnit::Error() {
	return target - GetOutput();
}

void OutputUnit::AdjustWeights() {
	//deltao = activation_func_derv(GetPreOutput()) * Error();
	for (int i = 0; i < input_count; i ++) {
		input_weights[i] += Unit::alfa * deltao * input_area[i]->GetOutput();
	}
}

void OutputUnit::ComputeDelta(double superior_layer_delta) {
	deltao = activation_func_derv(GetPreOutput()) * superior_layer_delta;
}

void OutputUnit::ComputeDelta() {
	deltao = activation_func_derv(GetPreOutput()) * Error();
}

double OutputUnit::GetDelta(Unit& u) {
	for (int i = 0; i < input_count; i++)
		if (&u == input_area[i])
			return deltao * old_weights[i];

	ASSERT(false);
	return 0;
}
/*  double OutputUnit::GetPreOutput()
    {
	double preoutput = 0;
	for (int i=0; i < input_count; i++)
		preoutput += input_area[i]->GetOutput() * input_weights[i];
	//if(activation_func == ActivationFunctions::AsLog)
	//FWhenLog(String("unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	 if (activation_func == ActivationFunctions::identity)
		FWhenLog(String("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	preoutput += bias;
	return preoutput;
    }*/