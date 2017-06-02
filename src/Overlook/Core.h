#ifndef _Overlook_Core_h_
#define _Overlook_Core_h_

#include <CtrlUtils/CtrlUtils.h>
#include <CoreUtils/CoreUtils.h>

namespace Overlook {
using namespace Upp;

class Overlook;





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

struct CoreIO;

// Class for default visual settings for a single visible line of a container
struct Buffer : public Moveable<Buffer> {
	Vector<double> value;
	String label;
	Color clr;
	int style, line_style, line_width, chr, begin, shift, earliest_write;
	bool visible;
	
	Buffer() : clr(Black()), style(0), line_width(1), chr('^'), begin(0), shift(0), line_style(0), visible(true), earliest_write(INT_MAX) {}
	void Serialize(Stream& s) {s % value % label % clr % style % line_style % line_width % chr % begin % shift % visible;}
	void SetCount(int i) {value.SetCount(i, 0.0);}
	
	int GetResetEarliestWrite() {int i = earliest_write; earliest_write = INT_MAX; return i;}
	int GetCount() const {return value.GetCount();}
	bool IsEmpty() const {return value.IsEmpty();}
	double GetUnsafe(int i) const {return value[i];}
	
	#ifdef flagDEBUG
	CoreIO* check_cio;
	void SafetyCheck(CoreIO* io) {check_cio = io;}
	double Get(int i) const;
	void Set(int i, double value);
	void Inc(int i, double value);
	#else
	double Get(int i) const {return value[i];}
	void Set(int i, double value) {this->value[i] = value; if (i < earliest_write) earliest_write = i;}
	void Inc(int i, double value) {this->value[i] += value;}
	#endif
	
	
};

typedef const Buffer ConstBuffer;


// Class for registering input and output types of values of classes
struct In : public ValueBase {
	In(int phase, int type, int scale) {this->phase = phase; this->type = type; this->scale = scale; data_type = IN_;}
};

struct InOptional : public ValueBase {
	InOptional(int phase, int type, int scale) {this->phase = phase; this->type = type; this->scale = scale; data_type = INOPT_;}
};

typedef bool (*FilterFunction)(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf);
struct InDynamic : public ValueBase {
	InDynamic(int phase, int type, FilterFunction fn) {this->phase = phase; this->type = type; this->scale = scale; data_type = INDYN_; data = (void*)fn;}
};

struct InHigherPriority : public ValueBase {
	InHigherPriority() {data_type = INHIGHPRIO_; data = NULL;}
	InHigherPriority(FilterFunction fn) {data_type = INHIGHPRIO_; data = (void*)fn;}
};

struct Out : public ValueBase {
	Out(int phase, int type, int scale, int count=0, int visible=0) {this->phase = phase; this->type = type; this->scale = scale; this->count = count; this->visible = visible; data_type = OUT_;}
};

struct Arg : public ValueBase {
	Arg(const char* key, bool& value)	{s0 = key; data = &value; data_type = BOOL_;}
	Arg(const char* key, int& value)	{s0 = key; data = &value; data_type = INT_;}
	Arg(const char* key, double& value)	{s0 = key; data = &value; data_type = DOUBLE_;}
	Arg(const char* key, Time& value)	{s0 = key; data = &value; data_type = TIME_;}
	Arg(const char* key, String& value)	{s0 = key; data = &value; data_type = STRING_;}
};

struct Persistent : public ValueBase, Moveable<Persistent> {
	Persistent(bool& b)					{data = &b; data_type = PERS_BOOL_;}
	Persistent(int& i)					{data = &i; data_type = PERS_INT_;}
	Persistent(double& d)				{data = &d; data_type = PERS_DOUBLE_;}
	Persistent(VectorMap<int,int>& m)	{data = &m; data_type = PERS_INTMAP_;}
	Persistent(QueryTable& q)			{data = &q; data_type = PERS_QUERYTABLE_;}
};

struct ArgChanger : public ValueRegister {
	ArgChanger() : cursor(0), storing(0) {}
	
	virtual void IO(const ValueBase& base) {
		if (!storing) {
			keys.SetCount(cursor+1);
			keys[cursor] = base.s0;
			args.SetCount(cursor+1);
			if (base.data_type == ValueBase::BOOL_)
				args[cursor++] = *(bool*)base.data;
			else if (base.data_type == ValueBase::INT_)
				args[cursor++] = *(int*)base.data;
			else if (base.data_type == ValueBase::DOUBLE_)
				args[cursor++] = *(double*)base.data;
			else if (base.data_type == ValueBase::TIME_)
				args[cursor++] = *(Time*)base.data;
			else if (base.data_type == ValueBase::STRING_)
				args[cursor++] = *(String*)base.data;
		} else {
			if (base.data_type == ValueBase::BOOL_)
				*(bool*)base.data = args[cursor++];
			else if (base.data_type == ValueBase::INT_)
				*(int*)base.data = args[cursor++];
			else if (base.data_type == ValueBase::DOUBLE_)
				*(double*)base.data = args[cursor++];
			else if (base.data_type == ValueBase::TIME_)
				*(Time*)base.data = args[cursor++];
			else if (base.data_type == ValueBase::STRING_)
				*(String*)base.data = args[cursor++];
		}
	}
	void SetLoading() {storing = false; cursor = 0;}
	void SetStoring() {storing = true;  cursor = 0;}
	
