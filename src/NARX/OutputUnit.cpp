#include "NARX.h"


OutputUnit::OutputUnit(void):Unit()
{

	target = 0;

}


OutputUnit::~OutputUnit(void)
{
}


void OutputUnit::setTarget(double target)
{
	this->target=target;
}

double OutputUnit::error()
{
	return target - get_output();
}

void OutputUnit::adjust_weights()
{
	//deltao = activation_func_derv(pre_output()) * error();

	for(int i = 0; i < input_count;i ++)
	{
		input_weights[i] += Unit::alfa * deltao * input_area[i]->get_output();
		//
	}
	
}

void OutputUnit::compute_delta(double superior_layer_delta)
{
	deltao = activation_func_derv(pre_output()) * superior_layer_delta;
}

void OutputUnit::compute_delta()
{
	deltao = activation_func_derv(pre_output()) * error();
}

double OutputUnit::get_delta(Unit *u)
{

	for (int i = 0;i<input_count;i++)
		if (u == input_area[i])
	      return deltao * old_weights[i];

	assert(false);
	return 0;
}
/*double OutputUnit::pre_output()
{
	double preoutput = 0;
	for (int i=0; i < input_count; i++)
		preoutput += input_area[i]->get_output() * input_weights[i];
	//if(activation_func == Activation_functions::aslog)
	//FLOG(QString("unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	 if (activation_func == Activation_functions::identity)
		FLOG(QString("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	preoutput += bias;
	return preoutput;
}*/