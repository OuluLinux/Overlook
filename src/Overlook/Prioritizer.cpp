#include "Overlook.h"

namespace Overlook {





void Combination::Zero() {
	for(int i = 0; i < value.GetCount(); i++) {
		value[i] = 0;
		//dep_mask[i] = 1;
		//exp_mask[i] = 1;
	}
}

bool Combination::GetValue(int bit) const {
	typedef const byte ConstByte;
	int byt = bit / 8;
	bit = bit % 8;
	ConstByte* b = value.Begin() + byt;
	byte mask = 1 << bit;
	return *b & mask;
}

void Combination::SetSize(int bytes) {
	value		.SetCount(bytes, 0);
	//dep_mask	.SetCount(bytes, 0);
	//exp_mask	.SetCount(bytes, 0);
}

void Combination::SetValue(int bit, bool value) {
	int byt = bit / 8;
	bit = bit % 8;
	byte* b = this->value.Begin() + byt;
	byte mask = 1 << bit;
	if (value)
		*b |= mask;
	else
		*b &= ~mask;
}

void Combination::SetOrValue(int bit, bool value) {
	int byt = bit / 8;
	bit = bit % 8;
	byte* b = this->value.Begin() + byt;
	byte mask = 1 << bit;
	if (value)
		*b |= mask;
}

void RemoveDuplicates(Vector<int>& sorted_vec) {
	for(int i = 0; i < sorted_vec.GetCount(); i++) {
		int a = sorted_vec[i];
		for(int j = i+1; j < sorted_vec.GetCount(); j++) {
			if (a == sorted_vec[j]) {
				sorted_vec.Remove(j);
				j--;
			}
		}
	}
}






Prioritizer::Prioritizer() {
	running = false;
	stopped = true;
	thread_count = 0;
	combination_errors = 0;
	
	
	
}

Prioritizer::~Prioritizer() {
	Stop();
}

void Prioritizer::Init() {
	bs.Init();
	CreateCombination();
	RefreshQueue();
	
	if (0) {
		if (combination_errors) {
			Panic("Found " + IntStr(combination_errors) + " errors. Can't run system.");
		}
		Start();
	}
}

void Prioritizer::Deinit() {
	Stop();
}

void Prioritizer::CreateCombination() {
	ASSERT(combparts.IsEmpty());
	combination_bits = 0;
	ASSERT_(Factory::GetRegs().GetCount() > 0, "Recompile Overlook.icpp to fix this stupid and weird problem");
	
	// Compare input types to output types and add matching pairs to input sources.
	for(int i = 0; i < Factory::GetRegs().GetCount(); i++) {
		const Vector<RegisterInput>& factory_inputs = Factory::GetRegs()[i].in;
		const Vector<ValueType>& factory_outputs = Factory::GetRegs()[i].out;
		
		// One part per slot-factory
		CombinationPart& part = combparts.Add();
		part.input_src.SetCount(factory_inputs.GetCount());
		part.begin = combination_bits;
		
		combination_bits++; // enabled bit
		combination_bits++; // all symbols enabled bit
		combination_bits++; // all timeframes enabled bit
		
		// Add inputs
		for(int i2 = 0; i2 < factory_inputs.GetCount(); i2++) {
			const RegisterInput& input = factory_inputs[i2];
			part.inputs.Add(input);
			
			// Find output to input.
			// Limit range to input position, because it simplifies dependency mask and this
			// way some wrong combinations can be easily avoided. Be sure to register slots in
			// correct order.
			for(int j = 0; j < i; j++) {
				const Vector<ValueType>& factory_outputs = Factory::GetRegs()[j].out;
				
				for(int k = 0; k < factory_outputs.GetCount(); k++) {
					const ValueType& output = factory_outputs[k];
					
					// If one of outputs of the registered slot factory is equal to input
					if (output.type == input.type && output.phase == input.phase) {
						
						part.input_src[i2].Add(IntPair(j, k));
						combination_bits++;
					}
				}
			}
		}
		// Add outputs
		for(int i2 = 0; i2 < factory_outputs.GetCount(); i2++) {
			const ValueType& output = factory_outputs[i2];
			part.outputs.Add(output);
		}
		
		part.end = combination_bits;
		part.size = part.end - part.begin;
		
		// Check if slot can branch with different sources
		part.single_sources = true;
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			if (part.input_src[j].GetCount() > 1) {
				part.single_sources = false;
				break;
			}
		}
	}
	
	// Connect input source slots to their enabled bits
	inputs_to_enabled.SetCount(combination_bits, -1);
	enabled_to_factory.SetCount(combination_bits, -1);
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<IntPair>& src = part.input_src[j];
			for(int k = 0; k < src.GetCount(); k++) {
				const IntPair& ip = src[k];
				int input_bit = GetBitCore(i, j, k);
				int enabled_bit = GetBitEnabled(ip.a);
				inputs_to_enabled[input_bit] = enabled_bit;
				enabled_to_factory[enabled_bit] = ip.a;
			}
		}
	}
	
	
	job_combination_bytes = combination_bits / 8 + (combination_bits % 8 != 0 ? 1 : 0);
	
	combination_bits += 200 + 10; // max symbols and timeframes
	combination_bytes = combination_bits / 8 + (combination_bits % 8 != 0 ? 1 : 0);
	
	// Print stuff
	LOG("Combination size: " << combination_bits << " bits, " << combination_bytes << " bytes");
	combination_errors = 0;
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		LOG("    part " << i << ": \"" << Factory::GetCtrlFactories()[i].a << "\" begin=" << part.begin << " size=" << part.size << " single-source=" << (int)part.single_sources);
		
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<IntPair>& inputs = part.input_src[j];
			if (inputs.IsEmpty()) {
				const RegisterInput& input = Factory::GetRegs()[i].in[j];
				if (input.input_type == REGIN_SLOWER) {
					LOG("         (same class, slower tf instances)");
				}
				else {
					LOG("        ERROR: input not found: " << part.inputs[j].ToString());
					combination_errors++;
				}
			} else {
				String str;
				for(int k = 0; k < inputs.GetCount(); k++) {
					const IntPair& ip = inputs[k];
					str << " (" << ip.a << "," << ip.b << ")";
				}
				LOG("        " << str);
			}
		}
	}
	if (combination_errors) {
		Panic("Found " + IntStr(combination_errors) + " errors. Can't run system.");
	}
}


