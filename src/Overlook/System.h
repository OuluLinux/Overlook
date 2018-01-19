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
	
	bool Process();
	bool IsFinished() const				{return state == STOPPED;}
	Job& SetBegin(Gate0 fn)				{begin   = fn; return *this;}
	Job& SetIterator(Gate0 fn)			{iter    = fn; return *this;}
	Job& SetEnd(Gate0 fn)				{end     = fn; return *this;}
	Job& SetInspect(Gate0 fn)			{inspect = fn; return *this;}
	template <class T> Job& SetCtrl()	{ctrl = new T(); ctrl->job = this; return *this;}
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

enum {TIMEBUF_WEEKTIME, TIMEBUF_COUNT};
enum {CORE_INDICATOR, CORE_EXPERTADVISOR, CORE_HIDDEN};

class System {
	
public:

	typedef Core* (*CoreFactoryPtr)();
	typedef Tuple<String, CoreFactoryPtr, CoreFactoryPtr> CoreSystem;
	typedef VectorMap<int, VectorMap<int, String> > FactoryAssistList;
	typedef VectorMap<int, Tuple<int, String> > AssistList;
	
	static void								AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef);
	template <class T> static Core*			CoreSystemFn() { return new T; }
	template <class T> static Core*			CoreSystemSingleFn() { return &Single<T>(); }
	inline static Vector<CoreSystem>&		CoreFactories() {static Vector<CoreSystem> list; return list;}
	inline static Vector<int>&				Indicators() {static Vector<int> list; return list;}
	inline static Vector<int>&				ExpertAdvisorFactories() {static Vector<int> list; return list;}
	inline static Index<int>&				PrioritySlowTf() {static Index<int> list; return list;}
	inline static FactoryAssistList&		AssistantFactories() {static FactoryAssistList list; return list;}
	inline static AssistList&				Assistants() {static AssistList list; return list;}
	
public:
	
	template <class CoreT> static void		Register(String name, int type=CORE_INDICATOR) {
		int id = GetId<CoreT>();
		if      (type == CORE_INDICATOR)		Indicators().Add(id);
		else if (type == CORE_EXPERTADVISOR)	ExpertAdvisorFactories().Add(id);
		AddCustomCore(name, &System::CoreSystemFn<CoreT>, &System::CoreSystemSingleFn<CoreT>);
	}
	
	template <class CoreT> static void		RegisterAssistant(String name, int type) {
		int id = Find<CoreT>();
		if (id == -1) Panic("Invalid assist: " + IntStr(type) + " " + name);
		AssistantFactories().GetAdd(id).GetAdd(type, name);
		Assistants().GetAdd(type) = Tuple<int, String>(id, CoreFactories()[id].a + " " + name);
	}
	
	template <class CoreT> static CoreT&	GetCore() {return *dynamic_cast<CoreT*>(CoreSystemFn<CoreT>());}
	template <class CoreT> static int		GetId() {
		static bool inited;
		static int id;
		if (!inited) {
			id = CoreFactories().GetCount();
			inited = true;
		}
		return id;
	}
	
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
	
	
	
	
protected:
	typedef Vector<Vector<Vector<ArrayMap<int, CoreItem> > > > Data;
	
	friend class DataBridgeCommon;
	friend class DataBridge;
	friend class SimBroker;
	friend class CoreIO;
	friend class Core;
	
	
	// Persistent
	Index<String>				symbols;
	Index<int>					periods;
	Vector<String>				period_strings;
	Vector<double>				spread_points;
	Vector<int>					proxy_id;
	Vector<int>					proxy_base_mul;
	Vector<int>					sym_priority;
	Vector<int>					common_symbol_id;
	Vector<int>					common_symbol_pos;
	Vector<Vector<int> >		common_symbol_group_pos;
	int							time_offset = 0;
	
	
	// Temporary
	Vector<FactoryRegister>		regs;
	Time						end;
	Data						data;
	String						addr;
	int							port;
	
	
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
	
	void	Process(CoreItem& ci, bool store_cache);
	int		GetCoreQueue(Vector<Ptr<CoreItem> >& ci_queue, const Index<int>& sym_ids, const Index<int>& tf_ids, const Vector<FactoryDeclaration>& indi_ids);
	int		GetCountTf(int sym, int tf) const;
	Time	GetTimeTf(int sym, int tf, int pos) const;
	int		GetShiftTf(int src_sym, int src_tf, int dst_sym, int dst_tf, int shift);
	int		GetShiftMainTf(int src_tf, int dst_sym, int dst_tf, int src_shift);
	Core*	CreateSingle(int factory, int sym, int tf);
	void	SetEnd(const Time& t);
	Time	GetEnd() const							{return end;}
	const Vector<FactoryRegister>& GetRegs() const	{return regs;}
	
