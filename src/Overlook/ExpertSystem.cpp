#if 0

#include "Overlook.h"

namespace Overlook {


int cls_tree_count	= 20;
int cls_max_depth	= 4;
int cls_hypothesis	= 10;



































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
	
	
}

void ExpertSystem::Start() {
	Stop();
	
	forced_update = true;
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Main));
}

void ExpertSystem::Stop() {
	running = false;
	while (stopped != true)
		Sleep(100);
}

void ExpertSystem::Main() {
	optimizer.SetCount(0);
	
	while (running) {
		
		if (phase == PHASE_TRAINING)
			MainTraining();
		
		else if (phase == PHASE_OPTIMIZING)
			MainOptimizing();
		
		else if (phase == PHASE_REAL)
			MainReal();
		
		Sleep(100);
	}

	StoreThis();
	
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
		case 0: len = 1 * 5 * 24 * 60 / MAIN_PERIOD_MINUTES; break;
		case 1: len = 2 * 5 * 24 * 60 / MAIN_PERIOD_MINUTES; break;
		case 2: len = 4 * 5 * 24 * 60 / MAIN_PERIOD_MINUTES; break;
		default: len = 8 * 5 * 24 * 60 / MAIN_PERIOD_MINUTES;
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
	train_begin		= 24*60 / MAIN_PERIOD_MINUTES;
	train_end		= data_count * 2/3;
	test0_begin		= train_end;
	test0_end		= data_count - 1*5*24*60 / MAIN_PERIOD_MINUTES;
	test1_begin		= test0_end;
	test1_end		= data_count;
}

void ExpertSystem::ProcessAllConfs() {
	// Split processing to chunks
	int cpus = GetUsedCpuCores();
	int block_size = cpus * 4 * 4;
	int block_count = acc_list.GetCount() / block_size;
	if (acc_list.GetCount() % block_size > 0) block_count++;
	
	
	// Find the begin position for processing
	int begin_pos = 0;
	for(begin_pos = 0; begin_pos < acc_list.GetCount(); begin_pos++)
		if (!acc_list[begin_pos].is_processed)
			break;
	
	CoWork co;
	co.SetPoolSize(cpus);
	TimeStop ts;
	for(int block = begin_pos / block_size; block < block_count && running; block++) {
		int conf_begin = block * block_size;
		int conf_end = Upp::min(acc_list.GetCount(), (block + 1) * block_size);
		
		rt_lock.EnterRead();
		for(int conf_id = conf_begin; conf_id < conf_end && running; conf_id++) {
			if (acc_list[conf_id].is_processed)
				continue;
			co & [=]
			{
				AccuracyConf& conf = acc_list[conf_id];
				ConfProcessor& proc = GetRandomForestCache().GetRandomForest();
				if (running && !conf.is_processed) {
					ProcessAccuracyConf(conf, proc);
					
					#if USE_PROC_CACHE
					// Store best cache
					cache_lock.Enter();
					proc.test_valuefactor = conf.test_valuefactor;
					ArrayMap<uint32, ConfProcessor>& proc_cache = this->proc_cache.GetAdd(conf.symbol);
					if (proc_cache.IsEmpty() || proc.test_valuefactor > proc_cache.Top().test_valuefactor) {
						uint32 conf_hash = conf.GetHashValue();
						GetRandomForestCache().Detach(proc);
						proc_cache.Add(conf_hash, &proc);
						SortByValue(proc_cache);
						int excess = proc_cache.GetCount() - LOCALPROB_DEPTH;
						if (excess > 0)
							proc_cache.Remove(LOCALPROB_DEPTH, excess);
					}
					cache_lock.Leave();
					#endif
				}
			};
		}
		co.Finish();
		rt_lock.LeaveRead();
		
		if (ts.Elapsed() >= 10*60*1000) {
			StoreThis();
			ts.Reset();
		}
	}
	
	StoreThis();
}

