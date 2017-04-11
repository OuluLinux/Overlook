#include "NARX.h"

namespace Narx {

double ActivationFunctions::Sigmoid(double arg) {
	return 1.0 / (1.0 + exp( - arg));
}
double ActivationFunctions::BSigmoid(double arg) {
	return 1.0 / (1.0 + exp( - 0.1 * arg));
}

double ActivationFunctions::SigmoidDerv(double arg) {
	return Sigmoid(arg) * (1 - Sigmoid(arg));
}
double ActivationFunctions::BSigmoidDerv(double arg) {
	return 0.1 * BSigmoid(arg) * (1 - BSigmoid(arg));
}

double ActivationFunctions::Test(double arg) {
	return arg + 1;
}

double ActivationFunctions::Linear(double arg) {
	return arg >= 1.0;
}

double ActivationFunctions::Identity(double arg) {
	return arg;
}

double ActivationFunctions::AsLog(double arg) {
	if (arg > 0) return log(1.0 + arg);
	else return - log(1.0 - arg);
}

double ActivationFunctions::IdentityDerv(double arg) {
	return 1;
}

double ActivationFunctions::Pol(double arg) {
	return arg / sqrt(1 + arg * arg);
}

double ActivationFunctions::PolDerv(double arg) {
	return sqrt( ((1 + arg * arg) - arg * arg / sqrt(1 + arg * arg) ) / (1 + arg * arg));
}

double ActivationFunctions::AsLogDerv(double arg) {
	if (arg > 0) return 1 / (1 + arg);
	else return 1 / (1 - arg);
}

}
