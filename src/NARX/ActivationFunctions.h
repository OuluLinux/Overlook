#pragma once

namespace Narx {
	
class ActivationFunctions {
public:
	static double Sigmoid(double arg);
	static double BSigmoid(double arg);
	static double Test(double arg);
	static double Linear(double arg);
	static double Identity(double arg);
	static double AsLog(double arg);
	static double Pol(double arg);


	static double SigmoidDerv(double arg);
	static double IdentityDerv(double arg);
	static double PolDerv(double arg);
	static double BSigmoidDerv(double arg);
	static double AsLogDerv(double arg);
};

}