void Prioritizer::RefreshQueue() {
	lock.Enter();
	
	// Create queue from last slot to first
	CreateNormal();
	
	
	// Move slot-objects from previous slot-queue to the new one and remove those which aren't
	// in the new. Store them to hard drive without deleting...
	
	
	
	// Create new slot-objects to the slot-queue and load previous data from hard drive.
	
	
	// Sort queues based on priorities
	
	
	lock.Leave();
}

void Prioritizer::Run() {
	ASSERT(!thread_count);
	thread_count = CPU_Cores();
	for(int i = 0; i < thread_count; i++)
		Thread::Start(THISBACK1(ProcessThread, i));
	
	while (running) {
		
		
		
		Sleep(100);
	}
	
	while (thread_count > 0) Sleep(100);
	stopped = true;
}

void Prioritizer::Process() {
	ASSERT_(!job_queue.IsEmpty(), "Job-queue must be created first");
	
	for(int i = 0; i < job_queue.GetCount(); i++) {
		LOG(i << "/" << job_queue.GetCount());
		JobItem& ji = job_queue[i];
		//LOG("Job " << i);
		//LOG(GetCombinationString(ji.value));
		Process(ji);
	}
}

void Prioritizer::ProcessThread(int thread_id) {
	while (running) {
		int i = cursor++;
		
		if (i >= job_queue.GetCount()) {
			LOG("ERROR: queue finished");
			Sleep(1000);
			continue;
		}
		
		JobItem& ji = job_queue[i];
		
		LOG("Process " << i << ", thread-id " << thread_id);
		
		Process(ji);
		
		
		Sleep(100);
	}
	
	thread_count--;
}

void Prioritizer::Process(JobItem& ji) {
	
	// Load dependencies to the scope
	if (!ji.core)
		CreateJobCore(ji);
	
	// Process core-object
	ji.core->Refresh();
	
	// Store cache file
	ji.core->StoreCache();
	
}

