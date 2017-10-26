#include "Overlook.h"

namespace Overlook {

int test0_begin = 0;
int test1_begin = 0;
int cls_tree_count	= 100;
int cls_max_depth	= 4;
int cls_hypothesis	= 10;

VectorBool full_mask;



uint32 DecisionClassifierConf::GetHashValue() const {
	CombineHash ch;
	for(int i = 0; i < DECISION_INPUTS; i++)
		ch << input[i].indi << 1 << input[i].symbol << 1;
	ch << label_id << 1;
	return ch;
}

void DecisionClassifierConf::Randomize() {
	System& sys	= GetSystem();
	for(int i = 0; i < DECISION_INPUTS; i++) {
		input[i].symbol		= Random(sys.GetTradingSymbolCount());
		input[i].indi		= Random(sys.GetTrueIndicatorCount());
	}
	label_id = Random(GetSystem().GetLabelIndicatorCount());
}






/*
DecisionClassifier::DecisionClassifier() {
	
}

void DecisionClassifier::Clear() {
	data0.Clear();
	data1.Clear();
	dec_data.Clear();
	train_labels.Clear();
	test_labels.Clear();
	predicted_labels.Clear();
	train_pos.Clear();
	test_pos.Clear();
}

void DecisionClassifier::Refresh() {
	
}

double DecisionClassifier::PredictOne(int data_pos) {
	System& sys = GetSystem();
	
	// Get references
	ConstBuffer& src0a = sys.GetTrueIndicator(conf.symbol0a, conf.indi0a);
	ConstBuffer& src0b = sys.GetTrueIndicator(conf.symbol0b, conf.indi0b);
	ConstBuffer& src1a = sys.GetTrueIndicator(conf.symbol1a, conf.indi1a);
	ConstBuffer& src1b = sys.GetTrueIndicator(conf.symbol1b, conf.indi1b);
	
	// Fill training data
	RandomForest::Pair data0, data1, dec_data;
	data0[0] = src0a.Get(data_pos);
	data0[1] = src0b.Get(data_pos);
	data1[0] = src1a.Get(data_pos);
	data1[1] = src1b.Get(data_pos);
	
	// Settings
	options.tree_count	= cls_tree_count;
	options.max_depth	= cls_max_depth;
	options.tries_count	= cls_hypothesis;
	
	double p0 = tree0.PredictOne(data0);
	double p1 = tree1.PredictOne(data1);
	dec_data[0] = -1.0 + p0 * 2.0;
	dec_data[1] = -1.0 + p1 * 2.0;
	
	return dec_tree.PredictOne(dec_data);
}*/








uint32 SectorConf::GetHashValue() const {
	CombineHash ch;
	for(int i = 0; i < SECTOR_2EXP; i++)
		ch << dec[i].GetHashValue() << 1;
	ch << symbol << 1 << period << 1 << minimum_prediction_len << 1;
	return ch;
}

void SectorConf::Randomize() {
	for(int i = 0; i < SECTOR_2EXP; i++)
		dec[i].Randomize();
	symbol = Random(GetSystem().GetTradingSymbolCount());
	period = Random(TF_COUNT);
	minimum_prediction_len = Random(1 << (sizeof(minimum_prediction_len) * 8));
}

bool operator == (const SectorConf& a, const SectorConf& b) {
	
}

bool operator < (const SectorConf& a, const SectorConf& b) {
	return a.accuracy > b.accuracy;
}










ExpertSectors::ExpertSectors() {
	for(int i = 0; i < SECTOR_2EXP; i++) {
		sectors[i].SetInputCount(DECISION_INPUTS);
	}
}

void ExpertSectors::Refresh() {
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	
	ConstBufferSource bufs;
	bufs.SetDepth(DECISION_INPUTS);
	
	for(int i = 0; i < SECTOR_2EXP; i++) {
		int tf = conf.period + i;
		
		for(int j = 0; j < DECISION_INPUTS; j++)
			bufs.SetSource(j, sys.GetTrueIndicator(conf.dec[i].input[j].symbol, tf, conf.dec[i].input[j].indi));
		
		// Settings
		sectors[i].options.tree_count	= cls_tree_count;
		sectors[i].options.max_depth	= cls_max_depth;
		sectors[i].options.tries_count	= cls_hypothesis;
		
		// Label from zigzag
		ConstVectorBool& real_label = sys.GetLabelIndicator(conf.symbol, tf, conf.dec[i].label_id);
		ASSERT(real_label.GetCount() == data_count);
		sectors[i].Process(bufs, real_label, full_mask, test0_begin, test1_begin);
	}
	
	// Refresh sector_predicted vectors
	int is_predicting[SECTOR_COUNT];
	for(int i = 0; i < SECTOR_COUNT; i++) {
		sector_predicted[i].Zero();
		is_predicting[i] = 0;
	}
	int minimum_prediction_len = conf.minimum_prediction_len;
	for(int i = 0; i < data_count; i++) {
		int active_sector = 0;
		for(int j = 0; j < SECTOR_2EXP; j++)
			if (sectors[j].predicted_label.Get(i))
				active_sector |= (1 << j);
		
		ASSERT(active_sector >= 0 && active_sector < SECTOR_COUNT);
		is_predicting[active_sector] = minimum_prediction_len;
		for(int j = 0; j < SECTOR_COUNT; j++) {
			if (is_predicting[j] > 0) {
				sector_predicted[j].Set(i, true);
				is_predicting[j]--;
			}
		}
	}
	
	
	// Write testing accuracy to conf
	// conf.
	Panic("TODO");
}








void AdvisorConf::Randomize() {
	sec.Randomize();
	sector = Random(SECTOR_COUNT);
	label.Randomize();
	signal.Randomize();
	minimum_prediction_len = Random(1 << (sizeof(minimum_prediction_len) * 8));
}

uint32 AdvisorConf::GetHashValue() const {
	CombineHash ch;
	ch	<< sec.GetHashValue() << 1
		<< sector << 1 << amp << 1 << minimum_prediction_len << 1
		<< label.GetHashValue() << 1
		<< signal.GetHashValue();
	return ch;
}

bool operator == (const AdvisorConf& a, const AdvisorConf& b) {
	
}

bool operator < (const AdvisorConf& a, const AdvisorConf& b) {
	return a.accuracy > b.accuracy;
}














ExpertAdvisor::ExpertAdvisor() {
	label.SetInputCount(DECISION_INPUTS);
	for(int i = 0; i < 2; i++) {
		signal[i].SetInputCount(DECISION_INPUTS);
	}
}

void ExpertAdvisor::Refresh() {
	ASSERT(conf.sector >= 0 && conf.sector < SECTOR_COUNT);
	System& sys = GetSystem();
	
	if (!sector) {
		ExpertCache& cache = GetExpertCache();
		sector = &cache.GetExpertSectors(conf.sec);
	}
	
	label.options.tree_count	= cls_tree_count;
	label.options.max_depth		= cls_max_depth;
	label.options.tries_count	= cls_hypothesis;
	for(int i = 0; i < 2; i++) {
		signal[i].options.tree_count	= cls_tree_count;
		signal[i].options.max_depth		= cls_max_depth;
		signal[i].options.tries_count	= cls_hypothesis;
	}
	
	ExpertSectors& sectors = *sector;
	sectors.Refresh();
	
	int data_count = sys.GetCountMain();
	int tf = conf.sec.period - 1;
	ASSERT(tf >= 0);
	
	ConstBufferSource bufs;
	bufs.SetDepth(DECISION_INPUTS);
	for(int i = 0; i < DECISION_INPUTS; i++)
		bufs.SetSource(i, sys.GetTrueIndicator(conf.label.input[i].symbol, tf, conf.label.input[i].indi));
	ConstVectorBool& mask = sectors.sector_predicted[conf.sector];
	ConstVectorBool& real_label = sys.GetLabelIndicator(conf.sec.symbol, tf, conf.label.label_id) ;
	label.Process(bufs, real_label, mask, test0_begin, test1_begin);
	
	int is_predicting[2];
	bool was_predicting[2];
	double section_open[2];
	int section_open_pos[2];
	for(int i = 0; i < 2; i++) {
		predicted[i].Zero();
		signal_mask[i].Zero();
		signal_label[i].Zero();
		is_predicting[i] = 0;
		was_predicting[i] = false;
	}
	int minimum_prediction_len = conf.minimum_prediction_len;
	ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.sec.symbol);
	double spread_point = sys.GetTradingSymbolSpreadPoint(conf.sec.symbol);
	for(int i = 0; i < data_count; i++) {
		bool is_predicted = mask.Get(i);
		if (!is_predicted)
			continue;
		
		bool label = this->label.predicted_label.Get(i);
		is_predicting[label] = minimum_prediction_len;
		for(int j = 0; j < 2; j++) {
			if (is_predicting[j] > 0) {
				predicted[j].Set(i, true);
				is_predicting[j]--;
				if (!was_predicting[j]) {
					section_open[j] = open_buf.Get(i);
					section_open_pos[j] = i;
				}
				was_predicting[j] = true;
			} else {
				if (was_predicting[j]) {
					double open = open_buf.Get(i);
					bool label;
					Panic("TODO which zigzag label is up?");
					if (j == 0) {
						label = (open / (section_open[j] + spread_point) - 1.0) > 0.0;
					} else {
						label = (1.0 - (open + spread_point) / section_open[j]) > 0.0;
					}
					signal_mask[j]	.Set(section_open_pos[j], true);
					signal_label[j]	.Set(section_open_pos[j], label);
				}
				was_predicting[j] = false;
			}
		}
	}
	
