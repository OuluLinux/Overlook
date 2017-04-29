#pragma once

#include "Unit.h"

namespace Narx {

class OutputUnit : public Unit, Moveable<OutputUnit> {
protected:

	double target;


	double deltao;

	//virtual double GetPreOutput();

public:
	OutputUnit();
	~OutputUnit();

	void SetTarget(double target);
	double Error();

	virtual void AdjustWeights();

	virtual void ComputeDelta(double superior_layer_delta);
	virtual void ComputeDelta();

	virtual double GetDelta(Unit& u);
	
	void Serialize(Stream& s) {s % target % deltao; Unit::Serialize(s);}
};

}
