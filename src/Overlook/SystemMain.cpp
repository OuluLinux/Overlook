#include "Overlook.h"

namespace Overlook {

void System::Start() {
	if (running) return;
	running = true;
	
	mgr.Start();
	
	busy_task = 0;
	nonstopped_workers = Upp::max(1, CPU_Cores() - 1);
	for(int i = 0; i < nonstopped_workers; i++)
		Thread::Start(THISBACK1(Worker, i));
	
	nonstopped_workers++;
	Thread::Start(THISBACK(Main));
}

void System::Stop() {
	mgr.Stop();
	running = false;
	while (nonstopped_workers > 0) Sleep(100);
}

void System::Main() {
	
	while (running) {
		mgr.Main();
		
		Sleep(1000);
	}
	
	nonstopped_workers--;
}

int System::AddTaskBusy(Callback task) {
	task_lock.Enter();
	ArrayMap<int, Callback>& busy_tasks = GetBusyTasklist<Callback>();
	Vector<One<Atomic> >& busy_running = GetBusyRunning<One<Atomic> >();
	int id = task_counter++;
	busy_tasks.Add(id, task);
	busy_running.Add(new Atomic(0));
	task_lock.Leave();
	return id;
}

void System::RemoveBusyTask(int main_id) {
	task_lock.Enter();
	ArrayMap<int, Callback>& busy_tasks = GetBusyTasklist<Callback>();
	Vector<One<Atomic> >& busy_running = GetBusyRunning<One<Atomic> >();
	int i = busy_tasks.Find(main_id);
	if (i != -1) {
		Atomic& task_running = *busy_running[i];
		task_running++;
		if (task_running > 1) {
			task_lock.Leave();
			while (task_running > 1)
				Sleep(1);
			task_lock.Enter();
			i = busy_tasks.Find(main_id);
		}
		busy_tasks.Remove(i);
		busy_running.Remove(i);
	}
	task_lock.Leave();
}

void System::Worker(int id) {
	
	while (running) {
		
		// Do some workarounds to have shared memory variables... some cross-platform dragons here...
		task_lock.Enter();
		ArrayMap<int, Callback>& busy_tasks = GetBusyTasklist<Callback>();
		Vector<One<Atomic> >& busy_running = GetBusyRunning<One<Atomic> >();
		
		
		// Avoid using too many threads for too little tasks
		if (id >= busy_tasks.GetCount()) {
			task_lock.Leave();
			Sleep(1000);
			continue;
		}
		
		
		// Try to find a task, which is not yet running.
		// Break this loop easily to avoid getting stuck.
		int task;
		bool do_break = false;
		bool do_continue = true;
		for (int i = 0; i < busy_running.GetCount(); i++) {
			task = busy_task++;
			if (busy_task >= busy_running.GetCount())
				busy_task = 0;
			if (task >= busy_running.GetCount())
				break;
			Atomic& ai = *busy_running[task];
			if (ai == 0) {
				do_continue = false;
				break;
			}
			if (!running) {
				do_break = true;
				break;
			}
		}
		if (do_break) {
			task_lock.Leave();
			break;
		}
		if (do_continue) {
			task_lock.Leave();
			continue;
		}
		
		
		// Run the task
		Atomic& task_running = *busy_running[task];
		task_running++;
		task_lock.Leave();
		
		busy_tasks[task]();
		task_running--;
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

}
