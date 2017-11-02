#include "Overlook.h"

namespace Overlook {

/*#ifdef flagDEBUG
int cls_tree_count	= 8;
int cls_max_depth	= 4;
int cls_hypothesis	= 4;
#else*/
int cls_tree_count	= 50; // orig 100
int cls_max_depth	= 4;
int cls_hypothesis	= 10;
//#endif











const int label_count = LABELINDI_COUNT;
const int sector_pred_dir_count = 2;
const int sector_label_count = 2;
int sector_type_count;

int GetConfCount() {
	if (!sector_type_count) {
		for(int i = 0; i < TF_COUNT; i++)
			sector_type_count += (TF_COUNT - i);
	}
	return SYM_COUNT * label_count * sector_pred_dir_count * sector_label_count * sector_type_count;
}

void GetConf(int i, AccuracyConf& conf) {
	int count = GetConfCount();
	ASSERT(i >= 0 && i < count);
	
	conf.id = i;
	
	conf.symbol = i % SYM_COUNT;
	ASSERT(conf.symbol >= 0 && conf.symbol < SYM_COUNT);
	i = i / SYM_COUNT;
	
	conf.label_id = i % label_count;
	ASSERT(conf.label_id >= 0 && conf.label_id < label_count);
	i = i / label_count;
	
	int sector_type = i % sector_type_count;
	i = i / sector_type_count;
	int j = 0;
	for(conf.period = 0; conf.period < TF_COUNT; conf.period++)
		for(conf.ext = 1; conf.ext <= (TF_COUNT - conf.period); conf.ext++)
			if (j++ == sector_type)
				goto found;
	found:
	ASSERT(conf.period >= 0 && conf.period < TF_COUNT);
	ASSERT(conf.ext > 0 && conf.ext <= TF_COUNT);
	
	conf.label = i % sector_label_count;
	i = i / sector_label_count;
	
	conf.ext_dir = i % sector_pred_dir_count;
	i = i / sector_pred_dir_count;
}

struct InterestingAccuracyConfSorter {
	bool operator() (const AccuracyConf& a, const AccuracyConf& b) const {
		if (a.symbol < b.symbol) return true;
		if (a.symbol > b.symbol) return false;
		if (abs(a.period - 5) < abs(b.period - 5)) return true;
		if (abs(a.period - 5) > abs(b.period - 5)) return false;
		if (abs(a.ext - 3) < abs(b.ext - 3)) return true;
		if (abs(a.ext - 3) > abs(b.ext - 3)) return false;
		if (a.ext_dir) return true;
		if (!a.ext_dir) return false;
		if (!a.label) return true;
		if (a.label) return false;
		return false;
	}
};

SectorCacheObject& GetSectorCacheObject(int sym, int tf_begin, int tf_end, int label_id) {
	typedef Tuple4<int,int,int,int> Key;
	static Mutex lock;
	static ArrayMap<Key, SectorCacheObject> objs;
	lock.Enter();
	SectorCacheObject& obj = objs.GetAdd(Key(sym, tf_begin, tf_end, label_id));
	lock.Leave();
	return obj;
}
















ExpertSystem::ExpertSystem(System* sys) : sys(sys) {
	running = false;
	stopped = true;
	
	created = GetSysTime();
}

ExpertSystem::~ExpertSystem() {
	Stop();
}

void ExpertSystem::Init() {
	ASSERT(sys);
	
	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);
	
	sys->SetFixedBroker(test_broker);
}

void ExpertSystem::Start() {
	Stop();
	
	forced_update = true;
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Main));
	
	if (is_training) {
		training_stopped = false;
		training_running = true;
		Thread::Start(THISBACK(MainTraining));
	}
}

void ExpertSystem::Stop() {
	running = false;
	training_running = false;
	while (stopped != true || training_stopped != true)
		Sleep(100);
}

void ExpertSystem::StopTraining() {
	training_running = false;
	while (training_stopped != true)
		Sleep(100);
}

