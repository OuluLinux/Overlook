#ifndef _Forecaster_HeatmapLooper_h_
#define _Forecaster_HeatmapLooper_h_

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

struct HeatmapLooper {
	
	
	// Persistent
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
	Vector<double> forecast_tmp;
	Mutex lock, view_lock;
	int actual = 0, total = 1;
	int forecast_read_pos = 0;
	bool debug = false;
	
	
	HeatmapLooper();
	void Init(double point, const Vector<double>& real_data);
	
	void RealPrice();
	void ResetPattern();
	void RefreshDescriptor();
	void CalculateError();
	void AddRandomPressure();
	void RefreshForecast();
	void ApplyPressureChanges();
	void ApplyIndicatorPressures();
	void GetSampleInput(NNSample& s, int result_id);
	
};



struct OptResult : Moveable<OptResult> {
	Vector<double> params;
	String heatmap;
	double err;
	int id, gen_id;
	
	OptResult() {}
	OptResult(const OptResult& s) {*this = s;}
	
	void operator=(const OptResult& s) {
		id = s.id;
		gen_id = s.gen_id;
		err = s.err;
		heatmap = s.heatmap;
		params <<= s.params;
	}
	
	void Serialize(Stream& s) {s % params % heatmap % err % id % gen_id;}
	bool operator()(const OptResult& a, const OptResult& b) const {return a.err < b.err;}
	
};



struct MultiHeatmapLooper {
	VectorMap<int, NNSample> nnsamples;
	Array<HeatmapLooper> loopers;
	const Vector<double>* real_data = NULL;
	
	
	MultiHeatmapLooper();
	void Init(double point, const Vector<double>& real_data, const Vector<Vector<double> >& params, BitStream& stream);
	void Run(bool get_samples);
	void GetSampleInput(NNSample& s);
	void Clear();
};

}

#endif
