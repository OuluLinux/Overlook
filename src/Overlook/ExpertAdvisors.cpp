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

void FillBufferSource(const AccuracyConf& conf, ConstBufferSource& bufs, int rel_tf) {
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









RandomForestAdvisor::RandomForestAdvisor() {
	
}

void RandomForestAdvisor::Init() {
	SetCoreSeparateWindow();
	
	// Last or first? The sum is bolder
	
}

void RandomForestAdvisor::Start() {
	LOG("RandomForestAdvisor::Start");
	
	
	if (phase == RF_IDLE) {
		phase = RF_OPTIMIZING;
		Thread::Start(THISBACK(Optimize));
	}
	
	if (phase == RF_IDLEREAL) {
		phase = RF_TRAINREAL;
		Thread::Start(THISBACK(TrainReal));
	}
}

void RandomForestAdvisor::Optimize() {
	#if 0
	rflist_pos.SetCount(LOCALPROB_DEPTH);
	rflist_neg.SetCount(LOCALPROB_DEPTH);
	
	
	int count = GetConfCount();
	
	
	System& sys = GetSystem();
	int data_count = sys.GetCountMain();
	ForestArea area;
	area.FillArea(data_count);
	
	One<RF> training_rf;
	training_rf.Create();
	
		
	for(; opt_counter < count && !Thread::IsShutdownThreads(); opt_counter++) {
		
		RF& rf = *training_rf;
		
		AccuracyConf& conf = rf.a;
		
		// Skip duplicate combinations
		// This is the easiest way... the hard way is really hard.
		if (conf.ext == 1) {
			if (conf.label >= 2 || conf.labelpattern > 0 || conf.ext_dir) continue;
		}
		
		
		// Get configuration
		GetConf(opt_counter, conf);
		
		ASSERT(conf.ext > 0);
		ConstBufferSource bufs;
		VectorBool& real_mask = proc.real_mask;
		real_mask.SetCount(data_count).One();
		
		
		// Get active label
		uint64 active_label_pattern = 0;
		switch (conf.label) {
			case 0: active_label_pattern = 0; break;
			case 1: active_label_pattern = ~(0ULL); break;
			case 2: for(int i = 1; i < 64; i+=2) active_label_pattern |= 1 << i; break;
			case 3: for(int i = 0; i < 64; i+=2) active_label_pattern |= 1 << i; break;
		}
		bool active_label = active_label_pattern & 1;
		Array<RF>& rflist = active_label == false ? rflist_pos : rflist_neg;
		
		
		for(int sid = 0; sid < conf.ext; sid++) {
			int tf = conf.GetBaseTf(sid);
			ASSERT(tf - conf.fastinput >= 0);
			bool sid_active_label = active_label_pattern & (1 << (conf.ext - 1 - sid));
			
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
			ConstVectorBool& src_label = sys.GetLabelIndicator(conf.symbol, tf, label_id) ;
			
			if (sid_active_label)	real_mask.And(src_label);
			else					real_mask.InverseAnd(src_label);
			
		}
		
		
		// Train
		FillBufferSource(conf, bufs, 0);
		rf_trainer.Process(area, bufs, real_label, real_mask);
		
		
		// Test
		bool mask_prev_active = false;
		int mask_prev_pos = 0;
		ConstBuffer& open_buf = sys.GetTradingSymbolOpenBuffer(conf.symbol);
		double spread_point = sys.GetTradingSymbolSpreadPoint(conf.symbol);
		double mask_open = open_buf.GetUnsafe(area.test0_begin);
		double mask_hour_total = 0.0, mask_change_total = 1.0;
		double mult_change_total = 1.0;
		for(int i = area.test0_begin; i < area.test0_end; i++) {
			bool is_mask = conf.pred_mask.Get(i);
			
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
		}
		mask_change_total -= 1.0;
		conf.test_valuefactor = mask_change_total;
		conf.test_valuehourfactor = mask_hour_total > 0.0 ? mask_change_total / mask_hour_total : 0.0;
		conf.test_hourtotal = mask_hour_total;
		
		conf.is_processed = true;
		
		
		rf.c = conf.test_valuefactor;
		
		
		// Sort list
		rflist.Add(training_rf.Detach());
		Sort(rflist);
		training_rf.Attach(rflist.Detach(rflist.GetCount()-1));
		
		
		// Refresh the graph object in the indicator
		
		
	}
	
	if (opt_counter >= count) {
		
		
		// Init genetic optimizer
		
		while (!optimizer.IsEnd() && !Thread::IsShutdownThreads()) {
			
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
	#endif
}

void RandomForestAdvisor::TrainReal() {
	
}

}
