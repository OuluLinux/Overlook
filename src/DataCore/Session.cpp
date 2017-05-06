#include "DataCore.h"

namespace DataCore {

Session::Session() {
	name = "default";
	
	running = false;
	stopped = true;
	timeslot = 2 * 60 * 1000;
	loop = 0;
}

Session::~Session() {
	Stop();
}

void Session::Stop() {
	running = false;
	while (!stopped) {Sleep(100);}
}

void Session::Enter(int slot, int sym_id, int tf_id) {
	thrd_lock.Enter();
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	if (locked.IsEmpty())
		locked.SetCount(sym_count * tf_count * tv.GetCustomSlotCount(), 0);
	int64 id = slot * sym_count * tf_count + sym_id * tf_count + tf_id;
	locked[id]++;
	thrd_lock.Leave();
}

void Session::Leave(int slot, int sym_id, int tf_id) {
	thrd_lock.Enter();
	ASSERT(!locked.IsEmpty());
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int64 id = slot * sym_count * tf_count + sym_id * tf_count + tf_id;
	int& locked = this->locked[id];
	locked--;
	ASSERT(locked >= 0);
	thrd_lock.Leave();
}

bool Session::IsLocked(int slot, int sym_id, int tf_id) {
	if (locked.IsEmpty()) return false;
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int64 id = slot * sym_count * tf_count + sym_id * tf_count + tf_id;
	int locked_count = locked[id];
	ASSERT(locked_count >= 0);
	return locked_count;
}

void Session::StoreThis() {
	ASSERT(!name.IsEmpty());
	String dir = AppendFileName(ConfigFile("profiles"), name);
	RealizeDirectory(dir);
	ASSERT(DirectoryExists(dir));
	
	String ses_path = AppendFileName(dir, "Session.bin");
	FileOut ses_out(ses_path);
	ses_out % begin % end % addr % port % timeslot % loop % tfs % link_core % link_ctrl % datadir;
	
	String tv_path = AppendFileName(dir, "TimeVector.bin");
	FileOut tv_out(tv_path);
	tv_out % tv;
	
	int custom_slots = tv.GetCustomSlotCount();
	for(int i = 0; i < custom_slots; i++) {
		Slot& slot = tv.GetCustomSlot(i);
		String link_path = slot.GetLinkPath().Mid(1);
		link_path.Replace("/", "__");
		String slotdir = AppendFileName(dir, link_path);
		RealizeDirectory(slotdir);
		String slot_path = AppendFileName(slotdir, "slot.bin");
		FileOut slot_out(slot_path);
		slot_out % slot;
	}
}

void Session::LoadThis() {
	ASSERT_(tv.GetCustomSlotCount() == 0, "Loading is allowed only at startup.");
	
	ASSERT(!name.IsEmpty());
	String dir = AppendFileName(ConfigFile("profiles"), name);
	RealizeDirectory(dir);
	ASSERT(DirectoryExists(dir));
	
	String ses_path = AppendFileName(dir, "Session.bin");
	FileIn ses_in(ses_path);
	ses_in % begin % end % addr % port % timeslot % loop % tfs % link_core % link_ctrl % datadir;
	
	Init();
	
	String tv_path = AppendFileName(dir, "TimeVector.bin");
	FileIn tv_in(tv_path);
	tv_in % tv;
	
	int custom_slots = tv.GetCustomSlotCount();
	for(int i = 0; i < custom_slots; i++) {
		Slot& slot = tv.GetCustomSlot(i);
		String link_path = slot.GetLinkPath().Mid(1);
		link_path.Replace("/", "__");
		String slotdir = AppendFileName(dir, link_path);
		RealizeDirectory(slotdir);
		String slot_path = AppendFileName(slotdir, "slot.bin");
		FileOut slot_in(slot_path);
		slot_in % slot;
	}
}

void Session::StoreProgress() {
	ASSERT(!name.IsEmpty());
	String dir = AppendFileName(ConfigFile("profiles"), name);
	RealizeDirectory(dir);
	ASSERT(DirectoryExists(dir));
	
	Time now = GetSysTime();
	for(int i = 0; i < batches.GetCount(); i++)
		batches[i].stored = now;
	
	String prog_path = AppendFileName(dir, "Progress.bin");
	FileOut prog_out(prog_path);
	prog_out % batches;
}

void Session::LoadProgress() {
	ASSERT(!name.IsEmpty());
	String dir = AppendFileName(ConfigFile("profiles"), name);
	RealizeDirectory(dir);
	ASSERT(DirectoryExists(dir));
	
	String prog_path = AppendFileName(dir, "Progress.bin");
	FileIn prog_in(prog_path);
	prog_in % batches;
	
	Time now = GetSysTime();
	for(int i = 0; i < batches.GetCount(); i++)
		batches[i].loaded = now;
}

void Session::Init() {
	MetaTrader& mt = GetMetaTrader();
	PathResolver& res = *GetPathResolver();
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetCacheSymbols();
	
	try {
		tv.EnableCache();
		tv.LimitMemory();
		
		
		// Init sym/tfs/time space
		ASSERTEXC(!mt.Init(addr, port));
		
		// Add symbols
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			tv.AddSymbol(s.name);
		}
		for(int i = 0; i < mt.GetCurrencyCount(); i++) {
			const Currency& c = mt.GetCurrency(i);
			tv.AddSymbol(c.name);
		}
		
		
		// TODO: store symbols to session file and check that mt supports them
		
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 15; // mins
		tv.SetBasePeriod(60*base);
		Vector<int> tfs;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			bool found = false;
			for(int j = 0; j < this->tfs.GetCount(); j++) {
				if (this->tfs[j] == tf) {found = true; break;}
			}
			if (!found ) continue;
			if (tf >= base) {
				tfs.Add(tf / base);
				tv.AddPeriod(tf * 60);
			}
		}
		
		
		// Init time range
		tv.SetBegin(begin);
		tv.SetEnd(end);
		MetaTime& mtime = res.GetTime();
		mtime.SetBegin(tv.GetBegin());
		mtime.SetEnd(tv.GetEnd());
		mtime.SetBasePeriod(tv.GetBasePeriod());
		
		
		
