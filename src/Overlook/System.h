#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Overlook {
using namespace Upp;

void MaskBits(Vector<byte>& vec, int bit_begin, int bit_count);

struct RegisterInput : Moveable<RegisterInput> {
	int factory, input_type;
	void* data;
	RegisterInput(int fac, int intype, void* filter_fn) {factory=fac; input_type=intype; data=filter_fn;}
	RegisterInput(const RegisterInput& o) {factory=o.factory; input_type=o.input_type; data=o.data;}
	String ToString() const {return Format(" factory=%d input_type=%d data=%X", factory, input_type, (int64)data);}
};

enum {REGIN_NORMAL, REGIN_OPTIONAL};

struct ArgType : Moveable<ArgType> {
	ArgType() {}
	ArgType(const ArgType& src) {*this = src;}
	ArgType(String desc, int min, int max, int def) : desc(desc), min(min), max(max), def(def) {}
	void operator=(const ArgType& src) {
		desc = src.desc;
		max  = src.max;
		min  = src.min;
		def  = src.def;
	}
	
	String desc;
	int min, max, def;
};

struct OutputType : Moveable<OutputType> {
	OutputType() : count(0), visible(0) {}
	OutputType(int count, int visible) : count(count), visible(visible) {}
	OutputType(const OutputType& src) {*this = src;}
	void operator=(const OutputType& src) {count = src.count; visible = src.visible;}
	int count, visible;
};

struct InputSource : Moveable<InputSource> {
	InputSource() : factory(-1), output(-1) {}
	InputSource(int factory, int output) : factory(factory), output(output) {}
	InputSource(const InputSource& src) {*this = src;}
	void operator=(const InputSource& src) {factory = src.factory; output = src.output;}
	int factory, output;
};

struct FactoryRegister : public ValueRegister, Moveable<FactoryRegister> {
	Vector<RegisterInput> in;
	Vector<OutputType> out;
	Vector<ArgType> args;
	
	FactoryRegister() {}
	FactoryRegister(const FactoryRegister& src) {*this = src;}
	void operator=(const FactoryRegister& src) {
		in <<= src.in;
		out <<= src.out;
		args <<= src.args;
	}
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::IN_) {
			in.Add(RegisterInput(base.factory, REGIN_NORMAL, base.data));
		}
		else if (base.data_type == ValueBase::INOPT_) {
			in.Add(RegisterInput(base.factory, REGIN_OPTIONAL, base.data));
		}
		else if (base.data_type == ValueBase::OUT_) {
			out.Add(OutputType(base.count, base.visible));
		}
		else if (base.data_type == ValueBase::BOOL_) {
			args.Add(ArgType(base.s0, 0, 1, *(bool*)base.data));
		}
		else if (base.data_type == ValueBase::INT_) {
			args.Add(ArgType(base.s0, base.min, base.max, *(int*)base.data));
		}
	}
};

struct PipelineItem : Moveable<PipelineItem> {
	typedef PipelineItem CLASSNAME;
	PipelineItem() {priority = INT_MAX;}
	
	Vector<byte> value;
	int priority;
};

class System {
	
protected:

	typedef Core*			(*CoreFactoryPtr)();
	typedef CustomCtrl*		(*CtrlFactoryPtr)();
	typedef Tuple3<String, CoreFactoryPtr, CtrlFactoryPtr> CoreCtrlSystem;
	
	static void AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static CustomCtrl*	CtrlSystemFn() { return new T; }
	inline static Vector<CoreCtrlSystem>&	CtrlFactories() {return Single<Vector<CoreCtrlSystem> >();}
	
public:
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		GetId<CoreT>();
		AddCustomCtrl(name, &System::CoreSystemFn<CoreT>, &System::CtrlSystemFn<CtrlT>);
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	template <class CoreT> static int GetId() {
		static bool inited;
		static int id;
		if (!inited) {
			id = CtrlFactories().GetCount();
			inited = true;
		}
		return id;
	}
	
	inline static const Vector<CoreCtrlSystem>& GetCtrlFactories() {return CtrlFactories();}
	static int GetCtrlSystemCount() {return GetCtrlFactories().GetCount();}
	
	template <class CoreT> static int Find() {
		CoreFactoryPtr System_fn = &System::CoreSystemFn<CoreT>;
		const Vector<CoreCtrlSystem>& facs = CtrlFactories();
		for(int i = 0; i < facs.GetCount(); i++) {
			if (facs[i].b == System_fn)
				return i;
		}
		return -1;
	}
	
protected:
	typedef Vector<Vector<Vector<ArrayMap<int, CoreItem> > > > Data;
	
	friend class DataBridgeCommon;
	friend class Core;
	
	Vector<FactoryRegister>		regs;
	Array<PipelineItem>			pl_queue;
	Data			data;
	Vector<String>	period_strings;
	QueryTable		table;
	Vector<int>		traditional_enabled_cols;
	Vector<int>		traditional_col_length;
	Vector<int>		tfbars_in_slowtf;
	Vector<int>		bars;
	Index<String>	symbols;
	Index<int>		periods;
	SpinLock		pl_queue_lock;
	String			addr;
	double			exploration;
	Atomic			nonstopped_workers;
	int64			memory_limit;
	int				port;
	bool			running, stopped;
	
	
protected:
	
	// Time
	Vector<Time> begin;
	Vector<int> begin_ts, end_ts;
	Time end;
	int timediff;
	int base_period;
	int structural_columns;
	int traditional_indicators, traditional_arg_count;
	int template_id, template_arg_count, slot_args;
	int ma_id;
	int structural_priorities;
	
	// Main loop
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	void MainLoop();
	void Worker(int id);
	void RefreshPipeline();
	
	// Pipeline
	void InitGeneticOptimizer();
	void RefreshRealtime();
	int  GetHash(const Vector<byte>& vec);
	int  GetCoreQueue(const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, Index<int>* tf_ids);
	int  GetCoreQueue(Vector<int>& path, const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids);
	void CreateCore(CoreItem& ci);
	int  GetEnabledColumn(const Vector<int>& path);
	int  GetIndicatorFactory(const Vector<int>& path);
	int  GetPathPriority(const Vector<int>& path);
	void InitRegistry();
	void ConnectCore(CoreItem& ci);
	void ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash);
	int  GetSymbolEnabled(int sym) const;
	void MaskPath(const Vector<byte>& src, const Vector<int>& path, Vector<byte>& dst) const;
	
	// Jobs
	void Process(CoreItem& ci);
	
public:
	
	int GetCountTf(int tf_id) const;
	Time GetTimeTf(int tf, int pos) const;// {return begin + base_period * period * pos;}
	Time GetBegin(int tf) const {return begin[tf];}
	Time GetEnd() const {return end;}
	int GetBeginTS(int tf) {return begin_ts[tf];}
	int GetEndTS(int tf) {return end_ts[tf];}
	int GetBasePeriod() const {return base_period;}
	int64 GetShiftTf(int src_tf, int dst_tf, int shift);
	int64 GetShiftFromTimeTf(int timestamp, int tf);
	int64 GetShiftFromTimeTf(const Time& t, int tf);
	Core* CreateSingle(int factory, int sym, int tf);
	
	
public:
	
	void AddPeriod(String nice_str, int period);
	void AddSymbol(String sym);
	
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	
	int GetPeriod(int i) const {return periods[i];}
	String GetPeriodString(int i) const {return period_strings[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int FindPeriod(int period) const {return periods.Find(period);}
	
	
public:
	
	typedef System CLASSNAME;
	System();
	~System();
	
	void Init();
	void Start();
	void Stop();
	
};

}

#endif
