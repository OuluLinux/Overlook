#include "DataCtrl.h"

namespace DataCtrl {




RunnerCtrl::RunnerCtrl() {
	CtrlLayout(progress);
	CtrlLayout(details);
	
	Add(tabs.SizePos());
	tabs.Add(progress, "Progress");
	tabs.Add(progress);
	tabs.Add(sysmon, "System Monitor");
	tabs.Add(sysmon);
	tabs.Add(dependencies, "Dependencies");
	tabs.Add(dependencies);
	tabs.Add(details, "Details");
	tabs.Add(details);
	tabs.WhenSet = THISBACK(RefreshData);
	
	progress.batches.AddColumn("#");
	progress.batches.AddColumn("Slot-count");
	progress.batches.AddColumn("Slots");
	progress.batches.AddColumn("Begin");
	progress.batches.AddColumn("End");
	progress.batches.AddColumn("Stored");
	progress.batches.AddColumn("Loaded");
	
	progress.incomplete.AddColumn("#");
	progress.incomplete.AddColumn("Slot");
	progress.incomplete.AddColumn("Symbol");
	progress.incomplete.AddColumn("Timeframe");
	progress.incomplete.AddColumn("Begin");
	progress.incomplete.AddColumn("Progress");
	
	running = false;
	stopped = true;
}

RunnerCtrl::~RunnerCtrl() {
	running = false;
	while (!stopped) {Sleep(100);}
}

void RunnerCtrl::RefreshBatches() {
	TimeVector& tv = GetTimeVector();
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	
	
	// Split slots into batches
	Vector<Slot*> not_added;
	for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
		const Slot& slot = tv.GetCustomSlot(i);
		not_added.Add((Slot*)&slot);
	}
	
	batches.Clear();
	while (!not_added.IsEmpty()) {
		
		// Add new batch
		Batch& current = batches.Add();
		
		for(int i = 0; i < not_added.GetCount(); i++) {
			Slot& slot = *not_added[i];
			
			// Check if all slot's dependencies are in earlier batches
			bool deps_earlier_only = true;
			for(int j = 0; j < slot.GetDependencyCount(); j++) {
				String path = slot.GetDependencyPath(j);
				bool earlier_deb = false;
				for(int k = 0; k < batches.GetCount()-1 && !earlier_deb; k++) {
					Batch& b = batches[k];
					for(int l = 0; l < b.slots.GetCount(); l++) {
						if (b.slots.GetKey(l) == path) {
							earlier_deb = true;
							break;
						}
					}
				}
				if (!earlier_deb) {
					deps_earlier_only = false;
					break;
				}
			}
			
			if (deps_earlier_only) {
				current.slots.Add(slot.GetLinkPath(), &slot);
				
				// Remove current from queue
				not_added.Remove(i);
				i--;
			}
		}
	}
	
	// Print some interesting stats
	for(int i = 0; i < batches.GetCount(); i++) {
		Batch& b = batches[i];
		LOG("Batch " << i);
		for(int j = 0; j < b.slots.GetCount(); j++) {
			LOG("      " << b.slots.GetKey(j));
		}
	}
	
	// Initialise status list
	for(int i = 0; i < batches.GetCount(); i++) {
		Batch& b = batches[i];
		
		int total = sym_count * tf_count * b.slots.GetCount();
		b.status.SetCount(total);
		
		int s = 0;
		for(int i = 0; i < b.slots.GetCount(); i++) {
			SlotPtr slot = b.slots[i];
			for(int j = 0; j < sym_count; j++) {
				for(int k = 0; k < tf_count; k++) {
					BatchPartStatus& stat = b.status[s++];
					stat.slot = slot;
					stat.sym_id = j;
					stat.tf_id = k;
				}
			}
		}
		ASSERT(s == total);
	}
}

void RunnerCtrl::RefreshData() {
	int tab = tabs.Get();
	switch (tab) {
		case 0: RefreshProgress();			break;
		case 1:								break;
		case 2: RefreshDependencyGraph();	break;
		case 3: RefreshDetails();			break;
	}
}