void Prioritizer::CreateJobCore(JobItem& ji) {
	ASSERT(ji.core == NULL);
	const CombinationPart& part = combparts[ji.factory];
	Vector<int> enabled_input_factories;
	Vector<byte> unique_slot_comb;
	
	
	// Create core-object
	ji.core = Factory::GetCtrlFactories()[ji.factory].b();
	
	// Set attributes
	ji.core->base = &bs;
	ji.core->factory = ji.factory;
	ji.core->RefreshIO();
	ji.core->SetUnique(ji.unique);
	ji.core->SetSymbol(ji.sym);
	ji.core->SetTimeframe(ji.tf, bs.GetPeriod(ji.tf));
	ji.core->LoadCache();
	
	// Connect input sources
	// Loop all inputs of the custom core-class
	for (int l = 0; l < part.input_src.GetCount(); l++) {
		const RegisterInput& input = part.inputs[l];
		FilterFunction fn = (FilterFunction)input.data;
		
		bool input_all_sym = input.scale == Sym || input.scale == All;
		bool input_all_tf  = input.scale == Tf  || input.scale == All;
		bool input_dynamic = input.input_type == REGIN_DYNAMIC && fn != NULL;
		bool input_slower = input.input_type == REGIN_SLOWER;
		
		// Loop possible sources for one input
		const Vector<IntPair>& input_src = part.input_src[l];
		#ifdef flagDEBUG
		int src_count = 0; // count enabled sources for debugging (1 is correct)
		int enabled_count = 0; // exactly one must be enabled
		#endif
		enabled_input_factories.SetCount(0);
		for (int m = 0; m < input_src.GetCount(); m++) {
			
			// Check if source is enabled
			const IntPair& src = input_src[m]; // src.a = factory, src.b = it's output id
			int src_enabled_bit = GetBitCore(ji.factory, l, m);
			bool src_enabled = ReadBit(ji.value, src_enabled_bit);
			if (!src_enabled) continue;
			
			#ifdef flagDEBUG
			int bit = InputToEnabled(src_enabled_bit);
			int enabled_bit = GetBitEnabled(src.a);
			ASSERT(bit == enabled_bit);
			ASSERT(ReadBit(ji.value, bit));
			#endif
			
			enabled_count++;
			enabled_input_factories.Add(src.a);
			/*DUMP(src);
			
			if (ji.factory == 2) {
				LOG("");
			}*/
			
			// Create unique combination for current input source factory
			CreateUniqueCombination(ji.value, src.a, unique_slot_comb);
			ASSERT(unique_slot_comb.GetCount() == job_combination_bytes);
			
			
			// Search source and loop existing job queue
			bool found = false;
			int input_count = 0;
			bool is_one_input_only = !input_all_sym && !input_all_tf && !input_dynamic && !input_slower;
			bool is_many_to_many_exceptions_allowed = input_all_sym && input_all_tf;
			for (int n = 0; n < job_queue.GetCount(); n++) {
				JobItem& src_ji = job_queue[n];
				
				/*LOG(src_ji.factory << " != " << src.a << "\t" <<
					src_ji.sym << " != " << ji.sym << "\t" <<
					src_ji.tf << " != " << ji.tf);*/
				
				// Factory must match
				if (src_ji.factory != src.a) continue;
				
				
				// With dynamic input, only the return value of the filter function matters.
				if (input_dynamic) {
					bool keep = fn(&bs, ji.sym, ji.tf, src_ji.sym, src_ji.tf);
					if (!keep) continue;
				}
				// With slower tf inputs, all slower tfs are kept
				else if (input_slower) {
					if (!input_all_sym && !src_ji.all_sym && src_ji.sym != ji.sym) continue;
					if (src_ji.tf <= ji.tf) continue;
				}
				// Normally, only symbol & timeframe are being matched.
				else {
					// If input takes all sym&tf, the symbol doesn't need to match
					// If source gives all sym&tf, the symbol doesn't need to match
					if (!input_all_sym && !src_ji.all_sym && src_ji.sym != ji.sym) continue;
					if (!input_all_tf  && !src_ji.all_tf  && src_ji.tf  != ji.tf ) continue;
				}
				
				bool equal = true;
				for (int o = 0; o < job_combination_bytes; o++) {
					if (unique_slot_comb[o] != src_ji.value[o]) {
						equal = false;
						break;
					}
				}
				if (!equal)	{
					/*LOG("DIFFERENT COMBOS");
					LOG("SRC:");
					LOG(GetCombinationString(ji.value));
					LOG("A:");
					LOG(GetCombinationString(unique_slot_comb));
					LOG("B:");
					LOG(GetCombinationString(src_ji.value));*/
					continue;
				}
				
				#ifdef flagDEBUG
				// Check for multiple types of inputs
				if (!is_many_to_many_exceptions_allowed &&
					!is_one_input_only &&
					(src_ji.all_sym || src_ji.all_tf) &&
					input_count > 0)
					Panic("ERROR: multiple many-to-many or one-to-many, but input can take only one");
				#endif
				
				
				// Source found
				ASSERT(src_ji.core);
				ji.core->AddInput(l, src_ji.sym, src_ji.tf, *src_ji.core, src.b);
				
				
				input_count++;
				found = true;
				if (is_one_input_only)
					break;
			}
			ASSERT_(is_one_input_only || input_count > 1 || input_dynamic, "Couldn't find multiple inputs");
			
			if (found) {
				// Catch invalid combinations
				#ifdef flagDEBUG
				src_count++;
				#endif
				break;
			}
		}
		#ifdef flagDEBUG
		if (enabled_count == 0) {
			LOG("Combination:");
			LOG(GetCombinationString(ji.value));
			DUMPC(job_queue);
			/*LOG("Checking all source pipeline combinations");
			for(int i = 0; i < ci.pipeline_src.GetCount(); i++) {
				const PipelineItem& pi = pl_queue[ci.pipeline_src[i]];
				CheckCombination(pi.value);
			}*/
		}
		ASSERT_(enabled_count > 0, "Combination is invalid: not a single source is enabled");
		ASSERT_(enabled_count < 2, "Combination is invalid: too many sources are enabled");
		if (src_count == 0 && !input_dynamic) {
			//LOG(GetCombinationString(ci.value));
			String inputs = "\"";
			for(int i = 0; i < enabled_input_factories.GetCount(); i++) {
				if (i) inputs.Cat(',');
				inputs << Factory::GetCtrlFactories()[enabled_input_factories[i]].a;
			}
			inputs << "\"";
			Panic("Creating object \"" +
				Factory::GetCtrlFactories()[ji.factory].a +
				"\" and can't find any of " +
				inputs +
				" inputs.");
			ASSERT_(src_count > 0, "Didn't find earlier job to connect as input. Maybe some core class has no input sources");
		}
		#endif
	}
	
	//ji.core->SetArguments(*args);
	
	// Initialize
	ji.core->InitAll();
	
}










bool Prioritizer::IsBegin() const {
	return results.GetCount() < 100; // TODO: fix
}

int Prioritizer::GetBitTf(int tf_id) const {
	ASSERT(tf_id >= 0 && tf_id < 10);
	int begin = combparts.Top().end;
	return begin + 200 + tf_id;
}

int Prioritizer::GetBitSym(int sym_id) const {
	ASSERT(sym_id >= 0 && sym_id < 200);
	int begin = combparts.Top().end;
	return begin + sym_id;
}

int Prioritizer::GetBitCore(int fac_id, int input_id, int src_id) const {
	const CombinationPart& part = combparts[fac_id];
	int begin = part.begin;
	
	begin++; // enabled bit
	begin++; // all symbols enabled bit
	begin++; // all timeframes enabled bit
	
	for(int i = 0; i < input_id; i++) {
		begin += part.input_src[i].GetCount();
	}
	#ifdef flagDEBUG
	const Vector<IntPair>& src = part.input_src[input_id];
	ASSERT(src_id >= 0 && src_id < src.GetCount());
	#endif
	return begin + src_id;
}

int Prioritizer::GetBitEnabled(int fac_id) const {
	return combparts[fac_id].begin;
}

int Prioritizer::GetBitAllSymbols(int fac_id) const {
	return combparts[fac_id].begin + 1;
}

int Prioritizer::GetBitAllTimeframes(int fac_id) const {
	return combparts[fac_id].begin + 2;
}

int Prioritizer::InputToEnabled(int bit) const {
	return inputs_to_enabled[bit];
}