public:
	
	// Persistent
	Vector<Vector<Vector<Time> > > pos_time;
	Vector<VectorMap<Time, byte> > main_time;
	Vector<Vector<Vector<int> > > main_conv;
	Vector<Vector<Vector<int> > > posconv_from, posconv_to;
	
	// Temporary
	bool main_time_changed = false;
	
	
	void	Serialize(Stream& s) {
		s % symbols % periods % period_strings % spread_points % proxy_id
		  % proxy_base_mul % sym_priority % common_symbol_id % common_symbol_pos % common_symbol_group_pos % time_offset
		  % pos_time % main_time % main_conv % posconv_from % posconv_to
		  % main_data % main_mem % logic0 % logic1 % logic2;
	}
	
	void	DataTimeBegin(int sym, int tf);
	void	DataTimeEnd(int sym, int tf);
	int		DataTimeAdd(int sym, int tf, Time utc_time);
	Time	TimeFromBroker(Time t) {return t - time_offset;}
	Time	TimeToBroker(Time t) {return t + time_offset;}
	void	RefreshTimeTfVectors(int tf);
	void	RefreshTimeTfVector(int tf_from, int tf_to);
	void	RefreshTimeSymVectors(int sym, int tf);
	void	LoadThis() {LoadFromFile(*this, ConfigFile("system.bin"));}
	void	StoreThis() {StoreToFile(*this, ConfigFile("system.bin"));}
	int		GetCountMain(int tf) const {return main_time[tf].GetCount();}
	Time	GetTimeMain(int tf, int i) const {return main_time[tf].GetKey(i);}
	int		GetShiftFromMain(int sym, int tf, int i) const {return posconv_to[sym][tf][i];}
	int		GetShiftToMain(int sym, int tf, int i) const {return posconv_from[sym][tf][i];}
	
public:
	
	void	AddPeriod(String nice_str, int period);
	void	AddSymbol(String sym);
	
	String	GetSymbol(int i) const					{return symbols[i];}
	String	GetPeriodString(int i) const			{return period_strings[i];}
	int		GetFactoryCount() const					{return GetRegs().GetCount();}
	int		GetBrokerSymbolCount() const			{return symbols.GetCount()-2;}
	int		GetTotalSymbolCount() const				{return symbols.GetCount();}
	int		GetSymbolCount() const					{return symbols.GetCount();}
	int		GetPeriod(int i) const					{return periods[i];}
	int		GetPeriodCount() const					{return periods.GetCount();}
	int		FindPeriod(int period) const			{return periods.Find(period);}
	int		FindSymbol(const String& s) const		{return symbols.Find(s);}
	void	GetWorkQueue(Vector<Ptr<CoreItem> >& ci_queue);
	void	StoreCores();
	
public:
	
	// Job threads for symbol/timeframe cores.
	// Both single-threaded and multi-threaded processing is supported for CLEANLINESS.
	// All optimizations are done in jobs, which can contain the generic genetic local optimizer too.
	// ExpertAdvisors are expected to contain their own optimization jobs, to automate usage.
	
	ArrayMap<int, JobThread>	job_threads;
	Vector<InspectionResult>	inspection_results;
	SpinLock					job_lock;
	SpinLock					inspection_lock;
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
	//const JobThread& GetJobThread(int i) const		{return job_threads[i];}
	JobThread& GetJobThread(int i)					{JobThread* thrd; LOCK(job_lock) {thrd = &job_threads[i];} return *thrd;}
	JobThread& GetJobThread(int sym, int tf)		{JobThread* thrd; LOCK(job_lock) {thrd = &job_threads.GetAdd(HashSymTf(sym, tf));} return *thrd;}
	#ifdef flagGUITASK
	void	PostProcessJobs() {jobs_tc.Kill(); PostCallback(THISBACK(ProcessJobs));}
	#endif
	void	InspectionFailed(const char* file, int line, int symbol, int tf, String msg);
	
	
