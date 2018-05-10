#include "Overlook.h"

namespace Overlook {

void System::InitRegistry() {
	ASSERT(regs.IsEmpty());
	ASSERT_(System::GetCoreFactories().GetCount() > 0, "Recompile Overlook.icpp to fix this stupid and weird problem");
	
	// Register factories
	for(int i = 0; i < System::GetCoreFactories().GetCount(); i++) {
		// unfortunately one object must be created, because IO can't be static and virtual at the same time and it is cleaner to use virtual.
		Ptr<Core> core = System::GetCoreFactories()[i].c();
		FactoryRegister& reg = regs.Add();
		core->IO(reg);
	}
	
	// Resize databank
	data.SetCount(symbols.GetCount());
	for(int i = 0; i < data.GetCount(); i++) {
		data[i].SetCount(periods.GetCount());
		for(int j = 0; j < periods.GetCount(); j++)
			data[i][j].SetCount(regs.GetCount());
	}
	
}

// This function is only a demonstration how to make work queues
void System::GetWorkQueue(Vector<Ptr<CoreItem> >& ci_queue) {
	Index<int> sym_ids, tf_ids;
	Vector<FactoryDeclaration> indi_ids;
	bool all = false;
	
	if (all) {
		for(int i = 0; i < symbols.GetCount(); i++)
			sym_ids.Add(i);
		
		for(int i = periods.GetCount()-1; i >= 0; i--)
			tf_ids.Add(i);
		
		int indi_limit = Find<PeriodicalChange>();
		ASSERT(indi_limit != -1);
		indi_limit++;
		for(int i = 0; i < indi_limit; i++)
			indi_ids.Add().Set(i);
	} else {
		for(int i = 0; i < symbols.GetCount(); i++)
			sym_ids.Add(i);
		
		for(int i = periods.GetCount()-1; i >= periods.GetCount()-1; i--)
			tf_ids.Add(i);
		
		int indi_limit = Find<MovingAverage>() + 1;
		ASSERT(indi_limit != 0);
		for(int i = 0; i < indi_limit; i++)
			indi_ids.Add().Set(i);
	}
	
	GetCoreQueue(ci_queue, sym_ids, tf_ids, indi_ids);
}

int System::GetCoreQueue(Vector<Ptr<CoreItem> >& ci_queue, const Index<int>& sym_ids, const Index<int>& tf_ids, const Vector<FactoryDeclaration>& indi_ids) {
	const int tf_count = GetPeriodCount();
	Vector<FactoryDeclaration> path;
	int total = tf_ids.GetCount() * indi_ids.GetCount() + 2;
	int actual = 0;
	for (int i = 0; i < tf_ids.GetCount(); i++) {
		int tf = tf_ids[i];
		ASSERT(tf >= 0 && tf < tf_count);
		for(int j = 0; j < indi_ids.GetCount(); j++) {
			path.Add(indi_ids[j]);
			GetCoreQueue(path, ci_queue, tf, sym_ids);
			path.Pop();
			WhenProgress(actual++, total);
		}
	}
	
	// Sort queue by priority
	struct PrioritySorter {
		bool operator()(const Ptr<CoreItem>& a, const Ptr<CoreItem>& b) const {
			if (a->priority == b->priority)
				return a->factory < b->factory;
			return a->priority < b->priority;
		}
	};
	Sort(ci_queue, PrioritySorter());
	WhenProgress(actual++, total);
	
	// Remove duplicates
	Vector<int> rem_list;
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		CoreItem& a = *ci_queue[i];
		for(int j = i+1; j < ci_queue.GetCount(); j++) {
			CoreItem& b = *ci_queue[j];
			if (a.sym == b.sym && a.tf == b.tf && a.factory == b.factory && a.hash == b.hash) {
				rem_list.Add(j);
				i++;
			}
			else break;
		}
	}
	ci_queue.Remove(rem_list);
	WhenProgress(actual++, total);
	
	for(int i = 0; i < ci_queue.GetCount(); i++) {
		CoreItem& ci = *ci_queue[i];
		//LOG(Format("%d: sym=%d tf=%d factory=%d hash=%d", i, ci.sym, ci.tf, ci.factory, ci.hash));
	}
	