int Prioritizer::EnabledToFactory(int bit) const {
	return enabled_to_factory[bit];
}




















void Prioritizer::CreateNormal() {
	
	// Skip all bad exports based on QueryTable reading results vector.
	// Add random combinations based on Randomf() < exploration_epsilon.
	// Add all meaningful combinations if the result-vector is too small for QueryTable.
	pl_queue.Clear();
	Combination comb;
	comb.SetSize(combination_bytes);
	Vector<int> factory_queue;
	factory_queue.Add(Factory::GetCtrlFactories().GetCount()-1);
	VisitSymTf(comb, factory_queue);
	
	
	// Create unique-slot-queue from the whole pipeline-queue.
	RefreshCoreQueue();
	
	
	// Create job-queue from unique-slot-queue
	RefreshJobQueue();
	
}

void Prioritizer::CreateSingle(int main_fac_id, int sym_id, int tf_id) {
	
	// Visit only given symbol
	pl_queue.Clear();
	Combination comb;
	comb.SetSize(combination_bytes);
	VisitSymTf(main_fac_id, sym_id, tf_id, comb);
	
	
	// Create unique-slot-queue from the whole pipeline-queue.
	RefreshCoreQueue();
	
	
	// Create job-queue from unique-slot-queue
	RefreshJobQueue();
	
}

void Prioritizer::VisitSymTf(Combination& comb, Vector<int>& factory_queue) {
	
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount();
	ASSERT(sym_count > 0 && tf_count > 0);
	
	if (IsBegin()) {
		
		// Find EURUSD and add smallest timeframe
		int tf_id = 0;
		int sym_id = 0;
		for(int i = 0; i < sym_count; i++) {
			if (bs.GetSymbol(i) == "EURUSD") {
				sym_id = i;
				break;
			}
		}
		
		// Set only that enabled in the combination vector
		comb.Zero();
		comb.SetValue(GetBitTf(tf_id), true);
		comb.SetValue(GetBitSym(sym_id), true);
		
	} else {
		typedef Tuple3<int, int, int> Priority;
		Vector<Priority> priorities;
		
		for(int i = 0; i < tf_count; i++) {
			for(int j = 0; j < sym_count; j++) {
				
				// Get priority from QueryTable
				int priority = 0;
				Panic("TODO");
				
				priorities.Add(Priority(i, j, priority));
			}
		}
		
		struct PrioritySorter {
			bool operator()(const Priority& a, const Priority& b) const {
				return a.c < b.c;
			}
		};
		Sort(priorities, PrioritySorter());
		
	}
	
	// Enable the last factory and visit it
	int factory = factory_queue[0];
	int factory_enabled = GetBitEnabled(factory);
	comb.SetValue(factory_enabled, true);
	VisitCombination(comb, factory_queue);
}

void Prioritizer::VisitSymTf(int fac_id, int sym_id, int tf_id, Combination& comb) {
	
	// Set only that enabled in the combination vector
	comb.Zero();
	comb.SetValue(GetBitTf(tf_id), true);
	comb.SetValue(GetBitSym(sym_id), true);
	
	// Visit the given factory
	int factory_enabled = GetBitEnabled(fac_id);
	comb.SetValue(factory_enabled, true);
	Vector<int> factory_queue;
	factory_queue.Add(fac_id);
	VisitCombination(comb, factory_queue);
}

