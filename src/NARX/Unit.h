#pragma once

namespace Narx {

class Unit : Moveable<Unit> {
private:
	double deltah;

protected:

	double (*activation_func) (double arg);
	double (*activation_func_derv) (double arg);
	int input_count;
	double output;
	Vector<Unit*> input_area;
	Vector<double> input_weights;

	double bias;

	Vector<double> old_weights;


	virtual void ComputeOutput();

	virtual double GetPreOutput();

public:

	static double alfa;

public:
	Unit();
	~Unit();

	void set_activation_func ( double (*f) (double arg));
	void set_activation_func_derv ( double (*f) (double arg));

	// returns the index of the added input unit
	int AddInputUnit(Unit& unit);

	virtual double GetOutput();

	virtual void AdjustWeights();

	double* GetWeights();
	int GetInputCount();

	void Copy(Unit& u);
	void Sum(Unit& u);
	void Divide(int len);

	virtual double GetDelta(Unit& u);

	virtual void ComputeDelta(double superior_layer_delta);

	virtual void FixWeights();
	
	void Serialize(Stream& s) {s % input_count % output % input_weights % bias % old_weights;}
};

}
