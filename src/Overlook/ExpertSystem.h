#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

#include "FixedSimBroker.h"
#include "RandomForest.h"


namespace Overlook {

enum {REAL, ENUM};

struct ConfEvolver {
	void* osrc = NULL;
	void* best = NULL;
	void* src1 = NULL;
	void* src2 = NULL;
	double scale = 0.7;
	double crossover_prob = 0.9;
	double rand_value_prob = 0.02;
	
	bool loop_type = false, loop_finished = false;
	int total = 0;
	
	int counter = 0;
	int begin = 0;
	
	template <class T>
	inline ConfEvolver& operator () (T& o, int type, int min, int max) {
		if (!loop_type) {total++; return *this;}
		if (loop_finished) return *this;
		
		if (counter >= begin) {
			if (Randomf() >= crossover_prob) {loop_finished = true; return *this;}
			
			if (Randomf() <= rand_value_prob) {
				o = Random(max - min) + min;
			} else {
				uint64 diff = ((uint64)&o) - ((uint64)osrc);
				const T& obest = *(T*)((uint64)best + diff);
				const T& osrc1 = *(T*)((uint64)src1 + diff);
				const T& osrc2 = *(T*)((uint64)src2 + diff);
				o = obest + scale * (osrc1 - osrc2);
				if (type == ENUM) {
					// pick which is closest
					int best_diff = abs(obest - o);
					int src1_diff = abs(osrc1 - o);
					int src2_diff = abs(osrc2 - o);
					if (best_diff <= src1_diff) {
						if (best_diff <= src2_diff)
							o = obest;
						else
							o = osrc2;
					} else {
						if (src1_diff <= src2_diff)
							o = osrc1;
						else
							o = osrc2;
					}
				}
				if (o < min) o = min;
				else if (o > (max-1)) o = max-1;
			}
			
			total--;
			if (total <= 0) {loop_finished = true;}
		}
		
		counter++;
		return *this;
	}
};

struct ArrayCtrlPrinter {
	ArrayCtrl* list = NULL;
	int i = 0;
	
	ArrayCtrlPrinter(ArrayCtrl& list) : list(&list) {}
	
	template <class T>
	void Add(String key, T value) {
		list->Set(i, 0, key);
		list->Set(i, 1, AsString(value));
		i++;
	}
	
	void Title(String key) {
		list->Set(i, 0, "");  list->Set(i, 1, ""); i++;
		list->Set(i, 0, key); list->Set(i, 1, ""); i++;
	}
};

struct ConfTestResults {
	double accuracy = 0.0;
	
	void operator=(const ConfTestResults& src) {
		accuracy = src.accuracy;
	}
	void Serialize(Stream& s) {
		s % accuracy;
	}
};

struct DecisionClassifierConf : Moveable<DecisionClassifierConf> {
	struct DecisionInput {
		int indi = 0;
		int symbol = 0;
	};
	
	DecisionInput input[DECISION_INPUTS];
	int label_id = 0;
	
	uint32 GetHashValue() const;
	void Randomize();
	String ToString() const;
	void Evolve(ConfEvolver& evo) {
		for(int i = 0; i < DECISION_INPUTS; i++)
			evo (input[i].indi, ENUM, 0, TRUEINDI_COUNT)
				(input[i].symbol, ENUM, 0, SYM_COUNT);
		evo (label_id, ENUM, 0, LABELINDI_COUNT);
	}
	void Serialize(Stream& s) {
		for(int i = 0; i < DECISION_INPUTS; i++)
			s % input[i].indi % input[i].symbol;
		s % label_id;
	}
	void Print(ArrayCtrlPrinter& printer) const {
		printer.Add("label_id", label_id);
		for(int i = 0; i < DECISION_INPUTS; i++) {
			String is = IntStr(i);
			printer.Add(is + " indi", input[i].indi);
			printer.Add(is + " symbol", input[i].symbol);
		}
	}
};

bool operator == (const DecisionClassifierConf& a, const DecisionClassifierConf& b);
inline bool operator != (const DecisionClassifierConf& a, const DecisionClassifierConf& b) {return !(a == b);}

struct SectorConf : public ConfTestResults, Moveable<SectorConf> {
	DecisionClassifierConf dec[SECTOR_2EXP];
	int symbol = 0; // symbol which is used for classes
	int period = 0; // base period for +/- change
	int minimum_prediction_len = 0;
	
