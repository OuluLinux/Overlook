#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

#include "FixedSimBroker.h"
#include "RandomForest.h"


namespace Overlook {

struct ConfTestResults {
	double accuracy = 0.0;
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
};

bool operator == (const SectorConf& a, const SectorConf& b);
bool operator < (const SectorConf& a, const SectorConf& b);
inline bool operator != (const SectorConf& a, const SectorConf& b) {return !(a == b);}

class ExpertSectors : public Pte<ExpertSectors> {
	
public:
	BufferRandomForest sectors[SECTOR_2EXP];
	VectorBool sector_predicted[SECTOR_COUNT];
	SectorConf conf;
	
public:
	typedef ExpertSectors CLASSNAME;
	ExpertSectors();
	
	void Refresh();
	
};

struct AdvisorConf : public ConfTestResults, Moveable<AdvisorConf> {
	SectorConf sec;
	uint8 sector = 0;
	uint8 amp = 0;
	uint8 minimum_prediction_len = 0;
	DecisionClassifierConf label, signal;
	
	
	uint32 GetHashValue() const;
	void Randomize();
};

bool operator == (const AdvisorConf& a, const AdvisorConf& b);
bool operator < (const AdvisorConf& a, const AdvisorConf& b);
inline bool operator != (const AdvisorConf& a, const AdvisorConf& b) {return !(a == b);}

class ExpertAdvisor : public Pte<ExpertAdvisor> {
	
public:
	BufferRandomForest label, signal[2];
	AdvisorConf conf;
	VectorBool predicted[2], signal_mask[2], signal_label[2]; // zigzag label
	Ptr<ExpertSectors> sector;
	
public:
	typedef ExpertAdvisor CLASSNAME;
	ExpertAdvisor();
	
	void Refresh();
	void AnalyzeSectorPoles(int begin, int end);
	
};

struct FusionConf : public ConfTestResults, Moveable<FusionConf> {
	Vector<AdvisorConf> advs;
	DecisionClassifierConf dec;
	uint8 period = 0;
	uint8 sector_period = 0;
	
	FusionConf() {}
	FusionConf(const FusionConf& move) {*this = move;}
	void operator=(const FusionConf& src) {advs <<= src.advs; dec = src.dec; period = src.period; sector_period = src.sector_period;}
	uint32 GetHashValue() const;
	void Randomize();
};

bool operator == (const FusionConf& a, const FusionConf& b);
bool operator < (const FusionConf& a, const FusionConf& b);
inline bool operator != (const FusionConf& a, const FusionConf& b) {return !(a == b);}

class ExpertFusion : public Pte<ExpertFusion> {
	
public:
	Vector<Ptr<ExpertAdvisor> >		advisors;
	BufferRandomForest				dec[FUSE_DEC_COUNT];
	VectorBool						mask[FUSE_DEC_COUNT];
	VectorBool						label[FUSE_DEC_COUNT];
	DerivZeroTrigger				triggers[FUSE_DEC_COUNT];
	FixedSimBroker					broker;
	FusionConf						conf;
	
public:
	typedef ExpertFusion CLASSNAME;
	ExpertFusion();
	
	void Refresh();
	
};

class ExpertCache {
	ArrayMap<SectorConf,	ExpertSectors>	sectors;
	ArrayMap<AdvisorConf,	ExpertAdvisor>	experts;
	ArrayMap<FusionConf,	ExpertFusion>	fusions;
	Mutex lock;
	
public:
	typedef ExpertCache CLASSNAME;
	ExpertCache();
	
	
	ExpertSectors& GetExpertSectors(const SectorConf& conf);
	ExpertAdvisor& GetExpertAdvisor(const AdvisorConf& conf);
	ExpertFusion& GetExpertFusion(const FusionConf& conf);
	
};

inline ExpertCache& GetExpertCache() {return Single<ExpertCache>();}

class ExpertOptimizer {
	
public:
	// Persistent
	Vector<SectorConf>		sec_confs;
	Vector<AdvisorConf>		adv_confs;
	Vector<FusionConf>		fus_confs;
	Vector<double>			sector_results;
	
	// Temporary
	const int pop_size = 100;
	SpinLock lock;
	
public:
	typedef ExpertOptimizer CLASSNAME;
	ExpertOptimizer();
	
	void EvolveSector(SectorConf& conf);
	void EvolveAdvisor(AdvisorConf& conf);
	void EvolveFusion(FusionConf& conf);
	void AddTestResult(const SectorConf& conf);
	void AddTestResult(const AdvisorConf& conf);
	void AddTestResult(const FusionConf& conf);
	void SortSectors()	{::Upp::Sort(sec_confs);}
	void SortAdvisors()	{::Upp::Sort(adv_confs);}
	void SortFusions()	{::Upp::Sort(fus_confs);}
	void ReduceMemory();
	
	bool IsSectorOptimizing() const;
	bool IsAdvisorOptimizing() const;
	bool IsFusionOptimizing() const;
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
