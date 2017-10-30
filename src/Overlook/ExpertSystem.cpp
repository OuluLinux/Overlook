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
	minimum_prediction_len = Random(MINPRED_LEN);
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
		ASSERT(&real_label != NULL);
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
	int minimum_prediction_len = 1 << conf.minimum_prediction_len;
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
	label.Randomize();
	signal.Randomize();
	sector = Random(SECTOR_COUNT);
	amp = Random(AMP_MAXSCALES);
	minimum_prediction_len = Random(MINPRED_LEN);
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
	if (tf < 0) tf = 0;
	
	
	bufs.SetDepth(DECISION_INPUTS);
	for(int i = 0; i < DECISION_INPUTS; i++)
		bufs.SetSource(i, sys.GetTrueIndicator(conf.label.input[i].symbol, tf, conf.label.input[i].indi));
	ConstVectorBool& mask = sectors.sector_predicted[conf.sector];
	ConstVectorBool& real_label = sys.GetLabelIndicator(conf.sectors.symbol, tf, conf.label.label_id) ;
	ASSERT(&real_label != NULL);
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
	int minimum_prediction_len = 1 << conf.minimum_prediction_len;
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
	return (a.test0_equity + a.test1_equity) > (b.test0_equity + b.test1_equity);
}

void FusionConf::operator=(const FusionConf& src) {
	for(int i = 0; i < ADVISOR_COUNT; i++)
		advisors[i] = src.advisors[i];
	dec = src.dec;
	period = src.period;
	sector_period = src.sector_period;
	
	test0_equity	= src.test0_equity;
	test0_dd		= src.test0_dd;
	test1_equity	= src.test1_equity;
	test1_dd		= src.test1_dd;
	
	ConfTestResults::operator=(src);
}














ExpertFusion::ExpertFusion() {
	locked = 0;
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
						sym_signals[sym] = +1 * (adv.conf.amp * AMP_MAXSCALE_MUL);
				}
				if (down_predicted) {
					if (sym_signals[sym] == 0)
						sym_signals[sym] = -1 * (adv.conf.amp * AMP_MAXSCALE_MUL);
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
		broker.CloseAll(end-1);
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
						sym_signals[sym] = +1 * (adv.conf.amp * AMP_MAXSCALE_MUL);
				}
				if (down_predicted) {
					if (sym_signals[sym] == 0)
						sym_signals[sym] = -1 * (adv.conf.amp * AMP_MAXSCALE_MUL);
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
		broker.CloseAll(end-1);
		
		
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
	if (fus_confs.GetCount() < pop_size ||
		Randomf() <= rand_prob) {
		conf.Randomize();
		return;
	}
	
	
	ConfEvolver evo;
	
	int r_best, r1, r2;
	r_best = Random(5); // switch between TOP 5
	{r1 = 1 + Random(fus_confs.GetCount() - 1);}
	do  {r2 = Random(fus_confs.GetCount());} while (r2 == r1);
	
	lock.Enter();
	
	evo.osrc = &conf;
	evo.best = &fus_confs[r_best];
	evo.src1 = &fus_confs[r1];
	evo.src2 = &fus_confs[r2];
	
	int loop = 0;
	while (!evo.loop_finished) {
		conf.Evolve(evo);
		if (!evo.loop_type) {
			evo.begin = Random(evo.total);
			evo.loop_type = true; // first loop is counting...
		}
		ASSERT(loop++ < 5);
	}
	
	lock.Leave();
}

void ExpertOptimizer::AddTestResult(const FusionConf& conf) {
	lock.Enter();
	
	int replace_i = -1;
	bool skip_add = false;
	
	if (fus_confs.GetCount() >= pop_limit) {
		double lowest_result = DBL_MAX;
		for(int i = 0; i < fus_confs.GetCount(); i++) {
			const FusionConf& cf = fus_confs[i];
			double eq = cf.test0_equity + cf.test1_equity;
			if (lowest_result > eq) {
				lowest_result = eq;
				replace_i = i;
			}
		}
		double confeq = conf.test0_equity + conf.test1_equity;
		skip_add = lowest_result > confeq;
	}
	
	if (!skip_add) {
		if (replace_i == -1)
			fus_confs.Add(conf);
		else
			fus_confs[replace_i] = conf;
		SortFusions();
	}
	
	int graph_limit = 10000;
	if (fus_results.GetCount() > graph_limit)
		fus_results.Remove(0, fus_results.GetCount() - graph_limit);
	fus_results.Add(conf.test0_equity);
	
	pop_counter++;
	
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

void ExpertSystem::StoreThis() {
	rt_lock.Enter();
	
	Time t = GetSysTime();
	String file = ConfigFile("expertsystem.bin");
	bool rem_bak = false;

	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}

	StoreToFile(*this,	file);

	if (rem_bak)
		DeleteFile(file + ".bak");

	last_store.Reset();
	
	rt_lock.Leave();
}

