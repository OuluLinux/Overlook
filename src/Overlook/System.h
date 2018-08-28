#ifndef _Overlook_System_h_
#define _Overlook_System_h_


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
	
	bool Process();
	bool IsFinished() const				{return state == STOPPED;}
	Job& SetBegin(Gate0 fn)				{begin   = fn; return *this;}
	Job& SetIterator(Gate0 fn)			{iter    = fn; return *this;}
	Job& SetEnd(Gate0 fn)				{end     = fn; return *this;}
	Job& SetInspect(Gate0 fn)			{inspect = fn; return *this;}
	template <class T> Job& SetCtrl()	{GuiLock __; ctrl = new T(); ctrl->job = this; return *this;}
	void SetProgress(int actual, int total) {this->actual = actual; this->total = total;}
	String GetStateString() const;
	
	void Serialize(Stream& s) {s % title % actual % total % state;}
	
	
	// Persistent
	String title;
	int actual = 0, total = 0;
	int state = INIT;
	
	
	// Temp
	Gate0 begin, iter, end, inspect;
	Ptr<Core> core;
	One<JobCtrl> ctrl;
	TimeStop ts;
	bool allow_processing = false;
};

struct JobThread : Moveable<JobThread> {
	typedef JobThread CLASSNAME;
	
	Vector<Job*> jobs;
	RWMutex job_lock;
	int job_iter = 0;
	int symbol = -1;
	int tf = -1;
	bool is_fail = false;
	
public:
	void SetIter(int i) {job_iter = i;}
	bool ProcessJob();
	
	#ifndef flagGUITASK
	bool running = false, stopped = true;
	void Start() {
		if (jobs.IsEmpty() || !stopped)
			return;
		
		Stop();
		
		if (is_fail)
			return;
		
		// The begin function is called always after loading, so switch state back to init.
		bool all_ready = true;
		for(auto& job : jobs) {
			if (job->state == Job::RUNNING)
				job->state = Job::INIT;
			all_ready &= job->state == Job::STOPPED;
		}
		
		if (!all_ready) {
			running = true;
			stopped = false;
			Thread::Start(THISBACK(Run));
		}
	}
	void Stop() {
		running = false;
		while (!stopped) Sleep(100);
	}
	void PutStop() {
		running = false;
	}
	void Run() {
		while (running) {
			if (!ProcessJob()) {
				running = false;
			}
		}
		stopped = true;
	}
	bool IsStopped() const {return stopped;}
	#endif
};

struct InspectionResult : Moveable<InspectionResult> {
	const char* file = "";
	int line = 0, symbol = 0, tf = 0;
	String msg;
};

struct VariantSymbol : Moveable<VariantSymbol> {
	String pair1, pair2;
	int math = 0, p1 = 0, p2 = 0;
};

struct VariantList : Moveable<VariantList> {
	Vector<VariantSymbol> symbols;
	Index<int> dependencies;
};

enum {TIMEBUF_WEEKTIME, TIMEBUF_COUNT};
enum {CORE_INDICATOR, CORE_EXPERTADVISOR, CORE_HIDDEN};

class System {
	
public:

	typedef Core* (*CoreFactoryPtr)();
	typedef Common* (*CommonFactoryPtr)();
	typedef Ctrl* (*CtrlFactoryPtr)();
	typedef Tuple<String, CoreFactoryPtr, CoreFactoryPtr> CoreSystem;
	typedef Tuple<String, CommonFactoryPtr, CtrlFactoryPtr> CommonSystem;
	typedef VectorMap<int, VectorMap<int, String> > FactoryAssistList;
	typedef VectorMap<int, Tuple<int, String> > AssistList;
	
	static void								AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static Core*			CoreSystemSingleFn() { return &Single<T>(); }
	template <class T> static Common*		CommonSystemSingleFn() { return &Single<T>(); }
	template <class T> static Ctrl*			CtrlSystemSingleFn() { return &Single<T>(); }
	inline static Vector<CoreSystem>&		CoreFactories() {static Vector<CoreSystem> list; return list;}
	inline static Vector<CommonSystem>&		CommonFactories() {static Vector<CommonSystem> list; return list;}
	inline static Vector<int>&				Indicators() {static Vector<int> list; return list;}
	inline static Vector<int>&				ExpertAdvisorFactories() {static Vector<int> list; return list;}
	inline static Index<int>&				PrioritySlowTf() {static Index<int> list; return list;}
	inline static FactoryAssistList&		AssistantFactories() {static FactoryAssistList list; return list;}
	inline static AssistList&				Assistants() {static AssistList list; return list;}
	
public:
	
