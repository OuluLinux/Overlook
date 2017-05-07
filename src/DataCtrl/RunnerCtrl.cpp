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
	progress.batches <<= THISBACK(RefreshData);
	
	progress.incomplete.AddColumn("#");
	progress.incomplete.AddColumn("Slot");
	progress.incomplete.AddColumn("Symbol");
	progress.incomplete.AddColumn("Timeframe");
	progress.incomplete.AddColumn("Begin");
	progress.incomplete.AddColumn("Progress");
	
	progress_batch_updating = false;
	progress_updating = false;
}

void RunnerCtrl::RefreshData() {
	int tab = tabs.Get();
	switch (tab) {
		case 0: RefreshProgress();			break;
		case 1:	break; // TODO: this shows the ctrl of currently running slot
		case 2: RefreshDependencyGraph();	break;
		case 3: RefreshDetails();			break;
	}
}

void RunnerCtrl::RefreshProgress() {
	DataCore::Session& ses = GetSession();
	DataCore::TimeVector& tv = GetTimeVector();
	
	int cursor = progress.batches.GetCursor();
	
	for(int i = 0; i < ses.GetBatchCount(); i++) {
		Batch& b = ses.GetBatch(i);
		
		progress.batches.Set(i, 0, i);
		progress.batches.Set(i, 1, b.slots.GetCount());
		
		String slotstr;
		for(int j = 0; j < b.slots.GetCount(); j++)
			slotstr += b.slots.GetKey(j) + " ";
		progress.batches.Set(i, 2, slotstr);
		
		progress.batches.Set(i, 3, b.begin.year		!= 1970 ? Format("%", b.begin)	: "");
		progress.batches.Set(i, 4, b.end.year		!= 1970 ? Format("%", b.end)	: "");
		progress.batches.Set(i, 5, b.stored.year	!= 1970 ? Format("%", b.stored)	: "");
		progress.batches.Set(i, 6, b.loaded.year	!= 1970 ? Format("%", b.loaded)	: "");
	}
	
	if (cursor != -1) {
		Batch& b = ses.GetBatch(cursor);
		
		int num = 0;
		for(int i = 0; i < b.status.GetCount(); i++) {
			BatchPartStatus& s = b.status[i];
			if (s.complete || s.actual == s.total) continue;
			
			progress.incomplete.Set(num, 0, num);
			progress.incomplete.Set(num, 1, s.slot->GetLinkPath());
			progress.incomplete.Set(num, 2, tv.GetSymbol(s.sym_id));
			progress.incomplete.Set(num, 3, tv.GetPeriod(s.tf_id));
			progress.incomplete.Set(num, 4, s.begin.year != 1970 ? Format("%", s.begin) : "");
			progress.incomplete.Set(num, 5, s.actual * 1000 / s.total);
			
			progress.incomplete.SetDisplay(num, 5, Single<ProgressDisplay>());
			
			num++;
		}
		progress.incomplete.SetCount(num);
		
		progress.incomplete.SetSortColumn(5, true);
	}
	
	progress_updating = false;
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
	DataCore::Session& ses = GetSession();
	TimeVector& tv = GetTimeVector();
	
	ses.WhenBatchFinished << THISBACK(PostRefreshData);
	ses.WhenPartFinished << THISBACK(PostRefreshProgress);
	ses.WhenProgress << THISBACK(PostProgress);
	ses.WhenPartProgress << THISBACK(PostPartProgress);
	
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
	int offset = 0;
	for(int i = 0; i < slot_count; i++) {
		const Slot& s = tv.GetCustomSlot(i);
		int resbytes = s.GetReservedBytes();
		details.slots.Add(
			s.GetKey(),
			Format("0x%X", resbytes),
			Format("0x%X", offset),
			s.GetPath());
		offset += resbytes;
	}
}

void RunnerCtrl::SetArguments(const VectorMap<String, Value>& args) {
	
}

void RunnerCtrl::PostRefreshProgress() {
	// If progress-tab is open, refresh stats
	if (tabs.Get() == 0 && !progress_updating) {
		progress_updating = true;
		PostCallback(THISBACK(RefreshProgress));
	}
}

void RunnerCtrl::PostPartProgress(int actual, int total) {
	if (!progress_batch_updating) {
		progress_batch_updating = true;
		PostCallback(THISBACK2(SetProgressBatch, actual, total));
	}
}

}