void ExpertSystem::ProcessUsedConfs() {
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	opt_actual = 0;
	opt_total = 2 * SYM_COUNT * LOCALPROB_DEPTH;
	for (int p = 0; p < 2; p++) {
		for(int i = 0; i < SYM_COUNT; i++) {
			int count = simcore.data[p][i].used_conf.GetCount();
			simcore.data[p][i].used_proc.SetCount(count);
			for(int j = 0; j < count; j++) {
				co & [=] {
					AccuracyConf& conf = simcore.data[p][i].used_conf[j];
					ConfProcessor& proc = simcore.data[p][i].used_proc[j];
					
					#if USE_PROC_CACHE
					// Load best cache
					cache_lock.Enter();
					uint32 conf_hash = conf.GetHashValue();
					ArrayMap<uint32, ConfProcessor>& proc_cache = this->proc_cache.GetAdd(conf.symbol);
					int k = proc_cache.Find(conf_hash);
					if (k != -1) {
						simcore.data[p][i].used_proc.Remove(j);
						simcore.data[p][i].used_proc.Insert(j, proc_cache.Detach(k));
						conf.is_processed = true;
					}
					cache_lock.Leave();
					#else
					int k = -1;
					#endif
					
					if (k == -1 && running && conf.is_processed == false) {
						ProcessAccuracyConf(conf, proc);
					}
					opt_actual++;
				};
			}
		}
	}
	co.Finish();
}

void ExpertSystem::ResetUsedProcessed() {
	for (int p = 0; p < 2; p++)
		for(int i = 0; i < SYM_COUNT; i++)
			for(int j = 0; j < simcore.data[p][i].used_conf.GetCount(); j++)
				simcore.data[p][i].used_conf[j].is_processed = false;
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
	
	
	ProcessAllConfs();
	
	
	// Dump argument usefulness stats
	DumpUsefulness();
	
	
	// Check for change of phase
	bool any_unprocessed = false;
	for(int i = acc_list.GetCount()-1; i >= 0 && !any_unprocessed; i--)
		if (!acc_list[i].is_processed)
			any_unprocessed = true;
	if (!any_unprocessed)
		phase = PHASE_OPTIMIZING;
}


