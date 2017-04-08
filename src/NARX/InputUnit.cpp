#include "NARX.h"

InputUnit::InputUnit(double input) {
	input_value = input;
}

InputUnit::InputUnit() {
}


InputUnit::~InputUnit() {
}

double InputUnit::GetOutput() {
	return input_value;
}

void InputUnit::SetInput(double arg) {
	input_value = arg;
}

double InputUnit::GetInput() {
	return input_value;
}