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
			
			total--;
			if (total <= 0) {loop_finished = true;}
		}
		
		counter++;
		return *this;
	}
};

struct ConfTestResults {
	double accuracy = 0.0;
	double test0_equity = 0.0, test0_dd = 1.0;
	double test1_equity = 0.0, test1_dd = 1.0;
	
	void operator=(const ConfTestResults& src) {
		accuracy = src.accuracy;
		test0_equity	= src.test0_equity;
		test0_dd		= src.test0_dd;
		test1_equity	= src.test1_equity;
		test1_dd		= src.test1_dd;
	}
};

struct DecisionClassifierConf : Moveable<DecisionClassifierConf> {
	struct DecisionInput {
		uint8 indi = 0;
		uint8 symbol = 0;
	};
	
	DecisionInput input[DECISION_INPUTS];
	uint8 label_id = 0;
	
	uint32 GetHashValue() const;
	void Randomize();
	String ToString() const;
	void Evolve(ConfEvolver& evo) {
		for(int i = 0; i < DECISION_INPUTS; i++)
			evo (input[i].indi, ENUM, 0, TRUEINDI_COUNT)
				(input[i].symbol, ENUM, 0, SYM_COUNT);
		evo (label_id, ENUM, 0, LABELINDI_COUNT);
	}
};

bool operator == (const DecisionClassifierConf& a, const DecisionClassifierConf& b);
inline bool operator != (const DecisionClassifierConf& a, const DecisionClassifierConf& b) {return !(a == b);}

struct SectorConf : public ConfTestResults, Moveable<SectorConf> {
	DecisionClassifierConf dec[SECTOR_2EXP];
	uint8 symbol = 0; // symbol which is used for classes
	uint8 period = 0; // base period for +/- change
	uint8 minimum_prediction_len = 0;
	
	uint32 GetHashValue() const;
	void Randomize();
	void Evolve(ConfEvolver& evo) {
		for(int i = 0; i < SECTOR_2EXP; i++)
			dec[i].Evolve(evo);
		evo (symbol, ENUM, 0, SYM_COUNT)
			(period, REAL, 1, TF_COUNT - SECTOR_2EXP)
			(minimum_prediction_len, REAL, 0, MINPRED_LEN);
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
	
public:
	typedef ExpertSectors CLASSNAME;
	ExpertSectors();
	
	void Refresh(const ForestArea& fa);
	void Configure();
	
};

struct AdvisorConf : public ConfTestResults, Moveable<AdvisorConf> {
	SectorConf sectors;
	uint8 sector = 0;
	uint8 amp = 0;
	uint8 minimum_prediction_len = 0;
	DecisionClassifierConf label, signal;
	
	
	uint32 GetHashValue() const;
	void Randomize();
	void Evolve(ConfEvolver& evo) {
		sectors.Evolve(evo);
		evo (sector, ENUM, 0, SECTOR_COUNT)
			(amp, REAL, 0, AMP_MAX)
			(minimum_prediction_len, REAL, 0, MINPRED_LEN);
		label.Evolve(evo);
		signal.Evolve(evo);
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
	uint8 period = 0;
	uint8 sector_period = 0;
	
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
	
	// Temp
	Vector<int> trigger_points;
	Vector<double> equities;
	
public:
	typedef ExpertFusion CLASSNAME;
	ExpertFusion();
	
	void Refresh(const ForestArea& fa);
	void Configure();
	
};

class ExpertCache {
	ArrayMap<int, ExpertFusion> fusions;
	SpinLock lock;
	
public:
	typedef ExpertCache CLASSNAME;
	
	
	ExpertFusion& GetExpertFusion(int i) {lock.Enter(); ExpertFusion& ep = fusions.GetAdd(i); lock.Leave(); return ep;}
	
};

inline ExpertCache& GetExpertCache() {return Single<ExpertCache>();}

class ExpertOptimizer {
	
public:
	// Persistent
	Vector<FusionConf>		fus_confs;
	Vector<double>			fus_results;
	
	// Temporary
	int pop_size = 100;
	int pop_limit = 500;
	double accuracy_limit_factor = 0.8;
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
};

class ExpertSystem {
	
public:
	
	// Persistent
	ExpertOptimizer fusion;
	Time created;
	int phase = 0;
	
	
	// Temp
	VectorMap<String, int> allowed_symbols;
	Index<int> sym_ids;
	TimeStop last_store, last_datagather;
	bool running = false, stopped = true;
	System* sys = NULL;
	
public:
	typedef ExpertSystem CLASSNAME;
	ExpertSystem(System* sys);
	~ExpertSystem();
	
	void Init();
	void Start();
	void Stop();
	void StoreThis();
	void LoadThis();
	void Main();
	void MainTraining();
	void MainReal();
	void Serialize(Stream& s);
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
};

}

#endif
