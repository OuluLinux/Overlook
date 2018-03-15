#ifndef _Overlook_Automation_h_
#define _Overlook_Automation_h_

namespace Overlook {


#define GROUP_RESULTS 4
#define MAX_STRANDS 300
#define MAX_STRAND_BITS 20
struct StrandItem {
	int bits[MAX_STRAND_BITS];
	int count = 0;
	
	bool Evolve(int bit);
	void Add(int i) {ASSERT(count < MAX_STRAND_BITS); bits[count++] = i;}
	void Clear() {count = 0;}
};

struct Strand {
	StrandItem enabled,
		signal_true, signal_false,
		trigger_true, trigger_false,
		limit_inc_true, limit_inc_false, limit_dec_true, limit_dec_false,
		weight_inc_true, weight_inc_false, weight_dec_true, weight_dec_false;
	double result[GROUP_RESULTS];
	int sig_bit = 0;
	
	String ToString(int res_id) const;
	String BitString() const;
	void Clear() {
		enabled.Clear(); signal_true.Clear(); signal_false.Clear();
		trigger_true.Clear(); trigger_false.Clear();
		limit_inc_true.Clear(); limit_inc_false.Clear();
		limit_dec_true.Clear(); limit_dec_false.Clear();
		weight_inc_true.Clear(); weight_inc_false.Clear();
		weight_dec_true.Clear(); weight_dec_false.Clear();
		for(int i = 0; i < GROUP_RESULTS; i++)
			result[i] = -DBL_MAX;
	}
};

struct StrandList : Moveable<Strand> {
	Strand strands[MAX_STRANDS * 3];
	int strand_count = 0;
	int cursor[GROUP_RESULTS][USEDSYMBOL_COUNT];
	
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
	void Sort(int res_id);
	void Dump(int res_id);
};


struct Job : Moveable<Job> {
	bool is_finished = false;
	bool is_processing = false;
};

struct JobGroup : Moveable<JobGroup> {
	Job jobs[USEDSYMBOL_COUNT];
	bool is_finished = false;
	
};

class Automation {
	
protected:
	friend class AutomationCtrl;
	friend class BooleansDraw;
	
	
	
	enum {GROUP_SOURCE, GROUP_BITS, GROUP_ENABLE, GROUP_TRIGGER, GROUP_LIMIT, GROUP_WEIGHT, GROUP_COUNT};
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int max_sym_mult = 4;
	static const int jobgroup_count = GROUP_COUNT;
	static const int maxcount = 14*365*5/7*24*12; // 14 years, M5
	
	static const int loadsource_reserved = maxcount;
	
	static const int processbits_period_count = 6;
	static const int processbits_descriptor_count = processbits_period_count + (sym_count - 1) * 2;
	static const int processbits_generic_row = (14 + processbits_descriptor_count);
	static const int processbits_inputrow_size = processbits_period_count * processbits_generic_row;
	static const int processbits_outputrow_size = GROUP_RESULTS * 2;
	static const int processbits_row_size = processbits_inputrow_size + processbits_outputrow_size;
	static const int processbits_reserved = processbits_row_size * sym_count * maxcount;
	static const int processbits_reserved_bytes = processbits_reserved / 64;
	
	FixedOnlineAverageWindow1<1 << 1>		av_wins0[sym_count];
	FixedOnlineAverageWindow1<1 << 2>		av_wins1[sym_count];
	FixedOnlineAverageWindow1<1 << 3>		av_wins2[sym_count];
	FixedOnlineAverageWindow1<1 << 4>		av_wins3[sym_count];
	FixedOnlineAverageWindow1<1 << 5>		av_wins4[sym_count];
	FixedOnlineAverageWindow1<1 << 6>		av_wins5[sym_count];
	FixedExtremumCache<1 << 1>				ec0[sym_count];
	FixedExtremumCache<1 << 2>				ec1[sym_count];
	FixedExtremumCache<1 << 3>				ec2[sym_count];
	FixedExtremumCache<1 << 4>				ec3[sym_count];
	FixedExtremumCache<1 << 5>				ec4[sym_count];
	FixedExtremumCache<1 << 6>				ec5[sym_count];
	StrandList	tmp_meta_added[sym_count];
	StrandList	tmp_single_added[sym_count];
	StrandList	strands[sym_count];
	int			loadsource_pos[sym_count];
	JobGroup	jobgroups[jobgroup_count];
	double		point[sym_count];
	double		spread[sym_count];
	double		output_fmlevel;
	double		open_buf[sym_count][loadsource_reserved];
	float		trigger_result[sym_count][loadsource_reserved];
	uint64		bits_buf[processbits_reserved_bytes];
	int			time_buf[loadsource_reserved];
	int			loadsource_cursor = 0;
	int			processbits_cursor = 0;
	int			worker_cursor = 0;
	int			output_signals[sym_count];
	int			tf;
	bool		running = false, stopped = true;
	
	
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
	void	ProcessBits();
	void	Evolve(int group_id, int job_id);
	void	TestStrand(int group_id, int job_id, Strand& strand, bool write=false);
	
	void	ProcessBitsSingle(int sym, int period_id, int& bit_pos);
	void	SetBit(int pos, int sym, int bit, bool value);
	void	SetBitOutput(int pos, int sym, int bit, bool value) {SetBit(pos, sym, processbits_inputrow_size + bit, value);}
	void	SetBitCurrent(int sym, int bit, bool value) {SetBit(processbits_cursor, sym, bit, value);}
	bool	GetBit(int pos, int sym, int bit) const;
	bool	GetBitOutput(int pos, int sym, int bit) const {return GetBit(pos, sym, processbits_inputrow_size + bit);}
	
	bool	IsRunning() const {return running;}
	int		GetSignal(int sym) {ASSERT(sym >= 0 && sym < sym_count); return output_signals[sym];}
	double	GetFreeMarginLevel() {return output_fmlevel;}
	int		GetFreeMarginScale() {return sym_count * max_sym_mult;}
	int		GetSymGroupJobId(int symbol) const;
	
	
	
	
};


inline Automation& GetAutomation() {return Single<Automation>();}


}

#endif
