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
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) s.Get(this, sizeof(JobGroup));
		else               s.Put(this, sizeof(JobGroup));
	}
};

enum {
	OUT_EVOLVE_SIG,	OUT_EVOLVE_ENA,
	OUT_COUNT
};

class SlowAutomation {
	
protected:
	friend class AutomationCtrl;
	friend class GameMatchCtrl;
	friend class Automation;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	enum {ACTION_LONG, ACTION_SHORT, ACTION_IDLE};
	
	static const int sym_count = USEDSYMBOL_COUNT;
	
	static const int input_length = 25;
	static const int input_count = input_length * 2;
	static const int output_count = 3;
	
	static const int brain_leftoffset = 10000;
	static const int brain_rightoffset = 12+1;
	#ifdef flagDEBUG
	static const int max_iters = 1100;
	#else
	static const int max_iters = 1000000;
	#endif
	
	
	Vector<double>	open_buf;
	Vector<int>		time_buf;
	ConvNet::Brain	brain;
	double			point = 0;
	double			spread = 0;
	Time			prev_sig_time;
	int				sym = -1, tf = 0, period = 0;
	int				prev_sig = 0;
	int				brain_iters = 0;
	int				loadsource_pos = 0;
	int				loadsource_cursor = 0;
	bool			not_first = 0;
	
	bool*			running = 0;
	
	
public:
	
	void Serialize(Stream& s);
	
	void	Evolve();
	double	TestAction(int pos, int action);
	void    GetOutputValues(bool& signal, int& level);
	
	void	LoadInput(Vector<double>& input, int pos);
};

class Automation {
	
	
protected:
	friend class AutomationCtrl;
	friend class GameMatchCtrl;
	friend class BooleansDraw;
	friend class System;
	friend class Game;
	
	
	enum {GROUP_SOURCE, GROUP_EVOLVE, GROUP_COUNT};
	
	
	static const int sym_count = USEDSYMBOL_COUNT;
	static const int jobgroup_count = GROUP_COUNT;
	
	Array<SlowAutomation>	slow;
	Array<JobGroup>			jobgroups;
	
	
	// Temp
	int						worker_cursor = 0;
	bool					running = false, stopped = true;
	Atomic					not_stopped;
	SpinLock				workitem_lock;
	
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
	int		GetSymGroupJobId(int symbol) const;
	
	
};


inline Automation& GetAutomation() {return Single<Automation>();}


}

#endif
