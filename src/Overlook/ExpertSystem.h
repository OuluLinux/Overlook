#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

namespace Overlook {

class SyncedPrice {
	static const int sym_count = USEDSYMBOL_COUNT;
	
protected:
	friend class SyncedPriceManager;
	friend class BitProcess;
	friend class ExchangeSlots;
	friend class SlotSignals;
	
	struct Data : Moveable<Data> {
		Vector<double> open, low, high;
		double point, spread;
		int loadsource_pos = 0;
		void Serialize(Stream& s) {s % open % low % high % point % spread % loadsource_pos;}
	};
	
	Vector<Data> data;
	Vector<Time> time;
	int loadsource_cursor = 0;
	int tf = -1, period = 0;
	
	Mutex lock;
	
public:
	SyncedPrice();
	void Refresh();
	
	String GetPath();
	void LoadThis();
	void StoreThis();
	void Serialize(Stream& s) {s % data % time % loadsource_cursor % tf % period;}
	
};

class SyncedPriceManager {
	ArrayMap<int, SyncedPrice> data;
	
public:
	SyncedPriceManager();
	SyncedPrice& Get(int tf);
	
};

inline SyncedPriceManager& GetSyncedPriceManager() {return Single<SyncedPriceManager>();}


class BitProcess {
	
protected:
	friend class BitProcessManagerCtrl;
	friend class BitProcessManager;
	friend class BooleansDraw;
	friend class SlotSignals;
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int processbits_period_count = 6;
	static const int processbits_descriptor_count = processbits_period_count + (sym_count - 1) * 2;
	static const int processbits_correlation_count = (sym_count - 1);
	static const int processbits_generic_row = (14 + processbits_descriptor_count + processbits_correlation_count);
	static const int processbits_inputrow_size = processbits_period_count * processbits_generic_row;
	static const int processbits_outputrow_size = processbits_period_count;
	static const int processbits_row_size = processbits_inputrow_size + processbits_outputrow_size;
	
	
	Vector<uint64> bits_buf;
	FixedOnlineAverageWindow1<1 << 1>		av_wins0;
	FixedOnlineAverageWindow1<1 << 2>		av_wins1;
	FixedOnlineAverageWindow1<1 << 3>		av_wins2;
	FixedOnlineAverageWindow1<1 << 4>		av_wins3;
	FixedOnlineAverageWindow1<1 << 5>		av_wins4;
	FixedOnlineAverageWindow1<1 << 6>		av_wins5;
	FixedExtremumCache<1 << 1>				ec0;
	FixedExtremumCache<1 << 2>				ec1;
	FixedExtremumCache<1 << 3>				ec2;
	FixedExtremumCache<1 << 4>				ec3;
	FixedExtremumCache<1 << 5>				ec4;
	FixedExtremumCache<1 << 6>				ec5;
	int used_sym = -1, tf = -1;
	int processbits_cursor = 0;
	
	
	void	ProcessBitsSingle(const SyncedPrice& data, int period_id, int& bit_pos);
	void	SetBit(int pos, int bit, bool value);
	void	SetBitOutput(int pos, int bit, bool value) {SetBit(pos, processbits_inputrow_size + bit, value);}
	void	SetBitCurrent(int bit, bool value) {SetBit(processbits_cursor, bit, value);}
	bool	GetBit(int pos, int bit) const;
	bool	GetBitOutput(int pos, int bit) const {return GetBit(pos, processbits_inputrow_size + bit);}
	
	
public:
	BitProcess();
	void Refresh();
	
	String GetPath();
	void LoadThis();
	void StoreThis();
	void Serialize(Stream& s);
	
};

class BitProcessManager {
	ArrayMap<int, BitProcess> data;
	
public:
	BitProcessManager();
	BitProcess& Get(int used_sym, int tf);
	
	
};

inline BitProcessManager& GetBitProcessManager() {return Single<BitProcessManager>();}


class Currency : Moveable<Currency> {
	typedef Tuple<int, int> OpenClose;
	
	String name, tz;
	Vector<OpenClose> local;
	
	
public:
	
	String GetTZ() const {return tz;}
	String GetName() const {return name;}
	int GetLocalCount() const {return local.GetCount();}
	void GetLocalOpen(int i, Time& t)  const {t.hour = local[i].a / 100; t.minute = local[i].a % 100;}
	void GetLocalClose(int i, Time& t) const {t.hour = local[i].b / 100; t.minute = local[i].b % 100;}
	