		// Link core
		Index<String> linkctrl_symtf;
		VectorMap<String, String> linkcustomctrl_symtf, linkcustomctrl_sym, linkcustomctrl_tf;
		for(int i = 0; i < link_core.GetCount(); i++) {
			const String& key = link_core.GetKey(i);
			String value = link_core[i];
			tv.LinkPath(key, value);
			
			// Link ctrl
			SlotPtr slot = tv.FindLinkSlot(key);
			String ctrlkey = slot->GetCtrl();
			
			// By default, slotctrl is Container and it is added separately for every symbol
			// and timeframe.
			if (ctrlkey == "default") {
				linkctrl_symtf.Add(key);
			}
			// Otherwise, link custom ctrl to /slotctrl/ folder.
			else {
				int type = slot->GetCtrlType();
				String ctrl_dest = "/slotctrl" + key;
				String ctrl_src = "/" + ctrlkey + "?slot=\"" + key + "\"";
				if (type == SLOT_SYMTF) {
					linkcustomctrl_symtf.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_SYM) {
					linkcustomctrl_sym.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_TF) {
					linkcustomctrl_tf.Add(ctrl_dest, ctrl_src);
				}
				else if (type == SLOT_ONCE) {
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				else Panic("Unknown slottype");
			}
		}
		
		
		// Link ctrls
		for(int i = 0; i < link_ctrl.GetCount(); i++) {
			const String& key = link_ctrl.GetKey(i);
			String value = link_ctrl[i];
			res.LinkPath(key, value);
		}
		
		
		// Link symbols and timeframes
		for(int i = 0; i < symbols.GetCount(); i++) {
			for(int j = 0; j < tfs.GetCount(); j++) {
				
				// Add frontpage
				String fp_dest = "/name/" + symbols[i].name;
				String fp_src = "/fp?id=" + IntStr(i) + "&symbol=\"" + symbols[i].name + "\"";
				fp_dest.Replace("#", "");
				res.LinkPath(fp_dest, fp_src);
				
				
				// Add by name
				String dest = "/name/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
				String src = "/bardata?bar=\"/open\"&id=" + IntStr(i) + "&period=" + IntStr(tfs[j]);
				dest.Replace("#", "");
				res.LinkPath(dest, src);
				
				
				// Add default sym/tf ctrls, by name
				for(int k = 0; k < linkctrl_symtf.GetCount(); k++) {
					const String& key = linkctrl_symtf[k];
					String ctrl_dest = "/slotctrl" + key + "/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
					ctrl_dest.Replace("#", "");
					String ctrl_src = dest + "/cont?slot=\"" + key + "\"&id=" + IntStr(i) + "&tf_id=" + IntStr(j);
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				
				// Add custom sym/tf ctrls
				for(int k = 0; k < linkcustomctrl_symtf.GetCount(); k++) {
					const String& key = linkcustomctrl_symtf.GetKey(k);
					String value = linkcustomctrl_symtf[k];
					String ctrl_dest = key + "/" + symbols[i].name + "/tf" + IntStr(tfs[j]);
					ctrl_dest.Replace("#", "");
					String ctrl_src = value + "&id=" + IntStr(i) + "&tf_id=" + IntStr(j);
					res.LinkPath(ctrl_dest, ctrl_src);
				}
				
				// Add custom tf ctrls
				if (i == 0) {
					for(int k = 0; k < linkcustomctrl_tf.GetCount(); k++) {
						const String& key = linkcustomctrl_tf.GetKey(k);
						String value = linkcustomctrl_tf[k];
						String ctrl_dest = key + "/tf" + IntStr(tfs[j]);
						ctrl_dest.Replace("#", "");
						String ctrl_src = value + "&tf_id=" + IntStr(j);
						res.LinkPath(ctrl_dest, ctrl_src);
					}
				}
			}
			
			// Add custom sym ctrls
			for(int k = 0; k < linkcustomctrl_sym.GetCount(); k++) {
				const String& key = linkcustomctrl_sym.GetKey(k);
				String value = linkcustomctrl_sym[k];
				String ctrl_dest = key + "/" + symbols[i].name;
				ctrl_dest.Replace("#", "");
				String ctrl_src = value + "&id=" + IntStr(i);
				res.LinkPath(ctrl_dest, ctrl_src);
			}
		}
		
		
		// Link parameter configuration ctrls for all slots
		for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
			const Slot& slot = tv.GetCustomSlot(i);
			String linkpath = slot.GetLinkPath();
			ASSERT(!linkpath.IsEmpty()); // sanity check
			String ctrl_dest = "/params" + linkpath;
			String ctrl_src = "/paramctrl?slot=\"" + linkpath + "\"";
			res.LinkPath(ctrl_dest, ctrl_src);
		}
		
		// Link runner (also starts processing, so keep this last)
		res.LinkPath("/runner", "/runnerctrl");
		
		
		tv.RefreshData();
		
		LoadProgress();
		RefreshBatches();
		
		StoreThis();
		
		running = true;
		stopped = false;
		Thread::Start(THISBACK(Run));
	}
	
