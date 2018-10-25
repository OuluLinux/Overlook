#ifndef _Forecaster_Generator_h_
#define _Forecaster_Generator_h_

namespace Forecast {

static const int APPLYPRESSURE_PARAMS = 1+5;
static const int INDIPRESSURE_PARAMS = 4;

struct AsmData : Moveable<AsmData> {
	double pres = 0;
	
	void Serialize(Stream& s) {s % pres;}
};


struct Asm {
	double low = 0.5;
	double high = 1.5;
	double step = 0.0001;
	int size = 0;
	
	Vector<AsmData> data;
	
	AsmData& Get(double d) {return data[GetPos(d)];}
	int GetPos(double d) {int i = (d - low) / step; return i;}

	Asm() {
		
	}
	void Init(int data_count, double low, double high, double step) {
		this->low = low;
		this->high = high;
		this->step = step;
		size = (high - low) / step;
		data.SetCount(0);
		data.SetCount(size);
	}
	
	void Serialize(Stream& s) {s % low % high % step % size % data;}
	
};

struct PressureDescriptor : Moveable<PressureDescriptor> {
	static const int size = 4;
	int64 descriptor[size];
	
	void Serialize(Stream& s) {for(int i = 0; i < size; i++) s % descriptor[i];}
};

struct Generator {
	
	enum {
		START,
		INITIALIZED,
		INDICATORS_REFRESHED,
		BITSTREAMED,
		ERROR_CALCULATED,
		FORECASTED
	};
	
	// Persistent
	Vector<FactoryDeclaration> decl;
	Vector<PressureDescriptor> descriptors;
	Vector<Point> pattern;
	BitStream stream;
	Heatmap image;
	Asm a;
	Vector<double> real_data;
	Vector<double> params;
	double price = 1.0, prev_price = 1.0;
	double forecast = 1.0, prev_forecast = 1.0;
	double err = 0.0;
	int state = 0;
	int iter = 0;
	
	
	// Temporary
	Vector<CoreItem> work_queue;
	Vector<ConstLabelSignal*> lbls;
	String bitstreamed;
	Mutex lock, view_lock;
	bool debug = false;
	
	
	Generator();
	void Init(double point, const Vector<double>& real_data, const Vector<double>& params);
	void GetBitStream();
	void GetLabels(Vector<CoreItem>& work_queue, Vector<ConstLabelSignal*>& lbls);
	void PushWarmup();
	void PopWarmup(const Vector<double>& params);
	bool DoNext();
	void RealPrice();
	void RefreshIndicators();
	void ResetPattern();
	void RefreshDescriptor();
	void CalculateError();
	void Forecast();
	void AddRandomPressure();
	void RandomizePatterns();
	void RefreshForecast();
	void Iteration(BitStream& stream, int i, const Vector<double>& params);
	void ApplyPressureChanges();
	void ApplyIndicatorPressures();
	void Serialize(Stream& s);
	
};



}

#endif
