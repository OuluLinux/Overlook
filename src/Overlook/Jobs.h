#ifndef _Overlook_Jobs_h_
#define _Overlook_Jobs_h_
#if 0


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
	const Strand& operator[] (int i) const {ASSERT(i >= 0 && i < MAX_STRANDS * 3); return strands[i];}
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
	static const int row_size = period_count * generic_row + extra_row;
	
	virtual int GetSymbol() = 0;
	virtual int GetTf() = 0;
	virtual double GetPoint() = 0;
	virtual double GetSpread() = 0;
	virtual const Vector<double>& GetOpen() = 0;
	virtual const Vector<double>& GetLow()  = 0;
	virtual const Vector<double>& GetHigh() = 0;
	virtual const Vector<int>& GetTime() = 0;
	virtual bool LoadSources() = 0;
	virtual void LoadBooleans();
	void LoadStats();
	void LoadTryStrands();
	void LoadCatchStrands();
	virtual void TestTryStrand(Strand& st, bool write=false);
	virtual void TestCatchStrand(Strand& st, bool write=false);
	int GetCatchSignal(int pos=-1);
	int GetSignal() {return GetCatchSignal();}
	double GetBestResult() const {return catch_strands.GetCount() ? catch_strands[0].result : 0.0;}
	
	// As job
	enum {PHASE_SOURCE, PHASE_BOOLEANS, PHASE_STATS, PHASE_TRYSTRANDS, PHASE_CATCHSTRANDS, PHASE_COUNT};
	String GetPhaseString() const;
	double GetProgress() const;
	bool IsFinished() const;
	void Process();
	
	
	
	void Serialize(Stream& s) {s % main_booleans % main_signal % main_stats % volat_divs % median_maps % stat_osc_ma % av_wins % ec % bbma % try_strands % catch_strands % strand_data % phase % end % is_finished;}
	
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
	virtual const Vector<int>& GetTime();
	virtual bool LoadSources();
};

struct MarginImage : public Job {
	typedef Tuple<int, int, double, int> State;
	
	int sym = -1, tf = -1;
	double point = 0.0000001;
	Vector<double> gain, sym_opens;
	StrandVector signals, overwrite_bits;
	double balance = 1000.0;
	Vector<State> current_state;
	Vector<int> src_signal, acc_signal, sym_times;
	Vector<byte> sym_orders;
	
	Vector<Job*> jobs, sym_jobs;
	VectorMap<int, Job*> src_jobs, acc_jobs;
	Vector<State> tmp_current_state;
	Vector<int> tmp_src_signal, tmp_acc_signal;
	Vector<double> tmp_sym_change;
	
	static const int overwrite_row_count = 36; // factorial USEDSYMBOL_COUNT
	
	void Serialize(Stream& s) {s % sym % tf % point % gain % sym_opens % signals % overwrite_bits % balance % current_state % src_signal % acc_signal % sym_times % sym_orders; Job::Serialize(s);}
	virtual int GetSymbol() {return sym;}
	virtual int GetTf() {return tf;}
	virtual double GetPoint() {return point;}
	virtual double GetSpread() {return point * 4;}
	virtual const Vector<double>& GetOpen() {return gain;}
	virtual const Vector<double>& GetLow()  {return gain;}
	virtual const Vector<double>& GetHigh() {return gain;}
	virtual const Vector<int>& GetTime();
	virtual bool LoadSources();
	virtual void LoadBooleans();
	virtual void TestTryStrand(Strand& st, bool write=false);
	virtual void TestCatchStrand(Strand& st, bool write=false);
};




#endif
#endif