void Prioritizer::VisitCombination(Combination& comb, Vector<int>& factory_queue) {
	
	// Visit combination until source slot
	if (!factory_queue.IsEmpty()) {
		int factory = factory_queue[0];
		factory_queue.Remove(0);
		
		// 'Enable all' bits must be recursively delivered to other factories
		bool all_symbols_enabled = comb.GetValue(GetBitAllSymbols(factory));
		bool all_timeframes_enabled = comb.GetValue(GetBitAllTimeframes(factory));
		
		
		const CombinationPart& part = combparts[factory];
		
		// Check, that slots are false
		#ifdef flagDEBUG
		ASSERT(comb.GetValue(GetBitEnabled(factory)) == true);
		for(int i = 0; i < part.input_src.GetCount(); i++) {
			const Vector<IntPair>& src = part.input_src[i];
			for(int j = 0; j < src.GetCount(); j++) {
				ASSERT(comb.GetValue(GetBitCore(factory, i, j)) == false);
			}
		}
		#endif
		
		// If inputs has multiple sources
		if (!part.single_sources) {
			
			// If query-table method can't be used yet, due the lack of results.
			if (IsBegin()) {
				
				
				// Copy existing.
				Combination comb2 = comb;
				
				// Add all minimal cases
				int case_count = 0;
				for(int i = 0; i < part.input_src.GetCount(); i++) {
					const RegisterInput& in = part.inputs[i];
					const Vector<IntPair>& src_list = part.input_src[i];
					FilterFunction filter = (FilterFunction)in.data;
					
					// Skip all optional inputs in the initial minimal phase.
					if (in.input_type == REGIN_OPTIONAL)
						continue;
					
					// Set all inputs with only one input source to that source.
					if (src_list.GetCount() == 1) {
						
						// Set 'enabled' bit
						int bit = GetBitCore(factory, i, 0);
						comb2.SetValue(bit, true);
						bit = InputToEnabled(bit);
						comb2.SetValue(bit, true);
						ASSERT(bit == GetBitEnabled(src_list[0].a));
						int src_factory = EnabledToFactory(bit);
						
						// Set 'activate all symbols/timeframes' bit
						comb2.SetOrValue(GetBitAllSymbols(src_factory),    all_symbols_enabled    || in.scale == Sym || in.scale == All);
						comb2.SetOrValue(GetBitAllTimeframes(src_factory), all_timeframes_enabled || in.scale == Tf  || in.scale == All);
					}
					// (else) Multiple sources are handled later with sub-combinations
					ASSERT(src_list.GetCount() != 0);
				}
				
				
				// Find the ranges for sub-combination
				typedef Tuple3<int, int, int> InpMaxSrc; // input-id, source-count, current source
				Vector<InpMaxSrc> sub_comb;
				for(int i = 0; i < part.input_src.GetCount(); i++) {
					const Vector<IntPair>& src = part.input_src[i];
					
					if (src.GetCount() > 1) {
						sub_comb.Add(InpMaxSrc(i, src.GetCount(), 0));
					}
				}
				int sub_comb_count = sub_comb.GetCount();
				ASSERT(sub_comb_count > 0);
				
				
				// Iterate all sub-combinations
				bool looping = true;
				while (looping) {
					
					// Copy existing with fixed flags
					Combination comb3 = comb2;
					Vector<int> comb_factory_queue;
					comb_factory_queue <<= factory_queue;
					
					// Set current combination values
					for(int i = 0; i < sub_comb_count; i++) {
						const InpMaxSrc& s = sub_comb[i];
						
						// Set 'enabled' bit
						int bit = GetBitCore(factory, s.a, s.c);
						comb3.SetValue(bit, true);
						bit = InputToEnabled(bit);
						comb3.SetValue(bit, true);
						int src_factory = EnabledToFactory(bit);
						comb_factory_queue.Add(src_factory);
						
						// Set 'activate all symbols/timeframes' bit
						const RegisterInput& input = part.inputs[s.a];
						comb3.SetOrValue(GetBitAllSymbols(src_factory),    all_symbols_enabled    || input.scale == Sym || input.scale == All);
						comb3.SetOrValue(GetBitAllTimeframes(src_factory), all_timeframes_enabled || input.scale == Tf  || input.scale == All);
						
					}
					Sort(comb_factory_queue, StdGreater<int>());
					RemoveDuplicates(comb_factory_queue);
					
					
					// Visit new combination
					VisitCombination(comb3, comb_factory_queue);
					case_count++;
					
					
					// Increase combination by one
					for(int i = 0; i < sub_comb_count; i++) {
						InpMaxSrc& s = sub_comb[i];
						s.c++; // increase current by one
						// if current more than maximum
						if (s.c == s.b) {
							s.c = 0; // reset current
							// if this was the last number, don't start over, but break the loop
							if (i == sub_comb_count-1) {
								looping = false;
								break;
							}
						}
						else break;
					}
				}
				ASSERT(case_count > 0);
			}
			else {
				
				// Limit results to current end masked combination
				Panic("TODO");
				
				// Use QueryTable to prune useless combinations
				
				VisitCombination(comb, factory_queue);
				
			}
		}
		
		// With single input add that
		else {
			// Copy existing.
			Combination comb2 = comb;
			
			for(int i = 0; i < part.input_src.GetCount(); i++) {
				if (part.input_src[i].IsEmpty()) continue;
				
				// Set slot value true
				int bit = GetBitCore(factory, i, 0);
				comb2.SetValue(bit, true);
				bit = InputToEnabled(bit);
				comb2.SetValue(bit, true);
				int src_factory = EnabledToFactory(bit);
				factory_queue.Add(src_factory);
				
				// Set 'activate all symbols/timeframes' bit
				const RegisterInput& input = part.inputs[i];
				comb2.SetOrValue(GetBitAllSymbols(src_factory),    all_symbols_enabled    || input.scale == Sym || input.scale == All);
				comb2.SetOrValue(GetBitAllTimeframes(src_factory), all_timeframes_enabled || input.scale == Tf  || input.scale == All);
				
			}
			
			Sort(factory_queue, StdGreater<int>());
			RemoveDuplicates(factory_queue);
			VisitCombination(comb2, factory_queue);
		}
		
		
	}
	// Add combination to the queue
	else {
		#ifdef flagDEBUG
		if (!CheckCombination(comb.value)) {
			LOG(GetCombinationString(comb.value));
			Panic("Combination failed check");
		}
		#endif
		int priority = pl_queue.GetCount();
		PipelineItem& pi = pl_queue.Add();
		pi.value <<= comb.value;
		pi.priority = priority;
	}
}