	catch (...) {
		LOG("Load failed");
	}
	
}

void Session::Run() {
	int cpus = CPU_Cores();
	int sym_count = tv.GetSymbolCount();
	
	TimeStop ts, ts_total;
	
	while (!Thread::IsShutdownThreads() && running) {
		
		for(int i = 0; i < batches.GetCount(); i++) {
			WhenProgress(i, batches.GetCount());
			
			Batch& b = batches[i];
			
			b.begin = GetSysTime();
			//b.cursor = 0;
			
			ts.Reset();
			
			// Reset cursor
			if (b.cursor >= b.status.GetCount()) {
				b.cursor = 0;
				for(int i = 0; i < b.status.GetCount(); i++) {
					BatchPartStatus& stat = b.status[i];
					if (stat.complete)
						continue;
					
					stat.actual = 0;
					stat.end = stat.begin = Time(1970,1,1);
				}
			}
			
			#if 0
			CoWork co;
			for(int j = 0; j < cpus; j++)
				co & THISBACK2(BatchProcessor, j, &b);
			co.Finish();
			#else
			BatchProcessor(0, &b);
			#endif
			
			b.end = GetSysTime();
			WhenBatchFinished();
		}
		
		Sleep(1000);
		
		loop++;
	}
	
	stopped = true;
}

