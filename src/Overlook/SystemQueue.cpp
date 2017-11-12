#include "Overlook.h"

namespace Overlook {

void System::InitRegistry() {
	ASSERT(regs.IsEmpty());
	ASSERT_(System::GetCtrlFactories().GetCount() > 0, "Recompile Overlook.icpp to fix this stupid and weird problem");
	
	// Register factories
	for(int i = 0; i < System::GetCtrlFactories().GetCount(); i++) {
		// unfortunately one object must be created, because IO can't be static and virtual at the same time and it is cleaner to use virtual.
		One<Core> core = System::GetCtrlFactories()[i].b();
		core->base = this;
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
		LOG(Format("%d: sym=%d tf=%d factory=%d hash=%d", i, ci.sym, ci.tf, ci.factory, ci.hash));
	}
	
	return 0;
}

int System::GetCoreQueue(Vector<FactoryDeclaration>& path, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids) {
	const int factory = path.Top().factory;
	const int tf_count = GetPeriodCount();
	const int sym_count = GetTotalSymbolCount();
	const int factory_count = GetFactoryCount();
	
	
	Vector<FactoryHash> input_hashes;
	
	// Loop inputs of the factory
	const FactoryRegister& reg = regs[factory];
	
	// Connect input sources
	// Loop all inputs of the custom core-class
	Index<int> sub_sym_ids;
	for (int l = input_hashes.GetCount(); l < reg.in.GetCount(); l++) {
		const RegisterInput& input = reg.in[l];
		ASSERT(input.factory >= 0);
		FilterFunction fn = (FilterFunction)input.data;
		int h = 0;
		
		// If equal timeframe is accepted as input
		int used_tf = -1;
		if (fn(this, -1, tf, -1, tf))
			used_tf = tf;
		else {
			for(int i = 0; i < tf; i++) {
				if (fn(this, -1, tf, -1, i)) {
					used_tf = i;
					break;
				}
			}
		}
		
		if (used_tf != -1) {
			
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
				path.Add().Set(input.factory);
				h = GetCoreQueue(path, ci_queue, used_tf, sub_sym_ids);
				path.Pop();
			}
		}
		
		input_hashes.Add(FactoryHash(input.factory, h));
	}
	
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
	
	
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		
		// Get CoreItem
		CoreItem& ci = data[sym][tf][factory].GetAdd(hash);
		
		// Init object if it was just created
		if (ci.sym == -1) {
			int path_priority = 0;//GetPathPriority(path);
			
			ci.sym = sym;
			ci.tf = tf;
			ci.priority = // lower value is more important
				
				// Faster tf is most important in this system.
				(tf * factory_count +
				
				// Factory might require all symbols, so it is more important than symbol.
				factory) * sym_count +
				
				// Lower symbols must be processed before higher, because cores are allowed to
				// require lower id symbols in the same timeframe and same structural column.
				sym;
			
			ci.factory = factory;
			ci.hash = hash;
			ci.input_hashes <<= input_hashes;
			ci.args <<= args;
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
		int factory = ci.input_hashes[l].a;
		int hash = ci.input_hashes[l].b;
		
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

void System::ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash) {
	Vector<int> symlist, tflist;
	const RegisterInput& input = regs[ci.factory].in[input_id];
	const int sym_count = GetTotalSymbolCount();
	const int tf_count = GetPeriodCount();
	
	
	FilterFunction fn = (FilterFunction)input.data;
	if (fn) {
		
		// Filter timeframes
		for(int i = 0; i <= ci.tf; i++) {
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
	ci.core = System::GetCtrlFactories()[ci.factory].b();
	Core& c = *ci.core;
	
	// Set attributes
	c.base = this;
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
	if (!skip_storecache)
		ci.core->StoreCache();
	
}

}
