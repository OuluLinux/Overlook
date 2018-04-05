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
	/*
	virtual int GetSymbol() {return db.GetSymbol();}
	virtual int GetTf() {return db.GetTf();}
	virtual double GetPoint() {return db.GetPoint();}
	virtual double GetSpread() {return db.GetSpread();}
	virtual const Vector<double>& GetOpen() {return db.open;}
	virtual const Vector<double>& GetLow()  {return db.low;}
	virtual const Vector<double>& GetHigh() {return db.high;}
	virtual const Vector<int>& GetTime() {return db.time;}
	virtual bool LoadSources();
	*/
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
	
	friend class SlotTrailingsCtrl;
	friend class SystemBooleanView;
	friend class DataBridgeCommon;
	friend class SlotSignalsCtrl;
	friend class AutomationCtrl;
	friend class SlowAutomation;
	friend class BooleansDraw;
	friend class SyncedPrice;
	friend class Automation;
	friend class DataBridge;
	friend class SimBroker;
	friend class GameCtrl;
	friend class Game;
	
	
	// Persistent
	Array<Array<SourceImage> >	data;
	
	
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
	
	void	Serialize(Stream& s) {s % data;}
	void	StoreThis();
	void	LoadThis();
	
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
	
	Vector<Tuple<String, Time> > journal;
	void	AddJournal(String what) {journal.Add(Tuple<String,Time>(what, GetSysTime()));}
	
	Callback2<int,int> WhenProgress;
	Callback2<int,int> WhenSubProgress;
	Callback1<String>  WhenInfo;
	Callback1<String>  WhenError;
	Callback1<String>  WhenPushTask;
	Callback           WhenRealtimeUpdate;
	Callback           WhenPopTask;
	Callback           WhenJobOrders;
};

inline System& GetSystem()							{return Single<System>();}


}

#endif
