#pragma once

#include "Unit.h"

class InputUnit : public Unit, Moveable<InputUnit> {
private:
	double input_value;
public:
	InputUnit(double input);
	InputUnit();
	~InputUnit();

	virtual double GetOutput();
	void SetInput(double arg);
	double GetInput();

};