	Vector<Value> args;
	Vector<String> keys;
	int cursor;
	bool storing;
};

class BaseSystem;

struct CoreIO : public ValueRegister, public Pte<CoreIO> {
	typedef Ptr<CoreIO> CoreIOPtr;
	
	struct Output : Moveable<Output> {
		Output() : visible(0) {}
		Vector<Buffer> buffers;
		int phase, type, visible;
	};
	typedef Tuple4<CoreIOPtr, Output*, int, int> Source;
	struct Input : Moveable<Input> {
		Input() {}
		void operator=(const Input& in) {sources <<= in.sources;}
		VectorMap<int, Source> sources;
	};
	Vector<Input> inputs, optional_inputs;
	Vector<Output> outputs;
	Vector<Buffer*> buffers;
	Vector<Persistent> persistents;
	BaseSystem* base;
	String unique;
	int factory;
	int sym_id, tf_id;
	int counted, bars;
	
	typedef const Output ConstOutput;
	typedef const Input  ConstInput;
	
	#ifdef flagDEBUG
	int read_safety_limit;
	void SafetyCheck(int i) {ASSERT(i <= read_safety_limit);}
	void SetSafetyLimit(int i) {read_safety_limit = i;}
	ConstBuffer& SafetyBuffer(ConstBuffer& cb) const {Buffer& b = (Buffer&)cb; b.SafetyCheck((CoreIO*)this); return cb;}
	Buffer& SafetyBuffer(Buffer& b) {b.SafetyCheck((CoreIO*)this); return b;}
	#else
	void SafetyCheck(int i) const {}
	void SetSafetyLimit(int i) const {}
	#endif
	
	CoreIO();
	virtual ~CoreIO();
	
	void StoreCache();
	void LoadCache();
	
	virtual void IO(const ValueBase& base);
	void RefreshBuffers();
	
	template <class T> T* Get() {
		T* t = dynamic_cast<T*>(this);
		if (t) return t;
		for(int i = 0; i < inputs.GetCount(); i++) {
			ConstInput& in = inputs[i];
			for(int j = 0; j < in.sources.GetCount(); j++) {
				CoreIO* c = in.sources[j].a;
				ASSERT(c);
				T* t = dynamic_cast<T*>(c);
				if (t) return t;
				t = c->Get<T>();
				if (t) return t;
			}
		}
		return NULL;
	}
	double GetBufferValue(int i, int shift) {return buffers[i]->value[shift];}
	double GetBufferValue(int shift) {return outputs[0].buffers[0].value[shift];}
	int GetBufferStyle(int i) {return buffers[i]->style;}
	int GetBufferArrow(int i) {return buffers[i]->chr;}
	int GetBufferLineWidth(int i) {return buffers[i]->line_width;}
	int GetBufferType(int i) {return buffers[i]->line_style;}
	Color GetBufferColor(int i) {return buffers[i]->clr;}
	int GetBufferCount() {return buffers.GetCount();}
	Buffer& GetBuffer(int buffer) {return SafetyBuffer(*buffers[buffer]);}
	ConstBuffer& GetBuffer(int buffer) const {return SafetyBuffer(*buffers[buffer]);}
	ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const {return SafetyBuffer(inputs[input].sources.Get(sym * 100 + tf).b->buffers[buffer]);}
	CoreIO* GetInputCore(int input, int sym, int tf) const {return inputs[input].sources.Get(sym * 100 + tf).a;}
	Output& GetOutput(int output) {return outputs[output];}
	ConstOutput& GetOutput(int output) const {return outputs[output];}
	int GetOutputCount() const {return outputs.GetCount();}
	BaseSystem& GetBaseSystem() {return *base;}
	const BaseSystem& GetBaseSystem() const {return *base;}
	inline const CoreIO& GetInput(int input, int sym, int tf) const {return *inputs[input].sources.Get(sym * 100 + tf).a;}
	String GetCacheDirectory() const;
	
	void AddInput(int input_id, int sym_id, int tf_id, CoreIO& core, int output_id);
	void SetBufferColor(int i, Color c) {buffers[i]->clr = c;}
	void SetBufferLineWidth(int i, int line_width) {buffers[i]->line_width = line_width;}
	void SetBufferType(int i, int style) {buffers[i]->line_style = style;}
	void SetBufferStyle(int i, int style) {buffers[i]->style = style;}
	void SetBufferShift(int i, int shift) {buffers[i]->shift = shift;}
	void SetBufferBegin(int i, int begin) {buffers[i]->begin = begin;}
	void SetBufferArrow(int i, int chr)   {buffers[i]->chr = chr;}
	void SetUnique(const String& s) {unique = s;}
	void SetSymbol(int i) {sym_id = i;}
	void SetTimeframe(int i) {tf_id = i;}
	
};

typedef Ptr<CoreIO> CoreIOPtr;

struct ArgumentBase {
	void Arg(const char* var, bool& v) {}
	void Arg(const char* var, int& v, int min=0, int max=0, int step=0) {}
	void Arg(const char* var, double& v, double min=0.0, double max=0.0, double step=0.0) {}
	void Arg(const char* var, Time& v, Time min=Time(1970,1,1), Time max=Time(2070,1,1), int step=0) {}
	void Arg(const char* var, String& v) {}
};

class BarData;

class Core : public CoreIO {
	