void RunnerCtrl::RefreshProgress() {
	DataCore::TimeVector& tv = GetTimeVector();
	
	int cursor = progress.batches.GetCursor();
	
	
	for(int i = 0; i < batches.GetCount(); i++) {
		Batch& b = batches[i];
		
		progress.batches.Set(i, 0, i);
		progress.batches.Set(i, 1, b.slots.GetCount());
		
		String slotstr;
		for(int j = 0; j < b.slots.GetCount(); j++)
			slotstr += b.slots.GetKey(j) + " ";
		progress.batches.Set(i, 2, slotstr);
		
		progress.batches.Set(i, 3, Format("%", b.begin));
		progress.batches.Set(i, 4, Format("%", b.end));
		progress.batches.Set(i, 5, Format("%", b.stored));
		progress.batches.Set(i, 6, Format("%", b.loaded));
	}
	
	if (cursor != -1) {
		Batch& b = batches[cursor];
		
		int num = 0;
		for(int i = 0; i < b.status.GetCount(); i++) {
			BatchPartStatus& s = b.status[i];
			if (s.complete) continue;
			
			progress.incomplete.Set(i, 0, num);
			progress.incomplete.Set(i, 1, s.slot->GetLinkPath());
			progress.incomplete.Set(i, 2, tv.GetSymbol(s.sym_id));
			progress.incomplete.Set(i, 3, tv.GetPeriod(s.tf_id));
			progress.incomplete.Set(i, 4, Format("%", s.begin));
			progress.incomplete.Set(i, 5, s.actual * 1000 / s.total);
			
			progress.incomplete.SetDisplay(i, 5, Single<ProgressDisplay>());
			
			num++;
		}
		
		progress.incomplete.SetSortColumn(5, true);
	}
}

void RunnerCtrl::RefreshDependencyGraph() {
	dependencies.Clear();
	
	DataCore::TimeVector& tv = GetTimeVector();
	for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
		const Slot& slot = tv.GetCustomSlot(i);
		
		String src_path = slot.GetLinkPath();
		dependencies.AddNode(src_path);
		
		int slot_deps = slot.GetDependencyCount();
		for(int j = 0; j < slot_deps; j++) {
			String dst_path = slot.GetDependencyPath(j);
			
			dependencies.AddEdge(src_path, dst_path).SetDirected();
		}
	}
	
	dependencies.RefreshLayout();
	dependencies.Refresh();
}

void RunnerCtrl::RefreshDetails() {
	/*details.timedraw.Refresh();
	details.slotdraw.Refresh();
	
	details.totaltime.SetLabel("Time total: " + IntStr(last_total_duration) + "ms");
	if (last_total) {
		details.totalready.SetLabel("Ready total: " +
			IntStr(last_total_ready * 100 / last_total) + "%" " (" +
			IntStr(last_total_ready) + "/" + IntStr(last_total) + ")");
	}*/
}


void RunnerCtrl::Init() {
	TimeVector& tv = GetTimeVector();
	
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int slot_count = tv.GetCustomSlotCount();
	
	
	// Progress ParentCtrl
	details.symbols.AddColumn("Symbol");
	for(int i = 0; i < sym_count; i++) {
		details.symbols.Add(tv.GetSymbol(i));
	}
	
	details.tfs.AddColumn("Period");
	details.tfs.AddColumn("Minutes");
	details.tfs.AddColumn("Hours");
	details.tfs.AddColumn("Days");
	for(int i = 0; i < tf_count; i++) {
		int tf = tv.GetPeriod(i);
		int mins = tf * tv.GetBasePeriod() / 60;
		int hours = mins / 60;
		int days = mins / (60 * 24);
		details.tfs.Add(tf, mins, hours, days);
	}
	
	details.slots.AddColumn("Name");
	details.slots.AddColumn("Size");
	details.slots.AddColumn("Offset");
	details.slots.AddColumn("Path");
	details.slots.ColumnWidths("1 1 1 3");
	for(int i = 0; i < slot_count; i++) {
		const Slot& s = tv.GetCustomSlot(i);
		details.slots.Add(
			s.GetKey(),
			Format("0x%X", s.GetReservedBytes()),
			Format("0x%X", s.GetOffset()),
			s.GetPath());
	}
	
	RefreshBatches();
	
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Run));
}