	return 0;
}

int System::GetCoreQueue(Vector<FactoryDeclaration>& path, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids) {
	const int factory = path.Top().factory;
	const int tf_count = GetPeriodCount();
	const int sym_count = GetTotalSymbolCount();
	const int factory_count = GetFactoryCount();
	
	
	// Loop inputs of the factory
	const FactoryRegister& reg = regs[factory];
	
	
	Vector<Vector<FactoryHash> > input_hashes;
	input_hashes.SetCount(reg.in.GetCount());
	
	
	// Get the unique hash for core item
	Vector<int> args;
	CombineHash ch;
	const FactoryDeclaration& factory_decl = path.Top();
	for(int i = 0; i < reg.args.GetCount(); i++) {
		const ArgType& arg = reg.args[i];
		int value;
		ASSERT(factory_decl.arg_count >= 0 && factory_decl.arg_count <= 8);
		if (i < factory_decl.arg_count) {
			value = factory_decl.args[i];
			ASSERT(value >= arg.min && value <= arg.max);
		} else {
			value = arg.def;
		}
		args.Add(value);
		ch << value << 1;
	}
	int hash = ch;
	
	
	// Connect input sources
	// Loop all inputs of the custom core-class
	for (int l = 0; l < reg.in.GetCount(); l++) {
		Index<int> sub_sym_ids, sub_tf_ids;
		const RegisterInput& input = reg.in[l];
		ASSERT(input.factory >= 0);
		FilterFunction fn = (FilterFunction)input.data;
		
		// If equal timeframe is accepted as input
		for(int i = 0; i < tf_count; i++) {
			if (fn(this, -1, tf, -1, i)) {
				sub_tf_ids.Add(i);
			}
		}
		
		if (!sub_tf_ids.IsEmpty()) {
			for(int k = 0; k < sub_tf_ids.GetCount(); k++) {
				int used_tf = sub_tf_ids[k];
				
				// Get all symbols what input requires
				sub_sym_ids.Clear();
				for(int i = 0; i < sym_ids.GetCount(); i++) {
					int in_sym = sym_ids[i];
					
					for(int j = 0; j < GetTotalSymbolCount(); j++) {
						if (fn(this, in_sym, -1, j, -1))
							sub_sym_ids.FindAdd(j);
					}
				}
				
				if (!sub_sym_ids.IsEmpty()) {
					FactoryDeclaration& decl = path.Add();
					
					// Optional: add arguments by calling defined function
					decl.Set(input.factory);
					if (input.data2)
						((ArgsFn)input.data2)(l, decl, args);
					
					int h = GetCoreQueue(path, ci_queue, used_tf, sub_sym_ids);
					path.Pop();
					
					input_hashes[l].Add(FactoryHash(input.factory, h));
				}
			}
		}
	}
	
	
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		
		// Get CoreItem
		CoreItem& ci = data[sym][tf][factory].GetAdd(hash);
		
		// Init object if it was just created
		if (ci.sym == -1) {
			int path_priority = 0;//GetPathPriority(path);
			
			ci.sym			= sym;
			ci.tf			= tf;
			
			if (System::PrioritySlowTf().Find(factory) == -1)
				ci.priority		= (factory * tf_count + tf) * sym_count + sym;
			else
				ci.priority		= (factory * tf_count + (tf_count - 1 - tf)) * sym_count + sym;
			
			ci.factory		= factory;
			ci.hash			= hash;
			ci.input_hashes	<<= input_hashes;
			ci.args			<<= args;
			//LOG(Format("%X\tfac=%d\tpath_priority=%d\tprio=%d", (int64)&ci, ci.factory, path_priority, ci.priority));
			//DUMPC(args);
			
			// Connect core inputs
			ConnectCore(ci);
		}
		
		ci_queue.Add(&ci);
	}
	
	return hash;
}

int System::GetHash(const Vector<byte>& vec) {
	CombineHash ch;
	
	int full_ints = vec.GetCount() / 4;
	int int_mod = vec.GetCount() % 4;
	
	int* i = (int*)vec.Begin();
	for(int j = 0; j < full_ints; j++) {
		ch << *i << 1;
		i++;
	}
	byte* b = (byte*)i;
	for(int j = 0; j < int_mod; j++) {
		ch << *b << 1;
		b++;
	}
	return ch;
}


