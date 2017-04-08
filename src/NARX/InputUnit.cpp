#include "InputUnit.h"


InputUnit::InputUnit(double input)
{
	input_value = input;
}

InputUnit::InputUnit()
{

}


InputUnit::~InputUnit(void)
{

}

double InputUnit::get_output()
{
	return input_value;
}

void InputUnit::set_input(double arg)
{
	input_value = arg;
}

double InputUnit::get_input()
{
	return input_value;
}