void ExpertSystem::Main() {
	
	while (running) {
		MainReal();
		
		Sleep(100);
	}


	stopped = true;
}

String expertsystem_bin() {
	#ifdef flagDEBUG
	return ConfigFile("expertsystem_debug.bin");
	#else
	return ConfigFile("expertsystem.bin");
	#endif
}

void ExpertSystem::StoreThis() {
	rt_lock.EnterWrite();
	
	Time t = GetSysTime();
	String file = expertsystem_bin();
	bool rem_bak = false;

	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}
	
	FileOut fout(file);
	fout % *this;

	if (rem_bak)
		DeleteFile(file + ".bak");

	last_store.Reset();
	
	rt_lock.LeaveWrite();
}

void ExpertSystem::LoadThis() {
	String file = expertsystem_bin();
	if (FileExists(file + ".bak"))
		file = file + ".bak";
	
	FileIn in(file);
	in % *this;
}

void ForestArea::FillArea(int level, int data_count) {
	int len;
	switch (level) {
		case 0: len = 1 * 5 * 24 * 60; break;
		case 1: len = 2 * 5 * 24 * 60; break;
		case 2: len = 4 * 5 * 24 * 60; break;
		default: len = 8 * 5 * 24 * 60;
	}
	train_begin = Random(data_count - len);
	train_end = train_begin + len;
	do {
		test0_begin = Random(data_count - len);
		test0_end = test0_begin + len;
	} while (abs(test0_begin - train_begin) < 2*len);
	do {
		test1_begin = Random(data_count - len);
		test1_end = test1_begin + len;
	}
	while (abs(test0_begin - train_begin) < 2*len ||
		   abs(test1_begin - train_begin) < 2*len);
}

void ForestArea::FillArea(int data_count) {
	train_begin		= 24*60;
	train_end		= data_count * 2/3;
	test0_begin		= train_end;
	test0_end		= data_count - 1*5*24*60;
	test1_begin		= test0_end;
	test1_end		= data_count;
}

void ExpertSystem::MainTraining() {
	
	int acc_count = GetConfCount();
	if (acc_list.GetCount() != acc_count) {
		acc_list.SetCount(acc_count);
		for(int i = 0; i < acc_list.GetCount(); i++)
			GetConf(i, acc_list[i]);
		
		// Test for duplicate settings
		for(int i = 0; i < acc_list.GetCount(); i++) {
			for(int j = i+1; j < acc_list.GetCount(); j++) {
				if (acc_list[i] == acc_list[j])
					Panic("Duplicate AccuracyConf");
			}
		}
	}
	
	// Repair damaged loading
	for(int i = 0; i < acc_list.GetCount(); i++) {
		AccuracyConf& conf = acc_list[i];
		if (conf.id < 0) {
			GetConf(i, conf);
			conf.is_processed = false;
		}
	}
	
	
	// Split processing to chunks
	int cpus = GetUsedCpuCores();
	int block_size = cpus * 4;
	int block_count = acc_list.GetCount() / block_size;
	if (acc_list.GetCount() % block_size > 0) block_count++;
	
	
	// Find the begin position for processing
	int begin_pos = 0;
	for(begin_pos = 0; begin_pos < acc_list.GetCount(); begin_pos++)
		if (!acc_list[begin_pos].is_processed)
			break;
	
	
	CoWork co;
	co.SetPoolSize(cpus);
	for(int block = begin_pos / block_size; block < block_count && training_running; block++) {
		int conf_begin = block * block_size;
		int conf_end = Upp::min(acc_list.GetCount(), (block + 1) * block_size);
		
		rt_lock.EnterRead();
		for(int conf_id = conf_begin; conf_id < conf_end && training_running; conf_id++) {
			if (acc_list[conf_id].is_processed)
				continue;
			co & [=]
			{
				ProcessAccuracyConf(acc_list[conf_id], false);
			};
		}
		co.Finish();
		rt_lock.LeaveRead();
		
		StoreThis();
	}
	
	training_stopped = true;
}