void System::ConnectCore(CoreItem& ci) {
	const FactoryRegister& part = regs[ci.factory];
	Vector<int> enabled_input_factories;
	Vector<byte> unique_slot_comb;

	// Connect input sources
	// Loop all inputs of the custom core-class
	ci.inputs.SetCount(part.in.GetCount());
	ASSERT(ci.input_hashes.GetCount() == part.in.GetCount());
	for (int l = 0; l < part.in.GetCount(); l++) {
		const RegisterInput& input = part.in[l];
		
		for(int k = 0; k < ci.input_hashes[l].GetCount(); k++) {
			int factory = ci.input_hashes[l][k].a;
			int hash = ci.input_hashes[l][k].b;
			
			// Regular inputs
			if (input.input_type == REGIN_NORMAL) {
				ASSERT(input.factory == factory);
				ConnectInput(l, 0, ci, input.factory, hash);
			}
			
			// Optional factory inputs
			else if (input.input_type == REGIN_OPTIONAL) {
				// Skip disabled
				if (factory == -1)
					continue;
				
				ConnectInput(l, 0, ci, factory, hash);
			}
			else Panic("Invalid input type");
		}
	}
}

void System::ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash) {
	Vector<int> symlist, tflist;
	const RegisterInput& input = regs[ci.factory].in[input_id];
	const int sym_count = GetTotalSymbolCount();
	const int tf_count = GetPeriodCount();
	
	
	FilterFunction fn = (FilterFunction)input.data;
	if (fn) {
		
		// Filter timeframes
		for(int i = 0; i < tf_count; i++) {
			if (fn(this, -1, ci.tf, -1, i)) {
				tflist.Add(i);
			}
		}
		
		
		// Filter symbols
		for(int i = 0; i < sym_count; i++) {
			if (fn(this, ci.sym, -1, i, -1)) {
				symlist.Add(i);
			}
		}
	}
	else {
		tflist.Add(ci.tf);
		symlist.Add(ci.sym);
	}
	
	for(int i = 0; i < symlist.GetCount(); i++) {
		int sym = symlist[i];
		
		for(int j = 0; j < tflist.GetCount(); j++) {
			int tf = tflist[j];
			
			CoreItem& src_ci = data[sym][tf][factory].GetAdd(hash);
			ASSERT_(src_ci.sym != -1, "Source CoreItem was not yet initialized");
			ASSERT_(src_ci.priority <= ci.priority, "Source didn't have higher priority than current");
			
			// Source found
			ci.SetInput(input_id, src_ci.sym, src_ci.tf, src_ci, output_id);
		}
	}
}

void System::CreateCore(CoreItem& ci) {
	ASSERT(ci.core.IsEmpty());
	
	// Create core-object
	ci.core = System::GetCoreFactories()[ci.factory].b();
	Core& c = *ci.core;
	
	// Set attributes
	c.RefreshIO();
	c.SetSymbol(ci.sym);
	c.SetTimeframe(ci.tf, GetPeriod(ci.tf));
	c.SetFactory(ci.factory);
	c.SetHash(ci.hash);
	
	
	// Connect object
	int obj_count = c.inputs.GetCount();
	int def_count = ci.inputs.GetCount();
	ASSERT(obj_count == def_count);
	for(int i = 0; i < ci.inputs.GetCount(); i++) {
		const VectorMap<int, SourceDef>& src_list = ci.inputs[i];
		Input& in = c.inputs[i];
		
		for(int j = 0; j < src_list.GetCount(); j++) {
			int key = src_list.GetKey(j);
			const SourceDef& src_def = src_list[j];
			Source& src_obj = in.Add(key);
			CoreItem& src_ci = *src_def.coreitem;
			ASSERT_(!src_ci.core.IsEmpty(), "Core object must be created before this point");
			
			src_obj.core = &*src_ci.core;
			src_obj.output = &src_obj.core->outputs[src_def.output];
			src_obj.sym = src_def.sym;
			src_obj.tf = src_def.tf;
		}
	}
	
	// Set arguments
	ArgChanger arg;
	arg.SetLoading();
	c.IO(arg);
	if (ci.args.GetCount() > 0) {
		ASSERT(ci.args.GetCount() <= arg.keys.GetCount());
		for(int i = 0; i < ci.args.GetCount(); i++)
			arg.args[i] = ci.args[i];
		arg.SetStoring();
		c.IO(arg);
	}
	
	// Initialize
	c.InitAll();
	c.LoadCache();
	c.AllowJobs();
}