	// Settings
	String short_name;
	
	
	// Visual settings
	Array<Core> subcores;
	Vector<int> subcore_factories;
	Vector<DataLevel> levels;
	Color levels_clr;
	RWMutex lock;
	double minimum, maximum;
	double point;
	int levels_style;
	int window_type;
	int next_count;
	int period;
	int end_offset;
	bool has_maximum, has_minimum;
	bool skip_setcount;
	bool skip_allocate;
	
protected:
	
	Core();
	
public:
	
	virtual ~Core();
	
	virtual void Arguments(ArgumentBase& args) {}
	virtual void Init() {}
	virtual void Deinit() {}
	virtual void Start() {}
	virtual void IO(ValueRegister& reg) {Panic("Never here");}
	virtual void Serialize(Stream& s) {
		/*s % short_name % counted % levels % outputs
		  % levels_clr % minimum % maximum % point % levels_style
		  % window_type % bars % next_count % has_maximum % has_minimum
		  % skip_setcount;
		if (s.IsLoading()) {
			RefreshBuffers();
		}*/
	}
	void InitAll();
	template <class T> Core& AddSubCore()  {
		ASSERT_(subcore_factories.Add(Factory::Find<T>()) != -1, "This class is not registered to the factory");
		return subcores.Add(new T);}
	Core& At(int i) {return subcores[i];}
	Core& Set(String key, Value value);
	
	
	// Get settings
	int GetBars() {return bars;}
	int GetWindowType() {return window_type;}
	int GetCounted() {return counted;}
	double GetPoint() const {return point;}
	int GetCoreLevelCount() const {return levels.GetCount();}
	int GetCoreLevelType(int i) const {return levels[i].style;}
	int GetCoreLevelLineWidth(int i) const {return levels[i].line_width;}
	double GetMaximum() const {return maximum;}
	double GetMinimum() const {return minimum;}
	double GetCoreLevelValue(int i) const {return levels[i].value;}
	double GetCoreMinimum() {return minimum;}
	double GetCoreMaximum() {return maximum;}
	bool IsCoreSeparateWindow() {return window_type == WINDOW_SEPARATE;}
	bool HasMaximum() const {return has_maximum;}
	bool HasMinimum() const {return has_minimum;}
	int GetMinutePeriod();
	int GetTimeframe() const {return tf_id;}
	int GetTf() const {return tf_id;}
	int GetSymbol() const {return sym_id;}
	int GetPeriod() const;
	int GetVisibleCount() const {return outputs[0].visible;}
	inline ConstBuffer& GetInputBuffer(int input, int buffer) const {return CoreIO::GetInputBuffer(input, GetSymbol(), GetTimeframe(), buffer);}
	inline ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const {return CoreIO::GetInputBuffer(input, sym, tf, buffer);}
	BarData* GetBarData();
	
	
	// Set settings
	void SetTimeframe(int i, int period);
	void SetWindowType(int i) {window_type = i;}
	void SetPoint(double d);
	void SetCoreLevelCount(int count) {levels.SetCount(count);}
	void SetCoreLevel(int i, double value) {levels[i].value = value;}
	void SetCoreLevelType(int i, int style) {levels[i].style = style;}
	void SetCoreLevelLineWidth(int i, int line_width) {levels[i].line_width = line_width;}
	void SetCoreLevelsColor(Color clr) {levels_clr = clr;}
	void SetCoreLevelsStyle(int style) {levels_style = style;}
	void SetCoreMinimum(double value) {minimum = value; has_minimum = true;}
	void SetCoreMaximum(double value) {maximum = value; has_maximum = true;}
	void SetCoreChartWindow() {window_type = WINDOW_CHART;}
	void SetCoreSeparateWindow() {window_type = WINDOW_SEPARATE;}
	void ForceSetCounted(int i) {counted = i; next_count = i;}
	void SetSkipSetCount(bool b=true) {skip_setcount = b;}
	void SetBufferLabel(int i, const String& s) {}
	void SetEndOffset(int i) {ASSERT(i > 0); end_offset = i;}
	void SetSkipAllocate(bool b=true) {skip_allocate = b;}
	
	// Visible main functions
	void Refresh();
	void ClearContent();
	void RefreshIO() {IO(*this);}
	
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
	
};


class BarData : public Core {
	
};



}

#endif
