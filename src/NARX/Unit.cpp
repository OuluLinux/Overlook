#include "NARX.h"


double Unit::alfa = 0.2;

Unit::Unit(void)
{
	input_area = new Unit * [MAX_INPUTS_PER_UNIT];
	input_count = 0;
	input_weights = new double[MAX_INPUTS_PER_UNIT];

	for(int i =0; i < MAX_INPUTS_PER_UNIT;i ++)
		input_weights[i] = (double) (rand() % 100) / 100;

	bias = (double) (rand() % 100) / 100;

	old_weights = new double[MAX_INPUTS_PER_UNIT];
	for(int i =0; i < MAX_INPUTS_PER_UNIT;i ++)
		old_weights[i] = input_weights[i];

	output = 0;
}


Unit::~Unit(void)
{
	delete [] input_area;
	delete [] input_weights;
	delete [] old_weights;
}


void Unit::set_activation_func( double (*f) (double arg))
{
	activation_func = f;
}

void Unit::set_activation_func_derv( double (*f) (double arg))
{
	activation_func_derv = f;
}

int Unit::add_input_unit (Unit *unit)
{
	if (input_count >= MAX_INPUTS_PER_UNIT) return -1;

	input_weights[input_count] = (double) (rand() % 100) / 100;
	input_area[input_count] = unit;
	
	//printf("%f\n", input_weights[input_count]);
	//FLOG(QString("input count=%1").arg(input_count).toStdString().c_str());
	return input_count++;
}

double Unit::pre_output()
{
	double preoutput = 0;
	for (int i=0; i < input_count; i++)
	{
		//FLOG(QString("input:%1\n").arg(input_area[i]->get_output()).toStdString().c_str());
		preoutput += input_area[i]->get_output() * input_weights[i];
	}
	//if(activation_func == Activation_functions::aslog)
	//FLOG(QString("unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	// if (activation_func == Activation_functions::identity)
	//	FLOG(QString("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	preoutput += bias;
	//FLOG(QString("output unit preoutput:%1\n").arg(preoutput).toStdString().c_str());
	return preoutput;
}

void Unit::compute_output()
{
	output = activation_func(pre_output());
}

double Unit::get_output()
{
	compute_output();
	return output;
}


void Unit::compute_delta(double superior_layer_delta)
{
		deltah =  activation_func_derv(pre_output()) * superior_layer_delta ; // * get_output();
}

void Unit::adjust_weights()
{
	
	for(int i = 0; i < input_count;i ++)
	{
		input_weights[i] += Unit::alfa * deltah * input_area[i]->get_output();
		//FLOG(QString("ok adjust=%1:%2\n").arg( pre_output()  ) .arg(activation_func_derv(pre_output())).toStdString().c_str());
	}
}

double *Unit::weights()
{
	return input_weights;
}

int Unit::inputcount()
{
	return input_count;
}

void Unit::copy(Unit *u)
{
	for(int i=0;i<input_count;i++)
		input_weights[i] = u->input_weights[i];
}

void Unit::sum(Unit *u)
{
	for(int i=0;i<input_count;i++)
		input_weights[i] += u->input_weights[i];
}

void Unit::divide(int len)
{
	for(int i=0;i<input_count;i++)
		input_weights[i] /= len;
}

double Unit::get_delta(Unit *u)
{

	for (int i = 0;i<input_count;i++)
		if (u == input_area[i])
	      return deltah * old_weights[i];

	assert(false);
	return 0;
}

void Unit::fix_weights()
{
	for(int i = 0; i < input_count;i ++)
		old_weights[i] = input_weights[i];
}