void ExpertSystem::ProcessAccuracyConf(AccuracyConf& conf, bool realtime) {
	if (!realtime && !training_running)
		return;
	
	if (!realtime && conf.is_processed)
		return;
	
	TimeStop proc_ts;
	
	BufferRandomForest& rf = GetRandomForestCache().GetRandomForest();
	rf.SetCacheId(conf.id);
	rf.UseCache(realtime);
	rf.options.tree_count	= cls_tree_count;
	rf.options.max_depth	= cls_max_depth;
	rf.options.tries_count	= cls_hypothesis;
	
	bool active_label = conf.label;
	
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	
	ForestArea area;
	area.FillArea(data_count);
	
	ASSERT(conf.ext > 0);
	ConstBufferSource bufs;
	bufs.SetDepth(TRUEINDI_COUNT);
	VectorBool &real_mask = conf.real_mask, &pred_mask = conf.pred_mask;
	real_mask.SetCount(data_count).One();
	pred_mask.SetCount(data_count).One();
	
	
	// Looping direction affects predicted mask, but not real mask
	int ext_begin = 0, ext_end = conf.ext-1, ext_step = +1;
	if (conf.ext_dir) {
		Swap(ext_begin, ext_end);
		ext_step = -1;
	}
	for(int j = ext_begin, sid = 0; (!conf.ext_dir ? j <= ext_end : j >= ext_end); j += ext_step, sid++) {
		int tf = conf.period + j;
		
		
		for(int i = 0; i < TRUEINDI_COUNT; i++)
			bufs.SetSource(i, sys.GetTrueIndicator(conf.symbol, tf, i));
		
		sys_lock.EnterRead();
		
		ConstVectorBool& real_label = sys.GetLabelIndicator(conf.symbol, tf, conf.label_id) ;
		
		// many has same real mask and settings at this point, so cache it
		// TODO: maybe remove this first cache?
		if (!realtime) {
			SectorCacheObject& sec_cache = GetSectorCacheObject(conf.symbol, conf.period + ext_begin, tf, conf.label_id);
			sec_cache.lock.Enter();
			if (sec_cache.pred_mask.GetCount() == 0) {
				rf.Process(10 + sid, area, bufs, real_label, real_mask);
				sec_cache.pred_mask = rf.predicted_label;
				sec_cache.stat = rf.stat;
			}
			sec_cache.lock.Leave();
			
			if (!active_label) {
				real_mask.InverseAnd(real_label);
				pred_mask.InverseAnd(sec_cache.pred_mask);
			} else {
				real_mask.And(real_label);
				pred_mask.And(sec_cache.pred_mask);
			}
			
			conf.sector_accuracy[sid] = sec_cache.stat;
		} else {
			// This uses cache too, but it's different...
			// The previous could use this, but not at first time
			rf.Process(10 + sid, area, bufs, real_label, real_mask);

			if (!active_label) {
				real_mask.InverseAnd(real_label);
				pred_mask.InverseAnd(rf.predicted_label);
			} else {
				real_mask.And(real_label);
				pred_mask.And(rf.predicted_label);
			}
			
			conf.sector_accuracy[sid] = rf.stat;
		}
		
		sys_lock.LeaveRead();
	}
	
	for(int i = 0; i < TRUEINDI_COUNT; i++)
		bufs.SetSource(i, sys.GetTrueIndicator(conf.symbol, conf.period, i));
	
	
	// Slightly lengthen predicted parts to fill too small holes
	{
		bool prev_active = false;
		int tail_active_count = 0;
		for(int i = 0; i < pred_mask.GetCount(); i++) {
			if (pred_mask.Get(i)) {
				if (!prev_active) {
					prev_active = true;
				}
			} else {
				if (prev_active) {
					pred_mask.Set(i, true);
					tail_active_count = 3;
					prev_active = false;
				}
				else if (tail_active_count > 0) {
					pred_mask.Set(i, true);
					tail_active_count--;
				}
			}
		}
	}
	
	
	// Gather stats about continuity in real and predicted mask
	{
		conf.real_hole_lengths.Clear();
		conf.real_active_lengths.Clear();
		conf.pred_hole_lengths.Clear();
		conf.pred_active_lengths.Clear();
		const int div = (1 << (3 + conf.period));
		bool prev_active = false;
		int prev_pos = 0;
		for(int i = 0; i < real_mask.GetCount(); i++) {
			if (real_mask.Get(i)) {
				if (!prev_active) {
					int len = i - prev_pos;
					int divlen = len - len % div;
					conf.real_hole_lengths.GetAdd(divlen, 0)++;
					prev_active = true;
					prev_pos = i;
				}
			} else {
				if (prev_active) {
					int len = i - prev_pos;
					int divlen = len - len % div;
					conf.real_active_lengths.GetAdd(divlen, 0)++;
					prev_active = false;
					prev_pos = i;
				}
			}
		}
		prev_active = false;
		prev_pos = 0;
		for(int i = 0; i < pred_mask.GetCount(); i++) {
			if (pred_mask.Get(i)) {
				if (!prev_active) {
					int len = i - prev_pos;
					int divlen = len - len % div;
					conf.pred_hole_lengths.GetAdd(divlen, 0)++;
					prev_active = true;
					prev_pos = i;
				}
			} else {
				if (prev_active) {
					int len = i - prev_pos;
					int divlen = len - len % div;
					conf.pred_active_lengths.GetAdd(divlen, 0)++;
					prev_active = false;
					prev_pos = i;
				}
			}
		}
		SortByKey(conf.real_hole_lengths,	StdLess<int>());
		SortByKey(conf.real_active_lengths,	StdLess<int>());
		SortByKey(conf.pred_hole_lengths,	StdLess<int>());
		SortByKey(conf.pred_active_lengths,	StdLess<int>());
	}
	
	
	// Train signal usefulness and multiplier
	{
		ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.symbol);
		double spread_point = sys.GetTradingSymbolSpreadPoint(conf.symbol);
		
		VectorBool &real_succ = conf.real_succ;
		VectorBool &real_mult = conf.real_mult;
		real_succ.SetCount(conf.real_mask.GetCount());
		real_mult.SetCount(conf.real_mask.GetCount());
		int change_count = 0;
		double av_change = 0.0;
		int largemult_count = 0;
		for (int part = 0; part < 2; part++) {
			bool prev_active = false;
			int prev_pos = 0;
			double open = open_buf.GetUnsafe(0);
			for(int i = 0; i < conf.real_mask.GetCount(); i++) {
				if (conf.real_mask.Get(i)) {
					if (!prev_active) {
						open = open_buf.GetUnsafe(i);
						prev_active = true;
						prev_pos = i;
					}
				} else {
					if (prev_active) {
						double cur = open_buf.GetUnsafe(i), change;
						int len = i - prev_pos;
						if (part == 0) {
							if (!active_label)
								change = cur / (open + spread_point) - 1.0;
							else
								change = 1.0 - (cur + spread_point) / open;
							bool is_success = change > 0.0;
							av_change += change;
							change_count += len;
							for(int j = prev_pos; j < i; j++)
								real_succ.Set(j, is_success);
						} else {
							if (!active_label)
								change = cur / (open + spread_point) - 1.0;
							else
								change = 1.0 - (cur + spread_point) / open;
							//bool is_largemult = (change / len) >= (av_change * 1.333);
							bool is_largemult = (change * 60 / len) >= 0.001;
							if (is_largemult)
								largemult_count += len;
							for(int j = prev_pos; j < i; j++)
								real_mult.Set(j, is_largemult);
						}
						prev_active = false;
						prev_pos = i;
					}
				}
			}
			
			if (part == 0) {
				av_change /= change_count;
				conf.av_hour_change = av_change * 60;
			} else {
				conf.largemult_count = largemult_count;
				conf.largemult_frac = (double)largemult_count / change_count;
			}
		}
		
		sys_lock.EnterRead();
		rf.Process(1, area, bufs, real_succ, conf.real_mask);
		sys_lock.LeaveRead();
		conf.pred_succ = rf.predicted_label;
		conf.label_stat = rf.stat;
		
		sys_lock.EnterRead();
		rf.Process(2, area, bufs, real_mult, conf.real_mask);
		conf.pred_mult = rf.predicted_label;
		conf.mult_stat = rf.stat;
		ConstBufferSourceIter iter(bufs);
		conf.mult_prob.SetCount(data_count, 128);
		for(int i = 0; i < data_count; i++, iter++) {
			double prob = rf.forest.PredictOne(iter);
			conf.mult_prob[i] = prob * 255;
		}
		sys_lock.LeaveRead();
	}
	
	
	// Test
	{
		bool mask_prev_active = false;
		bool succ_prev_active = false;
		int mask_prev_pos = 0;
		int succ_prev_pos = 0;
		ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.symbol);
		double spread_point = sys.GetTradingSymbolSpreadPoint(conf.symbol);
		double mask_open = open_buf.GetUnsafe(area.test0_begin);
		double succ_open = mask_open;
		double mask_hour_total = 0.0, mask_change_total = 1.0;
		double succ_hour_total = 0.0, succ_change_total = 1.0;
		double mult_change_total = 1.0;
		double succ_mult_prob = 0.0;
		for(int i = area.test0_begin; i < area.test0_end; i++) {
			bool is_mask = conf.pred_mask.Get(i);
			bool is_succ = conf.pred_succ.Get(i);
			
			if (is_mask) {
				if (!mask_prev_active) {
					mask_open = open_buf.GetUnsafe(i);
					mask_prev_active = true;
					mask_prev_pos = i;
				}
			} else {
				if (mask_prev_active) {
					double change, cur = open_buf.GetUnsafe(i);
					int len = i - mask_prev_pos;
					double hours = (double)len / 60.0;
					mask_hour_total += hours;
					if (!active_label)
						change = cur / (mask_open + spread_point);
					else
						change = mask_open / (cur + spread_point);
					mask_change_total *= change;
					mask_prev_active = false;
					mask_prev_pos = i;
				}
			}
			
			if (is_mask && is_succ) {
				if (!succ_prev_active) {
					succ_open = open_buf.GetUnsafe(i);
					succ_prev_active = true;
					succ_prev_pos = i;
					succ_mult_prob = conf.mult_prob[i] / 255.0;
				}
			} else {
				if (succ_prev_active) {
					double change, cur = open_buf.GetUnsafe(i);
					int len = i - succ_prev_pos;
					double hours = (double)len / 60.0;
					succ_hour_total += hours;
					if (!active_label)
						change = cur / (succ_open + spread_point);
					else
						change = succ_open / (cur + spread_point);
					succ_change_total *= change;
					double mult = succ_mult_prob * MULT_MAX;
					change = (change - 1.0) * mult + 1.0;
					if (change < 0.0) change = 0.0;
					mult_change_total *= change;
					succ_prev_active = false;
					succ_prev_pos = i;
				}
			}
		}
		mask_change_total -= 1.0;
		conf.test_mask_valuefactor = mask_change_total;
		conf.test_mask_valuehourfactor = mask_hour_total > 0.0 ? mask_change_total / mask_hour_total : 0.0;
		conf.test_mask_hourtotal = mask_hour_total;
		
		succ_change_total -= 1.0;
		conf.test_succ_valuefactor = succ_change_total;
		conf.test_succ_valuehourfactor = succ_hour_total > 0.0 ? succ_change_total / succ_hour_total : 0.0;
		conf.test_succ_hourtotal = succ_hour_total;
		
		mult_change_total -= 1.0;
		conf.test_mult_valuefactor = mult_change_total;
		conf.test_mult_valuehourfactor = succ_hour_total > 0.0 ? mult_change_total / succ_hour_total : 0.0;
		conf.test_mult_hourtotal = succ_hour_total;
		
		#if 0
		LOG("Processing took " << proc_ts.ToString());
		DUMP(conf.test_mask_valuefactor);
		DUMP(conf.test_mask_valuehourfactor);
		DUMP(conf.test_mask_hourtotal);
		DUMP(conf.test_succ_valuefactor);
		DUMP(conf.test_succ_valuehourfactor);
		DUMP(conf.test_succ_hourtotal);
		#endif
		
		conf.is_processed = true;
	}
	
	
	rf.lock.Leave();
}

