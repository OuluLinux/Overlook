#include "Overlook.h"

namespace Overlook {

void System::Start() {
	Stop();
	running = true;
	stopped = false;
	
	Thread::Start(THISBACK(MainLoop));
	/*
	nonstopped_workers = 1;//CPU_Cores();
	SetBasketCount(nonstopped_workers);
	for(int i = 0; i < nonstopped_workers; i++)
		Thread::Start(THISBACK1(Worker, i));*/
}

void System::Stop() {
	running = false;
	while (!stopped && nonstopped_workers != 0) Sleep(100);
}

void System::MainLoop() {
	
	while (running) {
		
		// Refresh current real-time combination
		RefreshRealtime();
		
		Sleep(100);
	}
	
	stopped = true;
}

void System::Worker(int id) {
	const int tf_count = GetPeriodCount();
	
	int counter = 0;
	
	while (running) {
		/*if (!pl_queue.IsEmpty()) {
			
			// Get pipeline to process
			pl_queue_lock.Enter();
			One<PipelineItem> pi = pl_queue.Detach(0);
			pl_queue_lock.Leave();
			
			LOG("Worker " << id << " starting to process a pipeline");
			
			int best_tf;
			int best_result;
			
			for (int tf = tf_count-1; tf >= 0; tf--) {
				Index<int> tfs;
				tfs.Add(tf);
				
				// Get core-item queue from pipeline-item
				Vector<Ptr<CoreItem> > ci_queue;
				GetCoreQueue(*pi, ci_queue, &tfs, id);
				
				// Process job-queue
				for(int i = 0; i < ci_queue.GetCount() && running; i++) {
					Process(*ci_queue[i]);
				}
				
				// Break if result is too bad;
				best_tf = tf;
				best_result = counter++;
				break;
			}
			
			// Return result to the query table
			table.lock.Enter();
			int row = table.GetCount();
			table.SetCount(row+1);
			table.Set(row, 0, best_result);
			table.Set(row, 1, best_tf);
			table.lock.Leave();
		}*/
		
		Sleep(100);
	}
	
	nonstopped_workers--;
}

Core* System::CreateSingle(int factory, int sym, int tf) {
	
	// Enable factory
	Vector<int> path;
	path.Add(factory);
	
	// Enable symbol
	ASSERT(sym >= 0 && sym < symbols.GetCount());
	Index<int> sym_ids;
	sym_ids.Add(sym);
	
	// Enable timeframe
	ASSERT(tf >= 0 && tf < periods.GetCount());
	
	// Get working queue
	Vector<Ptr<CoreItem> > ci_queue;
	GetCoreQueue(path, ci_queue, tf, sym_ids);
	
	// Process job-queue
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		WhenProgress(i, ci_queue.GetCount());
		Process(*ci_queue[i]);
	}
	
	return &*ci_queue.Top()->core;
}

void System::Process(CoreItem& ci) {
	
	// Load dependencies to the scope
	if (ci.core.IsEmpty())
		CreateCore(ci);
	
	// Process core-object
	ci.core->Refresh();
	
	// Store cache file
	ci.core->StoreCache();
	
}

void System::RefreshRealtime() {
	
	
	
}

}
