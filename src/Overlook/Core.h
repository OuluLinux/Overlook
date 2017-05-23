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


// Class for default visual settings for a single visible line of a container
struct Buffer : public Moveable<Buffer> {
	Vector<double> value;
	String label;
	Color clr;
	int style, line_style, line_width, chr, begin, shift;
	bool visible;
	
	Buffer() : clr(Black()), style(0), line_width(1), chr('^'), begin(0), shift(0), line_style(0), visible(true) {}
	void Serialize(Stream& s) {s % value % label % clr % style % line_style % line_width % chr % begin % shift % visible;}
	void Set(int i, double value) {this->value[i] = value;}
	void SetCount(int i) {value.SetCount(i, 0);}
	
	double Get(int i) const {return value[i];}
	int GetCount() const {return value.GetCount();}
	
};

typedef const Buffer ConstBuffer;


// Class for registering input and output types of values of classes
struct ValueBase {
	int phase, type, scale, count, visible, data_type;
	const char* s0;
	void* data;
	ValueBase() {phase=-1; type=-1; scale=-1; count=0; visible=0; s0=0; data=0; data_type = -1;}
	enum {IN_, INOPT_, OUT_, BOOL_, INT_, DOUBLE_, TIME_, STRING_};
};

struct In : public ValueBase {
	In(int phase, int type, int scale) {this->phase = phase; this->type = type; this->scale = scale; data_type = IN_;}
};
struct InOptional : public ValueBase {
	InOptional(int phase, int type, int scale) {this->phase = phase; this->type = type; this->scale = scale; data_type = INOPT_;}
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

struct ValueRegister {
	ValueRegister() {}
	
	virtual void IO(const ValueBase& base) = 0;
	virtual ValueRegister& operator % (const ValueBase& base) {IO(base); return *this;}
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

struct CoreIO : public ValueRegister {
	struct Output : Moveable<Output> {
		Output() : visible(0) {}
		Vector<Buffer> buffers;
		int visible;
		void Serialize(Stream& s) {s % buffers % visible;}
	};
	typedef Tuple4<CoreIO*, Output*, int, int> Source;
	struct Input : Moveable<Input> {
		Input() {}
		VectorMap<int, Source> sources;
	};
	Vector<Input> inputs, optional_inputs;
	Vector<Output> outputs;
	Vector<Buffer*> buffers;
	
	typedef const Output ConstOutput;
	typedef const Input  ConstInput;
	
	
	CoreIO() {}
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::IN_) {
			inputs.Add();
		}
		else if (base.data_type == ValueBase::OUT_) {
			Output& out = outputs.Add();
			out.buffers.SetCount(base.count);
			out.visible = base.visible;
			for(int i = 0; i < out.buffers.GetCount(); i++)
				buffers.Add(&out.buffers[i]);
		}
	}
	
	void RefreshBuffers() {
		buffers.SetCount(0);
		for(int j = 0; j < outputs.GetCount(); j++)
			for(int i = 0; i < outputs[j].buffers.GetCount(); i++)
				buffers.Add(&outputs[j].buffers[i]);
	}
	
	
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
	Buffer& GetBuffer(int buffer) {return *buffers[buffer];}
	ConstBuffer& GetBuffer(int buffer) const {return *buffers[buffer];}
	ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const {return inputs[input].sources.Get(sym * 100 + tf).b->buffers[buffer];}
	Output& GetOutput(int output) {return outputs[output];}
	ConstOutput& GetOutput(int output) const {return outputs[output];}
	int GetOutputCount() const {return outputs.GetCount();}
	
	
	void AddInput(int sym_id, int tf_id, int input_id, CoreIO& core, int output_id) {
		Input& in = inputs[input_id];
		if (core.GetOutputCount()) {
			in.sources.Add(sym_id * 100 + tf_id, Source(&core, &core.GetOutput(output_id), sym_id, tf_id));
		} else {
			in.sources.Add(sym_id * 100 + tf_id, Source(&core, NULL, sym_id, tf_id));
		}
	}
	void SetBufferColor(int i, Color c) {buffers[i]->clr = c;}
	void SetBufferLineWidth(int i, int line_width) {buffers[i]->line_width = line_width;}
	void SetBufferType(int i, int style) {buffers[i]->line_style = style;}
	void SetBufferStyle(int i, int style) {buffers[i]->style = style;}
	void SetBufferShift(int i, int shift) {buffers[i]->shift = shift;}
	void SetBufferBegin(int i, int begin) {buffers[i]->begin = begin;}
	
	
};

struct ArgumentBase {
	void Arg(const char* var, bool& v) {}
	void Arg(const char* var, int& v, int min=0, int max=0, int step=0) {}
	void Arg(const char* var, double& v, double min=0.0, double max=0.0, double step=0.0) {}
	void Arg(const char* var, Time& v, Time min=Time(1970,1,1), Time max=Time(2070,1,1), int step=0) {}
	void Arg(const char* var, String& v) {}
};

class Core : public CoreIO {
	
	// Settings
	String short_name;
	int counted;
	
	
	// Visual settings
	Vector<DataLevel> levels;
	Color levels_clr;
	RWMutex lock;
	double minimum, maximum;
	double point;
	int levels_style;
	int window_type;
	int bars, next_count;
	int sym_id, tf_id;
	int period;
	int visible_count;
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
		s % short_name % counted % levels % outputs
		  % levels_clr % minimum % maximum % point % levels_style
		  % window_type % bars % next_count % has_maximum % has_minimum
		  % skip_setcount;
		if (s.IsLoading()) {
			RefreshBuffers();
		}
	}
	
	
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
	int GetSymbol() const {return sym_id;}
	int GetPeriod() const;
	int GetVisibleCount() const {return visible_count;}
	inline ConstBuffer& GetInputBuffer(int input, int buffer) const {return CoreIO::GetInputBuffer(input, GetSymbol(), GetTimeframe(), buffer);}
	inline ConstBuffer& GetInputBuffer(int input, int sym, int tf, int buffer) const {return CoreIO::GetInputBuffer(input, sym, tf, buffer);}
	
	// Set settings
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
	void ForceSetCounted(int i) {counted = i; bars = i; next_count = i;}
	void SetSkipSetCount(bool b=true) {skip_setcount = b;}
	void SetSymbol(int i) {sym_id = i;}
	void SetTimeframe(int i, int period);
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
	
	virtual void RefreshSource();
	
};


class BarData : public Core {
	
};



}

#endif