	for(int i = 0; i < DECISION_INPUTS; i++)
		bufs.SetSource(i, sys.GetTrueIndicator(conf.signal.input[i].symbol, tf, conf.signal.input[i].indi));
	for(int i = 0; i < 2; i++)
		signal[i].Process(bufs, signal_label[i], signal_mask[i], test0_begin, test1_begin);
	
	
	
	// Return signal accuracy as result
	// conf. =
	
}














void FusionConf::Randomize() {
	for(int i = 0; i < advs.GetCount(); i++)
		advs[i].Randomize();
	dec.Randomize();
	period = Random(TF_COUNT);
	sector_period = Random(TF_COUNT);
}

uint32 FusionConf::GetHashValue() const {
	CombineHash ch;
	for(int i = 0; i < advs.GetCount(); i++)
		ch << advs[i].GetHashValue() << 1;
	ch << dec.GetHashValue() << 1 << period << 1 << sector_period << 1;
	return ch;
}

bool operator == (const FusionConf& a, const FusionConf& b) {
	
}

bool operator < (const FusionConf& a, const FusionConf& b) {
	return a.accuracy > b.accuracy;
}














ExpertFusion::ExpertFusion() {
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].SetInputCount(DECISION_INPUTS);
	}
}

void ExpertFusion::Refresh() {
	
	if (advisors.IsEmpty()) {
		ExpertCache& cache = GetExpertCache();
		
		// Reference advisors
		ASSERT(!conf.advs.IsEmpty());
		advisors.SetCount(conf.advs.GetCount());
		for(int i = 0; i < conf.advs.GetCount(); i++) {
			advisors[i] = &cache.GetExpertAdvisor(conf.advs[i]);
		}
	}
	
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].options.tree_count	= cls_tree_count;
		dec[i].options.max_depth		= cls_max_depth;
		dec[i].options.tries_count	= cls_hypothesis;
	}
	
	
	// Refresh advisors
	int adv_count = advisors.GetCount();
	for(int i = 0; i < adv_count; i++)
		advisors[i]->Refresh();
	
	
	// Init triggers
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	sys.SetFixedBroker(broker);
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		// First has longest period
		triggers[i].SetPeriod(1 << (3 + conf.period + (FUSE_DEC_COUNT-1 - i)));
	}
	
	
	// Get data points
	broker.Reset();
	Vector<int> trigger_points;
	Vector<double> equities;
	
	equities.SetCount(data_count);
	int sym_signals[SYM_COUNT];
	
	for(int i = 0; i < data_count; i++) {
		broker.RefreshOrders(i);
		
		double equity = broker.AccountEquity();
		equities[i] = equity;
		bool is_triggered = false;
		for(int j = 0; j < FUSE_DEC_COUNT; j++) {
			triggers[j].Add(equity);
			if (!is_triggered)
				is_triggered |= triggers[j].IsTriggered();
		}
		if (is_triggered)
			trigger_points.Add(i);
		
		for(int j = 0; j < SYM_COUNT; j++)
			sym_signals[j] = 0;
		
		for(int j = 0; j < adv_count; j++) {
			ExpertAdvisor& adv = *advisors[j];
			int sym = adv.conf.sec.symbol;
			
			// Is advisor sector activated
			Panic("TODO check zigzag label direction up/down... completely not checked");
			bool up_predicted = adv.predicted[0].Get(i);
			bool down_predicted = adv.predicted[1].Get(i);
			
			if (up_predicted) {
				if (sym_signals[sym] == 0)
					sym_signals[sym] = +1;
			}
			if (down_predicted) {
				if (sym_signals[sym] == 0)
					sym_signals[sym] = -1;
			}
		}
		
		for(int j = 0; j < SYM_COUNT; j++) {
			int signal = sym_signals[j];
			if (signal == broker.GetSignal(j) && signal != 0)
				broker.SetSignalFreeze(j, true);
			else {
				broker.SetSignal(j, signal);
				broker.SetSignalFreeze(j, false);
			}
		}
		
		broker.Cycle(i);
	}
	
	
	// Fill labels
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		mask[i].Zero();
		label[i].Zero();
	}
	for(int i = 0; i < trigger_points.GetCount(); i++) {
		int begin_point = trigger_points[i];
		double begin_equity = equities[begin_point];
		double max_diff = 0.0;
		int max_diff_j = -1;
		for(int j = 0; j < FUSE_DEC_COUNT; j++) {
			int step = triggers[j].GetPeriod();
			int end_point = min(data_count-1, begin_point + step);
			int len = end_point - begin_point;
			if (len <= 0) continue;
			double end_equity = equities[end_point];
			double equity_diff =  end_equity - begin_equity;
			double av_diff = equity_diff / len;
			if (av_diff > max_diff) {
				max_diff = av_diff;
				max_diff_j = j;
			}
		}
		bool main_label = max_diff > 0.0;
		for(int j = 0; j < FUSE_DEC_COUNT; j++) {
			bool label = main_label && j <= max_diff_j;
			this->mask[j].Set(begin_point, true);
			this->label[j].Set(begin_point, label);
		}
	}
	
	
	// Refresh classifiers
	int tf = conf.sector_period;
	ConstBufferSource bufs;
	bufs.SetDepth(DECISION_INPUTS);
	for(int j = 0; j < DECISION_INPUTS; j++)
		bufs.SetSource(j, sys.GetTrueIndicator(conf.dec.input[j].symbol, tf, conf.dec.input[j].indi));
		
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].Process(bufs, label[i], mask[i], test0_begin, test1_begin);
	}
	
	
	// Write results to FusionConf
	// conf. =
	
}