void Prioritizer::RefreshCoreQueue() {
	slot_queue.Clear();
	
	
	Vector<byte> unique_slot_comb;
	int fac_count = Factory::GetCtrlFactoryCount();
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount();
	
	
	// Loop all enabled pipeline-combinations
	for(int i = 0; i < pl_queue.GetCount(); i++) {
		PipelineItem& pi = pl_queue[i];
		ASSERT(!pi.value.IsEmpty());
		
		// Combination consist of all factories + sym + tf
		// Loop all factories from begin to the end
		for(int j = 0; j < fac_count; j++) {
			const CombinationPart& part = combparts[j];
			
			// Skip factories, which are disabled in the combination
			int enabled_bit = GetBitEnabled(j);
			bool enabled = ReadBit(pi.value, enabled_bit);
			if (!enabled) continue;
			
			// Check if some input requires all symbols and  timeframes
			bool input_all_sym = ReadBit(pi.value, GetBitAllSymbols(j));
			bool input_all_tf =  ReadBit(pi.value, GetBitAllTimeframes(j));
			
			
			// Here, the 'part' is the active factory in the combination.
			// Dependency mask has all earlier factories enabled, which this active factory needs.
			// We can't just collect all enabled factories until this point, because some of
			// them are not needed by this factory. They must be searched separately, which is
			// done next by creating unique combination from this factory and it's dependencies
			// and by comparing it to existing core-queue.
			CreateUniqueCombination(pi.value, j, unique_slot_comb);
			
			// Loop existing core-queue and check if the current slot is in the queue already.
			bool found = false;
			for(int k = 0; k < slot_queue.GetCount(); k++) {
				CoreItem& si = slot_queue[k];
				
				// Compare the unique combination
				bool equal = true;
				for(int i = 0; i < combination_bytes; i++) {
					if (si.value[i] != unique_slot_comb[i]) {
						equal = false;
						break;
					}
				}
				
				// If the combination is same
				if (equal) {
					found = true;
					
					// Add pipeline-item to iniator list (which is the list of reasons why the item was added)
					si.pipeline_src.Add(i);
					
					// Add priorities of symbols and timeframes
					for (int s = 0; s < sym_count; s++) {
						if (input_all_sym || ReadBit(pi.value, GetBitSym(s))) {
							si.symlist.FindAdd(s);
						}
					}
					for (int t = 0; t < tf_count; t++) {
						if (input_all_tf || ReadBit(pi.value, GetBitTf(t))) {
							si.tflist.FindAdd(t);
						}
					}
					break;
				}
			}
			
			
			// If combination wasn't added already to the core-queue, then it is original and
			// it can be added to the core-queue.
			if (!found) {
				CoreItem& si = slot_queue.Add();
				si.pipeline = &pi;
				si.value <<= unique_slot_comb;
				si.factory = j;
				si.priority = pi.priority * Factory::GetCtrlFactories().GetCount() + j;
				si.pipeline_src.Add(i);
				
				// Add priorities of symbols and timeframes
				for (int s = 0; s < sym_count; s++)
					if (input_all_sym || ReadBit(pi.value, GetBitSym(s)))
						si.symlist.Add(s, pi.priority); // Note: adding to empty vector, no min() required.
				for (int t = 0; t < tf_count; t++)
					if (input_all_tf || ReadBit(pi.value, GetBitTf(t)))
						si.tflist.Add(t, pi.priority);
			}
		}
	}
	
	
	
	struct PrioritySorter {
		bool operator()(const CoreItem& a, const CoreItem& b) const {
			return a.priority < b.priority;
		}
	};
	Sort(slot_queue, PrioritySorter());
	
	
	// Find which factories has dynamic inputs for fast checking
	Vector<bool> has_dynamic;
	has_dynamic.SetCount(Factory::GetRegs().GetCount(), false);
	for(int i = 0; i < Factory::GetRegs().GetCount(); i++) {
		const FactoryValueRegister& reg = Factory::GetRegs()[i];
		for(int j = 0; j < reg.in.GetCount(); j++) {
			const RegisterInput& input = reg.in[j];
			if (input.input_type == REGIN_DYNAMIC) {
				has_dynamic[i] = true;
			}
		}
	}
	
	
	// Queue for dynamic sym/tf additions
	Vector<int> process_slot_queue;
	for(int i = slot_queue.GetCount()-1; i >= 0; i--)
		if (has_dynamic[slot_queue[i].factory])
			process_slot_queue.Add(i);
	
	
	// Loop queue of dynamic sym/tf additions
	for(;process_slot_queue.GetCount();) {
		int i = process_slot_queue.Pop();
		
		const CoreItem& ci = slot_queue[i];
		const CombinationPart& part = combparts[ci.factory];
		const FactoryValueRegister& reg = Factory::GetRegs()[ci.factory];
		
		for(int j = 0; j < reg.in.GetCount(); j++) {
			const RegisterInput& in = reg.in[j];
			
			Index<int> add_sym, add_tf;
			
			if (in.input_type == REGIN_DYNAMIC) {
				FilterFunction filter = (FilterFunction)in.data;
				
				for(int k = 0; k < ci.symlist.GetCount(); k++) {
					int sym = ci.symlist[k];
					for(int l = 0; l < sym_count; l++) {
						if (filter(&bs, sym, -1, l, -1)) {
							add_sym.FindAdd(l);
						}
					}
				}
				
				for(int k = 0; k < ci.tflist.GetCount(); k++) {
					int tf = ci.tflist[k];
					for(int l = 0; l < tf_count; l++) {
						if (filter(&bs, -1, tf, -1, l)) {
							add_tf.FindAdd(l);
						}
					}
				}
			}
			else if (in.input_type == REGIN_SLOWER) {
				add_sym <<= ci.symlist;
				add_tf  <<= ci.tflist;
				int slowest_tf = INT_MAX;
				for(int k = 0; k < add_tf.GetCount(); k++)
					slowest_tf = Upp::min(slowest_tf, add_tf[k]);
				if (slowest_tf != INT_MAX) {
					for(int k = slowest_tf+1; k < tf_count; k++) {
						// Only add timeframes that are enabled in the pipeline combination
						if (!ReadBit(ci.pipeline->value, GetBitTf(k)))
							continue;
						add_tf.FindAdd(k);
					}
				}
			}
			else if (in.input_type == REGIN_NORMAL) {
				add_sym <<= ci.symlist;
				add_tf  <<= ci.tflist;
			}
			else Panic("TODO: dynamic input is undone yet");
			
			if (add_sym.IsEmpty() && add_tf.IsEmpty())
				continue;
			
			int factory_id = -1;
			const Vector<IntPair>& input_src = part.input_src[j];
			for(int k = 0; k < input_src.GetCount(); k++) {
				int src_enabled_bit = GetBitCore(ci.factory, j, k);
				if (ReadBit(ci.value, src_enabled_bit)) {
					factory_id = EnabledToFactory(InputToEnabled(src_enabled_bit));
					break;
				}
			}
			ASSERT_(factory_id != -1, "Some input must be enabled");
			
			Vector<byte> unique_slot_comb;
			CreateUniqueCombination(ci.value, factory_id, unique_slot_comb);
			
			bool found = false;
			int prev_slot_queue_count = process_slot_queue.GetCount();
			for(int k = 0; k < i; k++) {
				CoreItem& src_ci = slot_queue[k];
				const CombinationPart& part_cmp = combparts[src_ci.factory];
				
				bool equal = true;
				for (int o = 0; o < job_combination_bytes; o++) {
					if (unique_slot_comb[o] != src_ci.value[o]) {
						equal = false;
						break;
					}
				}
				if (!equal)	continue;
				
				found = true;
				bool reprocess = false;
				for(int l = 0; l < add_sym.GetCount(); l++) {
					int sym = add_sym[l];
					int m = src_ci.symlist.Find(sym);
					if (m == -1) {
						src_ci.symlist.Add(sym);
						reprocess = true;
					}
				}
				
				for(int l = 0; l < add_tf.GetCount(); l++) {
					int tf = add_tf[l];
					int m = src_ci.tflist.Find(tf);
					if (m == -1) {
						src_ci.tflist.Add(tf);
						reprocess = true;
					}
				}
				
				if (reprocess) {
					process_slot_queue.Insert(prev_slot_queue_count, k); // last must be lowest. avoid sorting with this
				}
				
				break;
			}
			ASSERT(found);
		}
		
	}
	
	LOG("Found " << slot_queue.GetCount() << " unique slots");
}

