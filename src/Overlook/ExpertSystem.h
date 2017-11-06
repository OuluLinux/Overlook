#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

#include "FixedSimBroker.h"
#include "RandomForest.h"


namespace Overlook {

struct ConfProcessor {
	Array<BufferRandomForest> sector;
	BufferRandomForest succ, mult;
	Vector<Vector<byte> > rf_prob;
	double sector_weight = 0.0, succ_weight = 0.0, mult_weight = 0.0;
	
	VectorBool real_mask, real_mult, real_succ;
	Mutex lock;
	
	void SetOptions(Option& options) {
		for(int i = 0; i < sector.GetCount(); i++)
			sector[i].options = options;
		succ.options = options;
		mult.options = options;
	}
	void Serialize(Stream& s) {
		s % sector % succ % mult % rf_prob % sector_weight % succ_weight % mult_weight;
	}
};

class LocalProbLogic {
	
public:
	
	// Persistent
	double sector_limit = -1.0;
	int max_depth[2] = {0,0};
	int trend_period = 0;
	int average_period = 0;
	int result_max = 1;
	int cursor = 0;
	int trend_multiplier = 3;
	bool single_source = false;
	bool active_label = false;
	
	
	// Temp
	double src_buf[2][LOCALPROB_DEPTH];
	double src_weight[2][LOCALPROB_DEPTH];
	double src_avbuf[LOCALPROB_BUFSIZE];
	const int max_trend_period = LOCALPROB_BUFSIZE;
	const int max_average_period = LOCALPROB_BUFSIZE;
	bool round_checked = false;
	bool empty_buf = true;
	
public:
	LocalProbLogic();
	
	int GetResult();
	void SetSource(bool pole, int buf, double prob);
	
	void Reset() {
		cursor = 0;
		empty_buf = true;
		for(int i = 0; i < LOCALPROB_DEPTH; i++) {
			src_buf[0][i] = 0;
			src_buf[1][i] = 0;
			src_weight[0][i] = 1.0;
			src_weight[1][i] = 1.0;
			src_avbuf[i] = 0;
		}
	}
	
	void BasicConfiguration(bool active_label) {
		Reset();
		sector_limit = 0.50;
		max_depth[ active_label] = 1;
		max_depth[!active_label] = 0;
		trend_period = 0;
		average_period = 5;
		result_max = 1;
		cursor = 0;
		trend_multiplier = 3;
		single_source = true;
		active_label = active_label;
	}
	
	void Serialize(Stream& s) {
		s % sector_limit % max_depth[0] % max_depth[1]
		  % trend_period % average_period % result_max
		  % cursor % trend_multiplier % single_source % active_label;
	}
	
};

class RandomForestCache {
	Array<ConfProcessor> rflist;
	SpinLock lock;
	
public:
	typedef RandomForest CLASSNAME;
	
	
	ConfProcessor& GetRandomForest() {
		lock.Enter();
		for(int i = 0; i < rflist.GetCount(); i++) {
			ConfProcessor& rf = rflist[i];
			if (rf.lock.TryEnter()) {
				lock.Leave();
				return rf;
			}
		}
		ConfProcessor& rf = rflist.Add();
		rf.lock.Enter();
		lock.Leave();
		return rf;
	}
	
};

inline RandomForestCache& GetRandomForestCache() {return Single<RandomForestCache>();}

struct AccuracyConf : Moveable<AccuracyConf> {
	
	// Distinctive attributes
	int id = 0;
	int symbol = 0;
	int label_id = 0;
	int period = 0;
	int ext = 0;
	int label = 0;
	int fastinput = 0;
	int labelpattern = 0;
	bool ext_dir = false;
	
	// Training stats
	RandomForestStat sector_accuracy[TF_COUNT];
	RandomForestStat label_stat, mult_stat;
	/*VectorBool real_mask, pred_mask;
	VectorBool real_succ, pred_succ;
	VectorBool real_mult, pred_mult;*/
	//Vector<byte> mult_prob;
	double test_valuefactor = 0.0, test_valuehourfactor = 0.0, test_hourtotal = 0.0;
	double av_hour_change = 0.0;
	double largemult_count = 0.0, largemult_frac = 0.0;
	bool is_processed = false;
	//VectorMap<int, int> real_active_lengths, real_hole_lengths;
	//VectorMap<int, int> pred_active_lengths, pred_hole_lengths;
	
	
	
	
	AccuracyConf() {}
	AccuracyConf(const AccuracyConf& conf) {*this = conf;}
	
	int GetBaseTf(int i) const {
		if (!ext_dir)
			return period + i;
		else
			return period + ext-1 - i;
	}
	
