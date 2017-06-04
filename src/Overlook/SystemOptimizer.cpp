#include "Overlook.h"

namespace Overlook {

const int max_sources = 7;

void System::InitGeneticOptimizer() {
	ASSERT(table.GetColumnCount() == 0);
	
	// Target value
	table.AddColumn("Result", 65536);
	table.EndTargets();
	
	// Columns for enabled symbols
	for(int i = 0; i < symbols.GetCount(); i++)
		table.AddColumn(symbols[i] + " enabled", 2);
	
	// Columns for structure
	const int structural_columns = 1 + 1 + max_sources + max_sources * max_sources;
	const int traditionals_in_slot = 3;
	const int traditional_maxargs = 3;
	const int model_id = Find<Model>();
	const Vector<ArgType>& model_args = GetRegs()[model_id].args;
	const int model_arg_count = model_args.GetCount();
	const int slot_args = model_arg_count + traditionals_in_slot * traditional_maxargs;
	
	ASSERT_(model_id != -1, "Model class have not been registered to the system");
	for(int i = 0; i < structural_columns; i++) {
		String slot_desc = "Slot #" + IntStr(i);
		
		// Columns in the Model of one structural slot
		for(int j = 0; j < model_args.GetCount(); j++) {
			const ArgType& arg = model_args[j];
			String desc = slot_desc + " " + arg.desc;
			int max_value = arg.max - arg.min + 1;
			
			// Add column
			table.AddColumn(desc, max_value);
		}
		
		// Columns in the traditional indicator inputs of one structural slot
		for(int j = 0; j < traditionals_in_slot; j++) {
			for(int k = 0; k < traditional_maxargs; k++) {
				String desc = slot_desc + Format(" s%d a%d", j, k);
				int max_value = 128;
				
				// Add column
				table.AddColumn(desc, max_value);
			}
		}
	}
	
}

void System::RefreshPipeline() {
	const int target_col = 0;
	const int cores = CPU_Cores();
	const int min_queue = cores * 4;
	const int max_queue = cores * 20;
	
	
	// Don't fill queue until it is small enough. Calculating decision tree is demanding.
	if (pl_queue.GetCount() > min_queue) return;
	
	
	// Sort table by descending result value: greatest at first
	// This also determines what is considered to be 'best' by the differential evolution
	table.Sort(target_col, true);
	const int table_rows = Upp::min(1000, table.GetCount());
	const int discard_limit = table_rows / 2;
	const int target_limit = table.Get(discard_limit, target_col);
	ASSERT(table_rows > 2);
	
	DecisionTreeNode tree;
	table.GetDecisionTree(target_col, tree, table_rows);
	
	
	// Fill the pipeline queue
	One<PipelineItem> pi;
	while (pl_queue.GetCount() < max_queue) {
		int new_count = Upp::min(table.GetCount()-1, max_queue - pl_queue.GetCount());
		ASSERT(new_count > 0);
		
		for(int i = 0; i < new_count; i++) {
			if (pi.IsEmpty()) pi.Create();
			
			
			// Create new combination with the differential evolution algorithm
			// - the best row is at position 0 after sorting
			// - the candidate is usually from whole population, but here it is only the best
			//   set, which also might make worse results...
			table.Evolve(0, 1+i, pi->value);
			
			
			// Evaluate new combination with decision tree
			int prediction = table.Predict(tree, pi->value, target_col);
			
			
			// If prediction is good or exploration can be done, then add this
			if (prediction >= target_limit || Randomf() <= exploration) {
				pi->priority = i;
				pl_queue_lock.Enter();
				pl_queue.Add(pi.Detach());
				pl_queue_lock.Leave();
			}
		}
	}
}

void System::GetCoreQueue(const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, Index<int>* tf_ids) {
	ASSERT(table.GetColumnCount() > 0);
	
	const int tf_count = GetPeriodCount();
	const int sym_count = GetSymbolCount();
	const int structural_columns = 1 + 1 + max_sources + max_sources * max_sources;
	Index<int> enabled_struct_cols;
	enabled_struct_cols.Add(0); // always enable the root
	
	const int model_factory = Find<Model>();
	ASSERT(model_factory != -1);
	
	Index<int> sym_ids;
	Panic("TODO");
	
	// Columns for structure
	for(int i = 0; i < structural_columns; i++) {
		if (enabled_struct_cols.Find(i) == -1)
			continue;
		
		// Check of leafs are enabled
		for(int j = 0; j < 7; j++) {
			int leaf_col = GetLeaf(i, j);
			if (IsLeafEnabled(leaf_col))
				enabled_struct_cols.Add(leaf_col);
		}
		
		for (int tf = tf_count -1; tf >= 0; tf--) {
			
			// Skip disallowed tfs if allowed tf-list is given
			if (tf_ids && tf_ids->Find(tf) == -1)
				continue;
			
			for (int sym = 0; sym < sym_count; sym++) {
				if (sym_ids.Find(sym) == -1) continue;
				
				int hash = GetHash(pi, sym, tf, i);
				
				// Get CoreItem
				CoreItem& ci = data[sym][tf][model_factory].GetAdd(hash);
				
				// Init object if it was just created
				if (ci.sym == -1) {
					ci.sym = sym;
					ci.tf = tf;
					ci.priority = // lower value is more important
						
						// Slowest tf is most important in this system.
						(tf_count-1-tf) * sym_count * structural_columns +
						
						// Structural column might require all symbols, so it is more important than symbol.
						// Also, root is the column 0 and it must be processed last.
						(structural_columns-1-i) * sym_count +
						
						// Lower symbols must be processed before higher, because cores are allowed to
						// require lower id symbols in the same timeframe and same structural column.
						sym;
					
					ci.factory = model_factory;
				}
				
				ci_queue.Add(&ci);
				
				// Add dependencies of the model-object to the queue
				VisitModel(ci, ci_queue);
			}
		}
	}
	
	struct PrioritySorter {
		bool operator()(const Ptr<CoreItem>& a, const Ptr<CoreItem>& b) const {
			return a->priority < b->priority;
		}
	};
	Sort(ci_queue, PrioritySorter());
	
	// Remove duplicates?
	Panic("TODO");
}

void System::VisitModel(const CoreItem& ci, Vector<Ptr<CoreItem> >& ci_queue) {
	
	Panic("TODO");
}

int System::GetLeaf(int model_col, int leaf_id) {
	
	Panic("TODO"); return 0;
}

bool System::IsLeafEnabled(int model_col) {
	
	Panic("TODO"); return 0;
}

int System::GetHash(const PipelineItem& pi, int sym, int tf, int model_col) {
	
	Panic("TODO"); return 0;
}

void System::CreateCore(CoreItem& ci) {
	ASSERT(ci.core == NULL);
	const CombinationPart& part = combparts[ci.factory];
	Vector<int> enabled_input_factories;
	Vector<byte> unique_slot_comb;
	
	
	// Create core-object
	ci.core = System::GetCtrlFactories()[ci.factory].b();
	
	// Set attributes
	ci.core->base = &bs;
	ci.core->factory = ci.factory;
	ci.core->RefreshIO();
	ci.core->SetUnique(ci.unique);
	ci.core->SetSymbol(ci.sym);
	ci.core->SetTimeframe(ci.tf, bs.GetPeriod(ci.tf));
	ci.core->LoadCache();
	
	// Connect input sources
	// Loop all inputs of the custom core-class
	for (int l = 0; l < part.input_src.GetCount(); l++) {
		const RegisterInput& input = part.inputs[l];
		bool input_dynamic = (input.input_type == REGIN_DYNAMIC && input.data != NULL) || (input.input_type == REGIN_HIGHPRIO && input.data != NULL);
		
		// For higher priority inputs, the only source is this same factory
		if (input.input_type == REGIN_HIGHPRIO) {
			ASSERT_(part.outputs.GetCount(), "The class must have at least one output to connect it as input.");
			int found_inputs = ConnectSystem(l, 0, input, ci, ci.factory);
		}
		// Normally loop possible sources for one input
		else {
			const Vector<IntPair>& input_src = part.input_src[l];
			#ifdef flagDEBUG
			int src_count = 0; // count enabled sources for debugging (1 is correct)
			int enabled_count = 0; // exactly one must be enabled
			#endif
			enabled_input_factories.SetCount(0);
			for (int m = 0; m < input_src.GetCount(); m++) {
				
				// Check if source is enabled
				const IntPair& src = input_src[m]; // src.a = factory, src.b = it's output id
				int src_enabled_bit = GetBitCore(ci.factory, l, m);
				bool src_enabled = ReadBit(ci.value, src_enabled_bit);
				if (!src_enabled) continue;
				
				#ifdef flagDEBUG
				int bit = InputToEnabled(src_enabled_bit);
				int enabled_bit = GetBitEnabled(src.a);
				ASSERT(bit == enabled_bit);
				ASSERT(ReadBit(ci.value, bit));
				#endif
				
				enabled_count++;
				enabled_input_factories.Add(src.a);
				/*DUMP(src);
				
				if (ci.factory == 2) {
					LOG("");
				}*/
				
				// Create unique combination for current input source factory
				CreateUniqueCombination(ci.value, src.a, unique_slot_comb);
				ASSERT(unique_slot_comb.GetCount() == job_combination_bytes);
				
				int found_inputs = ConnectSystem(l, src.b, input, ci, src.a, &unique_slot_comb);
				
				if (found_inputs) {
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
				LOG(GetCombinationString(ci.value));
				DUMPC(job_queue);
				/*LOG("Checking all source pipeline combinations");
				for(int i = 0; i < ci.pipeline_src.GetCount(); i++) {
					const PipelineItem& pi = pl_queue[ci.pipeline_src[i]];
					CheckCombination(pi.value);
				}*/
			}
			ASSERT_(enabled_count > 0, "Combination is invalid: not a single source is enabled");
			ASSERT_(enabled_count < 2, "Combination is invalid: too many sources are enabled");
			if (src_count == 0 && input.input_type == REGIN_DYNAMIC) {
				//LOG(GetCombinationString(ci.value));
				String inputs = "\"";
				for(int i = 0; i < enabled_input_factories.GetCount(); i++) {
					if (i) inputs.Cat(',');
					inputs << System::GetCtrlFactories()[enabled_input_factories[i]].a;
				}
				inputs << "\"";
				Panic("Creating object \"" +
					System::GetCtrlFactories()[ci.factory].a +
					"\" and can't find any of " +
					inputs +
					" inputs.");
				ASSERT_(src_count > 0, "Didn't find earlier job to connect as input. Maybe some core class has no input sources");
			}
			#endif
		}
	}
	
	//ci.core->SetArguments(*args);
	
	// Initialize
	ci.core->InitAll();
}

int System::ConnectSystem(int input_id, int output_id, const RegisterInput& input, JobItem& ci, int factory, Vector<byte>* unique_slot_comb) {
	FilterFunction fn = (FilterFunction)input.data;
	bool input_all_sym = input.scale == Sym || input.scale == All;
	bool input_all_tf  = input.scale == Tf  || input.scale == All;
	bool input_dynamic = (input.input_type == REGIN_DYNAMIC && fn != NULL) || (input.input_type == REGIN_HIGHPRIO && fn != NULL);
	bool input_highprio = input.input_type == REGIN_HIGHPRIO;
	
	// Search source and loop existing job queue
	bool found = false;
	int input_count = 0;
	bool is_one_input_only = !input_all_sym && !input_all_tf && !input_dynamic && !input_highprio;
	bool is_many_to_many_exceptions_allowed = input_all_sym && input_all_tf;
	for (int n = 0; n < job_queue.GetCount(); n++) {
		JobItem& src_ci = job_queue[n];
		
		/*LOG(src_ci.factory << " != " << src.a << "\t" <<
			src_ci.sym << " != " << ci.sym << "\t" <<
			src_ci.tf << " != " << ci.tf);*/
		
		// System must match
		if (src_ci.factory != factory) continue;
		
		// Never lower priority
		if (src_ci.priority > ci.priority)
			continue;
		
		// With dynamic input, only the return value of the filter function matters.
		if (input_dynamic) {
			bool keep = fn(&bs, ci.sym, ci.tf, src_ci.sym, src_ci.tf);
			if (!keep) continue;
		}
		// With higher priority inputs, higher priority instances of the same class are connected:
		else if (input_highprio) {
			// Without filter function
			//  - slower tf inputs has higher priority, all slower tfs are kept
			if (!fn) {
				if (!input_all_sym && !src_ci.all_sym && src_ci.sym != ci.sym) continue;
				if (src_ci.tf <= ci.tf) continue;
			}
			// With filter function
			else {
				bool keep = fn(&bs, ci.sym, ci.tf, src_ci.sym, src_ci.tf);
				if (!keep) continue;
			}
		}
		// Normally, only symbol & timeframe are being matched.
		else {
			// If input takes all sym&tf, the symbol doesn't need to match
			// If source gives all sym&tf, the symbol doesn't need to match
			if (!input_all_sym && !src_ci.all_sym && src_ci.sym != ci.sym) continue;
			if (!input_all_tf  && !src_ci.all_tf  && src_ci.tf  != ci.tf ) continue;
		}
		
		bool equal = true;
		if (unique_slot_comb) {
			for (int o = 0; o < job_combination_bytes; o++) {
				if ((*unique_slot_comb)[o] != src_ci.value[o]) {
					equal = false;
					break;
				}
			}
		}
		if (!equal)	{
			/*LOG("DIFFERENT COMBOS");
			LOG("SRC:");
			LOG(GetCombinationString(ci.value));
			LOG("A:");
			LOG(GetCombinationString(unique_slot_comb));
			LOG("B:");
			LOG(GetCombinationString(src_ci.value));*/
			continue;
		}
		
		#ifdef flagDEBUG
		// Check for multiple types of inputs
		if (!is_many_to_many_exceptions_allowed &&
			!is_one_input_only &&
			(src_ci.all_sym || src_ci.all_tf) &&
			input_count > 0)
			Panic("ERROR: multiple many-to-many or one-to-many, but input can take only one");
		#endif
		
		
		// Source found
		ASSERT(src_ci.core);
		ci.core->AddInput(input_id, src_ci.sym, src_ci.tf, *src_ci.core, output_id);
		
		
		input_count++;
		found = true;
		if (is_one_input_only)
			break;
	}
	ASSERT_(is_one_input_only || input_count > 1 || input_dynamic || input_highprio, "Couldn't find multiple inputs");
	
	return input_count;
}

}