void RunnerCtrl::SetArguments(const VectorMap<String, Value>& args) {
	
}

void RunnerCtrl::Run() {
	TimeVector& tv = GetTimeVector();
	
	int cpus = CPU_Cores();
	int sym_count = tv.GetSymbolCount();
	
	TimeStop ts, ts_total;
	
	while (!Thread::IsShutdownThreads() && running) {
		
		for(int i = 0; i < batches.GetCount(); i++) {
			PostCallback(THISBACK2(SetProgressTotal, i, batches.GetCount()));
			
			Batch& b = batches[i];
			b.begin = GetSysTime();
			
			cursor = 0;
			ts.Reset();
			
			CoWork co;
			for(int j = 0; j < cpus; j++)
				co & THISBACK2(BatchProcessor, j, &b);
			co.Finish();
			
			b.end = GetSysTime();
			PostCallback(THISBACK(RefreshData));
		}
		
		for(int i = 0; i < 60 && !Thread::IsShutdownThreads() && running; i++) {
			Sleep(1000);
		}
	}
	
	tv.StoreChangedCache();
	
	stopped = true;
}

void RunnerCtrl::BatchProcessor(int thread_id, Batch* batch) {
	Batch& b = *batch;
	
	for (;;) {
		int pos = cursor++;
		PostCallback(THISBACK2(SetProgressBatch, pos, b.status.GetCount()));
		
		if (pos >= b.status.GetCount())
			break;
		
		BatchPartStatus& stat = b.status[pos];
		
		Processor(stat);
		
		// If progress-tab is open, refresh stats
		if (tabs.Get() == 0)
			PostCallback(THISBACK(RefreshProgress));
	}
}

void RunnerCtrl::Processor(BatchPartStatus& stat) {
	int sym_id = stat.sym_id;
	int tf_id = stat.tf_id;
	Slot* slot = stat.slot;
	
	stat.begin = GetSysTime();
	
	TimeVector& tv = GetTimeVector();
	
	int id = slot->GetId();
	int tfs = tv.periods.GetCount();
	
	
	// Get attributes of the TimeVector
	int total_slot_bytes = tv.total_slot_bytes;
	int slot_flag_offset = tv.slot_flag_offset;
	int64 memory_limit = tv.memory_limit;
	int64& reserved_memory = tv.reserved_memory;
	Vector<int>& slot_bytes = tv.slot_bytes;
	
	
	// Set attributes of the current time-position
	SlotProcessAttributes attr;
	ASSERTEXC(tv.bars.GetCount() <= 16);
	for(int i = 0; i < tv.bars.GetCount(); i++) {
		attr.bars[i] = tv.bars[i];
		attr.periods[i] = tv.periods[i];
	}
	
	
	// Set attributes of TimeVector::Iterator
	attr.sym_count = tv.symbols.GetCount();
	attr.tf_count = tv.periods.GetCount();
	
	bool enable_cache = tv.enable_cache;
	bool check_memory_limit = memory_limit != 0;
	
	
	// Set attributes of current symbol/tf
	attr.tf_id = tf_id;
	attr.sym_id = sym_id;
	attr.slot_it = tv.data[sym_id][tf_id].Begin();
	
	
	// Set attributes of the begin of the slot memory area
	attr.slot_bytes = slot_bytes[id];
	attr.slot_pos = id;
	attr.slot = slot;
	
	stat.total = attr.bars[tf_id];
	
	for(int i = 0; i < stat.total; i++) {
		stat.actual = i;
		
		attr.time = tv.GetTime(tv.periods[0], attr.pos[0]).Get();
		
		if (attr.slot->IsReady(attr)) {
			continue;
		}
		
		if (!attr.slot->Process(attr)) {
			// failed, what now?
		}
		else {
			// Set sym/tf/pos/slot position as ready
			attr.slot->SetReady(attr);
		}
		
		attr.slot_it++;
		
		for(int i = 0; i < tfs; i++) {
			int& pos = attr.pos[i];
			pos++;
			if (i < tfs-1 && (pos % tv.tfbars_in_slowtf[i]) != 0) {
				break;
			}
		}
	}
	
	stat.end = GetSysTime();
	stat.complete = true;
}

}