ExpertCache::ExpertCache() {
	
}

ExpertSectors& ExpertCache::GetExpertSectors(const SectorConf& conf) {
	int i = sectors.Find(conf);
	if (i != -1) return sectors[i];
	
	lock.Enter();
	ExpertSectors& sec = sectors.Add(conf);
	sec.conf = conf;
	lock.Leave();
	
	sec.Refresh();
	return sec;
}

ExpertAdvisor& ExpertCache::GetExpertAdvisor(const AdvisorConf& conf) {
	int i = experts.Find(conf);
	if (i != -1) return experts[i];
	
	lock.Enter();
	ExpertAdvisor& adv = experts.Add(conf);
	adv.conf = conf;
	lock.Leave();
	
	adv.Refresh();
	return adv;
}

ExpertFusion& ExpertCache::GetExpertFusion(const FusionConf& conf) {
	int i = fusions.Find(conf);
	if (i != -1) return fusions[i];
	
	lock.Enter();
	ExpertFusion& fus = fusions.Add(conf);
	fus.conf = conf;
	lock.Leave();
	
	fus.Refresh();
	return fus;
}













ExpertOptimizer::ExpertOptimizer() {
	sec_confs.Reserve(10000);
	adv_confs.Reserve(10000);
	fus_confs.Reserve(10000);
	
}