	Currency& AddOpenLocal(int open, int close) {local.Add(OpenClose(open, close)); return *this;}
	
	Currency& Set(String name) {this->name = name; return *this;}
	Currency& SetTZ(String tz) {this->tz = tz; return *this;}
	
	void Serialize(Stream& s) {s % name % tz % local;}
	
};

class TimeSlot : Moveable<TimeSlot> {
	
public:
	String id;
	Vector<String> open, close;
	Vector<String> was_open, is_open;
	
public:
	
	String GetID() const {return id;}
	String ToString() const {return "open " + Join(open, ",") + " close " + Join(close, ",") + " was " + Join(was_open, ",") + " is " + Join(is_open, ",");}
	
	void Add(String s) {id << s;}
	void AddOpen(String s) {open.Add(s);}
	void AddClose(String s) {close.Add(s);}
	unsigned GetHashValue() const {
		CombineHash ch;
		for(int i = 0; i < was_open.GetCount(); i++) ch << ::GetHashValue(was_open[i]) << 1;
		for(int i = 0; i < is_open.GetCount(); i++)  ch << ::GetHashValue(is_open[i])  << 1;
		return ch;
	}
	
	void Serialize(Stream& s) {s % id % open % close % was_open % is_open;}
	
};

class DaySlots {
	
	
public:
	VectorMap<Time, TimeSlot> time_points;
	
	
	void Clear() {time_points.Clear();}
};

class Slot : Moveable<Slot> {
	
	
public:
	Vector<Time> open_time, close_time;
	Vector<int> open_pos, close_pos;
	String id;
	
public:
	
	void Serialize(Stream& s) {s % open_time % close_time % open_pos % close_pos % id;}
	
};

class ExchangeSlots  {
	
protected:
	friend class ExchangeSlotsCtrl;
	friend class SlotSignals;
	
	Vector<Currency> currencies;
	VectorMap<unsigned, Slot> slots;
	Time cursor;
	int findtime_cursor = 0;
	
	bool is_loaded = false;
	
public:
	typedef ExchangeSlots CLASSNAME;
	ExchangeSlots();
	~ExchangeSlots();
	
	void GetDaySlots(Date date, DaySlots& slot);
	
	void Refresh();
	
	String GetPath();
	void LoadThis() {LoadFromFile(*this, GetPath());}
	void StoreThis() {StoreToFile(*this, GetPath());}
	void Serialize(Stream& s) {s % currencies % slots % cursor % findtime_cursor;}
	
};

inline ExchangeSlots& GetExchangeSlots() {return Single<ExchangeSlots>();}

void TestExchangeSlots();

class SlotSignal : Moveable<SlotSignal> {
	
	
public:
	struct Data : Moveable<Data> {
		Time open_time, close_time;
		int open_pos = -1, close_pos = -1;
		double pred_value[USEDSYMBOL_COUNT];
		bool signal[USEDSYMBOL_COUNT];
		bool pred_signal[USEDSYMBOL_COUNT];
		 
		void Serialize(Stream& s) {
			if (s.IsLoading())
				s.Get(this, sizeof(Data));
			else
				s.Put(this, sizeof(Data));
		}
	};
	Vector<Vector<double> > correlation;
	Vector<Data> data;
	String id;
	
public:
	
	void Serialize(Stream& s) {s % correlation % data % id;}
	
	int GetSignal(int sym);
};


class SlotSignals {
	
public:
	VectorMap<unsigned, SlotSignal> slots;
	
	bool is_loaded = false;
	
public:
	typedef SlotSignals CLASSNAME;
	SlotSignals();
	~SlotSignals();
	
	void Refresh();
	double Correlation(SlotSignal& slot, int sym, int bit);
	double Predict(SlotSignal& slot, int sym, int datapos);
	bool TryGetSignal(SlotSignal& slot, int sym, int datapos);
	
	String GetPath() {return ConfigFile("SlotSignals.bin");}
	void LoadThis() {LoadFromFile(*this, GetPath());}
	void StoreThis() {StoreToFile(*this, GetPath());}
	void Serialize(Stream& s) {s % slots;}
	
	SlotSignal* FindCurrent();
	
};

inline SlotSignals& GetSlotSignals() {return Single<SlotSignals>();}

}

#endif