	uint32 GetHashValue() const;
	void Randomize();
	void Evolve(ConfEvolver& evo) {
		for(int i = 0; i < SECTOR_2EXP; i++)
			dec[i].Evolve(evo);
		evo (symbol, ENUM, 0, SYM_COUNT)
			(period, REAL, 1, TF_COUNT - SECTOR_2EXP)
			(minimum_prediction_len, REAL, 0, MINPRED_LEN);
	}
	void Serialize(Stream& s) {
		ConfTestResults::Serialize(s);
		for(int i = 0; i < SECTOR_2EXP; i++)
			s % dec[i];
		s % symbol % period % minimum_prediction_len;
	}
	void Print(ArrayCtrlPrinter& printer) const {
		for(int i = 0; i < SECTOR_2EXP; i++)
			dec[i].Print(printer);
		printer.Add("symbol", symbol);
		printer.Add("period", period);
		printer.Add("minimum_prediction_len", minimum_prediction_len);
		printer.Add("accuracy", accuracy);
	}
};

bool operator == (const SectorConf& a, const SectorConf& b);
bool operator < (const SectorConf& a, const SectorConf& b);
inline bool operator != (const SectorConf& a, const SectorConf& b) {return !(a == b);}

class ExpertSectors {
	
public:
	BufferRandomForest sectors[SECTOR_2EXP];
	VectorBool sector_predicted[SECTOR_COUNT];
	SectorConf conf;
	ConstBufferSource bufs;
	
public:
	typedef ExpertSectors CLASSNAME;
	ExpertSectors();
	
	void Refresh(const ForestArea& fa);
	void Configure();
	
};

struct AdvisorConf : public ConfTestResults, Moveable<AdvisorConf> {
	SectorConf sectors;
	int sector = 0;
	int amp = 0;
	int minimum_prediction_len = 0;
	DecisionClassifierConf label, signal;
	
	
	uint32 GetHashValue() const;
	void Randomize();
	void Evolve(ConfEvolver& evo) {
		sectors.Evolve(evo);
		evo (sector, ENUM, 0, SECTOR_COUNT)
			(amp, REAL, 0, AMP_MAXSCALES)
			(minimum_prediction_len, REAL, 0, MINPRED_LEN);
		label.Evolve(evo);
		signal.Evolve(evo);
	}
	void Serialize(Stream& s) {
		ConfTestResults::Serialize(s);
		s % sectors % sector % amp % minimum_prediction_len % label % signal;
	}
	void Print(ArrayCtrlPrinter& printer) const {
		printer.Title("Sectors");
		sectors.Print(printer);
		printer.Title("Label");
		label.Print(printer);
		printer.Title("Signal");
		signal.Print(printer);
		printer.Add("sector", sector);
		printer.Add("amp", amp);
		printer.Add("minimum_prediction_len", minimum_prediction_len);
		printer.Add("accuracy", accuracy);
	}
};

bool operator == (const AdvisorConf& a, const AdvisorConf& b);
bool operator < (const AdvisorConf& a, const AdvisorConf& b);
inline bool operator != (const AdvisorConf& a, const AdvisorConf& b) {return !(a == b);}

class ExpertAdvisor {
	
public:
	BufferRandomForest label, signal[2];
	AdvisorConf conf;
	VectorBool predicted[2], signal_mask[2], signal_label[2]; // zigzag label
	ExpertSectors sectors;
	ConstBufferSource bufs;
	
public:
	typedef ExpertAdvisor CLASSNAME;
	ExpertAdvisor();
	
	void Refresh(const ForestArea& fa);
	void Configure();
	void AnalyzeSectorPoles(int begin, int end);
	
};

struct FusionConf : public ConfTestResults, Moveable<FusionConf> {
	AdvisorConf advisors[ADVISOR_COUNT];
	DecisionClassifierConf dec;
	int period = 0;
	int sector_period = 0;
	
	double test0_equity = 0.0, test0_dd = 1.0;
	double test1_equity = 0.0, test1_dd = 1.0;
	
