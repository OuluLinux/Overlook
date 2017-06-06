#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Overlook {
using namespace Upp;

struct RegisterInput : Moveable<RegisterInput> {
	int input_type;
	void* data;
	RegisterInput(int intype, void* filter_fn) {input_type=intype; data=filter_fn;}
	RegisterInput(const RegisterInput& o) {input_type=o.input_type; data=o.data;}
	String ToString() const {return Format(" input_type=%d data=%X", input_type, (int64)data);}
};

enum {REGIN_NORMAL, REGIN_OPTIONAL, REGIN_DYNAMIC, REGIN_HIGHPRIO};


struct ArgType : Moveable<ArgType> {
	ArgType() {}
	ArgType(const ArgType& src) {*this = src;}
	ArgType(String desc, int min, int max) : desc(desc), min(min), max(max) {}
	void operator=(const ArgType& src) {
		
	}
	
	String desc;
	int min, max;
};

struct SystemValueRegister : public ValueRegister, Moveable<SystemValueRegister> {
	
	Vector<RegisterInput> in;
	Vector<ValueType> out;
	Vector<ArgType> args;
	
	virtual void IO(const ValueBase& base) {
		if (base.data_type == ValueBase::IN_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_NORMAL, NULL));
		}
		else if (base.data_type == ValueBase::INOPT_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_OPTIONAL, NULL));
		}
		else if (base.data_type == ValueBase::INDYN_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_DYNAMIC, base.data));
		}
		else if (base.data_type == ValueBase::INHIGHPRIO_) {
			in.Add(RegisterInput(base.phase, base.type, base.scale, REGIN_HIGHPRIO, base.data));
		}
		else if (base.data_type == ValueBase::OUT_) {
			out.Add(ValueType(base.phase, base.type, base.scale));
		}
		else if (base.data_type == ValueBase::BOOL_) {
			args.Add(ArgType(base.s0, 0, 1));
		}
		else if (base.data_type == ValueBase::INT_) {
			args.Add(ArgType(base.s0, base.min, base.max));
		}
	}
	
};

struct PipelineItem : Moveable<PipelineItem> {
	typedef PipelineItem CLASSNAME;
	PipelineItem() {priority = INT_MAX;}
	
	Vector<byte> value;
	int priority;
};

struct CoreItem : Moveable<CoreItem>, public Pte<CoreItem> {
	typedef CoreItem CLASSNAME;
	CoreItem() {core = NULL; sym = -1; tf = -1; priority = INT_MAX; factory = -1;}
	
	Core* core;
	String unique;
	int sym, tf, priority, factory;
};

struct InputSource : Moveable<InputSource> {
	InputSource() : factory(-1), output(-1) {}
	InputSource(int factory, int output) : factory(factory), output(output) {}
	InputSource(const InputSource& src) {*this = src;}
	void operator=(const InputSource& src) {factory = src.factory; output = src.output;}
	int factory, output;
};

struct CombinationPart : Moveable<CombinationPart> {
	Vector<RegisterInput> inputs;
	Vector<ValueType> outputs;
	Vector<Vector<InputSource> > input_src;
	bool single_sources;
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
	
	// These static values doesn't work between threads, unless a method returns them.
	static Vector<SystemValueRegister>& Regs() {static Vector<SystemValueRegister> v; return v;}
	
	
public:
	
	template <class CoreT, class CtrlT> static void Register(String name) {
		AddCustomCtrl(name, &System::CoreSystemFn<CoreT>, &System::CtrlSystemFn<CtrlT>);
		SystemValueRegister& reg = Regs().Add();
		CoreT().IO(reg); // unfortunately one object must be created, because IO can't be static and virtual at the same time and it is cleaner to use virtual.
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	
	inline static const Vector<CoreCtrlSystem>& GetCtrlFactories() {return CtrlFactories();}
	static int GetCtrlSystemCount() {return GetCtrlFactories().GetCount();}
	static const Vector<SystemValueRegister>& GetRegs() {return Regs();}
	
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
	
	Vector<CombinationPart>		combparts;
	Array<PipelineItem>			pl_queue;
	Data			data;
	Vector<String>	period_strings;
	QueryTable		table;
	Vector<int>		traditional_enabled_cols;
	Vector<int>		tfbars_in_slowtf;
	Vector<int>		bars;
	/*Vector<int>		inputs_to_enabled;
	Vector<int>		enabled_to_factory;*/
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
	
	// Main loop
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	void MainLoop();
	void Worker(int id);
	void RefreshPipeline();
	
	// Pipeline
	void InitGeneticOptimizer();
	void RefreshRealtime();
	int  GetHash(const PipelineItem& pi, int sym, int tf, Vector<int>& path);
	void GetCoreQueue(const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, Index<int>* tf_ids);
	void GetCoreQueue(Vector<int>& path, const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Vector<int>& sym_ids);
	void CreateCore(CoreItem& ci);
	int  GetEnabledColumn(const Vector<int>& path);
	int  GetIndicatorFactory(const Vector<int>& path);
	int  GetPathPriority(const Vector<int>& path);
	void SolveClassConnections();
	//int  GetBitCore(int struct_id, int fac_id, int input_id, int src_id) const;
	//int  GetBitEnabled(int struct_id, int fac_id) const;
	void ConnectCore(CoreItem& ci);
	int  ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash);
	int  GetSymbolEnabled(int sym) const;
	
	// Jobs
	void Process(CoreItem& ci);
	
public:
	
	//int GetCount(int period) const;
	int GetCountTf(int tf_id) const;
	Time GetTimeTf(int tf, int pos) const;// {return begin + base_period * period * pos;}
	Time GetBegin(int tf) const {return begin[tf];}
	Time GetEnd() const {return end;}
	int GetBeginTS(int tf) {return begin_ts[tf];}
	int GetEndTS(int tf) {return end_ts[tf];}
	int GetBasePeriod() const {return base_period;}
	//int64 GetShift(int src_period, int dst_period, int shift);
	//int64 GetShiftFromTime(int timestamp, int period);
	int64 GetShiftTf(int src_tf, int dst_tf, int shift);
	int64 GetShiftFromTimeTf(int timestamp, int tf);
	int64 GetShiftFromTimeTf(const Time& t, int tf);
	//int GetTfFromSeconds(int period_seconds);
	
	//void SetBegin(Time t)	{begin = t; begin_ts = (int)(t.Get() - Time(1970,1,1).Get());}
	//void SetEnd(Time t)	{end = t; end_ts = (int)(t.Get() - Time(1970,1,1).Get()); timediff = (int)(end.Get() - begin.Get());}
	//void SetBasePeriod(int period)	{base_period = period;}
	Core* CreateSingle(int column, int sym, int tf);
	
	
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
	
	
	virtual void IO(ValueRegister& reg) {
		reg % Out(SourcePhase, TimeValue, All);
	}
};

}

#endif
