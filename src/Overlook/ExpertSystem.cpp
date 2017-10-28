#include "Overlook.h"

namespace Overlook {

int cls_tree_count	= (DECISION_INPUTS * 3);
int cls_max_depth	= 4;
int cls_hypothesis	= 5;

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

String DecisionClassifierConf::ToString() const {
	String s;
	s << (int)label_id << " ";
	for(int i = 0; i < DECISION_INPUTS; i++) {
		s << "(" << (int)input[i].symbol << ", " << (int)input[i].indi << ") ";
	}
	return s;
}

bool operator == (const DecisionClassifierConf& a, const DecisionClassifierConf& b) {
	for(int i = 0; i < DECISION_INPUTS; i++) {
		if (a.input[i].indi != b.input[i].indi) return false;
		if (a.input[i].symbol != b.input[i].symbol) return false;
	}
	if (a.label_id != b.label_id) return false;
	return true;
}













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
	period = Random(TF_COUNT - SECTOR_2EXP - 1) + 1;
	minimum_prediction_len = Random(1 << (sizeof(minimum_prediction_len) * 8));
}

bool operator == (const SectorConf& a, const SectorConf& b) {
	for(int i = 0; i < SECTOR_2EXP; i++)
		if (a.dec[i] == b.dec[i])
			return false;
	if (a.symbol != b.symbol) return false;
	if (a.period != b.period) return false;
	if (a.minimum_prediction_len != b.minimum_prediction_len) return false;
	return true;
}

bool operator < (const SectorConf& a, const SectorConf& b) {
	return a.accuracy > b.accuracy;
}










ExpertSectors::ExpertSectors() {
	for(int i = 0; i < SECTOR_2EXP; i++) {
		sectors[i].SetInputCount(DECISION_INPUTS);
	}
}

void ExpertSectors::Configure() {
	
}

void ExpertSectors::Refresh(const ForestArea& area) {
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	
	ConstBufferSource bufs;
	bufs.SetDepth(DECISION_INPUTS);
	
	double accuracy = 0.0;
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
		sectors[i].Process(area, bufs, real_label, full_mask);
		accuracy += sectors[i].test0_accuracy;
	}
	accuracy /= SECTOR_2EXP;
	
	// Refresh sector_predicted vectors
	int is_predicting[SECTOR_COUNT];
	for(int i = 0; i < SECTOR_COUNT; i++) {
		sector_predicted[i].SetCount(data_count).Zero();
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
	conf.accuracy = accuracy;
}








void AdvisorConf::Randomize() {
	sectors.Randomize();
	sector = Random(SECTOR_COUNT);
	label.Randomize();
	signal.Randomize();
	minimum_prediction_len = Random(1 << (sizeof(minimum_prediction_len) * 8));
}

