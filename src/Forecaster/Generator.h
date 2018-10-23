#ifndef _Forecaster_Generator_h_
#define _Forecaster_Generator_h_

namespace Forecast {


struct AsmData : Moveable<AsmData> {
	double pres = 0;
};


struct Asm {
	static constexpr double low = 0.0;
	static constexpr double high = 2.0;
	static constexpr double step = 0.0001;
	static const int size = (high - low) / step;
	
	AsmData data[size];
	
	AsmData& Get(double d) {int i = (d - low) / step; return data[i];}
	
	
	Asm() {
		Reset();
	}
	void Reset() {
		for(int i = 0; i < size; i++)
			data[i].pres = 0;
	}
};

struct AmpPoint {
	double x, y;
};

struct Generator {
	static const int data_count = 1440*4.5;
	static const int test_count = 500;
	
	Asm a;
	double price = 1.0, prev_price = 1.0;
	const double step = 0.0001;
	int iter = 0;
	double err = 0.0;
	
	
	
	Generator();
	void AddRandomPressure();
	void RandomizePatterns();
	void RefreshPrice();
	void ResetGenerate();
	void GenerateTest(BitStream& stream, const Vector<double>& real_data, const Vector<double>& params);
	void RealPrice(const Vector<double>& real_data);
	void Iteration(BitStream& stream, int i, const Vector<double>& params);
	void ApplyPressureChanges();
	void ApplyIndicatorPressures(BitStream& stream, const Vector<double>& params);
	
};



}

#endif