Core* System::CreateSingle(int factory, int sym, int tf) {
	
	// Enable factory
	Vector<FactoryDeclaration> path;
	path.Add().Set(factory);
	
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
		Process(*ci_queue[i], true);
	}
	
	return &*ci_queue.Top()->core;
}

void System::Process(CoreItem& ci, bool store_cache) {
	
	// Load dependencies to the scope
	if (ci.core.IsEmpty())
		CreateCore(ci);
	
	// Process core-object
	ci.core->Refresh();
	
	// Store cache file
	if (store_cache)
		ci.core->StoreCache();
	
}

#ifdef flagGUITASK

void System::ProcessJobs() {
	bool break_loop = false;
	
	TimeStop ts;
	do {
		if (job_threads.IsEmpty())
			break;
		
		break_loop = ProcessJob(gui_job_thread++);
		
		if (gui_job_thread >= job_threads.GetCount())
			gui_job_thread = 0;
	}
	while (ts.Elapsed() < 200 && !break_loop);
	
	jobs_tc.Set(10, THISBACK(PostProcessJobs));
}

#else

void System::ProcessJobs() {
	jobs_stopped = false;
	
	while (jobs_running && !Thread::IsShutdownThreads()) {
		
		LOCK(job_lock) {
			int running = 0;
			int max_running = GetUsedCpuCores();
			for (auto& job : job_threads)
				if (!job.IsStopped())
					running++;
			
			for(int i = 0; i < job_threads.GetCount() && running < max_running; i++) {
				if (job_thread_iter >= job_threads.GetCount())
					job_thread_iter = 0;
				
				JobThread& job = job_threads[job_thread_iter++];
				
				if (job.IsStopped()) {
					job.Start();
					if (!job.IsStopped())
						running++;
				}
			}
		}
		
		for(int i = 0; i < 10 && jobs_running; i++)
			Sleep(100);
	}
	
	LOCK(job_lock) {
		for (auto& job : job_threads)	job.PutStop(); // Don't wait
		for (auto& job : job_threads)	job.Stop();
	}
	
	jobs_stopped = true;
}

#endif

bool System::ProcessJob(int job_thread) {
	return job_threads[job_thread].ProcessJob();
}

bool JobThread::ProcessJob() {
	bool r = true;
	
	READLOCK(job_lock) {
		if (!jobs.IsEmpty()) {
			Job& job = *jobs[job_iter];
			
			if (job.IsFinished()) {
				job_iter++;
				if (job_iter >= jobs.GetCount()) {
					job_iter = 0;
					r = false;
				}
			}
			else {
				if (job.core->IsInitialized()) {
					job.core->EnterJob(&job, this);
					bool succ = job.Process();
					job.core->LeaveJob();
					if (!succ && job.state == Job::INIT)
						r = false;
					// FIXME: postponing returns false also if (!succ) is_fail = true;
				}
				
				// Resist useless fast loop here
				else Sleep(100);
			}
			
			// Resist useless fast loop here
			if (job.state != Job::RUNNING) Sleep(100);
		}
		else r = false;
	}
	
	// Resist useless fast loop here
	if (r == false) Sleep(100);
	
	return r;
}

void System::StopJobs() {
	#ifndef flagGUITASK
	jobs_running = false;
	while (!jobs_stopped) Sleep(100);
	#endif
	
	StoreJobCores();
}

void System::StoreJobCores() {
	Index<Core*> job_cores;
	for(int i = 0; i < job_threads.GetCount(); i++) {
		JobThread& job_thrd = job_threads[i];
		for(int j = 0; j < job_thrd.jobs.GetCount(); j++) {
			Job& job = *job_thrd.jobs[j];
			Core* core = job.core;
			if (!core) continue;
			job_cores.FindAdd(core);
		}
	}
	for(int i = 0; i < job_cores.GetCount(); i++)
		job_cores[i]->StoreCache();
}