uint32 AdvisorConf::GetHashValue() const {
	CombineHash ch;
	ch	<< sectors.GetHashValue() << 1
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

void ExpertAdvisor::Configure() {
	sectors.conf = conf.sectors;
	sectors.Configure();
}

void ExpertAdvisor::Refresh(const ForestArea& area) {
	ASSERT(conf.sector >= 0 && conf.sector < SECTOR_COUNT);
	System& sys = GetSystem();
	
	
	label.options.tree_count	= cls_tree_count;
	label.options.max_depth		= cls_max_depth;
	label.options.tries_count	= cls_hypothesis;
	for(int i = 0; i < 2; i++) {
		signal[i].options.tree_count	= cls_tree_count;
		signal[i].options.max_depth		= cls_max_depth;
		signal[i].options.tries_count	= cls_hypothesis;
	}
	
	sectors.Refresh(area);
	
	int data_count = sys.GetCountMain();
	int tf = conf.sectors.period - 1;
	ASSERT(tf >= 0);
	
	ConstBufferSource bufs;
	bufs.SetDepth(DECISION_INPUTS);
	for(int i = 0; i < DECISION_INPUTS; i++)
		bufs.SetSource(i, sys.GetTrueIndicator(conf.label.input[i].symbol, tf, conf.label.input[i].indi));
	ConstVectorBool& mask = sectors.sector_predicted[conf.sector];
	ConstVectorBool& real_label = sys.GetLabelIndicator(conf.sectors.symbol, tf, conf.label.label_id) ;
	label.Process(area, bufs, real_label, mask);
	
	int is_predicting[2];
	bool was_predicting[2];
	double section_open[2];
	int section_open_pos[2];
	for(int i = 0; i < 2; i++) {
		predicted[i].SetCount(data_count).Zero();
		signal_mask[i].SetCount(data_count).Zero();
		signal_label[i].SetCount(data_count).Zero();
		is_predicting[i] = 0;
		was_predicting[i] = false;
	}
	int minimum_prediction_len = conf.minimum_prediction_len;
	ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.sectors.symbol);
	double spread_point = sys.GetTradingSymbolSpreadPoint(conf.sectors.symbol);
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
					section_open[j] = open_buf.GetUnsafe(i);
					section_open_pos[j] = i;
				}
				was_predicting[j] = true;
			} else {
				if (was_predicting[j]) {
					double open = open_buf.GetUnsafe(i);
					bool label;
					// 0 up, 1 down
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
	
	double accuracy = 0;
	for(int i = 0; i < 2; i++) {
		signal[i].Process(area, bufs, signal_label[i], signal_mask[i]);
		accuracy += signal[i].test0_accuracy;
	}
	accuracy /= 2;
	
	
	// Return signal accuracy as result
	conf.accuracy = accuracy;
	
}














void FusionConf::Randomize() {
	for(int i = 0; i < ADVISOR_COUNT; i++)
		advisors[i].Randomize();
	dec.Randomize();
	period = Random(TF_COUNT);
	sector_period = Random(TF_COUNT);
}

uint32 FusionConf::GetHashValue() const {
	CombineHash ch;
	for(int i = 0; i < ADVISOR_COUNT; i++)
		ch << advisors[i].GetHashValue() << 1;
	ch << dec.GetHashValue() << 1 << period << 1 << sector_period << 1;
	return ch;
}

bool operator == (const FusionConf& a, const FusionConf& b) {
	
}

bool operator < (const FusionConf& a, const FusionConf& b) {
	return a.accuracy > b.accuracy;
}

void FusionConf::operator=(const FusionConf& src) {
	for(int i = 0; i < ADVISOR_COUNT; i++)
		advisors[i] = src.advisors[i];
	dec = src.dec;
	period = src.period;
	sector_period = src.sector_period;
	
	ConfTestResults::operator=(src);
}














ExpertFusion::ExpertFusion() {
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].SetInputCount(DECISION_INPUTS);
	}
}

void ExpertFusion::Configure() {
	for(int i = 0; i < ADVISOR_COUNT; i++) {
		advisors[i].conf = conf.advisors[i];
		advisors[i].Configure();
	}
}

