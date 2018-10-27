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
	int Find(double d) {int i = GetPos(d); if (i < 0 || i >= data.GetCount()) return -1; return i;}
	
	Asm() {
		
	}
	void Init(double low, double high, double step) {
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

struct NNSample : Moveable<NNSample> {
	static const int single_size = 50;
	static const int single_count = 10;
	static const int fwd_count = 240/10;
	
	double input[single_count][single_size];
	double output[fwd_count];
	
	NNSample() {output[0] = 0.0;}
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(&input, sizeof(input));
			s.Get(&output, sizeof(output));
		} else {
			s.Put(&input, sizeof(input));
			s.Put(&output, sizeof(output));
		}
	}
};

struct Generator {
	
	static const int errtest_size = 1440*5*4;
	enum {
		START,
		INITIALIZED,
		INDICATORS_REFRESHED,
		BITSTREAMED,
		ERROR_CALCULATED,
		ERROR_CALCULATED_FULLY,
	};
	
	// Persistent
	Vector<FactoryDeclaration> decl;
	Vector<PressureDescriptor> descriptors;
	Vector<Point> pattern;
	BitStream stream;
	Heatmap image;
	Asm a;
	const Vector<double>* real_data = NULL;
	Vector<double> params;
	double price = 1.0, prev_price = 1.0;
	double forecast = 1.0, prev_forecast = 1.0;
	double err = 0.0;
	int state = 0;
	int iter = 0;
	
	
	// Temporary
	VectorMap<int, NNSample>* nnsamples = NULL;
	Vector<CoreItem> work_queue;
	Vector<ConstLabelSignal*> lbls;
	Vector<double> forecast_tmp;
	String bitstreamed;
	Mutex lock, view_lock;
	int result_id = -1;
	int actual = 0, total = 1;
	int forecast_read_pos = 0;
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
	void CalculateError(bool limited);
	void AddRandomPressure();
	void RefreshForecast();
	void RefreshNNSample();
	void Iteration(BitStream& stream, int i, const Vector<double>& params);
	void ApplyPressureChanges();
	void ApplyIndicatorPressures();
	void Serialize(Stream& s);
	void GetSampleInput(NNSample& s);
	
};



}

#endif
