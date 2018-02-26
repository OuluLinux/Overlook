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
struct Strand {
	int enabled[MAX_STRAND_BITS], signal[MAX_STRAND_BITS];
	int enabled_count = 0, signal_count = 0;
	double result = -DBL_MAX;
	
	void AddEnabled(int i) {ASSERT(signal_count < MAX_STRAND_BITS); enabled[enabled_count++] = i;}
	void AddSignal(int i) {ASSERT(signal_count < MAX_STRAND_BITS); signal[signal_count++] = i;}
	bool EvolveSignal(int bit, Strand& dst);
	bool EvolveEnabled(int bit, Strand& dst);
	String ToString() const;
	String BitString() const;
	void Clear() {enabled_count = 0; signal_count = 0; result = -DBL_MAX;}
};

struct StrandList : Moveable<Strand> {
	Strand strands[MAX_STRANDS * 3];
	int strand_count = 0;
	int cursor = 0;
	
	int GetCount() const {return strand_count;}
	bool IsEmpty() const {return strand_count == 0;}
	void SetCount(int i) {ASSERT(i >= 0 && i <= MAX_STRANDS); strand_count = i;}
	Strand& Add() {ASSERT(strand_count < MAX_STRANDS * 3); return strands[strand_count++];}
	Strand& Add(Strand& s) {ASSERT(strand_count < MAX_STRANDS * 3); strands[strand_count] = s; return strands[strand_count++];}
	Strand& operator[] (int i) {ASSERT(i >= 0 && i < MAX_STRANDS * 3); return strands[i];}
	Strand& Top() {ASSERT(strand_count > 0); return strands[strand_count-1];}
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(StrandList)); else s.Put(this, sizeof(StrandList));}
	void Sort();
	void Dump();
};



struct SourceImage {
	DataBridge db;
	Vector<Snap>					main_booleans;
	VectorBool						main_signal;
	SnapStatVector					main_stats;
	Vector<Vector<double> >			volat_divs;
	Vector<VectorMap<int,int> >		median_maps;
	OnlineAverageWindow1			stat_osc_ma;
	Vector<OnlineAverageWindow1>	av_wins;
	Vector<ExtremumCache>			ec;
	Vector<OnlineAverageWindow1>	bbma;
	StrandList						strands;
	
	int								lock = 0;
	
	void Serialize(Stream& s) {s % db % main_booleans % main_signal % main_stats % volat_divs % median_maps % stat_osc_ma % av_wins % ec % bbma % strands;}
	
	// NOTE: update SNAP_BITS
	static const int period_count = 6;
	static const int volat_div = 6;
	static const int extra_row = 2;
	static const int generic_row = (13 + volat_div);
	static const int row_size = period_count * generic_row + extra_row; // == SNAP_BITS
	
	void LoadSources();
	void LoadBooleans();
	void LoadStats();
	void LoadStrands();
	void LoadReal();
	void LoadAll() {if (lock++) {lock--; return;} LoadSources(); LoadBooleans(); LoadStats(); LoadStrands(); LoadReal(); lock--;}
	void TestStrand(Strand& st);
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
	
	
	// Temporary
	Index<String>				symbols, allowed_symbols;
	Vector<int>					signals;
	Index<int>					periods;
	Vector<String>				period_strings;
	Vector<double>				spread_points;
	Time						end;
	int							time_offset = 0;
	int							realtime_count = 0;
	
	
public:
	
	void	Serialize(Stream& s) {s % data;}
	void	StoreThis() {StoreToFile(*this, ConfigFile("system.bin"));}
	void	LoadThis() {LoadFromFile(*this, ConfigFile("system.bin"));}
	
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