void ExpertSystem::MainOptimizing() {
	
	
	// Run local optimizer for realtime SimCore
	if (optimizer.GetCount() == 0) {
		
		// Get usable sources
		opt_status = "Getting usable sources";
		int conf_count = acc_list.GetCount();
		
		for (int p = 0; p < 2; p++) {
			
			// Find useful sources for every symbol
			// Skip source if it overlaps too much with already used sources
			for(int i = 0; i < SYM_COUNT; i++) {
				VectorMap<int, double> useful_list;
				
				for(int j = 0; j < conf_count; j++) {
					const AccuracyConf& conf = acc_list[j];
					if (conf.is_processed && conf.symbol == i &&
						conf.test_valuehourfactor > 0)
						useful_list.Add(j, conf.test_valuehourfactor);
				}
				
				SortByValue(useful_list, StdGreater<double>());
				
				int max_count = LOCALPROB_DEPTH;
				if (useful_list.GetCount() > max_count)
					useful_list.Remove(max_count, useful_list.GetCount() - max_count);
				
				simcore.data[p][i].used_conf.SetCount(useful_list.GetCount());
				for(int j = 0; j < useful_list.GetCount(); j++) {
					AccuracyConf& conf = simcore.data[p][i].used_conf[j];
					conf = acc_list[useful_list.GetKey(j)];
					conf.is_processed = false;
				}
			}
		}
		ResetUsedProcessed();
		
		
		opt_status = "Initializing optimizer";
		int cols = SYM_COUNT * 3 * (4 + 2 * LOCALPROB_DEPTH) + 3;
		optimizer.SetArrayCount(1);
		optimizer.SetCount(cols);
		optimizer.SetPopulation(1000);
		optimizer.SetMaxGenerations(100);
		optimizer.UseLimits();
		
		int col = 0;
		for(int i = 0; i < SYM_COUNT; i++) {
			for(int j = 0; j < 3; j++) {
				String s; s << i << ", " << j << ", ";
				optimizer.Set(col++, 0.0, 0.5, 0.01, s + "sector_limit");
				optimizer.Set(col++, 0, LOCALPROB_BUFSIZE-2, 1, s + "trend_period");
				optimizer.Set(col++, 1, LOCALPROB_BUFSIZE-1, 1, s + "average_period");
				optimizer.Set(col++, 1, 100-1, 1, s + "trend_multiplier");
				for (int p = 0; p < 2; p++)
					for(int k = 0; k < LOCALPROB_DEPTH; k++)
						optimizer.Set(col++, 0.1, 2.0, 0.1, s + IntStr(p) + ", " + IntStr(k));
			}
		}
		optimizer.Set(col++, 0, 4*24, 1, "mult_freeze_period");
		optimizer.Set(col++, 0.0, 1.0, 0.01, "fixed_mult_factor");
		optimizer.Set(col++, 1, MULT_MAX, 1, "fixed_mult");
		ASSERT(col == cols);
		
		optimizer.Init(StrategyRandom2Bin);
		
		StoreThis();
	}
	
	
	// Retrain used confs with training data (and reuse random forests from training exists)
	opt_status = "Training for optimization";
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	ProcessUsedConfs();
	proc_cache.Clear();
	ForestArea area;
	area.FillArea(data_count);
	simcore.limit_begin = area.train_begin;
	simcore.limit_end = area.train_end;
	ASSERT(simcore.single_source == -1);
	
	opt_status = "Saving";
	StoreThis();
	
	
	double max_result = -DBL_MAX;
	for(int i = 0; i < opt_results.GetCount(); i++) {
		if (opt_results[i] > max_result)
			max_result = opt_results[i];
	}
	
	opt_status = "Optimizing";
	Vector<double> trial;
	opt_total = optimizer.GetMaxRounds();
	TimeStop ts;
	while (!optimizer.IsEnd() && running) {
		opt_actual = optimizer.GetRound();
		
		optimizer.Start();
		
		
		// Copy trial solution to simcore
		optimizer.GetLimitedTrialSolution(trial);
		simcore.LoadConfiguration(trial);
		
		
		// Completely reprocess simcore
		simcore.Reset();
		simcore.Process();
		
		
		// Return simcore equity to optimizer
		double eq = simcore.test_broker.AccountEquity();
		optimizer.Stop(eq);
		
		opt_results.Add(eq);
		
		if (eq > max_result) {
			max_result = eq;
			best_test_equity <<= simcore.last_test_equity;
		}
		
		if (ts.Elapsed() >= 15*60*1000) {
			opt_status = "Saving";
			StoreThis();
			opt_status = "Optimizing";
			ts.Reset();
		}
	}
	
	
	// Post-optimization
	if (running && optimizer.IsEnd()) {
		
		// Retrain used confs with full past data
		opt_status = "Training for final";
		post_optimization_process = true;
		ResetUsedProcessed();
		ProcessUsedConfs();
		post_optimization_process = false;
		
		
		// Put best solution to simcore
		opt_status = "Processing simcore";
		optimizer.GetLimitedBestSolution(trial);
		if (trial.IsEmpty()) {
			opt_status = "Fail";
			return;
		}
		simcore.LoadConfiguration(trial);
		
		
		// Completely reprocess simcore
		simcore.limit_begin = 1*5*24*60 / MAIN_PERIOD_MINUTES;
		simcore.limit_end = data_count;
		simcore.Reset();
		simcore.Process();
		
		
		// Change of phase
		last_update = GetSysTime();
		opt_status = "Saving...";
		phase = PHASE_REAL;
		StoreThis();
	}
}