void ExpertOptimizer::EvolveSector(SectorConf& conf) {
	
	//  - symbol matching (increasing prob.)
	//  - indicator matching (increasing prob.)
	
	// - symbol count
	
	
}

void ExpertOptimizer::EvolveAdvisor(AdvisorConf& conf) {
	
	if (adv_confs.GetCount() < pop_size) {
		conf.Randomize();
		return;
	}
	
	// combine rows
	// affecting variables:
	//  - indicator matching (increasing prob.)
	//  - symbol matching (increasing prob.)
	//  - period matching (increasing prob.) ?????
	//		- sector indi period should match sector +/- step period
	//  Note:
	//  DE solver, but break integers!
	
	
	// Use function from DE solver
	
	// - small randomness in used sectors
	// - big randomness in trigger ExpertAdvisor label and signals
	
	
	// - 1-100 separate evolving threads to have different advisors for fuse
	
	// - random enumerators boolean, but also completely random
	// - AdvisorConf::amp
	
}

void ExpertOptimizer::EvolveFusion(FusionConf& conf) {
	
	if (fus_confs.GetCount() < pop_size) {
		conf.Randomize();
		return;
	}
	
	
	// Use function from DE solver
	
	// Fuse stage:
	// - very small randomness in used sectors
	// - gets best advisors and just slightly evolves them by combining
	// - big randomness in trigger DecisionClassifier
	// Final stage:
	// - slightly evolves fusion by combining
	
	// Pick advisors, which matches conf values
	
	
	// - dd-limit of advisors
	// - set fmlevel 0.6
	
	// - advisor position affects, so it must be optimized
	
	
}

