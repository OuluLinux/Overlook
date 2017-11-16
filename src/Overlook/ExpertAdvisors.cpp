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
	SetCoreMinimum(0.0);  // normalized
	SetCoreMaximum(1.0);   // normalized
	
	SetBufferColor(0, Green);
	SetBufferLineWidth(0, 3);
	for(int i = 0; i < LOCALPROB_DEPTH; i++) {
		Color clr = GrayColor(i * 100 / LOCALPROB_DEPTH);
		SetBufferColor(1 + i, clr);
		SetBufferColor(1 + LOCALPROB_DEPTH + i, clr);
	}
	
	rf_trainer.options.tree_count	= 100;
	rf_trainer.options.max_depth	= 4;
	rf_trainer.options.tries_count	= 10;
	
	
	ForceSetCounted(0);
}

void RandomForestAdvisor::Start() {
	LOG("RandomForestAdvisor::Start");
	
	
	if (!running) {
		int bars = GetBars();
		Output& out = GetOutput(0);
		for(int i = 0; i < out.buffers.GetCount(); i++)
			out.buffers[i].SetCount(bars);
		
		if (phase == RF_IDLE || phase == RF_TRAINING) {
			ForceSetCounted(0);
			phase = RF_TRAINING;
			running = true;
			Thread::Start(THISBACK(Training));
		}
		
		else if (phase == RF_OPTIMIZING) {
			return;
			
			ForceSetCounted(0);
			running = true;
			Thread::Start(THISBACK(Optimizing));
		}
		
		else if (phase == RF_IDLEREAL || phase == RF_TRAINREAL) {
			phase = RF_TRAINREAL;
			running = true;
			Thread::Start(THISBACK(TrainReal));
		}
	}
	
	if (!running) {
		int counted = GetCounted();
		int bars = GetBars();
		if (counted < bars) {
			RefreshOutputBuffers();
		}
	}
}

void RandomForestAdvisor::RefreshOutputBuffers() {
	int counted = GetCounted();
	int bars = GetBars();
	
	VectorBool full_mask;
	full_mask.SetCount(bars).One();
	
	ConstBufferSource bufs;
	
	for (int p = 0; p < 2; p++) {
		Array<RF>& rflist = p == 0 ? rflist_pos : rflist_neg;
		
		for(int i = 0; i < rflist.GetCount(); i++) {
			RF& rf = rflist[i];
			AccuracyConf& conf = rf.a;
			VectorBool& real_mask = rf.c;
			
			if (!conf.is_processed || conf.id == -1) continue;
			
			
			rf_trainer.forest.memory.Attach(&rf.b);
			FillBufferSource(conf, bufs);
			
			int cursor = counted;
			ConstBufferSourceIter iter(bufs, &cursor);
			
			Buffer& buf = GetBuffer(1 + p * LOCALPROB_DEPTH + i);
			
			for(; cursor < bars; cursor++) {
				SetSafetyLimit(cursor);
				double d = rf_trainer.forest.PredictOne(iter);
				if (d > 1.0) d = 1.0;
				if (d < 0.0) d = 0.0;
				buf.Set(cursor, d);
			}
			rf_trainer.forest.memory.Detach();
		}
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
			ConstBuffer& buf = GetInputBuffer(1 + i + l * (indi_count + label_count), 0);
			ASSERT(&buf != NULL);
			bufs.SetSource(k++, buf);
		}
	}
	bufs.SetSource(k++, GetInputBuffer(1 + TF_COUNT * (indi_count + label_count), 0));
	
	
	ASSERT(k == depth);
}