void ExpertSystem::DumpUsefulness() {
	FileOut fout(ConfigFile("argstats.txt"));
	VectorMap<int, int> symbol_stat_pos, label_id_stat_pos, period_stat_pos, ext_stat_pos, label_stat_pos, fastinput_stat_pos, labelpattern_stat_pos, ext_dir_stat_pos;
	VectorMap<int, int> symbol_stat_neg, label_id_stat_neg, period_stat_neg, ext_stat_neg, label_stat_neg, fastinput_stat_neg, labelpattern_stat_neg, ext_dir_stat_neg;
	for(int i = 0; i < acc_list.GetCount(); i++) {
		const AccuracyConf& conf = acc_list[i];
		
		if (conf.test_valuehourfactor > 0) {
			symbol_stat_pos			.GetAdd(conf.symbol,		0)++;
			label_id_stat_pos		.GetAdd(conf.label_id,		0)++;
			period_stat_pos			.GetAdd(conf.period,		0)++;
			ext_stat_pos			.GetAdd(conf.ext,			0)++;
			label_stat_pos			.GetAdd(conf.label,			0)++;
			fastinput_stat_pos		.GetAdd(conf.fastinput,		0)++;
			labelpattern_stat_pos	.GetAdd(conf.labelpattern,	0)++;
			ext_dir_stat_pos		.GetAdd(conf.ext_dir,		0)++;
		} else {
			symbol_stat_neg			.GetAdd(conf.symbol,		0)++;
			label_id_stat_neg		.GetAdd(conf.label_id,		0)++;
			period_stat_neg			.GetAdd(conf.period,		0)++;
			ext_stat_neg			.GetAdd(conf.ext,			0)++;
			label_stat_neg			.GetAdd(conf.label,			0)++;
			fastinput_stat_neg		.GetAdd(conf.fastinput,		0)++;
			labelpattern_stat_neg	.GetAdd(conf.labelpattern,	0)++;
			ext_dir_stat_neg		.GetAdd(conf.ext_dir,		0)++;
		}
	}
	
	#define PRINT(x) \
		SortByKey(x##_pos, StdLess<int>()); \
		SortByKey(x##_neg, StdLess<int>()); \
		fout << #x "\n"; \
		int x##count = Upp::max(x##_pos.TopKey(), x##_neg.TopKey()) + 1; \
		for(int i = 0; i < x##count; i++) \
			fout << i << "\t" << x##_pos .GetAdd(i, 0) << "\tvs\t" << x##_neg .GetAdd(i, 0) << "\n"; \
		fout << "\n\n\n";
	
	PRINT(symbol_stat);
	PRINT(label_id_stat);
	PRINT(period_stat);
	PRINT(ext_stat);
	PRINT(label_stat);
	PRINT(fastinput_stat);
	PRINT(labelpattern_stat);
	PRINT(ext_dir_stat);
}

void ExpertSystem::FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs, int rel_tf) {
	System& sys = GetSystem();
	
	int depth = TRUEINDI_COUNT * (conf.fastinput + 1) + 1;
	bufs.SetDepth(depth);
	ASSERT(depth >= 2);
	
	int tf = conf.GetBaseTf(0);
	
	int k = 0;
	for(int i = 0; i < TRUEINDI_COUNT; i++) {
		for(int j = 0; j < conf.fastinput+1; j++) {
			ConstBuffer& buf = sys.GetTrueIndicator(conf.symbol, tf-j, i);
			ASSERT(&buf != NULL);
			bufs.SetSource(k++, buf);
		}
	}
	bufs.SetSource(k++, sys.GetTimeBuffer(TIMEBUF_WEEKTIME));
	
	ASSERT(k == depth);
}

void ExpertSystem::ProcessAccuracyConf(AccuracyConf& conf, ConfProcessor& proc) {
	TimeStop proc_ts;
	
	
	if (proc.sector.GetCount() < conf.ext)
		proc.sector.SetCount(conf.ext);
	
	Option options;
	options.tree_count	= cls_tree_count;
	options.max_depth	= cls_max_depth;
	options.tries_count	= cls_hypothesis;
	proc.SetOptions(options);
	
	uint64 active_label_pattern = 0;
	switch (conf.label) {
		case 0: active_label_pattern = 0; break;
		case 1: active_label_pattern = ~(0ULL); break;
		case 2: for(int i = 1; i < 64; i+=2) active_label_pattern |= 1 << i; break;
		case 3: for(int i = 0; i < 64; i+=2) active_label_pattern |= 1 << i; break;
	}
	bool active_label = active_label_pattern & 1;
	
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	
	ForestArea area;
	if (!post_optimization_process)
		area.FillArea(data_count);
	else {
		area.train_begin	= 1*5*24*60 / MAIN_PERIOD_MINUTES;
		area.train_end		= data_count - 2*5*24*60 / MAIN_PERIOD_MINUTES;
		area.test0_begin	= area.train_end;
		area.test0_end		= data_count;
		area.test1_begin	= data_count - 1*5*24*60 / MAIN_PERIOD_MINUTES;
		area.test1_end		= data_count;
	}
	
	ASSERT(conf.ext > 0);
	One<ConstBufferSource> one_bufs;
	ConstBufferSource& bufs = one_bufs.Create();
	
	VectorBool &real_mask = proc.real_mask;
	real_mask.SetCount(data_count).One();
	
	ConstBuffer& weektime = sys.GetTimeBuffer(TIMEBUF_WEEKTIME);
	ASSERT(weektime.GetCount() == data_count);
	
	
	for(int sid = 0; sid < conf.ext; sid++) {
		int tf = conf.GetBaseTf(sid);
		ASSERT(tf - conf.fastinput >= 0);
		bool sid_active_label = active_label_pattern & (1 << (conf.ext - 1 - sid));
		
		FillBufferSource(conf, bufs, sid);
		
		
		sys_lock.EnterRead();
		
		int label_id = 0;
		switch (conf.labelpattern) {
			case 0: label_id = conf.label_id; break;
			case 1: label_id = conf.label_id - sid; break;
			case 2: label_id = conf.label_id + sid; break;
			case 3: label_id = conf.label_id - sid % 2; break;
			case 4: label_id = conf.label_id + sid % 2; break;
		}
		while (label_id < 0)				label_id += LABELINDI_COUNT;
		while (label_id >= LABELINDI_COUNT)	label_id -= LABELINDI_COUNT;
		
		ConstVectorBool& src_label = sys.GetLabelIndicator(conf.symbol, tf, label_id) ;
		VectorBool train_label;
		ConstVectorBool& real_label = (!sid_active_label ? train_label.SetInverse(src_label) : src_label);
		
		
		proc.sector[sid].Process(area, bufs, real_label, real_mask);
		
		real_mask.And(real_label);
		
		conf.sector_accuracy[sid] = proc.sector[sid].stat;
		
		
		sys_lock.LeaveRead();
	}
	
	
	FillBufferSource(conf, bufs, 0);
	
	
	// Train signal usefulness and multiplier
	{
		ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.symbol);
		double spread_point = sys.GetTradingSymbolSpreadPoint(conf.symbol);
		
		VectorBool &real_succ = proc.real_succ;
		VectorBool &real_mult = proc.real_mult;
		real_succ.SetCount(real_mask.GetCount());
		real_mult.SetCount(real_mask.GetCount());
		int change_count = 0;
		double av_change = 0.0;
		int largemult_count = 0;
		for (int part = 0; part < 2; part++) {
			bool prev_active = false;
			int prev_pos = 0;
			double open = open_buf.GetUnsafe(0);
			for(int i = 0; i < real_mask.GetCount(); i++) {
				if (real_mask.Get(i)) {
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
							bool is_largemult = (change * (60.0 / MAIN_PERIOD_MINUTES) / len) >= 0.001;
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
				conf.av_hour_change = av_change * 60 / MAIN_PERIOD_MINUTES;
			} else {
				conf.largemult_count = largemult_count;
				conf.largemult_frac = (double)largemult_count / change_count;
			}
		}
		
		sys_lock.EnterRead();
		proc.succ.Process(area, bufs, real_succ, real_mask);
		sys_lock.LeaveRead();
		
		conf.label_stat = proc.succ.stat;
		
		
		sys_lock.EnterRead();
		proc.mult.Process(area, bufs, real_mult, real_mask);
		sys_lock.LeaveRead();
		
		conf.mult_stat = proc.mult.stat;
		
	}
	
	
	// Test
	{
		SimCore sc;
		sc.limit_begin		= area.test0_begin;
		sc.limit_end		= area.test1_end;
		sc.lightweight		= true;
		sc.single_source	= conf.symbol;
		sc.active_label		= active_label;
		
		SimCore::PoleData& data = sc.data[active_label][conf.symbol];
		data.used_conf.Clear();
		data.used_conf.Add(&conf);
		data.used_proc.Clear();
		data.used_proc.Add(&proc);
		
		sc.Process();
		
		data.used_conf.Detach(0);
		data.used_proc.Detach(0);
		
		conf.test_valuefactor		= sc.valuefactor;
		conf.test_valuehourfactor	= sc.valuehourfactor;
		conf.test_hourtotal			= sc.hourtotal;
	}
	
	conf.is_processed = true;
	proc.lock.Leave();
}


void ExpertSystem::MainReal() {
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
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
	
	
	WhenInfo("Updating test broker");
	simcore.limit_begin = 1*5*24*60 / MAIN_PERIOD_MINUTES;
	simcore.limit_end = data_count;
	simcore.Process();
	ASSERT(simcore.cursor == data_count);
	
	
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
			int sig = simcore.test_broker.GetSignal(i);
	
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


















SimCore::SimCore() {
	Reset();
}

void SimCore::Reset() {
	cursor = 0;
	test_broker.Reset();
	active_count = 0;
	for(int i = 0; i < SYM_COUNT; i++) {
		sector_logic[i].Reset();
		succ_logic  [i].Reset();
		mult_logic  [i].Reset();
		sector_logic[i].result_max = 1;
		succ_logic  [i].result_max = 1;
		mult_logic  [i].result_max = MULT_MAX;
	}
	/*
	Not resetting:
		- limit_begin, limit_end
		- single_source
	*/
}

void SimCore::LoadConfiguration(const Vector<double>& solution) {
	int col = 0;
	for(int i = 0; i < SYM_COUNT; i++) {
		for(int j = 0; j < 3; j++) {
			LocalProbLogic& log = j == 0 ? sector_logic[i] : (j == 1 ? succ_logic[i] : mult_logic[i]);
			log.sector_limit		= solution[col++];
			log.trend_period		= solution[col++];
			log.average_period		= solution[col++];
			log.trend_multiplier	= solution[col++];
			for(int p = 0; p < 2; p++)
				for(int k = 0; k < LOCALPROB_DEPTH; k++)
					log.src_weight[p][k] = solution[col++];
		}
	}
	mult_freeze_period		= solution[col++];
	fixed_mult_factor		= solution[col++];
	fixed_mult				= solution[col++];
	ASSERT(col == solution.GetCount());
}

void SimCore::Process() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	bool only_single_source = single_source != -1;
	if (only_single_source) {
		ASSERT(single_source >= 0 && single_source < SYM_COUNT);
		sector_logic[single_source].BasicConfiguration(active_label);
		succ_logic  [single_source].BasicConfiguration(active_label);
		mult_logic  [single_source].BasicConfiguration(active_label);
		mult_logic  [single_source].result_max = MULT_MAX;
		mult_logic  [single_source].average_period = 30;
	}
	
	if (!test_broker.init) {
		sys.SetFixedBroker(test_broker, single_source);
		
		for(int i = 0; i < 2; i++) {
			if (only_single_source && i != active_label)
				continue;
			for(int j = 0; j < SYM_COUNT; j++) {
				if (only_single_source && j != single_source)
					continue;
				
				int count = data[i][j].used_proc.GetCount();
				ASSERT(count == data[i][j].used_conf.GetCount());
				ASSERT(count <= LOCALPROB_DEPTH);
				
				sector_logic	[j].max_depth[i] = count;
				succ_logic		[j].max_depth[i] = count;
				mult_logic		[j].max_depth[i] = count;
				
				bufs[i][j].SetCount(count);
				for(int k = 0; k < count; k++) {
					const AccuracyConf& conf = data[i][j].used_conf[k];
					ConstBufferSource& buf = bufs[i][j][k];
					
					esys.FillBufferSource(conf, buf, 0);
					
					ConstBufferSourceIter& iter = iters[i][j].Add(new ConstBufferSourceIter(buf, &cursor));
				}
			}
		}
	}
	
	int data_count = sys.GetCountMain();
	
	if (!lightweight) {
		last_test_equity.SetCount(data_count, 0);
		last_test_spreadcost.SetCount(data_count, 0);
	}
	
	for(int i = 0; i < SYM_COUNT; i++) {
		mult_freeze_remaining[i]	= 0;
		mult_freeze_value[i]		= 0;
	}
	
	int cursor_end = data_count;
	if (cursor < limit_begin)		cursor = limit_begin;
	if (cursor_end > limit_end)		cursor_end = limit_end;
	
	for (int sym = 0; sym < SYM_COUNT; sym++) {
		sector_logic	[sym].cursor_ptr = &cursor;
		succ_logic		[sym].cursor_ptr = &cursor;
		mult_logic		[sym].cursor_ptr = &cursor;
	}
	
	int t0 = 0, t1 = 0, t2 = 0;
	TimeStop ts;
	
	for (; cursor < cursor_end; cursor++) {
		test_broker.RefreshOrders(cursor);
		
		if (!lightweight) {
			double equity = test_broker.AccountEquity();
			double spread_cost = test_broker.GetSpreadCost(cursor);
			last_test_equity[cursor] = equity;
			last_test_spreadcost[cursor] = spread_cost;
		}
		
		
		int active_symbols = 0;
		
		
		for (int sym = 0; sym < SYM_COUNT; sym++) {
			if (only_single_source && single_source != sym)
				continue;
			
			LocalProbLogic& sector_logic	= this->sector_logic[sym];
			LocalProbLogic& succ_logic		= this->succ_logic[sym];
			LocalProbLogic& mult_logic		= this->mult_logic[sym];
			int& mult_remaining				= this->mult_freeze_remaining[sym];
			int& mult_value					= this->mult_freeze_value[sym];
			
			ts.Reset();
			
			for (int p = 0; p < 2; p++) {
				PoleData& data = this->data[p][sym];
				ASSERT(data.used_proc.GetCount() <= LOCALPROB_DEPTH);
				for(int i = 0; i < data.used_proc.GetCount(); i++) {
					ConfProcessor& proc				= data.used_proc[i];
					const AccuracyConf& conf		= data.used_conf[i];
					ConstBufferSourceIter& iter		= iters[p][sym][i];
					
					
					double sector_prob_av = 0.0;
					ASSERT(conf.ext > 0);
					for(int j = 0; j < conf.ext; j++)
						sector_prob_av += Upp::max(0.0, Upp::min(1.0, proc.sector[j].forest.PredictOne(iter)));
					sector_prob_av /= conf.ext;
					sector_logic.SetSource(p, i, sector_prob_av);
					
					
					double succ_prob = Upp::max(0.0, Upp::min(1.0, proc.succ.forest.PredictOne(iter)));
					succ_logic.SetSource(p, i, succ_prob);
					
					
					double mult_prob = Upp::max(0.0, Upp::min(1.0, proc.mult.forest.PredictOne(iter)));
					mult_logic.SetSource(p, i, mult_prob);
				}
			}
			
			t0 += ts.Elapsed();
			ts.Reset();
			
			sector_logic.round_checked			= true;
			succ_logic.round_checked			= true;
			mult_logic.round_checked			= true;
			int active_sector					= sector_logic.GetResult();
			int active_succ						= succ_logic.GetResult();
			int active_mult						= mult_logic.GetResult();
			
			
			int signal = 0;
			if (active_sector && active_sector == active_succ) {
				active_symbols++;
				if (mult_remaining <= 0) {
					if (active_sector > 0)	mult_value = +Upp::max(+1, active_mult);
					else					mult_value = -Upp::min(-1, active_mult);
					mult_value = fixed_mult_factor * fixed_mult + (1.0 - fixed_mult_factor) * mult_value;
					mult_remaining = mult_freeze_period;
				} else {
					mult_remaining--;
				}
				signal = active_sector * mult_value;
			} else {
				mult_remaining = 0;
			}
			
			t1 += ts.Elapsed();
			ts.Reset();
			
			int prev_signal = test_broker.GetSignal(sym);
			if (signal == prev_signal && signal != 0)
				test_broker.SetSignalFreeze(sym, true);
			else {
				test_broker.SetSignal(sym, signal);
				test_broker.SetSignalFreeze(sym, false);
			}
		}
		
		
		if (active_symbols > 0)
			active_count++;
		
		ts.Reset();
		
		test_broker.Cycle(cursor);
		
		
		t2 += ts.Elapsed();
		ts.Reset();
	}
	
	DUMP(t0);
	DUMP(t1);
	DUMP(t2);
	
	hourtotal = active_count * MAIN_PERIOD_MINUTES / 60.0;
	valuefactor = test_broker.AccountEquity() / test_broker.begin_equity;
	valuehourfactor = hourtotal > 0.0 ? valuefactor / hourtotal : 0.0;
}























LocalProbLogic::LocalProbLogic() {
	Reset();
}

void LocalProbLogic::SetSource(bool pole, int buf, double prob) {
	ASSERT(buf >= 0 && buf < LOCALPROB_DEPTH);
	ASSERT(prob >= 0.0 && prob <= 1.0);
	src_buf[pole][buf] = prob;
}

int LocalProbLogic::GetResult() {
	ASSERT(round_checked); // a safety measure
	ASSERT(trend_period >= 0 && trend_period < (max_trend_period-1));
	ASSERT(average_period >= 1 && average_period < max_average_period);
	ASSERT(max_depth[0] > 0 || max_depth[1] > 0);
	
	double single_prob = 0;
	
	for (int p = 0; p < 2; p++) {
		if (single_source && active_label != p) continue;
		
		double weightedprob_sum = 0.0, weight_sum = 0.0;
		for(int i = 0; i < LOCALPROB_DEPTH && i < max_depth[p]; i++) {
			double prob = src_buf[p][i];
			double weight = src_weight[p][i];
			double weightedprob = weight * prob;
			weightedprob_sum += weightedprob;
			weight_sum += weight;
		}
		double prob_av = weightedprob_sum / weight_sum;
		
		double val = p == 0 ? prob_av : 1.0 - prob_av;
		if (!single_source)		single_prob += val * 0.5;
		else					single_prob  = val;
	}
	
	
	// Add single value to the buffer
	int buf_cursor = *cursor_ptr % max_trend_period;
	if (empty_buf) {
		for(int i = 0; i < LOCALPROB_BUFSIZE; i++)
			src_avbuf[i] = single_prob;
		empty_buf = false;
	} else {
		src_avbuf[buf_cursor] = single_prob;
	}
	
	
	// Use average
	double sector_prob;
	if (average_period > 1) {
		double prob_sum = 0;
		for(int i = 0; i < average_period; i++) {
			int buf_pos = buf_cursor - i;
			if (buf_pos < 0) buf_pos += max_trend_period;
			prob_sum += src_avbuf[buf_pos];
		}
		sector_prob = prob_sum / average_period;
	} else {
		sector_prob = single_prob;
	}
	
	
	// Use trend change
	double trend_change_sum = 0;
	if (trend_period) {
		for(int i = 0; i < trend_period; i++) {
			int buf_pos0 = buf_cursor - i;
			if (buf_pos0 < 0) buf_pos0 += max_trend_period;
			int buf_pos1 = buf_pos0 - 1;
			if (buf_pos1 < 0) buf_pos1 += max_trend_period;
			double change = src_avbuf[buf_pos0] - src_avbuf[buf_pos1];
			trend_change_sum += change;
		}
		ASSERT(trend_multiplier > 0 && trend_multiplier < 100);
		trend_change_sum = trend_change_sum / trend_period * trend_multiplier;
		sector_prob += trend_change_sum;
	}
	
	
	// Use limits for activation
	ASSERT(sector_limit >= 0.0 && sector_limit <= 0.50);
	double part;
	int mul;
	if (sector_prob >= 0.50) {
		mul = +1;
		part = 1.0 - sector_prob;
		if (single_source && active_label != false) mul = 0;
	} else {
		mul = -1;
		part = sector_prob;
		if (single_source && active_label != true)  mul = 0;
	}
	bool active_sector = part <= sector_limit;
	
	
	// Result value
	ASSERT(result_max >= 1);
	int result = mul * active_sector * result_max;
	
	round_checked = false;
	return result;
}

}
#endif
