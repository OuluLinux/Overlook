#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Config {
extern Upp::IniString arg_addr;
extern Upp::IniInt arg_port;
}

namespace Overlook {
using namespace Upp;


struct Snap : Moveable<Snap> {
	uint64 data[4];
	
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(Snap)); else s.Put(this, sizeof(Snap));}
	void Set(int i, bool value) {int j = i/64; i=i%64;if (value) data[j] |= 1 << i; else data[j] &= ~(1 << i);}
	bool Get(int i) const {int j = i/64; i=i%64; return data[j] & (1 << i);}
};

struct SnapStats : Moveable<SnapStats> {
	int actual = 0, total = 0;
};

struct SnapStatVector : Moveable<SnapStatVector> {
	SnapStats data[SNAP_BITS][2];
	int cursor = 100;
	
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(SnapStatVector)); else s.Put(this, sizeof(SnapStatVector));}
};

#define MAX_STRANDS 100
#define MAX_STRAND_BITS 20
struct StrandItem {
	int bits[MAX_STRAND_BITS];
	int count = 0;
	
	bool Evolve(int bit, StrandItem& dst);
	void Add(int i) {ASSERT(count < MAX_STRAND_BITS); bits[count++] = i;}
	void Clear() {count = 0;}
};

struct Strand {
	StrandItem enabled, signal_true, signal_false, trigger_true, trigger_false;
	long double result = 0.0;
	int sig_bit = 0;
	
	String ToString() const;
	String BitString() const;
	void Clear() {enabled.Clear(); signal_true.Clear(); signal_false.Clear(); trigger_true.Clear(); trigger_false.Clear(); result = -DBL_MAX;}
};

struct StrandList : Moveable<Strand> {
	Strand strands[MAX_STRANDS * 3];
	int strand_count = 0;
	int cursor = 0;
	
	int GetCount() const {return strand_count;}
	bool IsEmpty() const {return strand_count == 0;}
	void SetCount(int i) {ASSERT(i >= 0 && i <= MAX_STRANDS); strand_count = i;}
	void Add() {if (strand_count < MAX_STRANDS * 3) strand_count++;}
	void Add(Strand& s) {if (strand_count < MAX_STRANDS * 3) strands[strand_count++] = s;}
	Strand& operator[] (int i) {ASSERT(i >= 0 && i < MAX_STRANDS * 3); return strands[i];}
	Strand& Top() {ASSERT(strand_count > 0); return strands[strand_count-1];}
	bool Has(Strand& s);
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(StrandList)); else s.Put(this, sizeof(StrandList));}
	void Sort();
	void Dump();
};

struct StrandVector {
	
	VectorBool data;
	int row_size = 4;
	int try_cursor = 0, catch_cursor = 0;
	
	void Serialize(Stream& s) {s % data % row_size % try_cursor % catch_cursor;}
	void SetCount(int i) {data.SetCount(row_size * i);}
	void Reserve(int i) {data.Reserve(row_size * i);}
	int GetCount() const {return data.GetCount() / row_size;}
	void Set(int i, int j, bool b) {data.Set(i * row_size + j, b);}
	bool Get(int i, int j) const {return data.Get(i * row_size + j);}
};

struct Job {
	Vector<Snap>					main_booleans;
	VectorBool						main_signal;
	SnapStatVector					main_stats;
	Vector<Vector<double> >			volat_divs;
	Vector<VectorMap<int,int> >		median_maps;
	OnlineAverageWindow1			stat_osc_ma;
	Vector<OnlineAverageWindow1>	av_wins;
	Vector<ExtremumCache>			ec;
	Vector<OnlineAverageWindow1>	bbma;
	StrandList						try_strands, catch_strands;
	StrandVector					strand_data;
	int phase = 0;
	int end = 0;
	bool is_finished = false;
	
	Mutex							lock;
	Mutex							strand_lock;
	
	// NOTE: update SNAP_BITS
	static const int period_count = 6;
	static const int volat_div = 6;
	static const int extra_row = 2;
	static const int descriptor_count = 6;
	static const int generic_row = (14 + volat_div + descriptor_count);
	static const int row_size = period_count * generic_row + extra_row; // == SNAP_BITS
	
	virtual int GetSymbol() = 0;
	virtual int GetTf() = 0;
	virtual double GetPoint() = 0;
	virtual double GetSpread() = 0;
	virtual const Vector<double>& GetOpen() = 0;
	virtual const Vector<double>& GetLow()  = 0;
	virtual const Vector<double>& GetHigh() = 0;
	virtual bool LoadSources() = 0;
	void LoadBooleans();
	void LoadStats();
	void LoadTryStrands();
	void LoadCatchStrands();
	void TestTryStrand(Strand& st, bool write=false);
	void TestCatchStrand(Strand& st, bool write=false);
	int GetCatchSignal(int pos=-1);
	int GetSignal() {return GetCatchSignal();}
	double GetBestResult() {return catch_strands.GetCount() ? catch_strands[0].result : 0.0;}
	
	// As job
	enum {PHASE_SOURCE, PHASE_BOOLEANS, PHASE_STATS, PHASE_TRYSTRANDS, PHASE_CATCHSTRANDS, PHASE_COUNT};
	String GetPhaseString() const;
	double GetProgress() const;
	bool IsFinished() const;
	void Process();
	
	
	
