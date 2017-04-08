#pragma once

class InputUnit : public Unit
{
private:
	double input_value;
public:
	InputUnit(double input);
	InputUnit();
	~InputUnit(void);

	virtual double get_output();
	void set_input(double arg);
	double get_input();

};

