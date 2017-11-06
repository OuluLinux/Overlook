#include "Overlook.h"

namespace Overlook {

#ifdef flagDEBUG
int cls_tree_count	= 8;
int cls_max_depth	= 4;
int cls_hypothesis	= 4;
#else
int cls_tree_count	= 100;
int cls_max_depth	= 4;
int cls_hypothesis	= 10;
#endif











const int label_count = LABELINDI_COUNT;
const int sector_pred_dir_count = 2;
const int sector_label_count = 4;
const int period_begin = 5;
const int fastinput_count = 3;
const int labelpattern_count = 5;
int sector_type_count;

int GetConfCount() {
	if (!sector_type_count) {
		for(int i = period_begin; i < TF_COUNT; i++)
			sector_type_count += (TF_COUNT - i);
	}
	return SYM_COUNT * label_count * sector_pred_dir_count * sector_label_count * fastinput_count * labelpattern_count * sector_type_count;
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
	for(conf.period = period_begin; conf.period < TF_COUNT; conf.period++)
		for(conf.ext = 1; conf.ext <= (TF_COUNT - conf.period); conf.ext++)
			if (j++ == sector_type)
				goto found;
	found:
	ASSERT(conf.period >= period_begin && conf.period < TF_COUNT);
	ASSERT(conf.ext > 0 && conf.ext <= TF_COUNT);
	
	conf.label = i % sector_label_count;
	i = i / sector_label_count;
	
	conf.fastinput = i % fastinput_count;
	i = i / fastinput_count;
	
	conf.labelpattern = i % labelpattern_count;
	i = i / labelpattern_count;
	
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
	
	while (running) {
		
		if (phase == PHASE_TRAINING)
			MainTraining();
		
		else if (phase == PHASE_OPTIMIZING)
			MainOptimizing();
		
		else if (phase == PHASE_REAL)
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
	for(int block = begin_pos / block_size; block < block_count && running; block++) {
		int conf_begin = block * block_size;
		int conf_end = Upp::min(acc_list.GetCount(), (block + 1) * block_size);
		
		rt_lock.EnterRead();
		for(int conf_id = conf_begin; conf_id < conf_end && running; conf_id++) {
			if (acc_list[conf_id].is_processed)
				continue;
			co & [=]
			{
				ProcessAccuracyConf(acc_list[conf_id]);
			};
		}
		co.Finish();
		rt_lock.LeaveRead();
		
		StoreThis();
	}
	
	
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
	
	Panic("TODO");
	
	// Run local optimizer for realtime SimCore
	if (optimizer.GetRound() == 0) {
		
		// Get usable sources
		/*used_conf.SetCount(0);
		
		// Find useful sources for every symbol
		// Skip source if it overlaps too much with already used sources
		int conf_count = acc_list.GetCount();
		for(int i = 0; i < SYM_COUNT; i++) {
			VectorMap<int, double> useful_list;
			for(int j = 0; j < conf_count; j++) {
				const AccuracyConf& conf = acc_list[j];
				if (conf.is_processed && conf.symbol == i &&
					conf.test_valuehourfactor > 0)
					useful_list.Add(j, conf.test_valuehourfactor);
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
		}*/
		
		last_update = GetSysTime();
		
		
		
		/*optimizer.SetArrayCount();
		optimizer.SetCount(4);
		optimizer.SetPopulation(1000);
		optimizer.SetMaxGenerations(100);
		optimizer.UseLimits();
		optimizer.Set(0,	1,		MULT_MAX,	1,			"Amp fixed value");
		optimizer.Set(1,	0.0,	1.0,		0.001,		"Amp fixed mix");
		optimizer.Set(2,	0.0,	1.0,		0.001,		"Trigger prob limit");
		optimizer.Set(3,	1,		64,			1,			"Trigger average period");
		
		optimizer.Init();*/
		
		StoreThis();
	}
	
	
	while (!optimizer.IsEnd() && running) {
		optimizer.Start();
		
		
		// Calculate energy
		double energy = 0;
		//for(int i = 0; i < acc_count; i++) {
		//	AccuracyConf& conf = acc_list[i];
			
			
			/*double factor = optimizer.Get(i, 0);
			double offset = optimizer.Get(i, 1);
			//double factor = optimizer.GetTrialSolution()[i];
			for(int j = 0; j < vector_steps; j++) {
				double p = offset + sin(j+i) * factor;
				double e = p - points[j];
				if (e < 0) e *= -1.0;
				energy += e;
				points.Add(p);
			}*/
		//}
		
		energy = fabs((double)energy)  * -1;
		optimizer.Stop(energy);
	}
	/*String txt;
	txt = "Best solution: ";
	optimizer.Best();
	double error = 0;
	for(int i = 0; i < optimizer.GetArrayCount(); i++) {
		double factor = optimizer.Get(i, 0);
		double offset = optimizer.Get(i, 1);
		txt << int(factor + 0.5) << " ";
		txt << int(offset + 0.5) << " ";
		error += fabs((double) factor - vector_factors[i] );
		error += fabs((double) offset - vector_offsets[i] );
	}
	error /= vector_count * 2;
	LOG(txt);*/
	
	
	// Change of phase
	if (optimizer.IsEnd())
		phase = PHASE_REAL;
}

void ExpertSystem::DumpUsefulness() {
	FileOut fout(ConfigFile("argstats.txt"));
	VectorMap<int, int> symbol_stat_pos, label_id_stat_pos, period_stat_pos, ext_stat_pos, label_stat_pos, ext_dir_stat_pos;
	VectorMap<int, int> symbol_stat_neg, label_id_stat_neg, period_stat_neg, ext_stat_neg, label_stat_neg, ext_dir_stat_neg;
	for(int i = 0; i < acc_list.GetCount(); i++) {
		const AccuracyConf& conf = acc_list[i];
		
		if (conf.test_valuehourfactor > 0) {
			symbol_stat_pos		.GetAdd(conf.symbol,	0)++;
			label_id_stat_pos	.GetAdd(conf.label_id,	0)++;
			period_stat_pos		.GetAdd(conf.period,	0)++;
			ext_stat_pos		.GetAdd(conf.ext,		0)++;
			label_stat_pos		.GetAdd(conf.label,		0)++;
			ext_dir_stat_pos	.GetAdd(conf.ext_dir,	0)++;
		} else {
			symbol_stat_neg		.GetAdd(conf.symbol,	0)++;
			label_id_stat_neg	.GetAdd(conf.label_id,	0)++;
			period_stat_neg		.GetAdd(conf.period,	0)++;
			ext_stat_neg		.GetAdd(conf.ext,		0)++;
			label_stat_neg		.GetAdd(conf.label,		0)++;
			ext_dir_stat_neg	.GetAdd(conf.ext_dir,	0)++;
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
	PRINT(ext_dir_stat);
}

void ExpertSystem::FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs, int rel_tf) {
	System& sys = GetSystem();
	
	bufs.SetDepth(TRUEINDI_COUNT * (conf.fastinput + 1) + 1);
	
	int tf = conf.GetBaseTf(0);
	
	int k = 0;
	for(int i = 0; i < TRUEINDI_COUNT; i++)
		for(int j = 0; j < conf.fastinput+1; j++)
			bufs.SetSource(k++, sys.GetTrueIndicator(conf.symbol, tf-j, i));
		
	bufs.SetSource(k++, sys.GetTimeBuffer(TIMEBUF_WEEKTIME));
}

void ExpertSystem::ProcessAccuracyConf(AccuracyConf& conf) {
	if (!running || conf.is_processed)
		return;
	
	TimeStop proc_ts;
	
	ConfProcessor& rf = GetRandomForestCache().GetRandomForest();
	
	if (rf.sector.GetCount() < conf.ext)
		rf.sector.SetCount(conf.ext);
	
	Option options;
	options.tree_count	= cls_tree_count;
	options.max_depth	= cls_max_depth;
	options.tries_count	= cls_hypothesis;
	rf.SetOptions(options);
	
	uint64 active_label_pattern = 0;
	switch (conf.label) {
		case 0: active_label_pattern = 0; break;
		case 1: active_label_pattern = ~(0ULL); break;
		case 2: for(int i = 0; i < 64; i+=2) active_label_pattern |= 1 << i; break;
		case 3: for(int i = 1; i < 64; i+=2) active_label_pattern |= 1 << i; break;
	}
	bool active_label = active_label_pattern & 1;
	
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	
	ForestArea area;
	area.FillArea(data_count);
	
	ASSERT(conf.ext > 0);
	ConstBufferSource bufs;
	
	VectorBool &real_mask = rf.real_mask;
	real_mask.SetCount(data_count).One();
	
	ConstBuffer& weektime = sys.GetTimeBuffer(TIMEBUF_WEEKTIME);
	ASSERT(weektime.GetCount() == data_count);
	
	
	for(int sid = 0; sid < conf.ext; sid++) {
		int tf = conf.GetBaseTf(sid);
		ASSERT(tf - conf.fastinput >= 0);
		bool sid_active_label = active_label_pattern & (1 << sid);
		
		FillBufferSource(conf, bufs, sid);
		
		
		sys_lock.EnterRead();
		
		int label_id = 0;
		switch (conf.label_id) {
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
		
		
		rf.sector[sid].Process(area, bufs, real_label, real_mask);
		
		real_mask.And(real_label);
		
		conf.sector_accuracy[sid] = rf.sector[sid].stat;
		
		
		sys_lock.LeaveRead();
	}
	
	
	FillBufferSource(conf, bufs, 0);
	
	
	// Train signal usefulness and multiplier
	{
		ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.symbol);
		double spread_point = sys.GetTradingSymbolSpreadPoint(conf.symbol);
		
		VectorBool &real_succ = rf.real_succ;
		VectorBool &real_mult = rf.real_mult;
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
		rf.succ.Process(area, bufs, real_succ, real_mask);
		sys_lock.LeaveRead();
		
		conf.label_stat = rf.succ.stat;
		
		
		sys_lock.EnterRead();
		rf.mult.Process(area, bufs, real_mult, real_mask);
		sys_lock.LeaveRead();
		
		conf.mult_stat = rf.mult.stat;
		
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
		data.used_proc.Add(&rf);
		
		sc.Process();
		
		data.used_conf.Detach(0);
		data.used_proc.Detach(0);
		
		conf.test_valuefactor		= sc.valuefactor;
		conf.test_valuehourfactor	= sc.valuehourfactor;
		conf.test_hourtotal			= sc.hourtotal;
	}
	
	conf.is_processed = true;
	
	rf.lock.Leave();
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
	simcore.Process();
	
	
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
	}
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
	} else {
		/*for(int i = 0; i < SYM_COUNT; i++) {
			
		}*/
		Panic("TODO");
	}
	
	if (!test_broker.init) {
		sys.SetFixedBroker(test_broker);
		
		for(int i = 0; i < 2; i++) {
			if (only_single_source && i != active_label)
				continue;
			for(int j = 0; j < SYM_COUNT; j++) {
				if (only_single_source && j != single_source)
					continue;
				
				int count = data[i][j].used_proc.GetCount();
				ASSERT(count == data[i][j].used_conf.GetCount());
				bufs[i][j].SetCount(count);
				for(int k = 0; k < count; k++) {
					const AccuracyConf& conf = data[i][j].used_conf[k];
					ConstBufferSource& buf = bufs[i][j][k];
					
					esys.FillBufferSource(conf, buf, 0);
					
					ConstBufferSourceIter& iter = iters[i][j].Add(new ConstBufferSourceIter(buf));
					iter.Seek(cursor);
				}
			}
		}
	}
	
	int data_count = sys.GetCountMain();
	
	if (!lightweight) {
		last_test_equity.SetCount(data_count, 0);
		last_test_spreadcost.SetCount(data_count, 0);
	}
	
	int sym_mult[SYM_COUNT];
	for(int i = 0; i < SYM_COUNT; i++)
		sym_mult[i] = 1;
	
	int cursor_end = data_count;
	if (cursor < limit_begin)		cursor = limit_begin;
	if (cursor_end > limit_end)		cursor_end = limit_end;
	
	for (int sym = 0; sym < SYM_COUNT; sym++) {
		sector_logic	[sym].cursor = cursor;
		succ_logic		[sym].cursor = cursor;
		mult_logic		[sym].cursor = cursor;
	}
	
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
			
			for (int p = 0; p < 2; p++) {
				PoleData& data = this->data[p][sym];
				ASSERT(data.used_proc.GetCount() <= LOCALPROB_DEPTH);
				for(int i = 0; i < data.used_proc.GetCount(); i++) {
					const ConfProcessor& proc		= data.used_proc[i];
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
			
			ASSERT(sector_logic.cursor == cursor);
			ASSERT(succ_logic.cursor == cursor);
			ASSERT(mult_logic.cursor == cursor);
			sector_logic.round_checked = true;
			succ_logic.round_checked = true;
			mult_logic.round_checked = true;
			int active_sector	= sector_logic.GetResult();
			int active_succ		= succ_logic.GetResult();
			int active_mult		= mult_logic.GetResult();
			
			
			int signal = 0;
			if (active_sector && active_sector == active_succ) {
				active_symbols++;
				int mult;
				if (active_sector > 0)	mult = +Upp::max(+1, active_mult);
				else					mult = -Upp::min(-1, active_mult);
				signal = active_sector * mult;
			}
			
			
			int prev_signal = test_broker.GetSignal(sym);
			if (signal == prev_signal && signal != 0)
				test_broker.SetSignalFreeze(sym, true);
			else {
				test_broker.SetSignal(sym, signal);
				test_broker.SetSignalFreeze(sym, false);
			}
		}
		
		if (only_single_source) {
			for(int i = 0; i < iters[active_label][single_source].GetCount(); i++)
				iters[active_label][single_source][i]++;
		} else {
			for (int p = 0; p < 2; p++)
				for (int sym = 0; sym < SYM_COUNT; sym++)
					for(int i = 0; i < iters[p][sym].GetCount(); i++)
						iters[p][sym][i]++;
		}
		
		if (active_symbols > 0)
			active_count++;
		
		test_broker.Cycle(cursor);
	}
	
	
	hourtotal = active_count / 60.0;
	valuefactor = test_broker.AccountEquity() / test_broker.begin_equity;
	valuehourfactor = valuefactor / hourtotal;
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
	int buf_cursor = cursor % max_trend_period;
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
	
	cursor++;
	round_checked = false;
	return result;
}

}
