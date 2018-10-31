#ifndef _Agent_Looper_h_
#define _Agent_Looper_h_

namespace Agent {

struct Looper {
	
	Vector<double>* real_data = NULL;
	Vector<int>* matches = NULL;
	Vector<bool>* is_corner = NULL;
	Vector<double> params;
	double equity = 0.0, point = 0.0;
	Mutex lock;
	
	Looper();
	void Init(double point, const Vector<double>& real_data);
	
	void Run();
};

}

#endif
