#pragma once
class Unit
{
private:
	double deltah;

protected:

	double (*activation_func) (double arg);
	double (*activation_func_derv) (double arg);
	int input_count;
	double output;
	Unit **input_area;
	double *input_weights;

	double bias;

	double *old_weights;


	virtual void compute_output();

	virtual double pre_output();

public:

	static double alfa;

public:
	Unit();
	~Unit();

	void set_activation_func ( double (*f) (double arg));
	void set_activation_func_derv ( double (*f) (double arg));
	/* returns the index of the added input unit */
	int add_input_unit (Unit * unit);

	virtual double get_output();

	virtual void adjust_weights();

	double *weights();
	int inputcount();

	void copy(Unit *u);
	void sum(Unit *u);
	void divide(int len);

	virtual double get_delta(Unit * u);

	virtual void compute_delta(double superior_layer_delta);

	virtual void fix_weights();
};