void RandomForestAdvisor::Training() {
	running = true;
	serializer_lock.Enter();
	
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
	for(int opt_counter = 0; opt_counter < count && !Thread::IsShutdownThreads(); opt_counter++) {
		
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
			ConstVectorBool& src_label = GetInputLabel(1 + indi_count + label_id + tf * (indi_count + label_count));
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
	
	serializer_lock.Leave(); // leave before StoreCache
	
	if (!Thread::IsShutdownThreads()) {
		RefreshOutputBuffers();
		phase = RF_OPTIMIZING;
		StoreCache();
	}
	
	running = false;
}



void RandomForestAdvisor::Optimizing() {
	running = true;
	serializer_lock.Enter();
	
	
	for(int i = 0; i < rflist_pos.GetCount(); i++) {
		AccuracyConf& conf = rflist_pos[i].a;
		if (!conf.is_processed || conf.id == -1)
			rflist_pos.Remove(i--);
	}
	
	for(int i = 0; i < rflist_neg.GetCount(); i++) {
		AccuracyConf& conf = rflist_neg[i].a;
		if (!conf.is_processed || conf.id == -1)
			rflist_neg.Remove(i--);
	}
	
	// - randomforest cache ei päivity jos se vektori on jo iso... jää välistä uudelleenkäytöst
	 
	// Init genetic optimizer
	int cols = (rflist_pos.GetCount() + rflist_neg.GetCount()) * 2 + 2;
	optimizer.SetArrayCount(1);
	optimizer.SetCount(cols);
	optimizer.SetPopulation(1000);
	optimizer.SetMaxGenerations(100);
	optimizer.UseLimits();
	
	
	Vector<int> weight_pos;
	
	int col = 0;
	for(int i = 0; i < rflist_pos.GetCount(); i++) {
		weight_pos.Add(col);
		optimizer.Set(col++,  0.0, 1.0, 0.01, "pos mul");
		optimizer.Set(col++, -1.0, 1.0, 0.01, "pos off");
	}
	for(int i = 0; i < rflist_neg.GetCount(); i++) {
		weight_pos.Add(col);
		optimizer.Set(col++,  0.0, 1.0, 0.01, "neg mul");
		optimizer.Set(col++, -1.0, 1.0, 0.01, "neg off");
	}
	optimizer.Set(col++, 0.0, 1.0, 0.01, "trigger limit");
	optimizer.Set(col++, 0.0, 1.0, 0.01, "+ - balance");
	ASSERT(col == cols);
	
	
	optimizer.Init();
	
	Vector<double> trial;
	Vector<Vector<double> > buffers;
	buffers.SetCount(rflist_pos.GetCount() + rflist_neg.GetCount());
	while (!optimizer.IsEnd() && !Thread::IsShutdownThreads()) {
		
		// Get weights
		optimizer.GetLimitedTrialSolution(trial);
		Panic("TODO");
		
		
		// Normalize weights, so that sum is 1.0
		double sum = 0.0;
		for(int i = 0; i < weight_pos.GetCount(); i++)
			sum += trial[weight_pos[i]];
		double mul = 1.0 / sum;
		for (int i = 0; i < weight_pos.GetCount(); i++)
			trial[weight_pos[i]] *= mul;
		
		
		// Add values to sum with weight. Vector at once to get better optimization.
		int col = 0;
		for(int i = 0; i < rflist_pos.GetCount(); i++) {
			double pos_mul = trial[col++];
			double pos_off = trial[col++];
			const Buffer& src = GetBuffer(1 + i);
			Vector<double>& dst = buffers[col];
			dst.SetCount(src.GetCount(), 0.0);
			for(int j = 0; j < src.GetCount(); j++)
				dst[j] = src.Get(j) * pos_mul + pos_off;
		}
		for(int i = 0; i < rflist_neg.GetCount(); i++) {
			double neg_mul = trial[col++];
			double neg_off = trial[col++];
			const Buffer& src = GetBuffer(1 + LOCALPROB_DEPTH + i);
			Vector<double>& dst = buffers[col];
			dst.SetCount(src.GetCount(), 0.0);
			for(int j = 0; j < src.GetCount(); j++)
				dst[j] = src.Get(j) * neg_mul + neg_off;
		}
		double trigger_limit = trial[col++];
		double posneg_balance = trial[col++];
		ASSERT(col == cols);
		
		
		// Test with training data and with testing data
		Panic("TODO");
		//simcore.LoadConfiguration(trial);
		
		
		// Return training value with less than 10% of testing value.
		// Testing value should slightly direct away from weird locality...
		
		
		
		// Refresh the graph object in the indicator
	
	
	}
	
	
	if (optimizer.IsEnd()) {
		RefreshOutputBuffers();
		phase = RF_IDLEREAL;
		StoreCache();
	}
	
	serializer_lock.Leave();
	running = false;
}

void RandomForestAdvisor::TrainReal() {
	running = true;
	
	
	
	
	
	running = false;
}

}
