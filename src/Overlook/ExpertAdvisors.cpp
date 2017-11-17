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
	
	SetBufferColor(0, Color(28, 255, 150));
	SetBufferColor(1, Color(28, 127, 255));
	SetBufferColor(2, Color(255, 94, 78));
	SetBufferLineWidth(0, 3);
	SetBufferLineWidth(1, 2);
	SetBufferLineWidth(2, 2);
	for(int i = 0; i < LOCALPROB_DEPTH; i++) {
		Color clr = GrayColor(i * 100 / LOCALPROB_DEPTH);
		SetBufferColor(main_graphs + i, clr);
		SetBufferColor(main_graphs + LOCALPROB_DEPTH + i, clr);
	}
	
	rf_trainer.options.tree_count	= 100;
	rf_trainer.options.max_depth	= 4;
	rf_trainer.options.tries_count	= 10;
	
}

void RandomForestAdvisor::Start() {
	LOG("RandomForestAdvisor::Start");
	
	/*
	if (once) {
		ForceSetCounted(0);
		RefreshOutputBuffers();
		once = false;
	}*/
	
	if (!running) {
		int bars = GetBars();
		Output& out = GetOutput(0);
		for(int i = 0; i < out.buffers.GetCount(); i++)
			out.buffers[i].SetCount(bars);
		
		if (phase == RF_IDLE || phase == RF_TRAINING) {
			phase = RF_TRAINING;
			running = true;
			Thread::Start(THISBACK(Training));
		}
		
		else if (phase == RF_OPTIMIZING) {
			running = true;
			Thread::Start(THISBACK(Optimizing));
		}
		
		
		// AccountAdvisor switches from RF_IDLEREAL to RF_TRAINREAL
		
		else if (phase == RF_TRAINREAL) {
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
	
	SetSafetyLimit(bars);
	
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
			
			Buffer& buf = GetBuffer(main_graphs + p * LOCALPROB_DEPTH + i);
			
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
	
	TrainRF();
	
	serializer_lock.Leave(); // leave before StoreCache
	
	if (!Thread::IsShutdownThreads()) {
		ForceSetCounted(0);
		RefreshOutputBuffers();
		phase = RF_OPTIMIZING;
		StoreCache();
	}
	
	running = false;
}

void RandomForestAdvisor::TrainRF() {
	System& sys = GetSystem();
	
	int tf = GetTf();
	int symbol = GetSymbol();
	int data_count = sys.GetCountTf(tf);
	
	VectorBool full_mask;
	full_mask.SetCount(data_count).One();
	
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
}

void RandomForestAdvisor::Optimizing() {
	running = true;
	serializer_lock.Enter();
	
	
	// Remove invalid sources
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
	
	
	// Init genetic optimizer
	int cols = (rflist_pos.GetCount() + rflist_neg.GetCount()) * 2 + 2;
	if (optimizer.GetRound() == 0) {
		optimizer.SetArrayCount(1);
		optimizer.SetCount(cols);
		optimizer.SetPopulation(1000);
		optimizer.SetMaxGenerations(100);
		optimizer.UseLimits();
		
		
		// Set optimizer column value ranges
		int col = 0;
		for(int i = 0; i < rflist_pos.GetCount(); i++) {
			optimizer.Set(col++,  0.0, 1.0, 0.01, "pos mul");
			optimizer.Set(col++, -1.0, 1.0, 0.01, "pos off");
		}
		for(int i = 0; i < rflist_neg.GetCount(); i++) {
			optimizer.Set(col++,  0.0, 1.0, 0.01, "neg mul");
			optimizer.Set(col++, -1.0, 1.0, 0.01, "neg off");
		}
		optimizer.Set(col++, 0.0, 1.0, 0.01, "trigger limit");
		optimizer.Set(col++, 0.0, 1.0, 0.01, "+ - balance");
		ASSERT(col == cols);
		optimizer.Init(StrategyRandom2Bin);
	}
	int col = 0;
	pos_weight_i.Clear(); neg_weight_i.Clear();
	for(int i = 0; i < rflist_pos.GetCount(); i++) {pos_weight_i.Add(col); col += 2;}
	for(int i = 0; i < rflist_neg.GetCount(); i++) {neg_weight_i.Add(col); col += 2;}
	
	
	
	// Optimize
	while (!optimizer.IsEnd() && !Thread::IsShutdownThreads()) {
		
		// Get weights
		optimizer.Start();
		optimizer.GetLimitedTrialSolution(trial);
		
		RunMain();
		
		// Return training value with less than 10% of testing value.
		// Testing value should slightly direct away from weird locality...
		double change_total = (area_change_total[0] + area_change_total[1] * 0.1) / 1.1;
		LOG(GetSymbol() << " round " << optimizer.GetRound()
			<< ": tr=" << area_change_total[0]
			<<  " t0=" << area_change_total[1]
			<<  " t1=" << area_change_total[2]);
		optimizer.Stop(change_total);
	}
	
	serializer_lock.Leave(); // leave before StoreCache
	
	if (optimizer.IsEnd()) {
		optimizer.GetLimitedBestSolution(trial);
		
		RunMain();
		
		phase = RF_IDLEREAL;
		StoreCache();
	}
	
	running = false;
}

void RandomForestAdvisor::RunMain() {
	
	// Prepare variables
	ConstBuffer& open_buf = GetInputBuffer(0, 0);
	int symbol = GetSymbol();
	int tf = GetTf();
	int data_count = GetBuffer(main_graphs).GetCount();
	ASSERT(data_count > 0);
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, symbol, tf));
	double spread_point = db->GetPoint();
	ASSERT(spread_point > 0.0);
	ForestArea area;
	area.FillArea(data_count);
	Buffer& pos_buf = GetBuffer(1);
	Buffer& neg_buf = GetBuffer(2);
	
	SetSafetyLimit(data_count);
	
	
	for (int p = 0; p < 2; p++) {
		const Vector<int>& weight_i = p == 0 ? pos_weight_i : neg_weight_i;
		
		
		Buffer& dst = p == 0 ? pos_buf : neg_buf;
		dst.SetCount(data_count);
		for(int i = 0; i < dst.GetCount(); i++)
			dst.Set(i, 0.0);
		
		
		// Normalize weights, so that sum is 1.0
		double sum = 0.0;
		for(int i = 0; i < weight_i.GetCount(); i++)
			sum += trial[weight_i[i]];
		double mul = 1.0 / sum;
		for (int i = 0; i < weight_i.GetCount(); i++)
			trial[weight_i[i]] *= mul;
		
		
		// Add values to sum with weight. Vector at once to get better optimization.
		for(int i = 0; i < weight_i.GetCount(); i++) {
			int col					= weight_i[i];
			double mul				= trial[col++];
			double off				= trial[col++];
			const Buffer& src		= GetBuffer(main_graphs + p * LOCALPROB_DEPTH + i);
			ASSERT(src.GetCount() == dst.GetCount());
			ConstDouble *src_cursor	= src.Begin(), *src_end = src.End();
			double      *dst_cursor	= dst.Begin(), *dst_end = dst.End();
			for (; dst_cursor != dst_end; dst_cursor++, src_cursor++)
				*dst_cursor += (*src_cursor + off) * mul;
		}
	}
	
	int col = trial.GetCount() - 2;
	double trigger_limit = trial[col++];
	double posneg_balance = trial[col++];
	VectorBool& signal_label  = GetOutput(0).label;
	VectorBool& enabled_label = GetOutput(1).label;
	signal_label.SetCount(data_count);
	enabled_label.SetCount(data_count);
	
	for (int a = 0; a < 3; a++) {
		int begin = a == 0 ? area.train_begin : (a == 1 ? area.test0_begin : area.test1_begin);
		int end   = a == 0 ? area.train_end   : (a == 1 ? area.test0_end   : area.test1_end);
		
		// Test with training data and with testing data
		bool is_prev_active		= false;
		bool prev_signal		= false;
		int prev_pos			= 0;
		double open				= open_buf.GetUnsafe(begin);
		double hour_total		= 0.0;
		double change_total		= 1.0;
		
		
		for(int i = begin; i < end; i++) {
			double pos = pos_buf.Get(i);
			double neg = neg_buf.Get(i);
			
			
			bool is_pos = pos >= trigger_limit;
			bool is_neg = neg >= trigger_limit;
			
			
			bool is_active, signal;
			if (is_pos && is_neg) {
				is_active			= true;
				double pos_factor	= pos * (1.0 - posneg_balance);
				double neg_factor	= neg *        posneg_balance;
				signal				= neg_factor > pos_factor;
			}
			else if (is_pos) {
				is_active			= true;
				signal				= false;
			}
			else if (is_neg) {
				is_active			= true;
				signal				= true;
			}
			else {
				is_active			= false;
			}
			
			signal_label.Set(i, signal);
			enabled_label.Set(i, is_active);
			
			bool signal_activate	= !is_active &&  is_prev_active;
			bool signal_deactivate	=  is_active && !is_prev_active;
			bool signal_switch		=  is_active &&  is_prev_active && prev_signal != signal;
			
			
			if (signal_activate || signal_switch) {
				double change;
				double cur			 = open_buf.GetUnsafe(i);
				int len				 = i - prev_pos;
				double hours		 = (double)len / 60.0;
				hour_total			+= hours;
				if (!prev_signal)
					change			 = cur / (open + spread_point);
				else
					change			 = open / (cur + spread_point);
				change_total		*= change;
				prev_pos			 = i;
			}
			
		
			if (signal_deactivate || signal_switch) {
				open				= open_buf.GetUnsafe(i);
				prev_pos			= i;
			}
			
			
			is_prev_active			= is_active;
			prev_signal				= signal;
		}
		
		change_total -= 1.0;
		
		area_change_total[a] = change_total;
	}

}

void RandomForestAdvisor::TrainReal() {
	running = true;
	
	serializer_lock.Enter();
	
	ConstBuffer& open_buf	= GetInputBuffer(0, 0);
	int data_count			= open_buf.GetCount();
	area.train_begin		= 1*5*24*60 / MAIN_PERIOD_MINUTES;
	area.train_end			= data_count - 2*5*24*60 / MAIN_PERIOD_MINUTES;
	area.test0_begin		= area.train_end;
	area.test0_end			= data_count;
	area.test1_begin		= data_count - 1*5*24*60 / MAIN_PERIOD_MINUTES;
	area.test1_end			= data_count;
	
	TrainRF();
	
	serializer_lock.Leave(); // leave before StoreCache
	
	if (!Thread::IsShutdownThreads()) {
		ForceSetCounted(0);
		RefreshOutputBuffers();
		phase = RF_REAL;
		StoreCache();
	}
	
	running = false;
}

}
