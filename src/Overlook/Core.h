#ifndef _Overlook_Core_h_
#define _Overlook_Core_h_


namespace Overlook {
using namespace Upp;

class Overlook;

// Visual setting enumerators
enum {DRAW_LINE, DRAW_SECTION, DRAW_HISTOGRAM, DRAW_ARROW, DRAW_NONE};
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

// Class for registering input and output types of values of classes
typedef bool (*FilterFunction)(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf);

inline bool SymTfFilter(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
	if (match_tf)
		return in_tf == out_tf;
	else
		return in_sym == out_sym;
}

inline bool AnyTf(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
	if (match_tf)
		return true;
	else
		return in_sym == out_sym;
}

// Classes for IO arguments
template <class T>
struct In : public ValueBase {
	In(FilterFunction fn, ArgsFn args=NULL)	{data_type = IN_; data = (void*)fn;			 factory = System::Find<T>(); data2 = (void*)args;}
	In(ArgsFn args)							{data_type = IN_; data = (void*)SymTfFilter; factory = System::Find<T>(); data2 = (void*)args;}
	In()									{data_type = IN_; data = (void*)SymTfFilter; factory = System::Find<T>(); data2 = NULL;}
};

struct InOptional : public ValueBase {
	InOptional() {data_type = INOPT_;}
};

struct Out : public ValueBase {
	Out(int count, int visible) {this->count = count; this->visible = visible; data_type = OUT_;}
};

struct Lbl : public ValueBase {
	Lbl(int count) {this->count = count; data_type = LBL_;}
};

struct Arg : public ValueBase {
	Arg(const char* key, int& value, int min, int max=10000) {s0 = key; data = &value; data_type = INT_; this->min = min; this->max = max;}
};

struct Persistent : public ValueBase {
	Callback1<Stream&> serialize;
	
protected:
	Persistent() {data_type = PERS_;}
	
public:
	Persistent(Callback1<Stream&> serialize) : serialize(serialize) {data_type = PERS_;}
	Persistent(const Persistent& src) {*this = src;}
	virtual ~Persistent() {}
	
	void operator = (const Persistent& base) {
		serialize = base.serialize;
		ValueBase::operator=(base);
	}
	
	virtual void Serialize(Stream& s) {serialize(s);}
};


// The "easy" method for callbacks with partially template arguments and partly with constant
// arguments. It requires one single global object, but that's not too much to ask.
// Some purist would go with some static function method, but I don't have a clue what it is.
struct StreamSerializer {
	typedef StreamSerializer CLASSNAME;
	template <class T> void ItemSerialize(Stream& s, T* obj) {s % *obj;}
	template <class T> Callback1<Stream&> GetSerializer(T& obj) {
		return THISBACK1(ItemSerialize<T>, &obj);
	}
};

inline StreamSerializer& GetStreamSerializer() {return Single<StreamSerializer>();}

template <class T> inline Persistent Mem(T& t) {
	return Persistent(GetStreamSerializer().GetSerializer(t));
}


// Utility function for changing class arguments
struct ArgChanger : public ValueRegister {
	ArgChanger() : cursor(0), storing(0) {}
	
	virtual void IO(const ValueBase& base) {
		if (!storing) {
			keys.SetCount(cursor+1);
			keys[cursor] = base.s0;
			args.SetCount(cursor+1);
			if (base.data_type == ValueBase::INT_)
				args[cursor++] = *(int*)base.data;
		} else {
			if (base.data_type == ValueBase::INT_)
				*(int*)base.data = args[cursor++];
		}
	}
	void SetLoading() {storing = false; cursor = 0;}
	void SetStoring() {storing = true;  cursor = 0;}
	
	Vector<Value> args;
	Vector<String> keys;
	int cursor;
	bool storing;
};

struct ArgPtr : Moveable<ArgPtr> {
	int* ptr = NULL;
	int min = 0, max = 0;
};

class CoreIO : public ValueRegister, public Pte<CoreIO> {
	
protected:
	friend class System;
	friend class Buffer;
	friend class Job;
	
	typedef Ptr<CoreIO> CoreIOPtr;
	
	Vector<Input> inputs;
	Vector<Output> outputs;
	Vector<Label> labels;
	Vector<ArgPtr> args;
	Vector<Buffer*> buffers;
	Array<Job> jobs;
	Array<Persistent> persistents;
	Job* current_job = NULL;
	JobThread* current_thrd = NULL;
	SpinLock serialization_lock, refresh_lock;
	String cache_dir;
	int sym_id, tf_id, factory, hash;
	int counted, bars;
	int db_src;
	bool serialized;
	bool is_init = false;
	
	typedef const Output ConstOutput;
	typedef const Input  ConstInput;
	
