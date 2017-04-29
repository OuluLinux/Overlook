#pragma once

namespace Narx {

class EvaluationEngine : Moveable<EvaluationEngine> {
protected:
	int series_len;
	int curlen;

	Vector<double> series;
	Vector<double> predicted;



	double Fd(double val);
	double Fy(double val);


public:
	EvaluationEngine();
	~EvaluationEngine();
	
	void Init(int slen);
	void Insert(double target, double pred);

	double F3();
	double F1();
	double F2();
	double F4();

	double KS1();
	double KS2();
	double KS12();

	double DA();

	void Clear();
	
	void Serialize(Stream& s) {s % series_len % curlen % series % predicted;}
};

}
