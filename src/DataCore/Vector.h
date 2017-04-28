#ifndef _DataCore_Vector_h_
#define _DataCore_Vector_h_

#include "Slot.h"

namespace DataCore {





enum LEX_TYPES {
    LEX_EOF = 0,
    LEX_ID = 256,
    LEX_INT,
    LEX_FLOAT,
    LEX_STR,
    
    LEX_ASSIGN,
    LEX_SLASH,
    LEX_ARGBEGIN,
    LEX_ARGNEXT,
};

class PathLexer {
	
public:
	
	char curr_ch, next_ch;
    int tk;
    int token_start;
    int token_end;
    int token_last_end;
    String tk_str;
    String data;
    int data_start, data_end;
    bool data_owned;
    int data_pos;
    
public:
	PathLexer(String input);
	
    void GetNextCh();
    void GetNextToken();
    
    void Match(int expected_tk);
    static String GetTokenStr(int token);
    void Reset();
    
};


struct PathLink : Moveable<PathLink>, public Pte<PathLink> {
	Ptr<Slot> link;
	String path;
	VectorMap<String, PathLink> keys;
};

typedef Ptr<PathLink> PathLinkPtr;
typedef Tuple2<String, VectorMap<String, Value> > PathArg;
typedef Vector<PathArg> PathArgs;

String EncodePath(const PathArgs& path);
String EncodePath(const PathArgs& path, int pos);


/*
	Vector has:
	 - symbols
	 - timeframes
	 - registered slot processors
	 - slots linked to processors
*/
class TimeVector : public Pte<TimeVector> {
	
public:
	typedef Ptr<TimeVector> TimeVectorPtr;
	
	
protected:
	friend class Iterator;
	friend class Slot;
	friend class Session;
	
	PathLink link_root;
	Time begin, end;
	int64 reserved_memory;
	int64 memory_limit;
	int64 bars_total;
	int timediff;
	int base_period;
	int begin_ts, end_ts;
	bool enable_cache;
	
	VectorMap<String, SlotPtr> resolved_slots;
	VectorMap<String, String> linked_paths;
	Vector<SlotPtr> slot;
	Index<String> symbols;
	Index<int> periods;
	Vector<int> tfbars_in_slowtf;
	Vector<int> bars;
	SlotProcessAttributes* current_slotattr;
	
	
	typedef SlotPtr (*SlotFactory)();
	inline static VectorMap<String, SlotFactory>& GetFactories() {return Single<VectorMap<String, SlotFactory> >();}
	static void AddCustomSlot(String key, SlotFactory f);
	template <class T> static SlotPtr FactoryFn() { return new T; }
	
public:
	TimeVector();
	~TimeVector();
	
	void Serialize(Stream& s);
	
	void AddPeriod(int period);
	void AddSymbol(String sym);
	void RefreshData();
	void EnableCache(bool b=true) {enable_cache = b;}
	void LimitMemory(int64 limit=2147483648L) {memory_limit = limit;}
	
	Time GetTime(int period, int pos) const {return begin + base_period * period * pos;}
	Time GetBegin() const {return begin;}
	Time GetEnd() const {return end;}
	int GetSymbolCount() const {return symbols.GetCount();}
	String GetSymbol(int i) const {return symbols[i];}
	int GetCount(int period) const;
	int GetBeginTS() {return begin_ts;}
	int GetEndTS() {return end_ts;}
	int GetBasePeriod() const {return base_period;}
	int64 GetShift(int src_period, int dst_period, int shift);
	int64 GetShiftFromTime(int timestamp, int period);
	int GetTfFromSeconds(int period_seconds);
	int GetPeriod(int i) const {return periods[i];}
	int GetPeriodCount() const {return periods.GetCount();}
	int GetCustomSlotCount() const {return slot.GetCount();}
	const Slot& GetCustomSlot(int i) const {return *slot[i];}
	Slot& GetCustomSlot(int i) {return *slot[i];}
	int FindPeriod(int period) const {return periods.Find(period);}
	SlotProcessAttributes& GetCurrent() const {return *current_slotattr;}
	
	
	void SetBegin(Time t)	{begin = t; begin_ts = (int)(t.Get() - Time(1970,1,1).Get());}
	void SetEnd(Time t)	{end = t; end_ts = (int)(t.Get() - Time(1970,1,1).Get()); timediff = (int)(end.Get() - begin.Get());}
	void SetBasePeriod(int period)	{base_period = period;}
	
	void LinkPath(String dest, String src);
	PathLinkPtr FindLinkPath(String path);
	SlotPtr FindLinkSlot(const String& path);
	void ParsePath(String path, PathArgs& out);
	
	SlotPtr ResolvePath(String path);
	SlotPtr ResolvePath(String path, const PathArgs& parsed_path);
	
	template <class T> static void Register() {AddCustomSlot(T().GetKey(), &TimeVector::FactoryFn<T>); }
	static void Chk() {ASSERT_(GetFactories().GetCount(), "DataCore.icpp initblock loading failed. Add newline to DataCore.icpp and recompile program.");}
	
};










}

#endif
