#ifndef _Overlook_Automation_h_
#define _Overlook_Automation_h_

namespace Overlook {


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
	OUT_TRIM_SIG,	OUT_TRIM_ENA,
	OUT_COUNT
};

class Automation {
	
	
protected:
	friend class AutomationCtrl;
	friend class BooleansDraw;
	
	
	enum {GROUP_SOURCE, GROUP_BITS, GROUP_EVOLVE, GROUP_TRIM, GROUP_COUNT};
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int jobgroup_count = GROUP_COUNT;
	static const int maxcount = 14*365*5/7*24; // 14 years, M5
	
	static const int dqn_leftoffset = 10000;
	static const int dqn_rightoffset = 1+1;
	static const int dqn_levels = 5;
	#ifdef flagDEBUG
	static const int max_iters = 1000;
	#else
	static const int max_iters = 10000000;
	#endif
	
	static const int loadsource_reserved = maxcount;
	
	static const int processbits_period_count = 6;
	static const int processbits_descriptor_count = processbits_period_count + (sym_count - 1) * 2;
	static const int processbits_correlation_count = (sym_count - 1);
	static const int processbits_generic_row = (14 + processbits_descriptor_count + processbits_correlation_count);
	static const int processbits_inputrow_size = processbits_period_count * processbits_generic_row;
	static const int processbits_outputrow_size = OUT_COUNT + dqn_levels;
	static const int processbits_row_size = processbits_inputrow_size + processbits_outputrow_size;
	static const int processbits_reserved = processbits_row_size * sym_count * maxcount;
	static const int processbits_reserved_bytes = processbits_reserved / 64;
	
	static const int dqn_output_size = 2 * dqn_levels;
	static const int dqn_input_size = processbits_inputrow_size;
	typedef DQNTrainer<dqn_output_size, dqn_input_size, 100> Dqn;
	
	static const int trimbit_count = 10;
	
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
	Dqn			dqn;
	JobGroup	jobgroups[jobgroup_count];
	double		point[sym_count];
	double		spread[sym_count];
	double		output_fmlevel;
	double		open_buf[sym_count][loadsource_reserved];
	Time		prev_sig_time[sym_count];
	uint64		bits_buf[processbits_reserved_bytes];
	int			prev_sig[sym_count];
	int			dqn_iters[sym_count];
	int			loadsource_pos[sym_count];
	int			trim_cursor[sym_count];
	int			time_buf[loadsource_reserved];
	int			loadsource_cursor = 0;
	int			processbits_cursor = 0;
	int			dqn_cursor[sym_count];
	int			peak_cursor[sym_count];
	int			enable_bits[trimbit_count], possig_bits[trimbit_count], negsig_bits[trimbit_count];
	int			worker_cursor = 0;
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
	void	Evolve(int job_id);
	void	Trim(int job_id);
	
	void	ProcessBitsSingle(int sym, int period_id, int& bit_pos);
	void	SetBit(int pos, int sym, int bit, bool value);
	void	SetBitOutput(int pos, int sym, int bit, bool value) {SetBit(pos, sym, processbits_inputrow_size + bit, value);}
	void	SetBitCurrent(int sym, int bit, bool value) {SetBit(processbits_cursor, sym, bit, value);}
	bool	GetBit(int pos, int sym, int bit) const;
	bool	GetBitOutput(int pos, int sym, int bit) const {return GetBit(pos, sym, processbits_inputrow_size + bit);}
	double	TestTrim(int job_id, int bit, int type);
	
	bool	IsRunning() const {return running;}
	int		GetSignal(int sym);
	double	GetFreeMarginLevel() {return output_fmlevel;}
	int		GetFreeMarginScale() {return sym_count * dqn_levels;}
	int		GetSymGroupJobId(int symbol) const;
	
	void	LoadInput(Dqn::MatType& input, int sym, int pos);
	void	LoadOutput(double output[dqn_output_size], int sym, int pos);
	
};


inline Automation& GetAutomation() {return Single<Automation>();}


}

#endif
