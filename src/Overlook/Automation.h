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
	OUT_COUNT
};

class SlowAutomation {
	
protected:
	friend class AutomationCtrl;
	friend class Automation;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int wdayhours = 5*24*12; // M5
	static const int maxcount = 14*52*wdayhours; // 14 years
	
	static const int dqn_leftoffset = 10000;
	static const int dqn_rightoffset = 6+1;
	#ifdef flagDEBUG
	static const int max_iters = 10000;
	#else
	static const int max_iters = 5000000;
	#endif
	
	static const int loadsource_reserved = maxcount;
	
	static const int processbits_period_count = 6;
	static const int processbits_descriptor_count = processbits_period_count + (sym_count - 1) * 2;
	static const int processbits_correlation_count = (sym_count - 1);
	static const int processbits_generic_row = (14 + processbits_descriptor_count + processbits_correlation_count);
	static const int processbits_inputrow_size = processbits_period_count * processbits_generic_row;
	static const int processbits_outputrow_size = OUT_COUNT;
	static const int processbits_row_size = processbits_inputrow_size + processbits_outputrow_size;
	static const int processbits_reserved = processbits_row_size * maxcount;
	static const int processbits_reserved_bytes = processbits_reserved / 64;
	
	static const int dqn_output_size = 2;
	static const int dqn_input_size = processbits_inputrow_size;
	typedef DQNTrainer<dqn_output_size, dqn_input_size, 100> Dqn;
	
	static const int dqn_items_count = 1000;
	uint64 dqn_items_total = 0;
	int dqn_item_cursor = 0;
	Dqn::DQItemType train_cache[dqn_items_count];
	
	
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
	OnlineAverage1							slot_stats[wdayhours];
	Dqn			dqn;
	uint64		bits_buf[processbits_reserved_bytes];
	double		point;
	double		spread;
	double		open_buf[loadsource_reserved];
	Time		prev_sig_time;
	int			sym, tf, period;
	int			time_buf[loadsource_reserved];
	int			prev_sig;
	int			dqn_iters;
	int			loadsource_pos;
	int			processbits_cursor;
	int			loadsource_cursor = 0;
	int			dqn_cursor;
	int			peak_cursor;
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
	bool	GetSignal();
	int		GetLevel();
	
	void	LoadInput(Dqn::MatType& input, int pos);
	void	LoadOutput(double output[dqn_output_size], int pos);
};

class Automation {
	
	
protected:
	friend class AutomationCtrl;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	
	enum {GROUP_SOURCE, GROUP_BITS, GROUP_EVOLVE, GROUP_COUNT};
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int jobgroup_count = GROUP_COUNT;
	
	SlowAutomation	slow[sym_count];
	JobGroup	jobgroups[jobgroup_count];
	double		output_fmlevel;
	int			worker_cursor = 0;
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
	
	bool	IsRunning() const {return running;}
	double	GetFreeMarginLevel() {return output_fmlevel;}
	int		GetFreeMarginScale() {return sym_count;}
	int		GetSymGroupJobId(int symbol) const;
	
	
};


inline Automation& GetAutomation() {return Single<Automation>();}


}

#endif