void ExpertSystem::UpdateRealTimeConfs() {
	used_conf.SetCount(0);
	
	// Find useful sources for every symbol
	// Skip source if it overlaps too much with already used sources
	int conf_count = acc_list.GetCount();
	for(int i = 0; i < SYM_COUNT; i++) {
		VectorMap<int, double> useful_list;
		for(int j = 0; j < conf_count; j++) {
			const AccuracyConf& conf = acc_list[j];
			if (conf.is_processed && conf.symbol == i &&
				conf.test_mult_valuehourfactor > 0)
				useful_list.Add(j, conf.test_mult_valuehourfactor);
		}
		
		SortByValue(useful_list, StdGreater<double>());
		
		// Mask based method might give too little active sources
		#if 0
		VectorBool existing_mask;
		for(int j = 0; j < useful_list.GetCount(); j++) {
			int conf_id = useful_list.GetKey(j);
			AccuracyConf& conf = acc_list[conf_id];
			
			VectorBool active_mask;
			active_mask = conf.real_mask;
			active_mask.SetCount(conf.real_succ.GetCount());
			active_mask.And(conf.real_succ);
			
			if (existing_mask.GetCount() == 0) {
				existing_mask = active_mask;
				used_conf.Add(conf_id);
			} else {
				active_mask.SetCount(existing_mask.GetCount());
				double overlap_factor = existing_mask.GetOverlapFactor(active_mask);
				if (overlap_factor < 0.2) {
					existing_mask.Or(active_mask);
					used_conf.Add(conf_id);
				}
			}
		}
		#else
		int max_count = 5;
		if (useful_list.GetCount() > max_count)
			useful_list.Remove(max_count, useful_list.GetCount() - max_count);
		for(int j = 0; j < useful_list.GetCount(); j++)
			used_conf.Add(useful_list.GetKey(j));
		#endif
	}
	
	last_update = GetSysTime();
	
	if (!training_running && training_stopped)
		StoreThis();
}

