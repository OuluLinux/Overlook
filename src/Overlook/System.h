#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Config {
extern Upp::IniString arg_addr;
extern Upp::IniInt arg_port;
}

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
	PipelineItem() {priority = INT_MAX; sym = -1;}
	
	Vector<byte> value;
	int priority;
	int sym;
};

class CustomCtrl;

class System {
	
	static Index<int> true_indicators;
	
public:

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
	
	template <class T>
	inline static ArrayMap<int, T>& GetBusyTasklist() {
		static ArrayMap<int, T> list;
		return list;
	}
	template <class T>
	inline static Vector<T>& GetBusyRunning() {
		static Vector<T> list;
		return list;
	}
	
protected:
	typedef Vector<Vector<Vector<ArrayMap<int, CoreItem> > > > Data;
	
	friend class DataBridgeCommon;
	friend class DataBridge;
	friend class SimBroker;
	friend class Core;
	
	Vector<FactoryRegister>		regs;
	ExpertSystem	es;
	Data			data;
	Vector<String>	period_strings;
	Vector<int>		bars;
	Index<String>	symbols;
	Index<int>		periods;
	SpinLock		task_lock;
	SpinLock		pl_queue_lock;
	String			addr;
	double			exploration;
	int64			memory_limit;
	int				port;
	int				task_counter;
	
	
protected:
	
	// Time
	Vector<Time> begin;
	Vector<int> begin_ts;
	Time end;
	int timediff;
	int base_period;
	int source_symbol_count;
	
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	void RefreshRealtime();
	int  GetHash(const Vector<byte>& vec);
	int  GetCoreQueue(Vector<FactoryDeclaration>& path, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids);
	void CreateCore(CoreItem& ci);
	void InitRegistry();
	void ConnectCore(CoreItem& ci);
	void ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash);
	void MaskPath(const Vector<byte>& src, const Vector<int>& path, Vector<byte>& dst) const;
	
public:
	
	void Process(CoreItem& ci);
	int GetCoreQueue(Vector<Ptr<CoreItem> >& ci_queue, const Index<int>& sym_ids, const Index<int>& tf_ids, const Vector<FactoryDeclaration>& indi_ids);
	int GetCountTf(int tf_id) const;
	Time GetTimeTf(int tf, int pos) const;// {return begin + base_period * period * pos;}
	Time GetBegin(int tf) const {return begin[tf];}
	Time GetEnd() const {return end;}
	int GetBeginTS(int tf) {return begin_ts[tf];}
	int GetBasePeriod() const {return base_period;}
	int GetShiftTf(int src_tf, int dst_tf, int shift);
	int GetShiftFromTimeTf(int timestamp, int tf);
	int GetShiftFromTimeTf(const Time& t, int tf);
	Core* CreateSingle(int factory, int sym, int tf);
	const Vector<FactoryRegister>& GetRegs() const {return regs;}
	void SetEnd(const Time& t);
	ExpertSystem& GetExpertSystem() {return es;}
	
public:
	
	void AddPeriod(String nice_str, int period);
	void AddSymbol(String sym);
	
	String GetSymbol(int i) const {return symbols[i];}
	String GetPeriodString(int i) const {return period_strings[i];}
	int GetFactoryCount() const {return GetRegs().GetCount();}
	int GetBrokerSymbolCount() const {return source_symbol_count;}
	int GetTotalSymbolCount() const {return symbols.GetCount();}
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetPeriod(int i) const {return periods[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int FindPeriod(int period) const {return periods.Find(period);}
	void GetWorkQueue(Vector<Ptr<CoreItem> >& ci_queue);
	
public:
	VectorMap<String, int> allowed_symbols;
	Vector<Vector<ConstBuffer*> > value_buffers;
	Vector<Vector<ConstVectorBool*> > label_value_buffers;
	Vector<ConstBuffer*> open_buffers;
	Vector<Ptr<CoreItem> > work_queue, db_queue, label_queue;
	Vector<Core*> databridge_cores;
	Vector<double> spread_points;
	Vector<int> proxy_id, proxy_base_mul;
	Vector<FactoryDeclaration> indi_ids, label_indi_ids;
	Index<int> sym_ids;
	int data_begin = 0;
	int buf_count = 0;
	int label_buf_count = 0;
	int main_tf = -1;
	bool skip_storecache = false;
	Mutex work_lock;
	
	void InitContent();
	void RefreshWorkQueue();
	void ProcessWorkQueue();
	void ProcessLabelQueue();
	void ProcessDataBridgeQueue();
	void ResetValueBuffers();
	void ResetLabelBuffers();
	void InitBrokerValues();
	
	int GetTradingSymbolCount() const {return sym_ids.GetCount();}
	int GetTrueIndicatorCount() const {return TRUEINDI_COUNT;}
	int GetLabelIndicatorCount() const {return LABELINDI_COUNT;}
	int GetCountMain() const {return GetCountTf(main_tf);}
	ConstBuffer&		GetTrueIndicator(int sym, int tf, int i) const {ASSERT(tf>=0&&tf<TF_COUNT&&i>=0&&i<TRUEINDI_COUNT); return *value_buffers[sym][tf * TRUEINDI_COUNT + i];}
	ConstVectorBool&	GetLabelIndicator(int sym, int tf, int i) const {ASSERT(tf>=0&&tf<TF_COUNT&&i>=0&&i<LABELINDI_COUNT); return *label_value_buffers[sym][tf * LABELINDI_COUNT + i];}
	ConstBuffer&		GetOpenBuffer(int sym) const {return *open_buffers[sym];}
	ConstBuffer&		GetTradingSymbolOpenBuffer(int sym) const {return *open_buffers[sym_ids[sym]];}
	double GetTradingSymbolSpreadPoint(int sym) const {return spread_points[sym];}
	void SetFixedBroker(FixedSimBroker& broker, int sym_id=-1);
	
public:
	
	typedef System CLASSNAME;
	System();
	~System();
	
	void Init();
	void Start()	{es.Start();}
	void Stop()		{es.Stop();}
	
	Callback2<int,int> WhenProgress;
	Callback2<int,int> WhenSubProgress;
	Callback1<String>  WhenInfo;
	Callback1<String>  WhenError;
	Callback1<String>  WhenPushTask;
	Callback           WhenRealtimeUpdate;
	Callback           WhenPopTask;
};

inline System& GetSystem() {return Single<System>();}

}

#endif
