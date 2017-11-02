#ifndef _Overlook_ExpertSystem_h_
#define _Overlook_ExpertSystem_h_

#include "FixedSimBroker.h"
#include "RandomForest.h"


namespace Overlook {

/*
	Missing extensions:
	 - in sectors
		- up-down-up label combos
		- different label_id combos
		- additional +1 -1 timeframes as inputs
	 
*/


class RandomForestCache {
	Array<BufferRandomForest> rflist;
	SpinLock lock;
	
public:
	typedef RandomForest CLASSNAME;
	
	
	BufferRandomForest& GetRandomForest() {
		lock.Enter();
		for(int i = 0; i < rflist.GetCount(); i++) {
			BufferRandomForest& rf = rflist[i];
			if (rf.lock.TryEnter()) {
				lock.Leave();
				return rf;
			}
		}
		BufferRandomForest& rf = rflist.Add();
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
	bool label = false;
	bool ext_dir = false;
	
	RandomForestStat sector_accuracy[TF_COUNT];
	RandomForestStat label_stat, mult_stat;
	VectorBool real_mask, pred_mask;
	VectorBool real_succ, pred_succ;
	VectorBool real_mult, pred_mult;
	Vector<byte> mult_prob;
	double test_mask_valuefactor = 0.0, test_mask_valuehourfactor = 0.0, test_mask_hourtotal = 0.0;
	double test_succ_valuefactor = 0.0, test_succ_valuehourfactor = 0.0, test_succ_hourtotal = 0.0;
	double test_mult_valuefactor = 0.0, test_mult_valuehourfactor = 0.0, test_mult_hourtotal = 0.0;
	double av_hour_change = 0.0;
	double largemult_count = 0.0, largemult_frac = 0.0;
	bool is_processed = false;
	
	VectorMap<int, int> real_active_lengths, real_hole_lengths;
	VectorMap<int, int> pred_active_lengths, pred_hole_lengths;
	
	
	
	AccuracyConf() {}
	AccuracyConf(const AccuracyConf& conf) {*this = conf;}
	
	
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
		
		printer.Title("real_active_lengths");
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
			printer.Add(pred_hole_lengths.GetKey(i), pred_hole_lengths[i]);
	}
	
	bool operator==(const AccuracyConf& src) const {
		return	id			== src.id &&
				symbol		== src.symbol &&
				label_id	== src.label_id &&
				period		== src.period &&
				ext			== src.ext &&
				label		== src.label &&
				ext_dir		== src.ext_dir;
	}
	
	void operator=(const AccuracyConf& src) {
		id			= src.id;
		symbol		= src.symbol;
		label_id	= src.label_id;
		period		= src.period;
		ext			= src.ext;
		label		= src.label;
		ext_dir		= src.ext_dir;
		
		for(int i = 0; i < TF_COUNT; i++)
			sector_accuracy[i] = src.sector_accuracy[i];
		label_stat	= src.label_stat;
		mult_stat	= src.mult_stat;
		real_mask	= src.real_mask;
		pred_mask	= src.pred_mask;
		real_succ	= src.real_succ;
		pred_succ	= src.pred_succ;
		real_mult	= src.real_mult;
		pred_mult	= src.pred_mult;
		mult_prob	<<= src.mult_prob;
		test_mask_valuefactor		= src.test_mask_valuefactor;
		test_mask_valuehourfactor	= src.test_mask_valuehourfactor;
		test_mask_hourtotal			= src.test_mask_hourtotal;
		test_succ_valuefactor		= src.test_succ_valuefactor;
		test_succ_valuehourfactor	= src.test_succ_valuehourfactor;
		test_succ_hourtotal			= src.test_succ_hourtotal;
		test_mult_valuefactor		= src.test_mult_valuefactor;
		test_mult_valuehourfactor	= src.test_mult_valuehourfactor;
		test_mult_hourtotal			= src.test_mult_hourtotal;
		real_active_lengths			<<= src.real_active_lengths;
		real_hole_lengths			<<= src.real_hole_lengths;
		pred_active_lengths			<<= src.pred_active_lengths;
		pred_hole_lengths			<<= src.pred_hole_lengths;
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
		  % ext_dir;
		
		for(int i = 0; i < TF_COUNT; i++)
			s % sector_accuracy[i];
		s % label_stat
		  % mult_stat
		  % real_mask
		  % pred_mask
		  % real_succ
		  % pred_succ
		  % real_mult
		  % pred_mult
		  % mult_prob
		  % test_mask_valuefactor
		  % test_mask_valuehourfactor
		  % test_mask_hourtotal
		  % test_succ_valuefactor
		  % test_succ_valuehourfactor
		  % test_succ_hourtotal
		  % test_mult_valuefactor
		  % test_mult_valuehourfactor
		  % test_mult_hourtotal
		  % real_active_lengths
		  % real_hole_lengths
		  % pred_active_lengths
		  % pred_hole_lengths
		  % av_hour_change
		  % largemult_count
		  % largemult_frac
		  % is_processed;
	}
};

inline bool operator < (const AccuracyConf& a, const AccuracyConf& b) {
	return a.test_mult_valuehourfactor < b.test_mult_valuehourfactor;
}

struct SectorCacheObject : Moveable<SectorCacheObject> {
	RandomForestStat stat;
	VectorBool pred_mask;
	Mutex lock;
};

SectorCacheObject& GetSectorCacheObject(int sym, int tf_begin, int tf_end, int label_id);



int GetConfCount();
void GetConf(int i, AccuracyConf& conf);




class ExpertSystem {
	
public:
	
	// Persistent
	Vector<AccuracyConf> acc_list;
	Vector<int> used_conf;
	Time created, last_update;
	int is_training = true;
	
	
	// Temp
	Vector<double> last_test_equity, last_test_spreadcost;
	FixedSimBroker test_broker;
	TimeStop last_store, last_datagather;
	int realtime_count = 0;
	int processed_used_conf = 0;
	bool running = false, stopped = true;
	bool training_running = false, training_stopped = true;
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
	void StopTraining();
	void StoreThis();
	void LoadThis();
	void Main();
	void MainTraining();
	void MainReal();
	void UpdateRealTimeConfs();
	void Data();
	void Serialize(Stream& s) {
		s % acc_list % used_conf % created % last_update % is_training;
	}
	void ProcessAccuracyConf(AccuracyConf& conf, bool realtime);
	
	Callback1<String> WhenInfo;
	Callback1<String> WhenError;
};

}

#endif