void Session::BatchProcessor(int thread_id, Batch* batch) {
	Batch& b = *batch;
	TimeStop ts;
	
	while (running) {
		int pos = b.cursor++;
		
		WhenPartProgress(pos, b.status.GetCount());
		
		if (pos >= b.status.GetCount())
			break;
		
		BatchPartStatus& stat = b.status[pos];
		
		if (!stat.complete) {
			Processor(stat);
			WhenPartFinished();
		}
		
		if (thread_id == 0 && ts.Elapsed() > 10000) {
			StoreProgress();
			ts.Reset();
		}
	}
}

void Session::Processor(BatchPartStatus& stat) {
	ASSERT(!stat.complete);
	
	int sym_id = stat.sym_id;
	int tf_id = stat.tf_id;
	Slot* slot = stat.slot;
	
	stat.begin = GetSysTime();
	
	int id = slot->GetId();
	int tf_count = tv.GetPeriodCount();
	ASSERT(tf_count > 0);
	
	// Get attributes of the TimeVector
	int64 memory_limit = tv.memory_limit;
	
	
	// Set attributes of the current time-position
	SlotProcessAttributes attr;
	ASSERTEXC(tv.bars.GetCount() <= 16);
	for(int i = 0; i < tf_count; i++) {
		attr.pos[i] = 0;
		attr.bars[i] = tv.bars[i];
		attr.periods[i] = tv.periods[i];
	}
	
	
	// Set attributes of TimeVector::Iterator
	attr.sym_count = tv.symbols.GetCount();
	attr.tf_count = tf_count;
	
	bool enable_cache = tv.enable_cache;
	bool check_memory_limit = memory_limit != 0;
	
	
	// Set attributes of current symbol/tf
	attr.tf_id = tf_id;
	attr.sym_id = sym_id;
	
	
	// Set attributes of the begin of the slot memory area
	attr.slot_pos = id;
	attr.slot = slot;
	
	stat.total = attr.bars[tf_id];
	
	slot->Enter(stat);
	
	
	// At first big loop (epoch?), process data once entirely
	if (loop == 0) {
		
		// Some slots have attack (like the musical term). This can only be called once.
		if (slot->HasAttack()) {
			stat.actual = 0; // Start from begin
			Loop(stat, attr, NULL, PROC_ATTACK);
		}
		
		// All slots must have sustain
		stat.actual = 0; // Start from begin
		bool success = Loop(stat, attr, NULL, PROC_SUSTAIN);
		
		// Some slots have release (like the musical term)
		if (slot->HasRelease()) {
			stat.actual = 0; // Start from begin
			Loop(stat, attr, NULL, PROC_RELEASE);
		}
		// Slot without release is only processed once, and successful loop will make it complete
		else if (success)
			stat.complete = true;
	}
	// In later loops, process constant time-slots
	else {
		TimeStop ts;
		
		// "Attack" is only allowed once in the first round, so it won't be called here
		
		// All slots must have sustain. Continue from previous position.
		Loop(stat, attr, &ts, PROC_SUSTAIN);
		
		// Some slots have release (like the musical term), it must be called after sustain always
		// Reset position to zero while calling this.
		int actual = 0; // Start from begin
		Swap(stat.actual, actual);
		if (slot->HasRelease())
			Loop(stat, attr, NULL, PROC_RELEASE);
		Swap(stat.actual, actual);
	}
	
	stat.end = GetSysTime();
	
	slot->Leave(stat);
}