bool Job::Process() {
	if (!allow_processing)
		return false;
	
	bool r = true;
	
	int begin_state = state;
	
	core->serialization_lock.Enter();
	switch (state) {
		case INIT:			core->RefreshSources();
							if ((r=begin())						|| !begin)		state++; break;
		case RUNNING:		if ((r=iter()) && actual >= total	|| !iter)		state++; break;
		case STOPPING:		if ((r=end())						|| !end)		state++; break;
		case INSPECTING:	if ((r=inspect())					|| !inspect)	state++; break;
	}
	core->serialization_lock.Leave();
	
	if (begin_state < state || ts.Elapsed() >= 5 * 60 * 1000) {
		core->StoreCache();
		ts.Reset();
	}
	
	return r;
}

String Job::GetStateString() const {
	switch (state) {
		case INIT: return "Init";
		case RUNNING: return "Running";
		case STOPPING: return "Stopping";
		case INSPECTING: return "Inspecting";
		case STOPPED: return "Finished";
	}
	return "";
}

void System::InspectionFailed(const char* file, int line, int symbol, int tf, String msg) {
	LOCK(inspection_lock) {
		bool found = false;
		
		for(int i = 0; i < inspection_results.GetCount(); i++) {
			InspectionResult& r = inspection_results[i];
			if (r.file == file &&
				r.line == line &&
				r.symbol == symbol &&
				r.tf == tf &&
				r.msg == msg) {
				found = true;
				break;
			}
		}
		
		if (!found) {
			InspectionResult& r = inspection_results.Add();
			r.file		= file;
			r.line		= line;
			r.symbol	= symbol;
			r.tf		= tf;
			r.msg		= msg;
		}
	}
}

void System::StoreCores() {
	for (auto& s : data) {
		for (auto& t : s) {
			for (auto& f : t) {
				for (auto& ci : f) {
					if (!ci.core.IsEmpty())
						ci.core->StoreCache();
				}
			}
		}
	}
}

bool System::RefreshReal() {
	Time now				= GetUtcTime();
	int wday				= DayOfWeek(now);
	Time after_3hours		= now + 3 * 60 * 60;
	int wday_after_3hours	= DayOfWeek(after_3hours);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return true;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_3hours == 6) {
		WhenInfo("Closing all orders before market break");
		
		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}
		
		mt.SignalOrders(true);
		return true;
	}
	
	
	WhenInfo("Updating MetaTrader");
	WhenPushTask("Putting latest signals");
	
	// Reset signals
	#if 0
	bool is_calendar_speak = GetCalendar().IsMajorSpeak();
	bool is_calendar_fail  = GetCalendar().IsError();
	if (realtime_count == 0 || is_calendar_speak || is_calendar_fail)
	#else
	if (realtime_count)
	#endif
	{
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		//mt.RefreshLimits();
		int open_count = 0;
		const int MAX_SYMOPEN = 8;
		const double FMLEVEL = 0.6;
		
		for (int sym_id = 0; sym_id < GetSymbolCount(); sym_id++) {
			int sig = signals[sym_id];
			int prev_sig = mt.GetSignal(sym_id);
			
			if (sig == prev_sig && sig != 0)
				mt.SetSignalFreeze(sym_id, true);
			else {
				if ((!prev_sig && sig) || (prev_sig && sig != prev_sig)) {
					if (open_count >= MAX_SYMOPEN)
						sig = 0;
					else
						open_count++;
				}
				
				mt.SetSignal(sym_id, sig);
				mt.SetSignalFreeze(sym_id, false);
			}
			LOG("Real symbol " << sym_id << " signal " << sig);
		}
		
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale(MAX_SYMOPEN);
		mt.SignalOrders(true);
	}
	catch (UserExc e) {
		LOG(e);
		return false;
	}
	catch (...) {
		return false;
	}
	
	
	WhenRealtimeUpdate();
	WhenPopTask();
	
	return true;
}

}
