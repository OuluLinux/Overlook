#include "NARX.h"

double Unit::alfa = 0.2;

Unit::Unit() {
	input_area.SetCount(MAX_INPUTS_PER_UNIT);
	input_count = 0;
	input_weights.SetCount(MAX_INPUTS_PER_UNIT);
	for (int i = 0; i < MAX_INPUTS_PER_UNIT; i ++)
		input_weights[i] = Randomf();

	bias = Randomf();
	
	old_weights.SetCount(MAX_INPUTS_PER_UNIT);
	for (int i = 0; i < MAX_INPUTS_PER_UNIT; i ++)
		old_weights[i] = input_weights[i];

	output = 0;
}


Unit::~Unit() {
	
}


void Unit::set_activation_func( double (*f) (double arg)) {
	activation_func = f;
}

void Unit::set_activation_func_derv( double (*f) (double arg)) {
	activation_func_derv = f;
}

int Unit::AddInputUnit (Unit& unit) {
	if (input_count >= MAX_INPUTS_PER_UNIT) return -1;

	input_weights[input_count] = Randomf();
	input_area[input_count] = &unit;
	//printf("%f\n", input_weights[input_count]);
	//FLOG(String("input count=%1").arg(input_count).toStdString().c_str());
	return input_count++;
}

double Unit::GetPreOutput() {
	double preoutput = 0;

	for (int i = 0; i < input_count; i++) {
		//FLOG(String("input:%1\n").arg(input_area[i]->GetOutput()).toStdString().c_str());
		preoutput += input_area[i]->GetOutput() * input_weights[i];
	}

	//if(activation_func == ActivationFunctions::AsLog)
	//FLOG(String("unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	// if (activation_func == ActivationFunctions::identity)
	//	FLOG(String("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	preoutput += bias;
	//FLOG(String("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	return preoutput;
}

void Unit::ComputeOutput() {
	output = activation_func(GetPreOutput());
}

double Unit::GetOutput() {
	ComputeOutput();
	return output;
}


void Unit::ComputeDelta(double superior_layer_delta) {
	deltah =  activation_func_derv(GetPreOutput()) * superior_layer_delta ; // * GetOutput();
}

void Unit::AdjustWeights() {
	for (int i = 0; i < input_count; i ++) {
		input_weights[i] += Unit::alfa * deltah * input_area[i]->GetOutput();
		//FLOG(String("ok adjust=%1:%2\n").arg( GetPreOutput()  ) .arg(activation_func_derv(GetPreOutput())).toStdString().c_str());
	}
}

double* Unit::GetWeights() {
	return input_weights;
}

int Unit::GetInputCount() {
	return input_count;
}

void Unit::Copy(Unit& u) {
	for (int i = 0; i < input_count; i++)
		input_weights[i] = u.input_weights[i];
}

void Unit::Sum(Unit& u) {
	for (int i = 0; i < input_count; i++)
		input_weights[i] += u.input_weights[i];
}

void Unit::Divide(int len) {
	for (int i = 0; i < input_count; i++)
		input_weights[i] /= len;
}

double Unit::GetDelta(Unit& u) {
	for (int i = 0; i < input_count; i++)
		if (&u == input_area[i])
			return deltah * old_weights[i];

	ASSERT(false);
	return 0;
}

void Unit::FixWeights() {
	for (int i = 0; i < input_count; i ++)
		old_weights[i] = input_weights[i];
}