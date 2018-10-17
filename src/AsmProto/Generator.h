#ifndef _AsmProto_Generator_h_
#define _AsmProto_Generator_h_



enum {
	PP_LOW,
	PP_HIGH,
	PP_MIN,
	PP_MAX,
	PP_SIZE,
	PP_ACTION,
	//PP_ITER,
	PP_COUNT,
};

struct PricePressure : Moveable<PricePressure> {
	double low, high;
	double min, max;
	double size;
	bool action;
	int iter = -1;
	int id = -1;
	
	
	PricePressure& SetId(int i) {id = i; return *this;}
	
	bool operator()(const PricePressure& a, const PricePressure& b) const {
		if (a.iter < b.iter) return true;
		if (a.iter > b.iter) return false;
		if (a.size < b.size) return true;
		else return false;
	}
};

struct Asm {
	VectorMap<int, PricePressure> src;
	int id_counter = 0;
	int iter = 0;
	
	PricePressure& Add() {int id = id_counter++; return src.Add(id).SetId(id);}
	void Sort() {Upp::Sort(src, PricePressure());}
};

struct Generator {
	static const int data_count = 1000;
	
	Vector<PricePressure> active_pressures;
	Asm a;
	double price = 1.0;
	double step = 0.0001;
	int iter = 0;
	
	Generator();
	void AddRandomPressure();
	void Randomize(PricePressure& pp, double price, int iter);
	void GenerateData(Vector<double>& data, bool add_random, int count=0);
	void GetPricePressure(double price, double& buy_pres, double& sell_pres);
	void ReducePressure(double amount);
	void SimpleReducePressure(double amount);
	
};





#endif
