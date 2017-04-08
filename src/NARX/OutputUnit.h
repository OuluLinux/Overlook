#pragma once
#include "Unit.h"
class OutputUnit :
	public Unit
{
protected:

	double target;


	double deltao;

	//virtual double pre_output();

public:
	OutputUnit(void);
	~OutputUnit(void);

	void setTarget(double target);
	double error();

	virtual void adjust_weights();

	virtual void compute_delta(double superior_layer_delta);
	virtual void compute_delta();

	virtual double get_delta(Unit * u);


};