	void Print(ArrayCtrlPrinter& printer) const {
		for(int i = 0; i < ext && i < TF_COUNT; i++) {
			printer.Title("Sector " + IntStr(i) + " stats");
			sector_accuracy[i].Print(printer);
		}
		
		printer.Title("Label");
		label_stat.Print(printer);
		
		printer.Title("Multiplier");
		printer.Add("av_hour_change", av_hour_change);
		printer.Add("largemult_count", largemult_count);
		printer.Add("largemult_frac", largemult_frac);
		mult_stat.Print(printer);
		
		/*printer.Title("real_active_lengths");
		for(int i = 0; i < real_active_lengths.GetCount(); i++)
			printer.Add(real_active_lengths.GetKey(i), real_active_lengths[i]);
		
		printer.Title("real_hole_lengths");
		for(int i = 0; i < real_hole_lengths.GetCount(); i++)
			printer.Add(real_hole_lengths.GetKey(i), real_hole_lengths[i]);
		
		printer.Title("pred_active_lengths");
		for(int i = 0; i < pred_active_lengths.GetCount(); i++)
			printer.Add(pred_active_lengths.GetKey(i), pred_active_lengths[i]);
		
		printer.Title("pred_hole_lengths");
		for(int i = 0; i < pred_hole_lengths.GetCount(); i++)
			printer.Add(pred_hole_lengths.GetKey(i), pred_hole_lengths[i]);*/
	}
	
	bool operator==(const AccuracyConf& src) const {
		return	id				== src.id &&
				symbol			== src.symbol &&
				label_id		== src.label_id &&
				period			== src.period &&
				ext				== src.ext &&
				label			== src.label &&
				fastinput		== src.fastinput &&
				labelpattern	== src.labelpattern &&
				ext_dir			== src.ext_dir;
	}
	
	void operator=(const AccuracyConf& src) {
		id				= src.id;
		symbol			= src.symbol;
		label_id		= src.label_id;
		period			= src.period;
		ext				= src.ext;
		label			= src.label;
		fastinput		= src.fastinput;
		labelpattern	= src.labelpattern;
		ext_dir			= src.ext_dir;
		
		for(int i = 0; i < TF_COUNT; i++)
			sector_accuracy[i]		= src.sector_accuracy[i];
		label_stat					= src.label_stat;
		mult_stat					= src.mult_stat;
		test_valuefactor			= src.test_valuefactor;
		test_valuehourfactor		= src.test_valuehourfactor;
		test_hourtotal				= src.test_hourtotal;
		av_hour_change				= src.av_hour_change;
		largemult_count				= src.largemult_count;
		largemult_frac				= src.largemult_frac;
		is_processed				= src.is_processed;
	}
	void Serialize(Stream& s) {
		s % id
		  % symbol
		  % label_id
		  % period
		  % ext
		  % label
		  % fastinput
		  % labelpattern
		  % ext_dir;
		
		for(int i = 0; i < TF_COUNT; i++)
			s % sector_accuracy[i];
		s % label_stat
		  % mult_stat
		  % test_valuefactor
		  % test_valuehourfactor
		  % test_hourtotal
		  % av_hour_change
		  % largemult_count
		  % largemult_frac
		  % is_processed;
	}
};

inline bool operator < (const AccuracyConf& a, const AccuracyConf& b) {
	return a.test_valuehourfactor < b.test_valuehourfactor;
}


class SimCore {
	
public:
	
	
	struct PoleData {
		Array<ConfProcessor>	used_proc;
		Array<AccuracyConf>		used_conf;
		void Serialize(Stream& s) {s % used_conf % used_conf;}
	};
	
	
	// Persistent
	PoleData						data[2][SYM_COUNT];
	LocalProbLogic					sector_logic[SYM_COUNT];
	LocalProbLogic					succ_logic[SYM_COUNT];
	LocalProbLogic					mult_logic[SYM_COUNT];
	double hourtotal = 0.0, valuefactor = 0.0, valuehourfactor = 0.0;
	int limit_begin = INT_MIN, limit_end = INT_MAX;
	int active_count = 0;
	int single_source = -1;
	bool active_label = false;
	
	// Temp
	Array<ConstBufferSource>		bufs[2][SYM_COUNT];
	Array<ConstBufferSourceIter>	iters[2][SYM_COUNT];
	Vector<double> last_test_equity, last_test_spreadcost;
	FixedSimBroker					test_broker;
	int cursor = 0;
	bool lightweight = false;
	
	
public:
	typedef SimCore CLASSNAME;
	SimCore();
	
	void Reset();
	void Process();
	void Serialize(Stream& s) {
		for(int i = 0; i < SYM_COUNT; i++) {
			s %  data[0][i] %  data[1][i]
			  % sector_logic[i] % succ_logic[i] % mult_logic[i];
		}
		s % hourtotal % valuefactor % valuehourfactor
		  % limit_begin % limit_end % active_count % single_source % active_label;
	}
	
};


int GetConfCount();
void GetConf(int i, AccuracyConf& conf);

class ExpertSystem {
	
public:
	enum {PHASE_TRAINING, PHASE_OPTIMIZING, PHASE_REAL};
	
	// Persistent
	Vector<AccuracyConf> acc_list;
	GeneticOptimizer optimizer;
	SimCore simcore;
	Time created, last_update;
	int phase = 0;
	
	
	// Temp
	TimeStop last_store, last_datagather;
	int realtime_count = 0;
	int processed_used_conf = 0;
	bool running = false, stopped = true;
	bool forced_update = false;
	RWMutex sys_lock;
	RWMutex rt_lock;
	System* sys = NULL;
	Time prev_update;
	
	
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
	void MainOptimizing();
	void MainReal();
	void UpdateRealTimeConfs();
	void Data();
	void DumpUsefulness();
	void Serialize(Stream& s) {
		s % acc_list % optimizer % simcore % created % last_update % phase;
	}
	void ProcessAccuracyConf(AccuracyConf& conf);
	void FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs, int rel_tf);
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
};

}

#endif