	// Some utility functions for checking that indicator values are strictly L-R
	#if defined flagDEBUG && defined flagSAFETYLIMITS
	#define SAFETYASSERT(x) ASSERT(x)
	int read_safety_limit;
	void SafetyInspect(int i) {ASSERT(i <= read_safety_limit);}
	void SetSafetyLimit(int i) {read_safety_limit = i;}
	ConstBuffer& SafetyBuffer(ConstBuffer& cb) const {Buffer& b = (Buffer&)cb; b.SafetyInspect((CoreIO*)this); return cb;}
	Buffer& SafetyBuffer(Buffer& b) {b.SafetyInspect((CoreIO*)this); return b;}
	#else
	#define SAFETYASSERT(x)
	void SafetyInspect(int i) const {}
	void SetSafetyLimit(int i) const {}
	Buffer& SafetyBuffer(Buffer& cb) const {return cb;}
	ConstBuffer& SafetyBuffer(ConstBuffer& cb) const {return cb;}
	#endif
	
public:
	CoreIO();
	virtual ~CoreIO();
	
	void StoreCache();
	void LoadCache();
	void Put(Stream& out, const String& dir, int subcore_id);
	void Get(Stream& in, const String& dir, int subcore_id);
	
	virtual void IO(const ValueBase& base);
	void RefreshBuffers();
	
	template <class T> T* Get() {
		T* t = dynamic_cast<T*>(this);
		if (t) return t;
		for(int i = 0; i < inputs.GetCount(); i++) {
			ConstInput& in = inputs[i];
			for(int j = 0; j < in.GetCount(); j++) {
				CoreIO* c = in[j].core;
				if (c == this)
					continue;
				ASSERT(c);
				T* t = dynamic_cast<T*>(c);
				if (t) return t;
				t = c->Get<T>();
				if (t) return t;
			}
		}
		return NULL;
	}
	
	Buffer& GetBuffer(int buffer) {return SafetyBuffer(*buffers[buffer]);}
	ConstBuffer& GetBuffer(int buffer) const {return SafetyBuffer(*buffers[buffer]);}
	ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const;
	ConstLabelSignal& GetInputLabel(int input, int sym, int tf, int buf=0) const;
	CoreIO* GetInputCore(int input, int sym, int tf) const;
	CoreIO* GetInputCore(int input) const;
	Output& GetOutput(int output) {return outputs[output];}
	ConstOutput& GetOutput(int output) const {return outputs[output];}
	ConstLabel& GetLabel(int lbl) const {return labels[lbl];}
	ConstLabelSignal& GetLabelBuffer(int lbl, int buf) const {return labels[lbl].buffers[buf];}
	LabelSignal& GetLabelBuffer(int lbl, int buf) {return labels[lbl].buffers[buf];}
	const CoreIO& GetInput(int input, int sym, int tf) const;
	int GetTimeframe() const {return tf_id;}
	int GetTf() const {return tf_id;}
	int GetSymbol() const {return sym_id;}
	String GetCacheDirectory();
	Color GetBufferColor(int i) {return buffers[i]->clr;}
	double GetBufferValue(int i, int shift) {return buffers[i]->value[shift];}
	double GetBufferValue(int shift) {return outputs[0].buffers[0].value[shift];}
	int GetBufferStyle(int i) {return buffers[i]->style;}
	int GetBufferArrow(int i) {return buffers[i]->chr;}
	int GetBufferLineWidth(int i) {return buffers[i]->line_width;}
	int GetBufferType(int i) {return buffers[i]->line_style;}
	int GetBufferCount() {return buffers.GetCount();}
	int GetOutputCount() const {return outputs.GetCount();}
	int GetLabelCount() const {return labels.GetCount();}
	int GetLabelBufferCount(int l) const {return labels[l].buffers.GetCount();}
	int GetFactory() const {return factory;}
	bool IsInitialized() const {return is_init;}
	
	void SetInput(int input_id, int sym_id, int tf_id, CoreIO& core, int output_id);
	void SetBufferColor(int i, Color c) {buffers[i]->clr = c;}
	void SetBufferLineWidth(int i, int line_width) {buffers[i]->line_width = line_width;}
	void SetBufferType(int i, int style) {buffers[i]->line_style = style;}
	void SetBufferStyle(int i, int style) {buffers[i]->style = style;}
	void SetBufferShift(int i, int shift) {buffers[i]->shift = shift;}
	void SetBufferBegin(int i, int begin) {buffers[i]->begin = begin;}
	void SetBufferArrow(int i, int chr)   {buffers[i]->chr = chr;}
	void SetSymbol(int i) {sym_id = i;}
	void SetTimeframe(int i) {tf_id = i;}
	void SetFactory(int i) {factory = i;}
	void SetHash(int i) {hash = i;}
	
};

typedef Ptr<CoreIO> CoreIOPtr;

struct ArgumentBase {
	void Arg(const char* var, bool& v) {}
	void Arg(const char* var, int& v, int min=0, int max=0, int step=0) {}
	void Arg(const char* var, double& v, double min=0.0, double max=0.0, double step=0.0) {}
	void Arg(const char* var, Time& v, Time min=Time(1970,1,1), Time max=Time(2070,1,1), int step=0) {}
	void Arg(const char* var, String& v) {}
};

class DataBridge;

class Core : public CoreIO {
	
protected:
	friend class CoreIO;
	