void ExpertOptimizer::AddTestResult(const SectorConf& conf) {
	sec_confs.Add(conf);
}

void ExpertOptimizer::AddTestResult(const AdvisorConf& conf) {
	adv_confs.Add(conf);
}

void ExpertOptimizer::AddTestResult(const FusionConf& conf) {
	fus_confs.Add(conf);
}

void ExpertOptimizer::ReduceMemory() {
	Panic("TODO");
}

bool ExpertOptimizer::IsSectorOptimizing() const {
	Panic("TODO");
}

bool ExpertOptimizer::IsAdvisorOptimizing() const {
	Panic("TODO");
}

bool ExpertOptimizer::IsFusionOptimizing() const {
	Panic("TODO");
}











ExpertSystem::ExpertSystem(System* sys) : sys(sys) {
	running = false;
	stopped = true;
	

#ifndef flagHAVE_ALLSYM

	allowed_symbols.Add("EURUSD", 3); // 0.26
	allowed_symbols.Add("GBPUSD", 3); // 0.23
	allowed_symbols.Add("USDJPY", 3); // 0.27
	allowed_symbols.Add("USDCAD", 3); // 0.24
	allowed_symbols.Add("EURJPY", 3); // 0.23
	allowed_symbols.Add("EURCHF", 3); // 0.26
	
#else

	// also AUD,NZD,CHF included
	allowed_symbols.Add("EURUSD",  3); // lts
	allowed_symbols.Add("GBPUSD",  3); // lts
	allowed_symbols.Add("USDJPY",  3); // lts
	allowed_symbols.Add("USDCHF",  3); // lts
	allowed_symbols.Add("USDCAD",  3); // lts
	allowed_symbols.Add("AUDUSD",  3); // lts
	allowed_symbols.Add("NZDUSD",  3); // lts
	allowed_symbols.Add("EURJPY",  3); // lts
	allowed_symbols.Add("EURCHF",  3); // lts
	allowed_symbols.Add("EURGBP",  3); // lts
	allowed_symbols.Add("AUDCAD", 10); // lts
	allowed_symbols.Add("AUDJPY", 10); // lts
	allowed_symbols.Add("CADJPY", 10); // lts
	allowed_symbols.Add("CHFJPY", 10); // lts
	//allowed_symbols.Add("NZDCAD", 10);
	//allowed_symbols.Add("NZDCHF", 10);
	allowed_symbols.Add("EURAUD",  7); // lts
	allowed_symbols.Add("GBPCHF",  7); // lts
	allowed_symbols.Add("GBPJPY",  7); // lts
	allowed_symbols.Add("AUDNZD", 12); // lts
	allowed_symbols.Add("EURCAD", 12); // lts
	//allowed_symbols.Add("GBPAUD", 12);
	//allowed_symbols.Add("GBPCAD", 12);
	//allowed_symbols.Add("GBPNZD", 12);

#endif
	
	ASSERT(allowed_symbols.GetCount() == SYM_COUNT);

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
		
		else if (phase == PHASE_REAL)
			MainReal();
		
		Sleep(100);
	}


	stopped = true;
}