void Prioritizer::RefreshJobQueue() {
	
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount();
	int total = sym_count * tf_count;
	
	Vector<JobItem> new_job_queue;
	Vector<JobItem>& old_job_queue = this->job_queue;
	
	
	// Loop current unique-slot queue
	for(int i = 0; i < slot_queue.GetCount(); i++) {
		CoreItem& ci = slot_queue[i];
		const CombinationPart& part = combparts[ci.factory];
		
		// Get unique string and trim it
		String unique, unique_long = HexVector(ci.value);
		for (int j = unique_long.GetCount()-1; j >= 0; j--) {
			if (unique_long[j] != '0') {
				unique = unique_long.Left(j+1);
				break;
			}
		}
		ASSERT(!unique.IsEmpty());
		
		// Add room for symbol and timeframe in the priority-number
		int64 priority_base = (int64)ci.priority * (int64)total;
		
		// Loop enabled symbols in the current unique-slot
		//DUMPC(ci.symlist);
		for(int j = 0; j < ci.symlist.GetCount(); j++) {
			int sym = ci.symlist[j];
			
			// Loop enabled timeframes in the current unique-slot
			for(int k = 0; k < ci.tflist.GetCount(); k++) {
				int tf = ci.tflist[k];
				
				// Add new job item for current slot/symbol/tf combination
				JobItem& ji = new_job_queue.Add();
				ji.priority = priority_base + (int64)sym + (int64)(tf_count-1-tf) * (int64)sym_count;
				ji.factory = ci.factory;
				if (!part.outputs.IsEmpty()){
					const ValueType& first_output = part.outputs[0]; // all outputs should have the same scale
					ji.all_sym = first_output.scale == Sym || first_output.scale == All;
					ji.all_tf = first_output.scale == Tf || first_output.scale == All;
				} else {
					ji.all_sym = false;
					ji.all_tf = false;
				}
				ji.unique = unique;
				ji.sym = sym;
				ji.tf = tf;
				
				// Copy only the important part of the unique combination without symbol & timeframe.
				ASSERT(ci.value.GetCount() == combination_bytes);
				ji.value.SetCount(job_combination_bytes);
				for(int l = 0; l < job_combination_bytes; l++)
					ji.value[l] = ci.value[l];
				
				// Assign core-object to the JobItem.
				// Find existing object from previous queue or create and initialize a new one.
				
				// Search existing job-queue for existing core-object.
				bool found = false;
				for (int l = 0; l < old_job_queue.GetCount(); l++) {
					JobItem& old_ji = old_job_queue[l];
					
					// Symbol and timeframe must match. (works also for sym/tf/all types)
					if (old_ji.sym != sym || old_ji.tf != tf) continue;
					
					// Try to match the unique combination
					bool equal = true;
					for(int i = 0; i < job_combination_bytes; i++) {
						if (old_ji.value[i] != ji.value[i]) {
							equal = false;
							break;
						}
					}
					if (!equal) continue;
					
					// JobItem is equal. Move the core-object pointer.
					// Assume that dependencies are being moved also, because their slot should
					// be enabled with the same combination.
					ji.core = old_ji.core;
					old_ji.core = NULL;
					
					found = true;
				}
				
				// If old object wasn't found, create core-object and initialize it
				if (!found) {
					
					//LOG("Create " << Factory::GetCtrlFactories()[ji.factory].a);
					
					// Link permanentcore-object
					if (ji.factory == 0) {
						// Link BaseSystem object
						ji.core = &bs;
					}
				}
			}
		}
	}
	
	// Loop old queue and remove core-objects
	for(int i = 0; i < old_job_queue.GetCount(); i++) {
		JobItem& ji = old_job_queue[i];
		if (ji.core && ji.factory > 0) {
			delete ji.core;
			ji.core = NULL;
		}
	}
	
	// Sort job-queue by priority
	struct PrioritySorter {
		bool operator()(const JobItem& a, const JobItem& b) const {
			return a.priority < b.priority;
		}
	};
	Sort(new_job_queue, PrioritySorter());
	
	LOG("Job queue size: " << new_job_queue.GetCount());
	
	// Set new queue
	Swap(old_job_queue, new_job_queue);
}

