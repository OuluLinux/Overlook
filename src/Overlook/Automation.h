#ifndef _Overlook_Automation_h_
#define _Overlook_Automation_h_

namespace Overlook {

#define MAX_STRANDS 300
#define MAX_STRAND_BITS 20
struct StrandItem {
	int bits[MAX_STRAND_BITS];
	int count = 0;

	bool Evolve(int bit);
	void Add(int i) {ASSERT(count < MAX_STRAND_BITS); bits[count++] = i;}
	void Clear() {count = 0;}
	unsigned GetHashValue() const {
		CombineHash ch;
		for(int i = 0; i < count; i++)
			ch << bits[i] << 1;
		return ch;
	}
};

struct Strand {
	StrandItem enabled, signal_true, signal_false;
	double result = 0;
	int sig_bit = 0;

	String ToString() const;
	String BitString() const;
	void Clear() {
		enabled.Clear(); signal_true.Clear(); signal_false.Clear();
		result = -DBL_MAX;
	}
	unsigned GetHashValue() const {
		CombineHash ch;
		ch	<< enabled.GetHashValue() << 1
			<< signal_true.GetHashValue() << 1
			<< signal_false.GetHashValue() << 1;
		ch << (int)sig_bit << 1;
		return ch;
	}
};

struct StrandList : Moveable<Strand> {
	Strand strands[MAX_STRANDS * 3];
	int strand_count = 0;
	int cursor;
	
	StrandList() {Clear();}
	void Clear() {memset(this, 0, sizeof(StrandList));}
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



struct Job : Moveable<Job> {
	bool is_finished = false;
	bool is_processing = false;
};

struct JobGroup : Moveable<JobGroup> {
	Job jobs[USEDSYMBOL_COUNT];
	bool is_finished = false;
	
};

enum {
	OUT_EVOLVE_SIG,	OUT_EVOLVE_ENA,
	OUT_COUNT
};

#define PERIOD_SHIFT 1

class SlowAutomation {
	
protected:
	friend class AutomationCtrl;
	friend class GameMatchCtrl;
	friend class Automation;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int wdayhours = 5*24; // H1
	static const int maxcount = 14*52*wdayhours; // 14 years
	
	#ifdef flagDEBUG
	static const int MAX_ITERS = 2;
	#else
	static const int MAX_ITERS = 8;
	#endif
	
	static const int loadsource_reserved = maxcount;
	
	static const int processbits_period_count = 6;
	static const int processbits_descriptor_count = processbits_period_count + (sym_count - 1) * 2;
	static const int processbits_correlation_count = (sym_count - 1);
	static const int processbits_generic_row = (14 + processbits_descriptor_count + processbits_correlation_count);
	static const int processbits_inputrow_size = processbits_period_count * processbits_generic_row;
	static const int processbits_outputrow_size = OUT_COUNT;
	static const int processbits_row_size = processbits_inputrow_size + processbits_outputrow_size;
	static const int processbits_row_size_aliased = processbits_row_size - processbits_row_size % 64 + 64;
	static const int processbits_reserved = processbits_row_size_aliased * maxcount;
	static const int processbits_reserved_bytes = processbits_reserved / 64;
	
	enum {ACTION_LONG, ACTION_SHORT, ACTION_IDLE};
	
	
	
	FixedOnlineAverageWindow1<1 << (1 + PERIOD_SHIFT)>		av_wins0;
	FixedOnlineAverageWindow1<1 << (2 + PERIOD_SHIFT)>		av_wins1;
	FixedOnlineAverageWindow1<1 << (3 + PERIOD_SHIFT)>		av_wins2;
	FixedOnlineAverageWindow1<1 << (4 + PERIOD_SHIFT)>		av_wins3;
	FixedOnlineAverageWindow1<1 << (5 + PERIOD_SHIFT)>		av_wins4;
	FixedOnlineAverageWindow1<1 << (6 + PERIOD_SHIFT)>		av_wins5;
	FixedExtremumCache<1 << (1 + PERIOD_SHIFT)>				ec0;
	FixedExtremumCache<1 << (2 + PERIOD_SHIFT)>				ec1;
	FixedExtremumCache<1 << (3 + PERIOD_SHIFT)>				ec2;
	FixedExtremumCache<1 << (4 + PERIOD_SHIFT)>				ec3;
	FixedExtremumCache<1 << (5 + PERIOD_SHIFT)>				ec4;
	FixedExtremumCache<1 << (6 + PERIOD_SHIFT)>				ec5;
	OnlineAverage1							slot_stats[wdayhours];
	StrandList	strands, meta_added, single_added;
	uint64		bits_buf[processbits_reserved_bytes];
	double		point;
	double		spread;
	double		open_buf[loadsource_reserved];
	Time		prev_sig_time;
	int			sym, tf, period;
	int			time_buf[loadsource_reserved];
	int			prev_sig;
	int			loadsource_pos;
	int			processbits_cursor;
	int			loadsource_cursor = 0;
	int			output_cursor;
	int			peak_cursor;
	int			most_matching_pos;
	bool		not_first;
	bool		enabled_slot[wdayhours];
	
	double*		other_open_buf[sym_count];
	bool*		running;
	
	
public:
	
	
	
	void	ProcessBits();
	void	Evolve();
	
	void	ProcessBitsSingle(int period_id, int& bit_pos);
	void	SetBit(int pos, int bit, bool value);
	void	SetBitOutput(int pos, int bit, bool value) {SetBit(pos, processbits_inputrow_size + bit, value);}
	void	SetBitCurrent(int bit, bool value) {SetBit(processbits_cursor, bit, value);}
	bool	GetBit(int pos, int bit) const;
	bool	GetBitOutput(int pos, int bit) const {return GetBit(pos, processbits_inputrow_size + bit);}
	int		GetBitDiff(int a, int b);
	void    GetOutputValues(bool& signal, int& level);
	void	TestStrand(Strand& st);
	double	TestAction(int& pos, int action);
	int		GetAction(Strand& st, int cursor);
	int		GetAction(int cursor);
};

class Automation {
	
	
protected:
	friend class AutomationCtrl;
	friend class GameMatchCtrl;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	
	enum {GROUP_SOURCE, GROUP_BITS, GROUP_EVOLVE, GROUP_COUNT};
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int jobgroup_count = GROUP_COUNT;
	
	SlowAutomation	slow[sym_count];
	JobGroup		jobgroups[jobgroup_count];
	double			output_fmlevel;
	int				worker_cursor = 0;
	bool			running = false, stopped = true;
	
	
	// Temp
	Atomic		not_stopped;
	SpinLock	workitem_lock;
	
public:
	typedef Automation CLASSNAME;
	
	Automation();
	~Automation();
	void	StartJobs();
	void	StopJobs();
	void	JobWorker(int i);
	void	LoadThis() {LoadFromFile(*this, ConfigFile("Automation.bin"));}
	void	StoreThis() {StoreToFile(*this, ConfigFile("Automation.bin"));}
	void	Serialize(Stream& s);
	void	Process(int group_id, int job_id);
	
	void	LoadSource();
	
	bool	IsRunning() const {return running;}
	double	GetFreeMarginLevel() {return output_fmlevel;}
	int		GetFreeMarginScale() {return sym_count;}
	int		GetSymGroupJobId(int symbol) const;
	
	
};


inline Automation& GetAutomation() {return Single<Automation>();}


}

#endif
