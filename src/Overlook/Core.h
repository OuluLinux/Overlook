#ifndef _Overlook_Core_h_
#define _Overlook_Core_h_

#include <CtrlUtils/CtrlUtils.h>
#include <CoreUtils/CoreUtils.h>

namespace Overlook {
using namespace Upp;

class Overlook;




template <class T, bool FLT>
class DataBuffer {
	Vector<T> vec;
	double point;
public:
	DataBuffer() : point(0.01) {}
	void Serialize(Stream& s) {
		// This fast serialisation speeded one realm's loading: 5.434 -> 4.889
		#ifdef BIG_ENDIANG
			#error DataBuffer serialization is unimplemented.
		#endif
		if (s.IsStoring()) {
			int count = vec.GetCount();
			s.Put(&count, sizeof(int));
			if (count)
				s.Put(vec.Begin(), sizeof(T) * count);
		} else {
			int count;
			s.Get(&count, sizeof(int));
			ASSERT(count >= 0 && count < 1000000);
			vec.SetCount(count);
			if (count)
				s.Get(vec.Begin(), sizeof(T) * count);
		}
		s % point;
		
		// Use this for cleaner version, also big endian compatible
		//s % vec % point;
	}
	void Dump() {DUMP(vec);}
	
	//T& operator[] (int i) {return vec[i];}
	//const T& operator[] (int i) const {return vec[i];}
	
	void Set(int i, double d) {vec[i] = FLT ? d / point : d;}
	double Get(int i) const {return FLT ? vec[i] * point : vec[i];}
	
	void SetMax(int i, double d) {T& v = vec[i]; T t = FLT ? d / point : d; if (v < t) v = t;}
	void SetMin(int i, double d) {T& v = vec[i]; T t = FLT ? d / point : d; if (v > t) v = t;}
	void Inc(int i, double d) {T& v = vec[i]; T t = FLT ? d / point : d; v += t;}
	
	void Add(const T& value) {vec.Add(value);}
	void SetPoint(double d) {point = d;}
	void SetCount(int count) {vec.SetCount(count, 0);}
	void SetCount(int count, T value) {vec.SetCount(count, value);}
	void Clear() {vec.Clear();}
	
	int GetCount() const {return vec.GetCount();}
	double GetPoint() const {ASSERT(!FLT); return point;}
	
};

// The most used buffer classes
//typedef DataBuffer<double> DataDouble;
//typedef DataBuffer<char>   DataByte;
//typedef DataBuffer<uint16, false> Data16;
typedef DataBuffer<float, true> FloatVector;


// Visual setting enumerators
enum {DRAW_LINE, DRAW_SECTION, DRAW_HISTOGRAM, DRAW_ARROW, DRAW_ZIGZAG, DRAW_NONE};
enum {WINDOW_SEPARATE, WINDOW_CHART};
enum {STYLE_SOLID, STYLE_DASH, STYLE_DOT, STYLE_DASHDOT, STYLE_DASHDOTDOT};


// Class for vertical levels on containers
struct DataLevel : public Moveable<DataLevel> {
	int style, line_width;
	double value; // height
	Color clr;
	
	DataLevel() : value(0), style(0), line_width(1) {}
	DataLevel& operator=(const DataLevel& src) {value = src.value; clr = src.clr; style = src.style; line_width = src.line_width; return *this;}
	void Serialize(Stream& s) {s % style % line_width % value % clr;}
};


// Class for default visual settings for a single visible line of a container
struct DataBufferSettings : public Moveable<DataBufferSettings> {
	FloatVector* buffer;
	String label;
	Color clr;
	int style, line_style, line_width, chr, begin, shift;
	
	DataBufferSettings() : clr(Black()), style(0), line_width(1), chr('^'), begin(0), shift(0), line_style(0) {}
	void Serialize(Stream& s) {s % label % clr % style % line_style % line_width % chr % begin % shift;}
};

// Class for registering input and output types of values of classes
struct ValueRegister {
	ValueRegister() {}
	
	virtual void AddIn(int phase, int type, int scale, int count) = 0;
	virtual void AddInOptional(int phase, int type, int scale, int count) = 0;
	virtual void AddOut(int phase, int type, int scale, int count) = 0;
	
};

struct CoreIO : public ValueRegister {
	struct In : Moveable<In> {
		In() {core = NULL; output_id = -1;}
		Core* core;
		int output_id;
	};
	
	Vector<In> inputs;
	
	CoreIO() {}
	
	virtual void AddIn(int phase, int type, int scale, int count) {
		In& in = inputs.Add();
	}
	
	virtual void AddInOptional(int phase, int type, int scale, int count) {
		
	}
	
	virtual void AddOut(int phase, int type, int scale, int count) {
		
	}
	
	
	template <class T> T* Get() {
		for(int i = 0; i < inputs.GetCount(); i++) {
			Core* c = inputs[i].core;
			T* t = dynamic_cast<T*>(c);
			if (t) return t;
			t = c->Get<T>();
			if (t) return t;
		}
		return NULL;
	}
	
	
	void SetInput(int input_id, Core& core, int src_output_id) {
		In& in = inputs[input_id];
		in.core = &core;
		in.output_id = src_output_id;
	}
	
};

class Core : public CoreIO {
	
	// Settings
	Vector<FloatVector*> indices;
	String short_name;
	int counted;
	
	
	// Visual settings
	Vector<DataLevel> levels;
	Vector<DataBufferSettings> buffer_settings;
	Color levels_clr;
	RWMutex lock;
	double minimum, maximum;
	double point;
	int levels_style;
	int window_type;
	int bars, next_count;
	int sym_id, tf_id;
	bool has_maximum, has_minimum;
	bool skip_setcount;
	
protected:
	