	template <class CoreT> static void		Register(String name, int type=CORE_INDICATOR) {
		int id = CoreFactories().GetCount();
		if      (type == CORE_INDICATOR)		Indicators().Add(id);
		else if (type == CORE_EXPERTADVISOR)	ExpertAdvisorFactories().Add(id);
		AddCustomCore(name, &System::CoreSystemFn<CoreT>, &System::CoreSystemSingleFn<CoreT>);
	}
	
	template <class CoreT> static void		RegisterEvent(String name, int type) {
		int id = Find<CoreT>();
		if (id == -1) Panic("Invalid assist: " + IntStr(type) + " " + name);
		AssistantFactories().GetAdd(id).GetAdd(type, name);
		Assistants().GetAdd(type) = Tuple<int, String>(id, CoreFactories()[id].a + " " + name);
	}
	
	template <class CoreT, class CtrlT> static void		RegisterCommon(String name) {
		CommonFactories().Add(CommonSystem(name, &System::CommonSystemSingleFn<CoreT>, &System::CtrlSystemSingleFn<CtrlT>));
	}
	
	
	
	template <class CoreT> static CoreT&	GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	
	inline static const Vector<CoreSystem>&	GetCoreFactories() {return CoreFactories();}
	
	template <class CoreT> static int		Find() {
		CoreFactoryPtr System_fn = &System::CoreSystemFn<CoreT>;
		const Vector<CoreSystem>& facs = CoreFactories();
		for(int i = 0; i < facs.GetCount(); i++) {
			if (facs[i].b == System_fn)
				return i;
		}
		return -1;
	}
	
	template <class T>
	inline static ArrayMap<int, T>&			GetBusyTasklist() {
		static ArrayMap<int, T> list;
		return list;
	}
	template <class T>
	inline static Vector<T>&				GetBusyRunning() {
		static Vector<T> list;
		return list;
	}
	
	template <class T>
	T&										GetCommon() {
		return *dynamic_cast<T*>(CommonSystemSingleFn<T>());
	}
	
	struct NetSetting : Moveable<NetSetting> {
		VectorMap<String, int> symbols;
		VectorMap<int, int> symbol_ids;
		NetSetting& Set(String s, int i) {symbols.Add(s, i); return *this;}
		void Assign(String s, int i) {
			int j = symbols.Find(s);
			symbols[j] = i;
			symbol_ids[j] = i;
		}
	};
	
protected:
	typedef Vector<Vector<Vector<ArrayMap<int, CoreItem> > > > Data;
	
	
	
	friend class DataBridgeCommon;
	friend class DataBridge;
	friend class SimBroker;
	friend class CoreIO;
	friend class Core;
	
	
	// Temporary
	Vector<NetSetting>			nets;
	Vector<Vector<int> >		sym_currencies;
	VectorMap<String, Index<int> > currency_syms, currency_sym_dirs, major_currency_syms;
	Index<String>				symbols, allowed_symbols, currencies;
	Vector<int>					signals;
	Index<int>					periods, major_currencies;
	Vector<String>				period_strings;
	Vector<double>				spread_points;
	Vector<FactoryRegister>		regs;
	Vector<VariantList>			variants;
	Time						end;
	Data						data;
	double						limit_day_begin = 0, limit_day_best = 0, limit_day_worst = 0;
	double						fmlevel = 0.6;
	int							limit_wday = -1;
	int							time_offset = 0;
	int							realtime_count = 0;
	int							normal_symbol_count = 0;
	
public:
	int		GetCurrencyCount() const {return currencies.GetCount();}
	String	GetCurrency(int i) const {return currencies[i];}
	int		GetSymbolCurrency(int i, int j) const {if (sym_currencies[i].IsEmpty()) return -1; return sym_currencies[i][j];}
	int		GetMajorCurrencyCount() const {return major_currency_syms.GetCount();}
	String	GetMajorCurrency(int i) const {return major_currency_syms.GetKey(i);}
	int		FindMajorCurrency(int i) const {return major_currencies.Find(i);}
	int		GetNormalSymbolCount() const {return normal_symbol_count;}
	NetSetting& AddNet(String s) {AddSymbol(s); return nets.Add();}
	bool	IsNormalSymbol(int i) {return i < normal_symbol_count;}
	bool	IsCurrencySymbol(int i) {return i >= normal_symbol_count && i < normal_symbol_count + currencies.GetCount();}
	bool	IsNetSymbol(int i) {return i >= normal_symbol_count + currencies.GetCount();}
	int		GetNetSymbol(int i) {return i + normal_symbol_count + currencies.GetCount();}
	NetSetting& GetNet(int i) {return nets[i];}
	NetSetting& GetSymbolNet(int i) {return nets[i - normal_symbol_count - currencies.GetCount()];}
	int		GetNetCount() const {return nets.GetCount();}
	int		GetVtfWeekbars() const {return 98;}
	void	SetFreemarginLevel(double d) {fmlevel = d;}
	
protected:
	
	
	void	RefreshRealtime();
	int		GetHash(const Vector<byte>& vec);
	int		GetCoreQueue(Vector<FactoryDeclaration>& path, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids);
	void	CreateCore(CoreItem& ci);
	void	FirstStart();
	void	InitRegistry();
	void	ConnectCore(CoreItem& ci);
	void	ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash);
	void	MaskPath(const Vector<byte>& src, const Vector<int>& path, Vector<byte>& dst) const;
	