bool Session::Loop(BatchPartStatus& stat, SlotProcessAttributes& attr, TimeStop* ts, int call) {
	int tf_count = tv.GetPeriodCount();
	bool success = true;
	
	
	// Loop until time limit, or once if TimeStop is NULL
	do {
		
		// Reset position
		if (stat.actual >= stat.total) stat.actual = 0;
		
		// Reset time positions
		for(int i = 0; i < tf_count; i++) {
			attr.pos[i] = 0;
		}
		
		// Restore position vector to the actual
		while (attr.pos[attr.tf_id] != stat.actual) {
			for(int i = attr.tf_id; i < tf_count; i++) {
				int& pos = attr.pos[i];
				pos++;
				if (i < tf_count-1 && (pos % tv.tfbars_in_slowtf[i]) != 0) {
					break;
				}
			}
		}
		
		// Loop over time positions. Continue from previous position, if the previous loop was
		// unfinished because of time limit.
		for(; stat.actual < stat.total && running && (!ts || (ts && ts->Elapsed() < timeslot)); stat.actual++) {
			attr.time = tv.GetTime(tv.periods[0], attr.pos[0]).Get();
			
			// Only first loop will check if slot is processed. In later loops this will be
			// processed any way.
			if (!loop && attr.slot->IsReady(attr)) {
				continue;
			}
			
			bool proc_success;
			if (call == PROC_SUSTAIN)
				proc_success = attr.slot->Process(attr);
			else if (call == PROC_ATTACK)
				proc_success = attr.slot->ProcessAttack(attr);
			else if (call == PROC_RELEASE)
				proc_success = attr.slot->ProcessRelease(attr);
			else
				Panic("Invalid call: " + IntStr(call));
				
			if (!proc_success) {
				// failed, what now?
				success = false;
			}
			else {
				// Set sym/tf/pos/slot position as ready
				attr.slot->SetReady(attr);
			}
			
			// Increase position from current timeframe to longer.
			// Keep shorter than this timeframe as 0.
			for(int i = attr.tf_id; i < tf_count; i++) {
				int& pos = attr.pos[i];
				pos++;
				if (i < tf_count-1 && (pos % tv.tfbars_in_slowtf[i]) != 0) {
					break;
				}
			}
		}
	}
	while (ts && ts->Elapsed() < timeslot);
	
	return success;
}

void Session::RefreshBatches() {
	int sym_count = tv.GetSymbolCount();
	int tf_count = tv.GetPeriodCount();
	int total = sym_count * tf_count;
	
	// The initial batch setup
	if (batches.IsEmpty()) {
		
		// Split slots into batches
		Vector<Slot*> not_added;
		for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
			Slot& slot = tv.GetCustomSlot(i);
			not_added.Add(&slot);
		}
		
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
					current.slot_links.Add(slot.GetLinkPath());
					
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
			
			// Start from zero
			b.status.SetCount(0);
			
			// Reserve memory for faster adding
			int max_total = sym_count * tf_count * b.slots.GetCount();
			b.status.Reserve(max_total);
			
			// Loop all slots in the batch and add processing queue according to the settings of the slot
			int s = 0;
			for(int i = 0; i < b.slots.GetCount(); i++) {
				SlotPtr slot = b.slots[i];
				
				if (slot->other_symbols) {
					if (slot->other_timeframes) {
						BatchPartStatus& stat = b.status.Add();
						stat.slot = slot;
						stat.sym_id = 0;
						stat.tf_id = 0;
						stat.batch_slot = i;
					} else {
						for(int j = 0; j < tf_count; j++) {
							BatchPartStatus& stat = b.status.Add();
							stat.slot = slot;
							stat.sym_id = 0;
							stat.tf_id = j;
							stat.batch_slot = i;
						}
					}
				} else {
					if (slot->other_timeframes) {
						for(int j = 0; j < sym_count; j++) {
							BatchPartStatus& stat = b.status.Add();
							stat.slot = slot;
							stat.sym_id = j;
							stat.tf_id = 0;
							stat.batch_slot = i;
						}
					} else {
						for(int j = 0; j < sym_count; j++) {
							for(int k = 0; k < tf_count; k++) {
								BatchPartStatus& stat = b.status.Add();
								stat.slot = slot;
								stat.sym_id = j;
								stat.tf_id = k;
								stat.batch_slot = i;
							}
						}
					}
				}
			}
		}
	}
	
	// Restore existing batches for continuing
	else {
		
		// Update slot memory addresses
		for(int i = 0; i < batches.GetCount(); i++) {
			Batch& b = batches[i];
			
			b.slots.Clear();
			for(int j = 0; j < b.slot_links.GetCount(); j++) {
				const String& link = b.slot_links[j];
				
				SlotPtr slot = tv.FindLinkSlot(link);
				ASSERTEXC(slot);
				
				b.slots.Add(link, slot);
			}
			
			for(int j = 0; j < b.status.GetCount(); j++) {
				BatchPartStatus& s = b.status[j];
				s.slot = b.slots[s.batch_slot];
			}
		}
	}
}

}
