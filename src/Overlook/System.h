#ifndef _Overlook_System_h_
#define _Overlook_System_h_

namespace Config {
extern Upp::IniString arg_addr;
extern Upp::IniInt arg_port;
}

namespace Overlook {
using namespace Upp;



struct SourceImage {
	DataBridge db;
	
	void Serialize(Stream& s) {s % db;}
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
	SnapStats data[90][2];
	int cursor = 100;
	
	void Serialize(Stream& s) {if (s.IsLoading()) s.Get(this, sizeof(SnapStatVector)); else s.Put(this, sizeof(SnapStatVector));}
};

void RunFactory(ConstFactoryDeclaration& id, SourceImage& si, ChartImage& ci, GraphImage& gi);
void ConfFactory(ConstFactoryDeclaration& gi, ValueRegister& reg);

enum {TIMEBUF_WEEKTIME, TIMEBUF_COUNT};
enum {CORE_INDICATOR, CORE_EXPERTADVISOR, CORE_HIDDEN};

class System {
	
	
protected:
	
	friend class DataBridgeCommon;
	friend class BooleansDraw;
	friend class SystemCtrl;
	friend class DataBridge;
	friend class SimBroker;
	friend class CoreIO;
	friend class Core;
	
	
	// Persistent
	Array<Array<SourceImage> >	data;
	Vector<Vector<Snap> >		main_booleans;
	Vector<VectorBool>			main_signal;
	Vector<SnapStatVector>		main_stats;
	
	
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
	
	void	Serialize(Stream& s) {s % data % main_booleans % main_signal % main_stats;}
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
