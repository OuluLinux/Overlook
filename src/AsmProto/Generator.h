#ifndef _AsmProto_Generator_h_
#define _AsmProto_Generator_h_




struct AsmData : Moveable<AsmData> {
	double pres = 0;
};

struct Asm {
	//VectorMap<int, PricePressure> src;
	Vector<Vector<AsmData> > data;
	double low = 0, high = 0, step = 1;
	
	AsmData& Get(int iter, double d) {return data[iter][(d - low) / step];}
	
	void Init(int xsize, double l, double h, double s) {
		low = l;
		high = h;
		step = s;
		int count = (high - low) / step;
		data.SetCount(xsize);
		for(int i = 0; i < data.GetCount(); i++)
			data[i].SetCount(count);
	}
	
};

struct Generator {
	static const int data_count = 14400;
	static const int test_count = 500;
	static const int pattern_count = 6;
	
	//Vector<PricePressure> active_pressures;
	Asm a;
	double price = 1.0;
	double step = 0.0001;
	int iter = 0;
	double prev_price;
	Vector<double> data;
	Vector<Vector<int> > descriptors;
	Vector<Vector<Point> > pattern;
	
	
	// Indicators
	Vector<OnlineAverageWindow1> ma;
	Vector<double> prev_ma_mean;
	
	Generator();
	void InitMA();
	void RandomizePatterns();
	void AddRandomPressure();
	void AddMomentumPressure();
	void AddAntiPatternPressure();
	void AddMaPressure();
	void GenerateData(bool add_random, int count=0);
	void ReducePressure(double amount);
	void ApplyPressureChanges();
	int32 GetDescriptor(int pos, int pattern_id);
	
	
};





#endif
