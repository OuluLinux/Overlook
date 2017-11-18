#include "Overlook.h"


namespace Overlook {
using namespace Upp;


WeekSlotAdvisor::WeekSlotAdvisor() {
	
}

void WeekSlotAdvisor::Init() {
	SetCoreSeparateWindow();
	
}

void WeekSlotAdvisor::Start() {
	
	return;
	
	if (!running) {
		int bars = GetBars();
		Output& out = GetOutput(0);
		for(int i = 0; i < out.buffers.GetCount(); i++)
			out.buffers[i].SetCount(bars);
		
		if (phase == RF_IDLE || phase == RF_OPTIMIZING) {
			running = true;
			Thread::Start(THISBACK(Optimizing));
		}
		
		
		// waits until all EAs has trained real
		
		else if (phase == RF_TRAINREAL) {
			/*phase = RF_TRAINREAL;
			running = true;
			Thread::Start(THISBACK(RealTraining));*/
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

void WeekSlotAdvisor::RefreshOutputBuffers() {
	/*int counted = GetCounted();
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
	}*/
}

void WeekSlotAdvisor::Optimizing() {
	running = true;
	serializer_lock.Enter();
	
	
	// Init genetic optimizer
	int cols = 0;//(rflist_pos.GetCount() + rflist_neg.GetCount()) * 2 + 2;
	if (optimizer.GetRound() == 0) {
		optimizer.SetArrayCount(1);
		optimizer.SetCount(cols);
		optimizer.SetPopulation(1000);
		optimizer.SetMaxGenerations(100);
		optimizer.UseLimits();
		
		
		// Set optimizer column value ranges
		int col = 0;
		/*for(int i = 0; i < rflist_pos.GetCount(); i++) {
			optimizer.Set(col++,  0.0, 1.0, 0.01, "pos mul");
			optimizer.Set(col++, -1.0, 1.0, 0.01, "pos off");
		}
		for(int i = 0; i < rflist_neg.GetCount(); i++) {
			optimizer.Set(col++,  0.0, 1.0, 0.01, "neg mul");
			optimizer.Set(col++, -1.0, 1.0, 0.01, "neg off");
		}*/
		optimizer.Set(col++, 0.0, 1.0, 0.01, "trigger limit");
		optimizer.Set(col++, 0.0, 1.0, 0.01, "+ - balance");
		ASSERT(col == cols);
		optimizer.Init(StrategyRandom2Bin);
	}
	
	
	// Optimize
	while (!optimizer.IsEnd() && !Thread::IsShutdownThreads()) {
		
		// Get weights
		optimizer.Start();
		optimizer.GetLimitedTrialSolution(trial);
		
		RunMain();
		
		// Return training value with less than 10% of testing value.
		// Testing value should slightly direct away from weird locality...
		double change_total = 0;/*(area_change_total[0] + area_change_total[1] * 0.1) / 1.1;
		LOG(GetSymbol() << " round " << optimizer.GetRound()
			<< ": tr=" << area_change_total[0]
			<<  " t0=" << area_change_total[1]
			<<  " t1=" << area_change_total[2]);*/
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

void WeekSlotAdvisor::RunMain() {
	
	
	
	
	
}

}
