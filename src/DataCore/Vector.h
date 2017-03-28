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
	typedef Vector<byte> SlotData;
	typedef Ptr<TimeVector> TimeVectorPtr;
	
	class Iterator {
		Ptr<TimeVector> tv;
		Vector<Vector< Vector<SlotData>::Iterator > > iter;
		Vector<int> pos;
		
	public:
		Iterator(TimeVectorPtr tv);
		Iterator(const Iterator& it);
		
		int GetPosition(int tf_id) const {return pos[tf_id];}
		TimeVector& GetTimeVector() {return *tv;}
		
		void SetPosition(int i);
		bool Process();
		void ReleaseMemory();
		
		bool IsBegin() const {return pos[0] == 0;}
		bool IsEnd();
		
		void operator ++(int i);
		void operator =(const Iterator& it);
		
	};
	
protected:
	friend class Iterator;
	
	Upp::FileAppend cache_file;
	Upp::SpinLock cache_lock;
	PathLink link_root;
	String cache_file_path;
	Time begin, end;
	int64 reserved_memory;
	int64 memory_limit;
	int timediff;
	int base_period;
	int begin_ts, end_ts;
	int header_size;
	int bars_total;
	int slot_flag_bytes, slot_flag_offset;
	bool reversed;
	bool enable_cache;
	
	Index<int> periods;
	Vector<Vector<Vector<SlotData> > > data;
	Vector<SlotPtr> slot;
	Vector<int> tfbars_in_slowtf;
	VectorMap<String, SlotPtr> resolved_slots;
	VectorMap<String, String> linked_paths;
	Vector<String> symbols;
	Vector<int> bars, slot_bytes;
	int total_slot_bytes;
	
	
	typedef SlotPtr (*SlotFactory)();
	inline static VectorMap<String, SlotFactory>& GetFactories() {return Single<VectorMap<String, SlotFactory> >();}
	static void AddCustomSlot(String key, SlotFactory f);
	template <class T> static SlotPtr FactoryFn() { return new T; }
	
public:
	TimeVector();
	~TimeVector();
	
	void AddPeriod(int period);
	void AddSymbol(String sym);
	void RefreshData();
	void EnableCache(bool b=true) {enable_cache = b;}
	void LimitMemory(int64 limit=2147483648L) {memory_limit = limit;}
	void LoadCache(int sym_id, int tf_id, int pos);
	
	bool IsReversed() const {return reversed;}
	Time GetTime(int period, int pos) const {return begin + base_period * period * pos * (reversed ? -1 : 1);}
	Time GetBegin() const {return begin;}
	Time GetEnd() const {return end;}
	int GetCount(int period) const {return timediff / base_period / period * (reversed ? -1 : 1);}
	int GetBeginTS() {return begin_ts;}
	int GetEndTS() {return end_ts;}
	int GetBasePeriod() const {return base_period;}
	int GetShift(int src_period, int dst_period, int shift);
	int GetShiftFromTime(int timestamp, int period);
	int GetTfFromSeconds(int period_seconds);
	int64 GetPersistencyCursor(int sym_id, int tf_id, int shift);
	const SlotData& GetSlot(int sym_id, int tf_id, int shift) const {return data[sym_id][tf_id][shift];}
	int FindPeriod(int period) const {return periods.Find(period);}
	
	void SetCacheFile(String path) {cache_file_path = path;}
	void SetBegin(Time t)	{begin = t; begin_ts = (int)(t.Get() - Time(1970,1,1).Get());}
	void SetEnd(Time t)	{end = t; end_ts = (int)(t.Get() - Time(1970,1,1).Get()); timediff = (int)(end.Get() - begin.Get()); reversed = begin_ts > end_ts;}
	void SetBasePeriod(int period)	{base_period = period;}
	
	Iterator Begin();
	
	void LinkPath(String dest, String src);
	PathLinkPtr FindLinkPath(String path);
	void ParsePath(String path, PathArgs& out);
	
	SlotPtr ResolvePath(String path);
	SlotPtr ResolvePath(String path, const PathArgs& parsed_path);
	
	template <class T> static void Register() {AddCustomSlot(T().GetKey(), &TimeVector::FactoryFn<T>); }
	static void Chk() {ASSERT_(GetFactories().GetCount(), "DataCore.icpp initblock loading failed. Add newline to DataCore.icpp and recompile program.");}
	
};


inline TimeVector& GetTimeVector() {return Single<TimeVector>();}







}

#endif