	FusionConf() {}
	FusionConf(const FusionConf& move) {*this = move;}
	void operator=(const FusionConf& src);
	uint32 GetHashValue() const;
	void Randomize();
	void Evolve(ConfEvolver& evo) {
		for(int i = 0; i < ADVISOR_COUNT; i++)
			advisors[i].Evolve(evo);
		dec.Evolve(evo);
		evo (period, REAL, 0, TF_COUNT)
			(sector_period, REAL, 0, TF_COUNT);
	}
	void Serialize(Stream& s) {
		ConfTestResults::Serialize(s);
		for(int i = 0; i < ADVISOR_COUNT; i++)
			s % advisors[i];
		s % dec % period % sector_period;
		s % test0_equity % test0_dd % test1_equity % test1_dd;
	}
	void Print(ArrayCtrlPrinter& printer) const {
		printer.Add("test0_equity", test0_equity);
		printer.Add("test0_dd", test0_dd);
		printer.Add("test1_equity", test1_equity);
		printer.Add("test1_dd", test1_dd);
		printer.Add("accuracy", accuracy);
		printer.Title("Fusion trigger input");
		dec.Print(printer);
		for(int i = 0; i < ADVISOR_COUNT; i++) {
			printer.Title("Advisor " + IntStr(i));
			advisors[i].Print(printer);
		}
	}
};

bool operator == (const FusionConf& a, const FusionConf& b);
bool operator < (const FusionConf& a, const FusionConf& b);
inline bool operator != (const FusionConf& a, const FusionConf& b) {return !(a == b);}

class ExpertFusion {
	
public:
	ExpertAdvisor					advisors[ADVISOR_COUNT];
	BufferRandomForest				dec[FUSE_DEC_COUNT];
	VectorBool						mask[FUSE_DEC_COUNT];
	VectorBool						label[FUSE_DEC_COUNT];
	DerivZeroTrigger				triggers[FUSE_DEC_COUNT];
	FixedSimBroker					broker;
	FusionConf						conf;
	ConstBufferSource				bufs;
	
	// Temp
	Vector<int> trigger_points;
	Vector<double> equities;
	Atomic locked;
	
public:
	typedef ExpertFusion CLASSNAME;
	ExpertFusion();
	
	void Refresh(const ForestArea& fa);
	void Configure();
	void Leave() {locked--;}
	
};

class ExpertCache {
	Array<ExpertFusion> fusions;
	SpinLock lock;
	
public:
	typedef ExpertCache CLASSNAME;
	
	
	ExpertFusion& GetExpertFusion() {
		for(int i = 0; i < fusions.GetCount(); i++) {
			ExpertFusion& ep = fusions[i];
			if (++ep.locked == 1) {
				return ep;
			}
			else --ep.locked;
		}
		lock.Enter();
		ExpertFusion* ep = new ExpertFusion();
		ep->locked = 1;
		fusions.Add(ep);
		lock.Leave();
		return *ep;
	}
	
};

inline ExpertCache& GetExpertCache() {return Single<ExpertCache>();}

class ExpertOptimizer {
	
public:
	// Persistent
	Vector<FusionConf>		fus_confs;
	Vector<double>			fus_results;
	int						level = 0;
	
	// Temporary
	int pop_size = 100;
	int pop_limit = 1000;
	int64 pop_counter = 0;
	double rand_prob = 0.01;
	SpinLock lock;
	
public:
	typedef ExpertOptimizer CLASSNAME;
	ExpertOptimizer();
	
	void EvolveFusion(FusionConf& conf);
	void AddTestResult(const FusionConf& conf);
	void SortFusions()	{::Upp::Sort(fus_confs);}
	void ReduceMemory();
	int GetPopulationSize() const {return pop_size;}
	void Serialize(Stream& s) {s % fus_confs % fus_results % level;}
};

class ExpertSystem {
	
public:
	
	// Persistent
	ExpertOptimizer fusion;
	Vector<FusionConf> rt_confs;
	Time created, last_update;
	int is_training = true;
	
	
	// Temp
	VectorMap<String, int> allowed_symbols;
	TimeStop last_store, last_datagather;
	int realtime_count = 0;
	bool training_running = false, training_stopped = true;
	bool running = false, stopped = true;
	bool forced_update = false;
	RWMutex sys_lock;
	Mutex rt_lock;
	System* sys = NULL;
	Time prev_update;
	
	
public:
	typedef ExpertSystem CLASSNAME;
	ExpertSystem(System* sys);
	~ExpertSystem();
	
	void Init();
	void Start();
	void Stop();
	void StopTraining();
	void StoreThis();
	void LoadThis();
	void Main();
	void MainTraining();
	void MainReal();
	void UpdateRealTimeConfs();
	void Data();
	void Serialize(Stream& s) {
		s % fusion % rt_confs % created % last_update % is_training;
	}
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
};

}

#endif
