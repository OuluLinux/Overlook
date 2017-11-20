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
	int factory = -1, input_type = -1;
	void* data = NULL, *data2 = NULL;
	RegisterInput(int fac, int intype, void* filter_fn, void* args_fn) {factory=fac; input_type=intype; data=filter_fn; data2=args_fn;}
	RegisterInput(const RegisterInput& o) {factory=o.factory; input_type=o.input_type; data=o.data; data2=o.data2;}
	String ToString() const {return Format(" factory=%d input_type=%d data=%X data2=%X", factory, input_type, (int64)data, (int64)data2);}
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
			in.Add(RegisterInput(base.factory, REGIN_NORMAL, base.data, base.data2));
		}
		else if (base.data_type == ValueBase::INOPT_) {
			in.Add(RegisterInput(base.factory, REGIN_OPTIONAL, base.data, base.data2));
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

struct Job;

struct JobCtrl : public ParentCtrl {
	Job* job = NULL;
};

struct Job {
	typedef Job CLASSNAME;
	Job() {}
	
	enum {INIT, RUNNING, STOPPING, INSPECTING, STOPPED};
	
	void Process();
	bool IsFinished() const				{return state == STOPPED;}
	Job& SetBegin(Gate0 fn)				{begin   = fn; return *this;}
	Job& SetIterator(Gate0 fn)			{iter    = fn; return *this;}
	Job& SetEnd(Gate0 fn)				{end     = fn; return *this;}
	Job& SetInspect(Gate0 fn)			{inspect = fn; return *this;}
	template <class T> Job& SetCtrl()	{ctrl = new T(); ctrl->job = this; return *this;}
	void SetProgress(int actual, int total) {this->actual = actual; this->total = total;}
	String GetStateString() const;
	
	void Serialize(Stream& s) {
		s % title % actual % total % state;
		
		// The begin function is called always after loading, so switch state back to init.
		if (s.IsLoading() && state == RUNNING)
			state = INIT;
	}
	
	
	// Persistent
	String title;
	int actual = 0, total = 0;
	int state = INIT;
	
	
	// Temp
	Gate0 begin, iter, end, inspect;
	Ptr<Core> core;
	One<JobCtrl> ctrl;
	TimeStop ts;
};

enum {TIMEBUF_WEEKTIME, TIMEBUF_COUNT};
enum {CORE_INDICATOR, CORE_EXPERTADVISOR, CORE_ACCOUNTADVISOR, CORE_HIDDEN};

class System {
	
	static Index<int> true_indicators;
	
public:

	typedef Core*			(*CoreFactoryPtr)();
	typedef Tuple<String, CoreFactoryPtr, CoreFactoryPtr> CoreSystem;
	
	static void AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static Core*			CoreSystemSingleFn() { return &Single<T>(); }
	inline static Vector<CoreSystem>&	CoreFactories() {static Vector<CoreSystem> list; return list;}
	inline static Vector<int>&	Indicators() {static Vector<int> list; return list;}
	inline static Vector<int>&	ExpertAdvisorFactories() {static Vector<int> list; return list;}
	inline static Vector<int>&	AccountAdvisorFactories() {static Vector<int> list; return list;}
	
	
public:
	
	template <class CoreT> static void Register(String name, int type=CORE_INDICATOR) {
		int id = GetId<CoreT>();
		if      (type == CORE_INDICATOR)		Indicators().Add(id);
		else if (type == CORE_EXPERTADVISOR)	ExpertAdvisorFactories().Add(id);
		else if (type == CORE_ACCOUNTADVISOR)	AccountAdvisorFactories().Add(id);
		AddCustomCore(name, &System::CoreSystemFn<CoreT>, &System::CoreSystemSingleFn<CoreT>);
	}
	
	template <class CoreT> static CoreT& GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	template <class CoreT> static int GetId() {
		static bool inited;
		static int id;
		if (!inited) {
			id = CoreFactories().GetCount();
			inited = true;
		}
		return id;
	}
	
	inline static const Vector<CoreSystem>& GetCoreFactories() {return CoreFactories();}
	
	template <class CoreT> static int Find() {
		CoreFactoryPtr System_fn = &System::CoreSystemFn<CoreT>;
		const Vector<CoreSystem>& facs = CoreFactories();
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
	friend class CoreIO;
	friend class Core;
	
	Vector<FactoryRegister>		regs;
	Data			data;
	Vector<String>	period_strings;
	Vector<int>		bars;
	Vector<int>		priority;
	Vector<int>		sym_priority;
	Vector<Job*>	jobs;
	Index<String>	symbols;
	Index<int>		periods;
	SpinLock		task_lock;
	SpinLock		pl_queue_lock;
	SpinLock		job_lock;
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
	TimeCallback jobs_tc;
	int timediff;
	int base_period;
	int source_symbol_count;
	int job_iter = 0;
	bool jobs_running = false, jobs_stopped = true;
	
	void Serialize(Stream& s) {s % begin % end % timediff % base_period % begin_ts;}
	void RefreshRealtime();
	int  GetHash(const Vector<byte>& vec);
	int  GetCoreQueue(Vector<FactoryDeclaration>& path, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids);
	void CreateCore(CoreItem& ci);
	void InitRegistry();
	void ConnectCore(CoreItem& ci);
	void ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash);
	void MaskPath(const Vector<byte>& src, const Vector<int>& path, Vector<byte>& dst) const;
	void ProcessJobs();
	bool ProcessJob();
	void PostProcessJobs() {jobs_tc.Kill(); PostCallback(THISBACK(ProcessJobs));}
	void StopJobs();
	
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
	
	
public:
	
	void AddPeriod(String nice_str, int period);
	void AddSymbol(String sym);
	
	String GetSymbol(int i) const {return symbols[i];}
	String GetPeriodString(int i) const {return period_strings[i];}
	int GetSymbolPriority(int i) const {return priority[i];}
	int GetPrioritySymbol(int i) const {return sym_priority[i];}
	int GetFactoryCount() const {return GetRegs().GetCount();}
	int GetBrokerSymbolCount() const {return source_symbol_count;}
	int GetTotalSymbolCount() const {return symbols.GetCount();}
	int GetSymbolCount() const {return symbols.GetCount();}
	int GetPeriod(int i) const {return periods[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int FindPeriod(int period) const {return periods.Find(period);}
	int FindSymbol(const String& s) const {return symbols.Find(s);}
	void GetWorkQueue(Vector<Ptr<CoreItem> >& ci_queue);
	void SetFixedBroker(FixedSimBroker& broker, int sym_id);
	int GetJobCount() const {return jobs.GetCount();}
	const Job& GetJob(int i) const {return *jobs[i];}
	Job& GetJob(int i) {return *jobs[i];}
	
public:
	
	Vector<double> spread_points;
	Vector<int> proxy_id, proxy_base_mul;
	
	int GetAccountSymbol() const {return symbols.GetCount()-1;}
	
public:
	
	typedef System CLASSNAME;
	System();
	~System();
	
	void Init();
	
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
