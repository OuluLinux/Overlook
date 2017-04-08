#include "NARX.h"


EvaluationEngine::EvaluationEngine() {
	series_len = 0;
	curlen = 0;
}


EvaluationEngine::~EvaluationEngine() {
	
}

void EvaluationEngine::Init(int slen) {
	series_len = slen;
	series.SetCount(slen, 0);
	predicted.SetCount(slen ,0);
}

void EvaluationEngine::Insert(double target, double pred) {
	series[curlen] = target;
	predicted[curlen] = pred;
	//LOG(Format("series:%n, predicted:%n", target, pred));
	curlen++;
}

double EvaluationEngine::F1() {
	double ret = 0;

	for (int i = 0; i < curlen; i++) {
		ret += pow(series[i] - predicted[i], 2);
		//FWhenLog(String(":%1,%2\n").arg(series[i]).arg(predicted[i]).toStdString().c_str());
	}

	return ret;
}

double EvaluationEngine::F2() {
	return sqrt(F1() / (curlen));
}

double EvaluationEngine::F3() {
	double ret = 0;
	double sum = 0;

	for (int i = 0; i < curlen; i++)
		sum += fabs(series[i]);

	return F2() / sum * curlen;
}

double EvaluationEngine::F4() {
	double s1 = 0;
	double s2 = 0;
	double avg = 0;

	for (int i = 0; i < curlen; i++)
		avg += fabs(series[i]);

	avg /= curlen;

	for (int i = 0; i < curlen; i++) {
		s1 += pow(series[i] - predicted[i], 2);
		s2 += pow(series[i] - avg, 2);
	}

	//char test[256];
	//	sprintf(test, "aa %f %f\n", s1, s2);
	//FWhenLog(test);
	return sqrt(s1 / s2);
}


double EvaluationEngine::Fd(double val) {
	double c = 0;

	for (int i = 0; i < curlen; i++)
		if (series[i] <= val) c++;

	return c / curlen;
}

double EvaluationEngine::Fy(double val) {
	double c = 0;

	for (int i = 0; i < curlen; i++)
		if (predicted[i] <= val) c++;

	return c / curlen;
}

double EvaluationEngine::KS1() {
	double max = 0;

	for (int i = 0; i < curlen; i++) {
		double t = fabs(Fd(series[i]) - Fy(series[i]));

		if (max < t) max = t;
	}

	return max;
}

double EvaluationEngine::KS2() {
	double max = 0;

	for (int i = 0; i < curlen; i++) {
		double t = fabs(Fd(predicted[i]) - Fy(predicted[i]));

		if (max < t) max = t;
	}

	return max;
}

double EvaluationEngine::KS12() {
	double max = 0;

	for (int i = 0; i < curlen; i++) {
		double t = fabs(Fd(series[i]) - Fy(predicted[i]));

		if (max < t) max = t;
	}

	return max;
}

double EvaluationEngine::DA() {
	double ret = 0;

	for (int i = 0; i < curlen; i++) {
		if (Fd(predicted[i]) != 0 && Fd(predicted[curlen - 1 - i]) != 0)
			ret += (2 * i + 1) / curlen * ( log(Fd(predicted[i])) + log(Fd(predicted[curlen - 1 - i])));

		//char test[256];
		//sprintf(test, "aa %f %f %f\n", Fd(predicted[i]),(2*i+1)/curlen * ( log(Fd(predicted[i])) + log(Fd(predicted[curlen-1-i])))
		//	,ret
		//	);
		//FWhenLog(test);
	}

	return - curlen - ret;
}

void EvaluationEngine::Clear() {
	//for( int i=0;i<series_len;i++)
	//	series[i]=predicted[i]=0;
	//FWhenLog(String("curlen:%1\n").arg(curlen).toStdString().c_str());
	curlen = 0;
}