	// Settings
	String short_name;
	
	
	// Visual settings
	Array<Core> subcores;
	Vector<int> subcore_factories;
	Vector<DataLevel> levels;
	Color levels_clr;
	double minimum, maximum;
	int levels_style;
	int window_type = WINDOW_CHART;
	int next_count;
	int period;
	int end_offset;
	int future_bars;
	bool has_maximum, has_minimum;
	bool skip_setcount;
	bool skip_allocate;
	bool has_heatmap = false;
	bool avoid_refresh = false;
	bool ready = false;
	
	Core();
	
public:
	
	virtual ~Core();
	
	virtual void Arguments(ArgumentBase& args) {}
	virtual void Init() {}
	virtual void Deinit() {}
	virtual void Start() {}
	virtual void IO(ValueRegister& reg) = 0;
	virtual double GetHeatmapValue(int i, double price) {return 0;}
	
	void InitAll();
	void AllowJobs();
	template <class T> Core& AddSubCore()  {
		int i = System::Find<T>();
		ASSERT_(i != -1, "This class is not registered to the factory");
		subcore_factories.Add(i);
		return subcores.Add(new T);
	}
	void ResetSubCores();
	Core& At(int i) {return subcores[i];}
	Core& Set(String key, Value value);
	
	
	// Get settings
	int GetBars() {return bars;}
	int GetWindowType() {return window_type;}
	int GetCounted() {return counted;}
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
	int GetPeriod() const;
	int GetVisibleCount() const {return outputs[0].visible;}
	int GetFutureBars() const {return future_bars;}
	bool IsHeatmap() {return has_heatmap;}
	inline ConstBuffer& GetInputBuffer(int input, int buffer) const {return CoreIO::GetInputBuffer(input, GetSymbol(), GetTimeframe(), buffer);}
	inline ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const {return CoreIO::GetInputBuffer(input, sym, tf, buffer);}
	inline ConstLabelSignal& GetInputLabel(int input) const {return CoreIO::GetInputLabel(input, GetSymbol(), GetTimeframe());}
	DataBridge* GetDataBridge();
	bool IsJobsFinished() const;
	bool IsDependencyJobsFinished() const;
	Job& GetCurrentJob() {return *current_job;}
	JobThread& GetCurrentThread() {return *current_thrd;}
	
	
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
	void SetFutureBars(int i) {future_bars = i;}
	Job& SetJob(int i, String job_title);
	Job& GetJob(int i);
	void SetJobFinished(bool b=true);
	void SetJobCount(int i) {jobs.SetCount(i);}
	void EnterJob(Job* job, JobThread* thrd) {current_job = job; current_thrd = thrd;}
	void LeaveJob() {current_job = NULL; current_thrd = NULL;}
	void SetHeatmap(bool b=true) {has_heatmap=b;}
	void SetAvoidRefresh(bool b=true) {avoid_refresh = b;}
	void Ready() {ready = true;}
	
	// Visible main functions
	void Refresh();
	void RefreshSubCores();
	void RefreshSources();
	void RefreshSourcesOnlyDeep();
	void ClearContent();
	void RefreshIO() {IO(*this);}
	void InitSubCores();
	
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



struct ArgScript {
	Vector<String> titles;
	Vector<int> mins, maxs, steps;
	Vector<int*> args;
	
	void Add(String title, int min, int max, int step, int& arg) {titles.Add(title); mins.Add(min); maxs.Add(max); steps.Add(step); args.Add(&arg);}
};

class ScriptCore {
	
protected:
	friend class System;
	friend class ScriptAutomationCtrl;
	
	
	// Persistent
	
	
	// Temporary
	Vector<ScriptCore*> input_cores;
	Vector<int> args;
	String qtf_test_result;
	int factory = -1;
	int actual = 0, total = 1;
	
	
	ScriptCore& GetInputCore(int i) {return *input_cores[i];}
	
public:
	
	#ifdef flagDEBUG
	static const int fast_tf = 4;
	#else
	static const int fast_tf = 2;
	#endif
	
	ScriptCore();
	virtual void Init() {};
	virtual void Run() {};
	virtual void Arg(ArgScript& arg) = 0;
	virtual void SerializeEvent(Stream& s) {}
	virtual String GetTitle() = 0;
	virtual void GetSignal(int symbol, LabelSignal& signal) {Panic("Not implemented");}
	
	void Load();
	void Store();
	void Serialize(Stream& s) {SerializeEvent(s);}
	
	void SetInputCore(int i, ScriptCore& c) {if (i >= input_cores.GetCount()) input_cores.SetCount(i+1, NULL); input_cores[i] = &c;}
	
	int GetActual() const {return actual;}
	int GetTotal() const {return total;}
	String GetTestResultQTF() const {return qtf_test_result;}
};








}

#endif