public:
	
	void	Process(CoreItem& ci, bool store_cache, bool store_cache_if_init=true);
	int		GetCoreQueue(Vector<Ptr<CoreItem> >& ci_queue, const Index<int>& sym_ids, const Index<int>& tf_ids, const Vector<FactoryDeclaration>& indi_ids);
	Core*	CreateSingle(int factory, int sym, int tf);
	Time	GetEnd() const							{return end;}
	const Vector<FactoryRegister>& GetRegs() const	{return regs;}
	
public:
	
	void	AddPeriod(String nice_str, int period);
	void	AddSymbol(String sym);
	
	String	GetSymbol(int i) const					{return symbols[i];}
	String	GetPeriodString(int i) const			{return period_strings[i];}
	int		GetFactoryCount() const					{return GetRegs().GetCount();}
	int		GetBrokerSymbolCount() const			{return symbols.GetCount();}
	int		GetTotalSymbolCount() const				{return symbols.GetCount();}
	int		GetSymbolCount() const					{return symbols.GetCount();}
	int		GetPeriod(int i) const					{return periods[i];}
	int		GetPeriodCount() const					{return periods.GetCount();}
	int		FindPeriod(int period) const			{return periods.Find(period);}
	int		FindSymbol(const String& s) const		{return symbols.Find(s);}
	void	GetWorkQueue(Vector<Ptr<CoreItem> >& ci_queue);
	void	StoreCores();
	bool	RefreshReal();
	void	SetSignal(int sym, int i)				{signals[sym] = i;}
	const VariantList& GetVariants(int i) const		{return variants[i];}
	
public:
	
	ArrayMap<int, JobThread>	job_threads;
	Vector<InspectionResult>	inspection_results;
	SpinLock					job_lock;
	SpinLock					inspection_lock;
	Mutex						core_queue_lock;
	#ifdef flagGUITASK
	TimeCallback				jobs_tc;
	int							gui_job_thread = 0;
	#else
	int							job_thread_iter = 0;
	bool						jobs_running = false;
	bool						jobs_stopped = true;
	#endif
	
	int		GetJobThreadCount() const				{return job_threads.GetCount();}
	void	ProcessJobs();
	bool	ProcessJob(int job_thread);
	void	StopJobs();
	void	StoreJobCores();
	JobThread& GetJobThread(int i)					{JobThread* thrd; LOCK(job_lock) {thrd = &job_threads[i];} return *thrd;}
	JobThread& GetJobThread(int sym, int tf)		{JobThread* thrd; LOCK(job_lock) {thrd = &job_threads.GetAdd(HashSymTf(sym, tf));} return *thrd;}
	#ifdef flagGUITASK
	void	PostProcessJobs() {jobs_tc.Kill(); PostCallback(THISBACK(ProcessJobs));}
	#endif
	void	InspectionFailed(const char* file, int line, int symbol, int tf, String msg);
	
	
public:
	
	typedef System CLASSNAME;
	System();
	~System();
	
	void Init();
	void Deinit();
	
	Callback2<int,int> WhenProgress;
	Callback2<int,int> WhenSubProgress;
	Callback1<String>  WhenInfo;
	Callback1<String>  WhenError;
	Callback1<String>  WhenPushTask;
	Callback           WhenRealtimeUpdate;
	Callback           WhenPopTask;
};

inline System& GetSystem()							{return Single<System>();}


}

#endif