public:
	
	
	int		GetAccountSymbol() const				{return symbols.GetCount()-1;}
	int		GetCommonSymbolId(int common_pos) const {ASSERT(common_pos >= 0 && common_pos < COMMON_COUNT); return symbols.GetCount()-1-COMMON_COUNT+common_pos;}
	int		GetCommonSymbolId(int common_pos, int symbol_pos) const;
	int		GetCommonCount() const					{return COMMON_COUNT;}
	int		GetCommonSymbolCount() const			{return SYM_COUNT;}
	int		FindCommonSymbolId(int sym_id) const;
	int		FindCommonSymbolPos(int sym_id) const;
	int		FindCommonSymbolPos(int common_id, int sym_id) const;
	
public:
	
	static const int L2_INPUT = 4;
	
	enum {
		BIT_L0BITS_BEGIN,
		BIT_L0BITS_LAST=ASSIST_COUNT-1,
		
		BIT_L2BITS_BEGIN,
		BIT_L2BITS_LAST=ASSIST_COUNT + L2_INPUT - 1,
		
		BIT_WRITTEN_REAL, BIT_WRITTEN_L0, BIT_WRITTEN_L1, BIT_WRITTEN_L2,
		
		BIT_REALSIGNAL,  BIT_REALENABLED,
		BIT_L0_SIGNAL,
		BIT_L1_SIGNAL,
		BIT_L2_ENABLED,
		
		BIT_SKIP_CALENDAREVENT,
		
		BIT_COUNT
	};
	enum {
		REG_INS, REG_WORKQUEUE_CURSOR,
		
		REG_WORKQUEUE_INITED, REG_INDIBITS_INITED,
		REG_LOGICTRAINING_L0_ISRUNNING, REG_LOGICTRAINING_L1_ISRUNNING, REG_LOGICTRAINING_L2_ISRUNNING,
		REG_COUNT
	};
	enum {
		INS_WAIT_NEXTSTEP, INS_REFRESHINDI, INS_INDIBITS, INS_TRAINABLE, INS_CUSTOMLOGIC, INS_CALENDARLOGIC,
		INS_REALIZE_LOGICTRAINING, INS_WAIT_LOGICTRAINING, INS_LOGICBITS, INS_REFRESH_REAL,
		INS_COUNT
	};
	enum {
		MEM_TRAINABLESET, MEM_INDIBARS, MEM_COUNTED_INDI, MEM_COUNTED_ENABLED, MEM_COUNTED_CALENDAR,
		MEM_TRAINBARS,
		MEM_TRAINMIDSTEP,		MEM_TRAINMIDSTEP_LAST=MEM_TRAINMIDSTEP+COMMON_COUNT-1,
		MEM_TRAINBEGIN,			MEM_TRAINBEGIN_LAST=MEM_TRAINBEGIN+COMMON_COUNT-1,
		
		MEM_COUNTED_L0, MEM_COUNTED_L1, MEM_COUNTED_L2,
		MEM_TRAINED_L0, MEM_TRAINED_L1, MEM_TRAINED_L2,
		
		MEM_COUNT
	};
	
	static const int level_count = 3;
	
	struct MainJob {
		Vector<double> training_pts;
		int level = -1, common_pos = -1;
		int actual = 0, total = 1;
		Atomic being_processed;
		bool is_finished = false;
	};
	
	struct LogicLearner0 : Moveable<LogicLearner0> {
		
		static const int SYM_BITS			= 1;
		static const int TIME_BITS			= 5 + 24 + 4;
		static const int INPUT_SIZE			= TIME_BITS + (SYM_COUNT+1) * ASSIST_COUNT * TF_COUNT;
		static const int OUTPUT_SIZE		= (SYM_COUNT+1) * SYM_BITS * TF_COUNT;
		
		typedef DQNTrainer<OUTPUT_SIZE, INPUT_SIZE, 60> DQN;
		
		
		// Persistent
		DQN							dqn_trainer;
		int							dqn_round = 0;
		
		// Temporary
		#ifdef flagDEBUG
		int							dqn_max_rounds		= 500;
		#else
		int							dqn_max_rounds		= 1000000;
		#endif
		
		void	Serialize(Stream& s) {s % dqn_trainer % dqn_round;}
		
	};
	
	typedef LogicLearner0 LogicLearner1;
	
	struct LogicLearner2 : Moveable<LogicLearner2> {
		
		static const int SYM_BITS			= 1;
		static const int TIME_BITS			= 5 + 24 + 4;
		static const int INPUT_SIZE			= TIME_BITS + (SYM_COUNT+1) * L2_INPUT * TF_COUNT;
		static const int OUTPUT_SIZE		= (SYM_COUNT+1) * SYM_BITS * TF_COUNT;
		
		typedef DQNTrainer<OUTPUT_SIZE, INPUT_SIZE, 60> DQN;
		
		
		// Persistent
		DQN							dqn_trainer;
		int							dqn_round = 0;
		
		// Temporary
		#ifdef flagDEBUG
		int							dqn_max_rounds		= 500;
		#else
		int							dqn_max_rounds		= 1000000;
		#endif
		
		void	Serialize(Stream& s) {s % dqn_trainer % dqn_round;}
		
	};
	
	// Persistent
	VectorBool main_data;
	Vector<dword> main_mem;
	Vector<LogicLearner0> logic0;
	Vector<LogicLearner1> logic1;
	Vector<LogicLearner2> logic2;
	
	
	// Temporary
	Vector<int> main_begin;
	Array<MainJob> main_jobs;
	Vector<Core*> ordered_cores;
	Vector<FactoryDeclaration> main_indi_ids;
	Vector<Ptr<CoreItem> > main_work_queue;
	Index<int> main_tf_ids, main_sym_ids, main_factory_ids;
	Atomic workers_started;
	const int main_tf_pos = 1;
	dword main_reg[REG_COUNT];
	int realtime_count = 0;
	int prev_step = -1;
	bool main_stopped = true, main_running = false;
	bool store_this = false;
	RWMutex main_lock;
	Mutex workqueue_lock;
	
	void	StartMain();
	void	StopMain();
	void	MainLoop();
	void	Worker(int id);
	void	RealizeMainWorkQueue();
	void	ProcessEnds();
	void	ProcessMainWorkQueue(bool store_cache=false);
	void	FillIndicatorBits();
	void	FillTrainableBits();
	void	FillCustomLogicBits();
	void	FillCalendarBits();
	void	RealizeLogicTraining();
	void	FillLogicBits();
	bool	RefreshReal();
	void	LearnLogic(int level, int common_pos);
	int		ProcessMainJob(MainJob& job);
	int		GetOrderedCorePos(int sym_pos, int tf_pos, int factory_pos);
	int64	GetMainDataPos(int64 cursor, int64 sym_pos, int64 tf_pos, int64 bit_pos) const;
	void	LoadInput(int level, int common_pos, int cursor, double* buf, int bufsize);
	void	LoadOutput(int level, int common_pos, int cursor, double* buf, int bufsize);
	void	StoreOutput(int level, int common_pos, int cursor, double* buf, int bufsize);
	void	EnterProcessing() {workqueue_lock.Enter();}
	void	LeaveProcessing() {if (store_this) {store_this = false; StoreThis();} workqueue_lock.Leave();}
	String	GetRegisterKey(int i) const;
	String	GetRegisterValue(int i, int j) const;
	String	GetMemoryKey(int i) const;
	String	GetMemoryValue(int i, int j) const;
	void	StoreAll();
	
	template <class T> void LoadInput(int level, int common_pos, int cursor, T& state) {LoadInput(level, common_pos, cursor, state.weights, state.length);}
	
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