void ExpertFusion::Refresh(const ForestArea& area) {
	
	
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].options.tree_count	= cls_tree_count;
		dec[i].options.max_depth	= cls_max_depth;
		dec[i].options.tries_count	= cls_hypothesis;
	}
	
	
	// Refresh advisors
	for(int i = 0; i < ADVISOR_COUNT; i++)
		advisors[i].Refresh(area);
	
	
	// Init triggers
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	sys.SetFixedBroker(broker);
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		// First has longest period
		triggers[i].SetPeriod(1 << (3 + conf.period + (FUSE_DEC_COUNT-1 - i)));
	}
	
	
	// Get data points
	trigger_points.SetCount(0);
	equities.SetCount(data_count);
	int sym_signals[SYM_COUNT];
	
	for (int test_area = 0; test_area < 2; test_area++) {
		int begin = test_area == 0 ? area.test0_begin : area.test1_begin;
		int end   = test_area == 0 ? area.test0_end   : area.test1_end;
		broker.Reset();
		
		for(int i = begin; i < end; i++) {
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
			
			for(int j = 0; j < ADVISOR_COUNT; j++) {
				ExpertAdvisor& adv = advisors[j];
				int sym = adv.conf.sectors.symbol;
				
				// Is advisor sector activated
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
	}
	
	
	// Fill labels
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		mask[i].SetCount(data_count).Zero();
		label[i].SetCount(data_count).Zero();
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
	
	double accuracy = 0.0;
	int acc_sum = FUSE_DEC_COUNT;
	for(int i = 0; i < FUSE_DEC_COUNT; i++) {
		dec[i].Process(area, bufs, label[i], mask[i]);
		accuracy += dec[i].test0_accuracy;
	}
	for(int i = 0; i < ADVISOR_COUNT; i++) {
		const ExpertAdvisor& adv = advisors[i];
		accuracy += adv.label.test0_accuracy;		acc_sum++;
		accuracy += adv.signal[0].test0_accuracy;	acc_sum++;
		accuracy += adv.signal[1].test0_accuracy;	acc_sum++;
		for(int j = 0; j < SECTOR_2EXP; j++) {
			accuracy += adv.sectors.sectors[j].test0_accuracy;
			acc_sum++;
		}
	}
	accuracy /= acc_sum;
	
	
	// Run testing
	ConstBufferSourceIter iter(bufs);
	for (int test_area = 0; test_area < 2; test_area++) {
		int begin = test_area == 0 ? area.test0_begin : area.test1_begin;
		int end   = test_area == 0 ? area.test0_end   : area.test1_end;
		broker.Reset();
		
		iter.Seek(begin-1);
		int cur_trigger = -1;
		bool enabled_any = false;
		for(int i = 0; i < FUSE_DEC_COUNT; i++) {
			if (dec[i].forest.PredictOne(iter) < 0.5)
				break;
			cur_trigger++;
			enabled_any = true;
		}
		iter++;
		
		for(int i = begin; i < end; i++, iter++) {
			broker.RefreshOrders(i);
			
			bool is_triggered = false;
			double equity = broker.AccountEquity();
			for(int j = 0; j < FUSE_DEC_COUNT; j++)
				triggers[j].Add(equity);
			if (enabled_any)
				is_triggered = triggers[cur_trigger].IsTriggered();
			else
				is_triggered = triggers[FUSE_DEC_COUNT-1].IsTriggered();
			
			if (is_triggered) {
				cur_trigger = -1;
				enabled_any = false;
				for(int j = 0; j < FUSE_DEC_COUNT; j++) {
					if (dec[j].forest.PredictOne(iter) < 0.5)
						break;
					cur_trigger++;
					enabled_any = true;
				}
			}
			
			for(int j = 0; j < SYM_COUNT; j++)
				sym_signals[j] = 0;
			
			for(int j = 0; j < ADVISOR_COUNT; j++) {
				ExpertAdvisor& adv = advisors[j];
				int sym = adv.conf.sectors.symbol;
				
				// Is advisor sector activated
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
				if (!enabled_any)
					signal = 0;
				if (signal == broker.GetSignal(j) && signal != 0)
					broker.SetSignalFreeze(j, true);
				else {
					broker.SetSignal(j, signal);
					broker.SetSignalFreeze(j, false);
				}
			}
			
			broker.Cycle(i);
		}
		
		
		if (test_area == 0) {
			conf.test0_equity = broker.AccountEquity();
			conf.test0_dd = broker.GetDrawdown();
		} else {
			conf.test1_equity = broker.AccountEquity();
			conf.test1_dd = broker.GetDrawdown();
		}
	}
	
	
	// Write results to FusionConf
	conf.accuracy = accuracy;
	
}




























ExpertOptimizer::ExpertOptimizer() {
	fus_confs.Reserve(10000);
	
}

void ExpertOptimizer::EvolveFusion(FusionConf& conf) {
	
	if (fus_confs.GetCount() < pop_size) {
		conf.Randomize();
		return;
	}
	
	
	conf.Randomize();
	
	//  - symbol matching (increasing prob.)
	//  - indicator matching (increasing prob.)
	
	// - symbol count
	
	
	/*
	int r1, r2;
	int n;

	SelectSamples(candidate,&r1,&r2);
	n = (int)RandomUniform(0.0,(double)dimension);

	Vector<double> *pop_ptr = &population[candidate];
	int c1 = pop_ptr->GetCount();
	trial_solution.SetCount(c1);
	for(int i = 0; i < c1; i++ )
		trial_solution[i] = (*pop_ptr)[i];
	
	for (int i=0; (RandomUniform(0.0,1.0) < probability) && (i < dimension); i++) {
		trial_solution[n] = best_solution[n]
						   + scale * (Element(population,r1,n)
									  - Element(population,r2,n));
		n = (n + 1) % dimension;
	}
	*/
	
	
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

void ExpertOptimizer::AddTestResult(const FusionConf& conf) {
	DUMP(conf.accuracy);
	
	lock.Enter();
	
	int replace_i = -1;
	bool skip_add = false;
	
	if (fus_confs.GetCount() >= pop_limit) {
		double lowest_accuracy = 1.0;
		double av_accuracy = 0.0;
		for(int i = 0; i < fus_confs.GetCount(); i++) {
			const FusionConf& cf = fus_confs[i];
			av_accuracy += cf.accuracy;
			if (lowest_accuracy > cf.accuracy) {
				lowest_accuracy = cf.accuracy;
				replace_i = i;
			}
		}
		av_accuracy /= fus_confs.GetCount();
		
		skip_add =	lowest_accuracy > conf.accuracy ||
					av_accuracy * accuracy_limit_factor > conf.accuracy;
	}
	
	if (!skip_add) {
		if (replace_i == -1)
			fus_confs.Add(conf);
		else
			fus_confs[replace_i] = conf;
	}
	
	fus_results.Add(conf.test0_equity);
	
	lock.Leave();
}

void ExpertOptimizer::ReduceMemory() {
	LOG("ReduceMemory");
	
	
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
	
	full_mask.SetCount(data_count).One();
	
	while (running && phase == PHASE_TRAINING && !Thread::IsShutdownThreads())
	{
		
		// If enough advisor optimizing, optimize fusions
		for(int i = 0; i < fusion.GetPopulationSize() && running && !Thread::IsShutdownThreads(); i++) {
			co & [=] {
				if (!running || Thread::IsShutdownThreads()) return;
				
				ExpertFusion& fus = GetExpertCache().GetExpertFusion(CoWork::GetWorkerIndex());
				
				// Load configuration
				fusion.EvolveFusion(fus.conf);
				fus.Configure();
				
				// Refresh advisor
				ForestArea area;
				int len = 1 * 5 * 24 * 60;
				area.train_begin = Random(data_count - len);
				area.train_end = area.train_begin + len;
				do {
					area.test0_begin = Random(data_count - len);
					area.test0_end = area.test0_begin + len;
				} while (abs(area.test0_begin - area.train_begin) < 2*len);
				do {
					area.test1_begin = Random(data_count - len);
					area.test1_end = area.test1_begin + len;
				}
				while (abs(area.test0_begin - area.train_begin) < 2*len ||
					   abs(area.test1_begin - area.train_begin) < 2*len);
				fus.Refresh(area);
				
				// Return test result
				fusion.AddTestResult(fus.conf);
			};
		}
		
		co.Finish();
		
		fusion.SortFusions();
		fusion.ReduceMemory();
	}
	
	stopped = true;
}

void ExpertSystem::MainReal() {
	System& sys = GetSystem();
	
	int data_count = sys.GetCountMain();
	//test0_begin = data_count - 2;
	//test1_begin = data_count - 1;
	
	full_mask.SetCount(data_count).One();
	
	
	
	// - set solver fmlevel
	
	
}

}