	void Serialize(Stream& s) {s % main_booleans % main_signal % main_stats % volat_divs % median_maps % stat_osc_ma % av_wins % ec % bbma % try_strands % catch_strands % strand_data % phase % end % is_finished;}
	
};

struct SourceImage : public Job {
	DataBridge db;
	
	
	void Serialize(Stream& s) {s % db; Job::Serialize(s);}
	
	
	double GetAppliedValue ( int applied_value, int i );
	double Open(int shift) {return db.open[shift];}
	double High(int shift) {return db.high[shift];}
	double Low(int shift) {return db.low[shift];}
	double Volume(int shift) {return db.volume[shift];}
	int Time(int shift) {return db.time[shift];}
	int HighestHigh(int period, int shift);
	int LowestLow(int period, int shift);
	int HighestOpen(int period, int shift);
	int LowestOpen(int period, int shift);
	
	virtual int GetSymbol() {return db.GetSymbol();}
	virtual int GetTf() {return db.GetTf();}
	virtual double GetPoint() {return db.GetPoint();}
	virtual double GetSpread() {return db.GetSpread();}
	virtual const Vector<double>& GetOpen() {return db.open;}
	virtual const Vector<double>& GetLow()  {return db.low;}
	virtual const Vector<double>& GetHigh() {return db.high;}
	virtual bool LoadSources();
	
};


struct AccountImage : public Job {
	typedef Tuple<int, int, double, int> State;
	
	int sym = -1, tf = -1;
	double point = 0.0000001;
	Vector<double> gain;
	StrandVector signals;
	double balance = 1000.0;
	Vector<State> current_state;
	
	Vector<Job*> jobs;
	Vector<DataBridge*> dbs;
	
	void Serialize(Stream& s) {s % sym % tf % point % gain % signals % balance % current_state; Job::Serialize(s);}
	virtual int GetSymbol() {return sym;}
	virtual int GetTf() {return tf;}
	virtual double GetPoint() {return point;}
	virtual double GetSpread() {return point * 4;}
	virtual const Vector<double>& GetOpen() {return gain;}
	virtual const Vector<double>& GetLow()  {return gain;}
	virtual const Vector<double>& GetHigh() {return gain;}
	virtual bool LoadSources();
};

struct ImageCompiler {
	static const int MAX_PIPELINE = 128;
	
	FactoryDeclaration pipeline[MAX_PIPELINE];
	int pipeline_size = 0;
	
	void Remove(int i, int replace_id);
public:
	typedef ImageCompiler CLASSNAME;
	ImageCompiler();
	
	void SetMain(const FactoryDeclaration& decl);
	void Compile(SourceImage& si, ChartImage& img);
	
};

void RunFactory(ConstFactoryDeclaration& id, SourceImage& si, ChartImage& ci, GraphImage& gi);
void ConfFactory(ConstFactoryDeclaration& gi, ValueRegister& reg);

enum {TIMEBUF_WEEKTIME, TIMEBUF_COUNT};
enum {CORE_INDICATOR, CORE_EXPERTADVISOR, CORE_HIDDEN};



class System {
	
	
protected:
	
	friend class SystemBooleanView;
	friend class DataBridgeCommon;
	friend class BooleansDraw;
	friend class SystemCtrl;
	friend class DataBridge;
	friend class SimBroker;
	friend class CoreIO;
	friend class Core;
	
	
	// Persistent
	Array<Array<SourceImage> >	data;
	Array<AccountImage>			account;
	
	
	// Temporary
	Index<String>				symbols, allowed_symbols, used_symbols;
	Vector<int>					signals, used_symbols_id;
	Index<int>					periods;
	Vector<String>				period_strings;
	Vector<double>				spread_points;
	Time						end;
	int							time_offset = 0;
	int							realtime_count = 0;
	
	
public:
	
	void	Serialize(Stream& s) {s % data % account;}
	void	StoreThis() {AddJournal("System saving to file"); StoreToFile(*this, ConfigFile("system.bin"));}
	void	LoadThis() {AddJournal("System loading from file"); LoadFromFile(*this, ConfigFile("system.bin"));}
	
	void	AddPeriod(String nice_str, int period);
	void	AddSymbol(String sym);
	
	String	GetSymbol(int i) const					{return symbols[i];}
	String	GetPeriodString(int i) const			{return period_strings[i];}
	int		GetSymbolCount() const					{return symbols.GetCount();}
	int		GetPeriod(int i) const					{return periods[i];}
	int		GetPeriodCount() const					{return periods.GetCount();}
	int		FindPeriod(int period) const			{return periods.Find(period);}
	int		FindSymbol(const String& s) const		{return symbols.Find(s);}
	bool	RefreshReal();
	void	SetSignal(int sym, int i)				{signals[sym] = i;}
	SourceImage& GetSource(int sym, int tf)			{return data[sym][tf];}
	
public:
	Vector<Tuple<String, Time> > journal;
	Vector<Job*> jobs;
	Atomic not_stopped;
	SpinLock workitem_lock;
	int worker_cursor = 0;
	bool running = false, stopped = true;
	Callback WhenJobOrders;
	
	void	StartJobs();
	void	StopJobs();
	void	JobWorker(int i);
	void	AddJournal(String what) {journal.Add(Tuple<String,Time>(what, GetSysTime()));}
	
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