void ExpertSystem::MainReal() {
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	if (used_conf.IsEmpty() || last_update < (now - 4*60*60) || forced_update) {
		UpdateRealTimeConfs();
		if (used_conf.IsEmpty())
			return;
	}
	
	if (!forced_update) {
		
		if (now == prev_update)
			return;
		
		
		// Skip weekends and first hours of monday
		if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
			return;
		}
		
		
		// Check for market closing (weekend and holidays)
		else if (wday == 5 && wday_after_hour == 6) {
			sys->WhenInfo("Closing all orders before market break");
	
			for (int i = 0; i < mt.GetSymbolCount(); i++) {
				mt.SetSignal(i, 0);
				mt.SetSignalFreeze(i, false);
			}
	
			mt.SignalOrders(true);
			return;
		}
	}
	
	forced_update = false;
	prev_update = now;
	sys->WhenInfo("Shift changed");
	
	if (prev_update.minute % 5 == 0)
		Data();
	
	WhenInfo("Updating system content");
	DataBridgeCommon& common = GetDataBridgeCommon();
	common.DownloadAskBid();
	common.RefreshAskBidData(true);
	sys_lock.EnterWrite();
	System& sys = GetSystem();
	sys.skip_storecache = true;
	sys.SetEnd(mt.GetTime());
	int data_count = sys.GetCountMain();
	sys.ProcessWorkQueue();
	sys.ProcessLabelQueue(); // not needed, but without it things break
	sys.ProcessDataBridgeQueue();
	sys.skip_storecache = false;
	sys_lock.LeaveWrite();
	
	
	WhenInfo("Refreshing used configurations");
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	rt_lock.EnterRead();
	processed_used_conf = 0;
	for(int i = 0; i < used_conf.GetCount(); i++) {
		int conf_id = used_conf[i];
		co & [=] {
			ProcessAccuracyConf(acc_list[conf_id], true);
			processed_used_conf++;
		};
	}
	co.Finish();
	rt_lock.LeaveRead();
	
	
	WhenInfo("Updating test broker");
	last_test_equity.SetCount(data_count, 0);
	last_test_spreadcost.SetCount(data_count, 0);
	test_broker.Reset();
	for(int i = 0; i < data_count; i++) {
		test_broker.RefreshOrders(i);
		
		double equity = test_broker.AccountEquity();
		double spread_cost = test_broker.GetSpreadCost(i);
		last_test_equity[i] = equity;
		last_test_spreadcost[i] = spread_cost;
		
		bool sym_used[SYM_COUNT];
		for(int j = 0; j < SYM_COUNT; j++)
			sym_used[j] = false;
		
		for(int j = 0; j < used_conf.GetCount(); j++) {
			const AccuracyConf& conf = acc_list[used_conf[j]];
			
			// Use best source which is masked for using
			if (sym_used[conf.symbol]) continue;
			if (!conf.pred_mask.Get(i)) continue;
			if (!conf.pred_succ.Get(i)) continue;
			sym_used[conf.symbol] = true;
			
			
			// Get signal
			int signal;
			/*if (!conf.pred_succ.Get(i))
				signal = 0;
			else */{
				double succ_mult_prob = conf.mult_prob[i] / 255.0;
				int mult = succ_mult_prob * MULT_MAX + 0.5;
				signal = (conf.label == false ? +1 : -1) * mult;
			}
			
			
			// Put signal to simbroker
			if (signal == test_broker.GetSignal(conf.symbol) && signal != 0)
				test_broker.SetSignalFreeze(conf.symbol, true);
			else {
				test_broker.SetSignal(conf.symbol, signal);
				test_broker.SetSignalFreeze(conf.symbol, false);
			}
		}
		
		test_broker.Cycle(i);
	}
	test_broker.CloseAll(data_count-1);
	
	
	
	WhenInfo("Updating MetaTrader");
	sys.WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	try {
		mt.Data();
		mt.RefreshLimits();
		for (int i = 0; i < sys.sym_ids.GetCount(); i++) {
			int sym = sys.sym_ids[i];
			int sig = test_broker.GetSignal(i);
	
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale((MULT_MAXSCALES - 1)*MULT_MAXSCALE_MUL * SYM_COUNT);
		mt.SignalOrders(true);
	}
	catch (...) {
		
	}
	
	
	sys.WhenRealtimeUpdate();
	sys.WhenPopTask();
}

void ExpertSystem::Data() {
	MetaTrader& mt = GetMetaTrader();
	Vector<Order> orders;
	Vector<int> signals;

	
	try {
		orders <<= mt.GetOpenOrders();
		signals <<= mt.GetSignals();
	
		mt.Data();
	
		int file_version = 1;
		double balance = mt.AccountBalance();
		double equity = mt.AccountEquity();
		Time time = mt.GetTime();
	
		FileAppend fout(ConfigFile("expertsystem.log"));
		int64 begin_pos = fout.GetSize();
		int size = 0;
		fout.Put(&size, sizeof(int));
		fout % file_version % balance % equity % time % signals % orders;
		int64 end_pos = fout.GetSize();
		size = end_pos - begin_pos - sizeof(int);
		fout.Seek(begin_pos);
		fout.Put(&size, sizeof(int));
	}
	catch (...) {
		
	}
}

}