bool Prioritizer::CheckCombination(const Vector<byte>& comb) {
	//LOG("\t" << i << ": " << HexVector(comb));
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		if (!ReadBit(comb, GetBitEnabled(i))) continue;
		
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<IntPair>& src_list = part.input_src[j];
			int enabled_count = 0;
			
			for(int k = 0; k < src_list.GetCount(); k++) {
				const IntPair& src = src_list[k];
				
				int bit = GetBitCore(i, j, k);
				if (ReadBit(comb, bit)) {
					enabled_count++;
					bit = InputToEnabled(bit);
					if (!ReadBit(comb, bit)) {
						LOG("ERROR: Source factory is not enabled");
						return false;
					}
				}
			}
			if (!enabled_count) {
				const RegisterInput& input = Factory::GetRegs()[i].in[j];
				if (input.input_type == REGIN_SLOWER) continue;
				
				LOG("ERROR: No enabled sources");
				return false;
			}
		}
	}
	
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount();
	
	bool enabled_sym = false;
	for(int i = 0; i < sym_count; i++) {
		if (ReadBit(comb, GetBitSym(i))) {
			enabled_sym = true;
			break;
		}
	}
	if (!enabled_sym) {
		LOG("ERROR: no symbols are enabled");
		return false;
	}
	
	bool enabled_tf = false;
	for(int i = 0; i < tf_count; i++) {
		if (ReadBit(comb, GetBitTf(i))) {
			enabled_tf = true;
			break;
		}
	}
	if (!enabled_tf) {
		LOG("ERROR: no timeframes are enabled");
		return false;
	}
	
	return true;
}

void Prioritizer::CreateUniqueCombination(const Vector<byte>& src, int fac_id, Vector<byte>& unique_slot_comb) {
	const CombinationPart& part = combparts[fac_id];
	
	int bytes = src.GetCount();
	ASSERT(bytes > 0);
	ASSERT(bytes == combination_bytes || bytes == job_combination_bytes);
	
	Vector<int> depmask_begins, depmask_ends;
	Vector<byte> dep_mask;
	Vector<bool> enabled_fac;
	
	int fac_count = Factory::GetCtrlFactoryCount();
	enabled_fac.SetCount(fac_count);
	
	// Create dependency mask based on combination
	dep_mask.SetCount(0);
	dep_mask.SetCount(bytes, 0);
	
	// Get true-value ranges in the mask
	depmask_begins.SetCount(0);
	depmask_ends.SetCount(0);
	depmask_begins.Add(part.begin);
	depmask_ends.Add(part.end);
	
	// Enable this factory and disable others
	for(int k = 0; k < fac_count; k++)
		enabled_fac[k] = false;
	enabled_fac[fac_id] = true;
	for(int k = fac_id; k >= 0; k--) {
		if (!enabled_fac[k]) continue;
		
		const CombinationPart& part = combparts[k];
		depmask_begins.Add(part.begin);
		depmask_ends.Add(part.end);
		
		// Enable sources of current
		for (int l = 0; l < part.input_src.GetCount(); l++) {
			
			// Get enabled input
			const Vector<IntPair>& src_list = part.input_src[l];
			for (int m = 0; m < src_list.GetCount(); m++) {
				int enabled_bit = GetBitCore(k, l, m);
				bool src_is_enabled = ReadBit(src, enabled_bit);
				
				// Unmask source if it was enabled
				if (src_is_enabled) {
					const IntPair& src = src_list[m];
					enabled_fac[src.a] = true;
					ASSERT(src.a < k); // factory id must be less than current
				}
			}
		}
	}
	ASSERT(!depmask_begins.IsEmpty());
	
	// Write ranges to the mask
	for(int k = 0; k < depmask_begins.GetCount(); k++) {
		int begin = depmask_begins[k];
		int end = depmask_ends[k];
		for (int l = begin; l < end; l++) {
			int byt = l / 8;
			int bit = l % 8;
			byte* b = dep_mask.Begin() + byt;
			byte mask = 1 << bit;
			*b |= mask;
		}
	}
	
	// Create unique slot combination
	unique_slot_comb.SetCount(0);
	unique_slot_comb.SetCount(bytes, 0);
	int nonzero_count = 0;
	for(int k = 0; k < bytes; k++) {
		byte b = dep_mask[k] & src[k];
		if (b != 0) nonzero_count++;
		unique_slot_comb[k] = b;
	}
	ASSERT_(nonzero_count > 0, "ERROR: creating unique combination from factory which was not enabled"); // empty is not unique
}

String Prioritizer::GetCombinationString(const Vector<byte>& vec) {
	String s;
	
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		
		int enabled_bit = GetBitEnabled(i);
		bool enabled = ReadBit(vec, enabled_bit);
		if (!enabled) continue;
		
		s << i << ":\t\"" << Factory::GetCtrlFactories()[i].a << "\"\n";
		
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<IntPair>& src_list = part.input_src[j];
			
			s << "\tinput " << j << ":\n";
			for(int k = 0; k < src_list.GetCount(); k++) {
				const IntPair& src = src_list[k];
				
				int src_enabled_bit = GetBitCore(i, j, k);
				bool src_enabled = ReadBit(vec, src_enabled_bit);
				if (!src_enabled) continue;
				
				s << "\t\tsrc " << k << ":\t\"" << Factory::GetCtrlFactories()[src.a].a << "\", output=" << src.b << "\n";
			}
		}
	}
	
	return s;
}

}
