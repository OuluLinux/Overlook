#include "Overlook.h"

namespace Overlook {

void System::Start() {
	Stop();
	running = true;
	stopped = false;
	
	Thread::Start(THISBACK(MainLoop));
	
	nonstopped_workers = CPU_Cores();
	for(int i = 0; i < nonstopped_workers; i++)
		Thread::Start(THISBACK1(Worker, i));
}

void System::Stop() {
	running = false;
	while (!stopped && nonstopped_workers != 0) Sleep(100);
}

void System::MainLoop() {
	
	while (running) {
		
		// Create new combinations for threads to evaluate
		RefreshPipeline();
		
		// Refresh current real-time combination
		RefreshRealtime();
		
		Sleep(100);
	}
	
	stopped = true;
}

void System::Worker(int id) {
	
	while (running) {
		pl_queue_lock.Enter();
		One<PipelineItem> pi = pl_queue.Detach(0);
		pl_queue_lock.Leave();
		
		
		// Get core-item queue from pipeline-item
		Vector<Ptr<CoreItem> > ci_queue;
		//GetCoreQueue(*pi, ci_queue);
		Panic("TODO");
		
		
		// Process job-queue
		for(int i = 0; i < ci_queue.GetCount(); i++) {
			Process(*ci_queue[i]);
		}
		
		
		Sleep(100);
	}
	
	nonstopped_workers--;
}

Core* System::CreateSingle(int factory, int sym, int tf) {
	
	// Enable column
	Vector<int> path;
	path.Add(1000 + factory);
	PipelineItem pi;
	pi.value.SetCount(table.GetRowBytes(), 0);
	int col = GetEnabledColumn(path);
	if (col != -1)
		table.Set0(col, true, pi.value);
	
	// Enable symbol
	ASSERT(sym >= 0 && sym < symbols.GetCount());
	Index<int> sym_ids;
	sym_ids.Add(sym);
	
	// Enable timeframe
	ASSERT(tf >= 0 && tf < periods.GetCount());
	
	// Get working queue
	Vector<Ptr<CoreItem> > ci_queue;
	GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
	
	// Process job-queue
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		Process(*ci_queue[i]);
	}
	
	return &*ci_queue.Top()->core;
}

void System::Process(CoreItem& ci) {
	
	// Load dependencies to the scope
	if (!ci.core)
		CreateCore(ci);
	
	// Process core-object
	ci.core->Refresh();
	
	// Store cache file
	ci.core->StoreCache();
	
}

void System::RefreshRealtime() {
	
	
	
}

}