void ExpertSystem::StoreThis() {
	Time t = GetSysTime();
	String file = ConfigFile("agentgroup.bin");
	bool rem_bak = false;

	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}

	StoreToFile(*this,	file);

	if (rem_bak)
		DeleteFile(file + ".bak");

	last_store.Reset();
}

void ExpertSystem::LoadThis() {
	if (FileExists(ConfigFile("agentgroup.bin.bak")))
		LoadFromFile(*this,	ConfigFile("agentgroup.bin.bak"));
	else
		LoadFromFile(*this,	ConfigFile("agentgroup.bin"));
}

void ExpertSystem::Serialize(Stream& s) {
	
}

void ExpertSystem::MainTraining() {
	System& sys = GetSystem();
	
	sys.ProcessWorkQueue();
	
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	
	int data_count = sys.GetCountMain();
	test0_begin = data_count / 2;
	test1_begin = data_count * 99 / 100;
	
	full_mask.SetCount(data_count).One();
	
	//while (running && phase == PHASE_TRAINING)
	{
		if (fusion.IsSectorOptimizing() && running) {
			for(int i = 0; i < fusion.GetPopulationSize(); i++) {
				co & [=] {
					ExpertCache& cache = GetExpertCache();
					
					
					// Load configuration
					SectorConf conf;
					fusion.EvolveSector(conf);
					
					
					// Load advisor
					ExpertSectors& sec = cache.GetExpertSectors(conf);
					
					
					// Refresh advisor and add test result to table
					sec.Refresh();
					fusion.AddTestResult(sec.conf);
				};
			}
			
			co.Finish();
							
			
			// Sort table
			fusion.SortSectors();
		}
		
		
		
		if (fusion.IsAdvisorOptimizing() && running) {
			for(int i = 0; i < fusion.GetPopulationSize(); i++) {
				co & [=] {
					ExpertCache& cache = GetExpertCache();
					
					
					// Load configuration
					AdvisorConf conf;
					fusion.EvolveAdvisor(conf);
					
					
					// Load advisor
					ExpertAdvisor& adv = cache.GetExpertAdvisor(conf);
					
					
					// Refresh advisor and add test result to table
					adv.Refresh();
					fusion.AddTestResult(adv.conf);
				};
			}
			
			co.Finish();
							
			
			// Sort table
			fusion.SortAdvisors();
		}
		
		
		
		// If enough advisor optimizing, optimize fusions
		if (fusion.IsFusionOptimizing() && running) {
			
			for(int i = 0; i < fusion.GetPopulationSize(); i++) {
				co & [=] {
					ExpertCache& cache = GetExpertCache();
					
					
					// Load configuration
					FusionConf conf;
					fusion.EvolveFusion(conf);
					
					
					// Load advisor
					ExpertFusion& fus = cache.GetExpertFusion(conf);
					
					
					// Refresh advisor and add test result to table
					fus.Refresh();
					fusion.AddTestResult(fus.conf);
				};
			}
			
			co.Finish();
			
			
			fusion.SortFusions();
		}
		
		
		
		fusion.ReduceMemory();
	}
	
	
}

void ExpertSystem::MainReal() {
	System& sys = GetSystem();
	
	int data_count = sys.GetCountMain();
	test0_begin = data_count - 2;
	test1_begin = data_count - 1;
	
	full_mask.SetCount(data_count).One();
	
	
	
	// - set solver fmlevel
	
	
}

}
