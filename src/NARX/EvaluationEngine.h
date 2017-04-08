#pragma once
class EvaluationEngine
{
protected:
	int series_len;
	int curlen;

	double *series;
	double *predicted;
	


	double Fd(double val);
	double Fy(double val);


public:
	EvaluationEngine(int slen);
	~EvaluationEngine(void);

	void insertvalue(double target, double pred);

	double F3();
	double F1();
	double F2();
	double F4();

	double KS1();
	double KS2();
	double KS12();

	double DA();

	void reset();
};

