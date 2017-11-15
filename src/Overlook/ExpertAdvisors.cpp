#include "Overlook.h"


namespace Overlook {
using namespace Upp;





const int label_count = LABELINDI_COUNT;
const int sector_pred_dir_count = 2;
const int sector_label_count = 4;
const int period_begin = 3;
const int fastinput_count = 3;
const int labelpattern_count = 5;
int sector_type_count;

int GetConfCount() {
	if (!sector_type_count) {
		for(int i = period_begin; i < TF_COUNT; i++)
			sector_type_count += (TF_COUNT - i);
	}
	return label_count * sector_pred_dir_count * sector_label_count * fastinput_count * labelpattern_count * sector_type_count;
}

void GetConf(int i, AccuracyConf& conf) {
	int count = GetConfCount();
	ASSERT(i >= 0 && i < count);
	
	conf.id = i;
	
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
















RandomForestAdvisor::RandomForestAdvisor() {
	
}

void RandomForestAdvisor::Init() {
	SetCoreSeparateWindow();
	
	// Last or first? The sum is bolder
	
	rf_trainer.options.tree_count	= 100;
	rf_trainer.options.max_depth	= 4;
	rf_trainer.options.tries_count	= 10;
	
	
	
	for(int i = 0; i < TF_COUNT; i++) {
		int period = (1 << (1 + i));
		AddSubCore<VolatilityAverage>()	.Set("period", period);
		AddSubCore<OsMA>().Set("fast_ema_period", period).Set("slow_ema_period", period*2).Set("signal_sma", period);
		AddSubCore<StochasticOscillator>().Set("k_period", period);
		
		AddSubCore<ZigZag>().Set("depth", period).Set("deviation", period).Set("backstep", Upp::max(1, period/2));
		AddSubCore<MovingAverage>().Set("period", period).Set("offset", -period/2);
		AddSubCore<Momentum>().Set("period", period).Set("shift", -period/2);
	}
	
	AddSubCore<LinearWeekTime>();
	
}

void RandomForestAdvisor::Start() {
	LOG("RandomForestAdvisor::Start");
	
	
	if (phase == RF_IDLE) {
		phase = RF_TRAINING;
		Thread::Start(THISBACK(Optimize));
	}
	
	if (phase == RF_IDLEREAL) {
		phase = RF_TRAINREAL;
		Thread::Start(THISBACK(TrainReal));
	}
}

void RandomForestAdvisor::FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs) {
	int depth = TRUEINDI_COUNT * (conf.fastinput + 1) + 1;
	bufs.SetDepth(depth);
	ASSERT(depth >= 2);
	
	
	int k = 0;
	int symbol = GetSymbol();
	int tf = conf.GetBaseTf(0);
	
	ASSERT(TRUEINDI_COUNT == indi_count);
	for(int i = 0; i < TRUEINDI_COUNT; i++) {
		for(int j = 0; j < conf.fastinput+1; j++) {
			int l = tf-j;
			ASSERT(l >= 0 && l < TF_COUNT);
			ConstBuffer& buf = At(i + l * (indi_count + label_count)).GetOutput(0).buffers[0];
			ASSERT(&buf != NULL);
			bufs.SetSource(k++, buf);
		}
	}
	bufs.SetSource(k++, At(TF_COUNT * (indi_count + label_count)).GetOutput(0).buffers[0]);
	
	
	ASSERT(k == depth);
}

void RandomForestAdvisor::Training() {
	System& sys = GetSystem();
	
	// Prepare training functions and output variables
	rflist_pos.SetCount(LOCALPROB_DEPTH);
	rflist_neg.SetCount(LOCALPROB_DEPTH);
	
	
	int symbol = GetSymbol();
	int count = GetConfCount();
	int tf = GetTf();
	int data_count = sys.GetCountTf(tf);
	ForestArea area;
	area.FillArea(data_count);
	
	One<RF> training_rf;
	training_rf.Create();
	
	VectorBool full_mask;
	full_mask.SetCount(data_count).One();
	
	
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, symbol, tf));
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	double spread_point = db->GetPoint();
	ASSERT(spread_point > 0.0);
	
	
	// Iterate trough configuration combinations, and keep the best confs
		
	for(; opt_counter < count && !Thread::IsShutdownThreads(); opt_counter++) {
		
		// Use common trainer for performance reasons
		RF& rf = *training_rf;
		
		
		// Get configuration
		AccuracyConf& conf = rf.a;
		GetConf(opt_counter, conf);
		VectorBool& real_mask = rf.c;
		real_mask.SetCount(data_count).One();
		
		
		// Skip duplicate combinations
		// This is the easiest way... the harder way would be to not count duplicates.
		if (conf.ext == 1) {
			if (conf.label >= 2 || conf.labelpattern > 0 || conf.ext_dir)
				continue;
		}
		
		
		// Refresh masks
		ConstBufferSource bufs;
		
		
		
		// Get active label
		uint64 active_label_pattern = 0;
		switch (conf.label) {
			case 0: active_label_pattern = 0; break;
			case 1: active_label_pattern = ~(0ULL); break;
			case 2: for (int i = 1; i < 64; i += 2) active_label_pattern |= 1 << i; break;
			case 3: for (int i = 0; i < 64; i += 2) active_label_pattern |= 1 << i; break;
		}
		bool active_label = active_label_pattern & 1;
		Array<RF>& rflist = active_label == false ? rflist_pos : rflist_neg;
		
		
		// Get label indicator. Do AND operation for all layers.
		int layer_count = conf.ext;
		ASSERT(layer_count > 0);
		for(int sid = 0; sid < layer_count; sid++) {
			int tf = conf.GetBaseTf(sid);
			bool sid_active_label = active_label_pattern & (1 << (layer_count - 1 - sid));
			
			// Get label id
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
			
			
			// Get filter label
			ConstVectorBool& src_label = At(indi_count + label_id + tf * (indi_count + label_count)).GetOutput(0).label;
			int count0 = real_mask.GetCount();
			int count1 = src_label.GetCount();
			if (sid_active_label)	real_mask.And(src_label);
			else					real_mask.InverseAnd(src_label);
		}
		
		
		// Test
		bool mask_prev_active		= false;
		int mask_prev_pos			= 0;
		double mask_open			= open_buf.GetUnsafe(area.train_begin);
		double mask_hour_total		= 0.0;
		double mask_change_total	= 1.0;
		double mult_change_total	= 1.0;
		for(int i = area.train_begin; i < area.train_end; i++) {
			bool is_mask =  real_mask.Get(i);
			
			if (is_mask) {
				if (!mask_prev_active) {
					mask_open			= open_buf.GetUnsafe(i);
					mask_prev_active	= true;
					mask_prev_pos		= i;
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
		}
		mask_change_total -= 1.0;
		conf.test_valuefactor = mask_change_total;
		conf.test_valuehourfactor = mask_hour_total > 0.0 ? mask_change_total / mask_hour_total : 0.0;
		conf.test_hourtotal = mask_hour_total;
		conf.is_processed = true;
		
		
		if (conf.test_valuehourfactor > 0.0) {
			LOG(symbol << ": " << conf.test_valuehourfactor);
		}
		
		// Sort list
		rflist.Add(training_rf.Detach());
		Sort(rflist, RFSorter());
		training_rf.Attach(rflist.Detach(rflist.GetCount()-1));
		
	}
	
	
	ConstBufferSource bufs;
	
	for (int p = 0; p < 2; p++) {
		Array<RF>& rflist = p == 0 ? rflist_pos : rflist_neg;
		
		// Actually train random forests
		for(int i = 0; i < rflist.GetCount() && !Thread::IsShutdownThreads(); i++) {
			RF& rf = rflist[i];
			AccuracyConf& conf = rf.a;
			VectorBool& real_mask = rf.c;
			LOG(symbol << ": " << i << ": " << p << ": test_valuehourfactor: " << conf.test_valuehourfactor);
			
			
			rf_trainer.forest.memory.Attach(&rf.b);
			
			
			FillBufferSource(conf, bufs);
			rf_trainer.Process(area, bufs, real_mask, full_mask);
			
			conf.stat = rf_trainer.stat;
			LOG(symbol << ": " << i << ": " << p << ": train_accuracy: " << conf.stat.train_accuracy << " test_accuracy: " << conf.stat.test0_accuracy);
			
			rf_trainer.forest.memory.Detach();
		}
	}
	
	if (!Thread::IsShutdownThreads())
		phase = RF_OPTIMIZING;
}

void RandomForestAdvisor::Optimizing() {
	
	
	// Init genetic optimizer
	
	//while (!optimizer.IsEnd() && !Thread::IsShutdownThreads())
	{
		
		// Get weights
		
		
		// Normalize weights, so that sum is 1.0
		
		
		// Add values to sum with weight. Vector at once to get better optimization.
		
		
		// Test with training data and with testing data
		
		
		// Return training value with less than 10% of testing value.
		// Testing value should slightly direct away from weird locality...
		
		
		
		// Refresh the graph object in the indicator
	
	
	}
	
	
	if (optimizer.IsEnd())
		phase = RF_IDLEREAL;
}

void RandomForestAdvisor::Optimize() {
	System& sys = GetSystem();
	
	
	if (phase == RF_TRAINING)
		Training();
	
	if (phase == RF_OPTIMIZING)
		Optimizing();
	
	if (!Thread::IsShutdownThreads())
		StoreCache();
}

void RandomForestAdvisor::TrainReal() {
	
}

}