	Core();
	
public:
	
	virtual ~Core();
	
	virtual void SetArguments(const VectorMap<String, Value>& args) {}
	virtual void Init() {}
	virtual void Deinit() {}
	virtual void Start() {}
	virtual void GetIO(ValueRegister& reg) {Panic("Never here");}
	virtual void Serialize(Stream& s) {
		s % short_name % counted % levels % buffer_settings
		  % levels_clr % minimum % maximum % point % levels_style
		  % window_type % bars % next_count % has_maximum % has_minimum
		  % skip_setcount;
	}
	
	// Get settings
	const String& GetBufferLabel(int i) {return buffer_settings[i].label;}
	int GetBars() {return bars;}
	int GetWindowType() {return window_type;}
	int GetCounted() {return counted;}
	double GetPoint() const {return point;}
	int GetCoreLevelCount() const {return levels.GetCount();}
	int GetCoreLevelType(int i) const {return levels[i].style;}
	int GetCoreLevelLineWidth(int i) const {return levels[i].line_width;}
	int GetBufferStyle(int i) {return buffer_settings[i].style;}
	int GetBufferArrow(int i) {return buffer_settings[i].chr;}
	double GetMaximum() const {return maximum;}
	double GetMinimum() const {return minimum;}
	double GetCoreLevelValue(int i) const {return levels[i].value;}
	double GetCoreMinimum() {return minimum;}
	double GetCoreMaximum() {return maximum;}
	bool IsCoreSeparateWindow() {return window_type == WINDOW_SEPARATE;}
	bool HasMaximum() const {return has_maximum;}
	bool HasMinimum() const {return has_minimum;}
	int GetBufferCount() {return buffer_settings.GetCount();}
	FloatVector& GetBuffer(int buffer) {return *buffer_settings[buffer].buffer;}
	const FloatVector& GetBuffer(int buffer) const {return *buffer_settings[buffer].buffer;}
	FloatVector& GetIndex(int buffer) {return *indices[buffer];}
	const DataBufferSettings& GetBufferSettings(int buffer) {return buffer_settings[buffer];}
	double GetBufferValue(int i, int shift) {return buffer_settings[i].buffer->Get(shift);}
	double GetBufferValue(int shift) {return buffer_settings[0].buffer->Get(shift);}
	Color GetBufferColor(int i) {return buffer_settings[i].clr;}
	int GetBufferLineWidth(int i) {return buffer_settings[i].line_width;}
	int GetBufferType(int i) {return buffer_settings[i].line_style;}
	int GetIndexCount() {return indices.GetCount();}
	double GetIndexValue(int i, int shift) {return indices[i]->Get(shift);}
	double GetIndexValue(int shift) {return indices[0]->Get(shift);}
	int GetMinutePeriod();
	
	
	
	// Set settings
	void SetWindowType(int i) {window_type = i;}
	void SetBufferCount(int count) {buffer_settings.SetCount(count);}
	void SetBufferColor(int i, Color c) {buffer_settings[i].clr = c;}
	void SetBufferLineWidth(int i, int line_width) {buffer_settings[i].line_width = line_width;}
	void SetBufferType(int i, int style) {buffer_settings[i].line_style = style;}
	//void SetCoreDigits(int digits) {this->digits = digits;}
	void SetPoint(double d);
	void SetCoreLevelCount(int count) {levels.SetCount(count);}
	void SetCoreLevel(int i, double value) {levels[i].value = value;}
	void SetCoreLevelType(int i, int style) {levels[i].style = style;}
	void SetCoreLevelLineWidth(int i, int line_width) {levels[i].line_width = line_width;}
	void SetCoreLevelsColor(Color clr) {levels_clr = clr;}
	void SetCoreLevelsStyle(int style) {levels_style = style;}
	void SetCoreMinimum(double value) {minimum = value; has_minimum = true;}
	void SetCoreMaximum(double value) {maximum = value; has_maximum = true;}
	void SetBufferLabel(int i, String label)  {buffer_settings[i].label = label;}
	void SetBufferStyle(int i, int style)     {buffer_settings[i].style = style;}
	void SetBufferArrow(int i, int chr)       {buffer_settings[i].chr   = chr;}
	void SetCoreChartWindow() {window_type = WINDOW_CHART;}
	void SetCoreSeparateWindow() {window_type = WINDOW_SEPARATE;}
	void SetBufferShift(int i, int shift) {buffer_settings[i].shift = shift;}
	void SetBufferBegin(int i, int begin) {buffer_settings[i].begin = begin;}
	void SetIndexCount(int count) {indices.SetCount(count);}
	void SetIndexBuffer(int i, FloatVector& vec);
	void ForceSetCounted(int i) {counted = i; bars = i; next_count = i;}
	void SetSkipSetCount(bool b=true) {skip_setcount = b;}
	void SetSymbol(int i) {sym_id = i;}
	void SetTimeframe(int i) {tf_id = i;}
	
	
	// Visible main functions
	void Refresh();
	void ClearContent();
	void RefreshIO() {GetIO(*this);}
	
protected:
	
	// Value data functions
	double GetAppliedValue ( int applied_value, int i );
	double Open(int shift);
	double High(int shift);
	double Low(int shift);
	double Volume(int shift);
	int HighestHigh(int period, int shift);
	int LowestLow(int period, int shift);
	int HighestOpen(int period, int shift);
	int LowestOpen(int period, int shift);
	
	virtual void RefreshSource();
	
};






}

#endif