void ExpertSystem::LoadThis() {
	if (FileExists(ConfigFile("expertsystem.bin.bak")))
		LoadFromFile(*this,	ConfigFile("expertsystem.bin.bak"));
	else
		LoadFromFile(*this,	ConfigFile("expertsystem.bin"));
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

void ExpertSystem::MainTraining() {
	System& sys = GetSystem();
	
	while (training_running && is_training && !Thread::IsShutdownThreads()) {
		sys_lock.EnterWrite();
		int data_count = sys.GetCountMain();
		sys.ProcessWorkQueue();
		sys.ProcessLabelQueue();
		sys.ProcessDataBridgeQueue();
		full_mask.SetCount(data_count).One();
		sys_lock.LeaveWrite();
		
		
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		
		TimeStop ts_break;
		int ts_break_interval = 10 * (60 * 1000); // 10 mins
		
		while (training_running && is_training && !Thread::IsShutdownThreads()) {
			
			if (ts_break.Elapsed() >= ts_break_interval) {
				break;
			}
			
			// If enough advisor optimizing, optimize fusions
			for(int i = 0; i < fusion.GetPopulationSize() && training_running && !Thread::IsShutdownThreads(); i++) {
				co & [=] {
					if (!training_running || Thread::IsShutdownThreads()) return;
					
					ExpertFusion& fus = GetExpertCache().GetExpertFusion();
					
					
					// Load configuration
					fusion.EvolveFusion(fus.conf);
					fus.Configure();
					
					
					// Refresh advisor
					ForestArea area;
					area.FillArea(fusion.level, data_count);
					sys_lock.EnterRead();
					fus.Refresh(area);
					sys_lock.LeaveRead();
					
					// Return test result
					fusion.AddTestResult(fus.conf);
					
					fus.Leave();
				};
			}
			
			co.Finish();
			
			fusion.ReduceMemory();
			
			// Update results in controlled way when level changes
			int level = fusion.pop_counter / OPT_PHASE_ITERS;
			if (level > fusion.level) {
				fusion.level = level;
				
				for(int i = 0; i < fusion.fus_confs.GetCount(); i++) {
					co & [=] {
						ExpertFusion& fus = GetExpertCache().GetExpertFusion();
						fus.conf = fusion.fus_confs[i];
						fus.Configure();
						ForestArea area;
						area.FillArea(fusion.level, data_count);
						sys_lock.EnterRead();
						fus.Refresh(area);
						sys_lock.LeaveRead();
						fusion.fus_confs[i] = fus.conf;
						fus.Leave();
					};
				}
				co.Finish();
				fusion.SortFusions();
				
				StoreThis();
			}
		}
		
		StoreThis();
	}
	
	training_stopped = true;
}

void ExpertSystem::UpdateRealTimeConfs() {
	
	// Safely copy confs from optimizer
	rt_lock.Enter();
	fusion.lock.Enter();
	rt_confs <<= fusion.fus_confs;
	fusion.lock.Leave();
	rt_lock.Leave();
	
	
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	full_mask.SetCount(data_count).One();
	
	int limit = 30;
	if (rt_confs.GetCount() > limit)
		rt_confs.Remove(limit, rt_confs.GetCount() - limit);
	
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());
	for(int i = 0; i < rt_confs.GetCount() && running; i++) {
		co & [=] {
			ExpertFusion& fus = GetExpertCache().GetExpertFusion();
			fus.conf = rt_confs[i];
			fus.Configure();
			ForestArea area;
			area.train_begin	= 24*60;
			area.train_end		= data_count * 2/3;
			area.test0_begin	= area.train_end;
			area.test0_end		= data_count - 1*5*24*60;
			area.test1_begin	= area.test0_end;
			area.test1_end		= data_count;
			sys_lock.EnterRead();
			fus.Refresh(area);
			sys_lock.LeaveRead();
			rt_confs[i] = fus.conf;
			fus.Leave();
		};
	}
	co.Finish();
	
	Sort(rt_confs);
	
	last_update = GetSysTime();
	
	StoreThis();
}

void ExpertSystem::MainReal() {
	Time now				= GetSysTime();
	int wday				= DayOfWeek(now);
	Time after_hour			= now + 60 * 60;
	int wday_after_hour		= DayOfWeek(after_hour);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	
	if (rt_confs.IsEmpty() || last_update < (now - 4*60*60) || forced_update) {
		UpdateRealTimeConfs();
		if (rt_confs.IsEmpty())
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
	full_mask.SetCount(data_count).One();
	sys_lock.LeaveWrite();
	
	
	WhenInfo("Updating ExpertFusion");
	ExpertFusion fus;
	fus.conf = rt_confs[0];
	fus.Configure();
	ForestArea area;
	int week			= 1*5*24*60;
	area.train_begin	= 24*60;
	area.train_end		= data_count - 2 * week;
	area.train_end		-= area.train_end % week;
	area.test0_begin	= area.train_end;
	area.test0_end		= area.test0_begin + week;
	area.test1_begin	= area.test0_end;
	area.test1_end		= data_count;
	sys_lock.EnterRead();
	fus.Refresh(area);
	sys_lock.LeaveRead();
	
	
	
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
			int sig = fus.broker.GetSignal(i);
	
			if (sig == mt.GetSignal(sym) && sig != 0)
				mt.SetSignalFreeze(sym, true);
			else {
				mt.SetSignal(sym, sig);
				mt.SetSignalFreeze(sym, false);
			}
		}
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale((AMP_MAXSCALES - 1)*AMP_MAXSCALE_MUL * SYM_COUNT);
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
