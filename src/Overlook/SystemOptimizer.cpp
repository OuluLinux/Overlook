/*
SystemOptimizer.cpp

Genetic optimizer is embedded in the QueryTable class, and InitGeneticOptimizer prepares
columns in the QueryTable instance. Target value is the profit encoded to 0-65536 range
integer. Currently, the structure has maximum size of 1+1+2+2*2, where the first is
multiplexer-like class, which combines all symbols. After that, there is one Template class for
every symbol. That is also the first step of tree-like structure, where 7-sub-Templates can be
used as input value. The tree-structure is limited to 2-leaf-nodes and 3 steps, and the total
size of Template instances in the tree is 1+2+2*2. The size can be changed, but currently there is
no need to make it any other. To sum all columns, there is 1 for target, x for all symbols, y
for columns inside one Template blocks and 1+2*2*2 for Template blocks, which gives 1+x+y*(1+1+2+2*2).
The fixed size allows the fixed size of rows and matching bit-ranges in every row. That makes
the processing faster and the algorithm less complex.

Genetic optimizer is based on differential evolution algorithm. Only tweak is that population
and generation concepts are not being used. This rather uses the top values of the table as
population and allows multi-threading by generating more than one candidate to the queue. I
consider it to be similar enough to the original and studied algorithm. Basically, it takes the
best rows and combines it with every unit in the population in a random way. Some leaf-values
are not used in the tree-structure, if some parent node disables the whole leaf, which causes
the need to clean all useless mutations, or otherwise duplicate rows accumulates into the table.
The useless mutation would cause the row comparison to fail, even when the row is equal in
practice.

All new rows, what genetic optimizer produces, can be evaluated roughly with a decision tree.
That gives a heuristic value, that is the row expected to give good results even with new
mutations. However, small amount of random mutating must always be allowed through exploration
or otherwise the evolution will overfit old top values. The balance of exploration and
preferring top values is difficult to find.

New rows, combinations of the tree structure, must be evaluated by simulating their real-world
performance with the SimBroker class. The data should be separate for training and testing, but
it is actually difficult to provide, because the broker doesn't provide fastest timeframe-data
from the same beginning than slower. The signal is provided from more predictable slow timeframes
to the faster, and if you are tied to a small range in the smallest timeframe, then you can't
test slow timeframes well. The result is that the separate training and testing data doesn't work
in practice for the whole system. Even if the small timeframe data was available, the
multiplier between that and largest would be 40320, which would consume too much memory in my
computer. Different parts can be tested separately, however.

Row is evaluated by processing it's component in correct order and by taking the output value
from the last object. The row is usually called pipeline for that reason here. The whole tree
can be in the writing moment 1+1+7+7*7. It can be masked based on recursive dependencies of the
root node. GetCoreQueue doesn't have separate unmasking part, but it first unmasks only the
root and then recursively unmasks all it's dependencies while also increasing the queue.
One template might have traditional indicators as inputs, which also must be added to the queue
with VisitTemplate funcion. This might cause duplicate items in the queue, which must be removed
at the end.

Traditional indicator arguments are in the argument space of the structural Template node. Due to
the fixed bit-ranges in the row, there must be fixed size of maximum indicators and arguments.
Most important arguments of indicators must then be first. Also, the range of row values
must be converted to the range of the indicator argument. Indicators must be enabled with
boolean switches rather than 'how many' indicators can be used, so that the genetic mutation
doesn't have wrong kind of leverage. That is also how sub-nodes of template nodes are enabled.

All Template objects and traditional indicator objects are in the same memory structure. They are
located by a symbol/timeframe/class/hash path. The hash is made from all values that make the object
unique, which excludes the node what uses the object as input. For traditional indicator, it is
only arguments which makes them unique, but for nodes in the template-object tree, all sub-nodes
makes them unique also. The hash is still quite straightforward to make. The row mask is formed
by selecting node and all subnodes of the node. The row and the mask is then operated with
a bitwise AND operation. Resulting vector is then hashed with the CombineHash class. Symbol, tf
and class-id is left out from the hash only because of faster path resolving.

CoreItem objects have inputs and outputs, which are connected before making the Core object.
The connections are made by symbol/timeframe/class/hash/buffer paths what they create and accept.
CoreItem connections are completely connected only after core objects have been created due to
the demand of their content. In the previous version, inputs and outputs were connected by types
and there was multiple sources for one type, which were enabled or disabled by the combination
row, but the template class made it obsolete, since traditional indicators have only one source
anyway. In the latest version of overlook, only one possible source for input is accepted.



Remaining issues:
 - testing: slow tf template inputs provides testing data at the beginning of fast's testing data
 - overfitting: too much steps or steps instead of boolean enablers can cause uncontrolled mutation
				(e.g. mutation to type-column gives completely unrelated new type)
*/

#include "Overlook.h"

namespace Overlook {

const int max_sources = 2;

void System::SolveClassConnections() {
	ASSERT(combparts.IsEmpty());
	ASSERT_(System::GetRegs().GetCount() > 0, "Recompile Overlook.icpp to fix this stupid and weird problem");
	
	// Compare input types to output types and add matching pairs to input sources.
	for(int i = 0; i < System::GetRegs().GetCount(); i++) {
		const Vector<RegisterInput>& factory_inputs = System::GetRegs()[i].in;
		const Vector<ValueType>& factory_outputs = System::GetRegs()[i].out;
		
		// One part per slot-factory
		CombinationPart& part = combparts.Add();
		part.input_src.SetCount(factory_inputs.GetCount());
		
		// Add inputs
		for(int i2 = 0; i2 < factory_inputs.GetCount(); i2++) {
			const RegisterInput& input = factory_inputs[i2];
			part.inputs.Add(input);
			
			// Find output to input.
			// Limit range to input position, because it simplifies dependency mask and this
			// way some wrong combinations can be easily avoided. Be sure to register slots in
			// correct order.
			for(int j = 0; j < i; j++) {
				const Vector<ValueType>& factory_outputs = System::GetRegs()[j].out;
				
				for(int k = 0; k < factory_outputs.GetCount(); k++) {
					const ValueType& output = factory_outputs[k];
					
					// If one of outputs of the registered slot factory is equal to input
					if (output.type == input.type && output.phase == input.phase) {
						
						part.input_src[i2].Add(InputSource(j, k));
					}
				}
			}
		}
		// Add outputs
		for(int i2 = 0; i2 < factory_outputs.GetCount(); i2++) {
			const ValueType& output = factory_outputs[i2];
			part.outputs.Add(output);
		}
		
		// Check if slot can branch with different sources
		part.single_sources = true;
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			if (part.input_src[j].GetCount() > 1) {
				part.single_sources = false;
				break;
			}
		}
	}
	
	// Debugging checks
	#ifdef flagDEBUG
	int combination_errors = 0;
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		LOG("    part " << i << ": \"" << System::GetCtrlFactories()[i].a << "\" single-source=" << (int)part.single_sources);
		
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<InputSource>& inputs = part.input_src[j];
			if (inputs.IsEmpty()) {
				const RegisterInput& input = System::GetRegs()[i].in[j];
				if (input.input_type == REGIN_HIGHPRIO) {
					LOG("         (same class, higher priority instances)");
				}
				else {
					LOG("        ERROR: input not found: " << part.inputs[j].ToString());
					combination_errors++;
				}
			} else {
				String str;
				for(int k = 0; k < inputs.GetCount(); k++) {
					const InputSource& is = inputs[k];
					str << " (" << is.factory << "," << is.output << ")";
				}
				LOG("        " << str);
			}
		}
	}
	if (combination_errors) {
		Panic("Found " + IntStr(combination_errors) + " errors. Can't run system.");
	}
	#endif
}

int  System::GetSymbolEnabled(int sym) const {
	ASSERT(sym >= 0 && sym < symbols.GetCount());
	int col = 0;
	col++; // target
	col += sym;
	return col;
}

inline uint32 Pow2(int exp) {
	uint32 r = 1;
	r <<= exp;
	return r;
}

int System::GetEnabledColumn(const Vector<int>& path) {
	ASSERT(max_sources == 2);
	int col = 1 + symbols.GetCount();
	for(int i = 0; i < path.GetCount(); i++) {
		int j = path[i];
		ASSERT(j >= 0);
		
		// Structural node
		if (j < 1000) {
			ASSERT(j < max_sources);
			// Combiner
			if (i == 0) {
				ASSERT(j == 0);
				col += slot_args;
			}
			// Template-tree
			else {
				ASSERT(i <= 3);
				int sub_nodes = 0;
				for(int k = i; k < 4; k++) {
					int e = 3 - k;
					ASSERT(e >= 0);
					sub_nodes += Pow2(e);
				}
				col += j * sub_nodes * slot_args;
			}
		}
		// Traditional indicator
		else {
			j -= 1000;
			col += traditional_enabled_cols[j];
			ASSERT_(i == path.GetCount()-1, "This must be the last position in the path");
		}
	}
	ASSERT(col >= 0 && col < table.GetColumnCount());
	return col;
}

void System::InitGeneticOptimizer() {
	ASSERT(table.GetColumnCount() == 0);
	ASSERT(traditional_enabled_cols.GetCount() == 0);
	
	// Target value
	table.AddColumn("Result", 65536);
	table.EndTargets();
	
	// Columns for enabled symbols
	for(int i = 0; i < symbols.GetCount(); i++)
		table.AddColumn(symbols[i] + " enabled", 2);
	
	// Columns for structure
	structural_columns = 1 + 1 + max_sources + max_sources * max_sources;
	template_id = Find<Template>();
	const Vector<ArgType>& template_args = GetRegs()[template_id].args;
	template_arg_count = template_args.GetCount();
	
	// Columns for traditional indicator arguments
	ma_id = Find<MovingAverage>();
	int psych_id = Find<Psychological>();
	ASSERT_(ma_id != -1, "MovingAverage is not registered");
	ASSERT_(psych_id != -1, "Psychological is not registered");
	traditional_indicators = psych_id - ma_id + 1;
	traditional_arg_count = 0;
	for(int j = 0; j < traditional_indicators; j++) {
		traditional_enabled_cols.Add(traditional_arg_count + j);
		traditional_arg_count += GetRegs()[ma_id + j].args.GetCount();
	}
	
	// Enable slot, template arguments, enable trad. indicators, arguments for trad. indicators
	slot_args = 1 + template_arg_count + traditional_indicators + traditional_arg_count;
	
	ASSERT_(template_id != -1, "Template class have not been registered to the system");
	for(int i = 0; i < structural_columns; i++) {
		String slot_desc = "Slot #" + IntStr(i);
		
		// Is column enabled
		table.AddColumn(slot_desc + " enabled", 2);
		
		// Columns in the Template of one structural slot
		for(int j = 0; j < template_args.GetCount(); j++) {
			const ArgType& arg = template_args[j];
			String desc = slot_desc + " " + arg.desc;
			int max_value = arg.max - arg.min + 1;
			
			// Add column
			table.AddColumn(desc, max_value);
		}
		
		// Columns in the traditional indicator inputs of one structural slot
		for(int j = 0; j < traditional_indicators; j++) {
			int factory = ma_id + j;
			const CombinationPart& part = combparts[factory];
			const SystemValueRegister& reg = GetRegs()[factory];
			const String& title = GetCtrlFactories()[factory].a;
			String desc = slot_desc + " " + title + " enabled";
			
			// Is traditional indicator enabled
			table.AddColumn(desc, 2);
			
			// Arguments for a traditional indicator
			for(int k = 0; k < reg.args.GetCount(); k++) {
				const ArgType& arg = reg.args[k];
				String desc = slot_desc + " " + title + " " + desc;
				int max_value = arg.max - arg.min + 1;
				table.AddColumn(desc, max_value);
			}
		}
	}
	
	// Connect input source slots to their enabled bits
	/*int combination_bits = table.GetRowBits();
	inputs_to_enabled.SetCount(combination_bits, -1);
	enabled_to_factory.SetCount(combination_bits, -1);
	
	for(int i = 0; i < combparts.GetCount(); i++) {
		const CombinationPart& part = combparts[i];
		for(int j = 0; j < part.input_src.GetCount(); j++) {
			const Vector<InputSource>& src = part.input_src[j];
			for(int k = 0; k < src.GetCount(); k++) {
				const InputSource& is = src[k];
				for(int l = 0; l < structural_columns; l++) {
					int input_bit = GetBitCore(l, i, j, k);
					int enabled_bit = GetBitEnabled(l, is.factory);
					inputs_to_enabled[input_bit] = enabled_bit;
					enabled_to_factory[enabled_bit] = is.factory;
				}
			}
		}
	}*/
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
	table.SetPruning(QueryTable::PRUNE_ERROREST);
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
			
			
			// If prediction is good or exploration can be done, then try to add this
			if (prediction >= target_limit || Randomf() <= exploration) {
				
				// Clean useless mutation
				Panic("TODO");
				
				// Trim excess traditional nodes away
				Panic("TODO");
				
				// Check that row doesn't exist in the table
				Panic("TODO");
				
				// Add the combination to the working queue
				pi->priority = i;
				pl_queue_lock.Enter();
				pl_queue.Add(pi.Detach());
				pl_queue_lock.Leave();
			}
		}
	}
}

void System::GetCoreQueue(const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, Index<int>* tf_ids) {
	
	Vector<int> sym_ids;
	const int sym_count = GetSymbolCount();
	for (int sym = 0; sym < sym_count; sym++) {
		int sym_enabled_col = GetSymbolEnabled(sym);
		if (table.Get0(sym_enabled_col, pi.value))
			sym_ids.Add(sym);
	}
	ASSERT_(!sym_ids.IsEmpty(), "No symbol is enabled");
	
	const int tf_count = GetPeriodCount();
	for (int tf = tf_count -1; tf >= 0; tf--) {
		
		// Skip disallowed tfs if allowed tf-list is given
		if (tf_ids && tf_ids->Find(tf) == -1)
			continue;
		
		// Columns for structure
		Vector<int> path;
		GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
		
	}
	
	// Sort queue by priority
	struct PrioritySorter {
		bool operator()(const Ptr<CoreItem>& a, const Ptr<CoreItem>& b) const {
			return a->priority < b->priority;
		}
	};
	Sort(ci_queue, PrioritySorter());
	
	// Remove duplicates
	Panic("TODO");
}

void System::GetCoreQueue(Vector<int>& path, const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Vector<int>& sym_ids) {
	ASSERT(table.GetColumnCount() > 0);
	ASSERT(ma_id != -1);
	
	int sub_pos = path.Top();
	bool is_template = sub_pos < 1000;
	int factory = is_template ? template_id : path.Top() - 1000;
	const int tf_count = GetPeriodCount();
	const int sym_count = GetSymbolCount();
	
	
	// Template objects
	if (is_template) {
		ASSERT(sub_pos >= 0 && sub_pos < max_sources);
		
		// Columns for structure
		for(int i = 0; i < max_sources; i++) {
			
			// Check if sub-node is enabled
			path.Add(i);
			int col = GetEnabledColumn(path);
			if (!table.Get0(col, pi.value)) {
				continue;
			}
			
			GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
			path.Pop();
		}
		
		for(int i = 0; i < traditional_indicators; i++) {
			
			// Check if sub-node is enabled
			path.Add(1000 + ma_id + i);
			int col = GetEnabledColumn(path);
			if (!table.Get0(col, pi.value)) {
				continue;
			}
			
			GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
			path.Pop();
		}
	}
	// Traditional indicator
	else {
		// Loop inputs of the factory
		const CombinationPart& part = combparts[factory];
		const SystemValueRegister& reg = System::GetRegs()[factory];
		
		// Connect input sources
		// Loop all inputs of the custom core-class
		for (int l = 0; l < part.input_src.GetCount(); l++) {
			const RegisterInput& input = reg.in[l];
			if (input.input_type == REGIN_DYNAMIC) {
				
			} else {
				// In current version, where traditional indicators and template model are separated,
				// only one source is allowed for traditional indicator input.
				const Vector<InputSource>& sources = part.input_src[l];
				ASSERT_(sources.GetCount() == 1, "Traditional indicator inputs should have only 1 possible source output");
				const InputSource& source = sources[0];
				
				
				//const RegisterInput& input = part.inputs[l];
				
				
				path.Add(1000 + source.factory);
				GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
				path.Pop();
			}
		}
	}
	
	// Mask this and all dependencies in the pipeline combination
	Vector<byte> comb;
	Panic("TODO");
	
	// Get unique string and trim it
	String unique, unique_long = HexVector(comb);
	for (int j = unique_long.GetCount()-1; j >= 0; j--) {
		if (unique_long[j] != '0') {
			unique = unique_long.Left(j+1);
			break;
		}
	}
	ASSERT(!unique.IsEmpty());
	
	
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		
		int hash = GetHash(pi, sym, tf, path);
		
		// Get CoreItem
		CoreItem& ci = data[sym][tf][factory].GetAdd(hash);
		
		// Init object if it was just created
		if (ci.sym == -1) {
			ci.sym = sym;
			ci.tf = tf;
			ci.priority = // lower value is more important
				
				// Slowest tf is most important in this system.
				(tf_count-1-tf) * sym_count * structural_columns +
				
				// Structural column might require all symbols, so it is more important than symbol.
				// Also, root is the column 0 and it must be processed last.
				GetPathPriority(path) * sym_count +
				
				// Lower symbols must be processed before higher, because cores are allowed to
				// require lower id symbols in the same timeframe and same structural column.
				sym;
			
			ci.factory = factory;
			ci.unique = unique;
			
			// Set arguments from combination
			Panic("TODO");
			
			// Connect core inputs
			ConnectCore(ci);
		}
		
		ci_queue.Add(&ci);
	}
}

int System::GetHash(const PipelineItem& pi, int sym, int tf, Vector<int>& path) {
	
	Panic("TODO"); return 0;
}


void System::ConnectCore(CoreItem& ci) {
	ASSERT(ci.core == NULL);
	const CombinationPart& part = combparts[ci.factory];
	Vector<int> enabled_input_factories;
	Vector<byte> unique_slot_comb;

	// Connect input sources
	// Loop all inputs of the custom core-class
	for (int l = 0; l < part.input_src.GetCount(); l++) {
		const RegisterInput& input = part.inputs[l];
		bool input_dynamic = (input.input_type == REGIN_DYNAMIC && input.data != NULL) || (input.input_type == REGIN_HIGHPRIO && input.data != NULL);
		
		// For higher priority inputs, the only source is this same factory
		if (input.input_type == REGIN_HIGHPRIO) {
			ASSERT_(part.outputs.GetCount(), "The class must have at least one output to connect it as input.");
			int hash;
			Panic("TODO");
			int found_inputs = ConnectInput(l, 0, ci, ci.factory, hash);
		}
		// Normally loop possible sources for one input
		else {
			const Vector<InputSource>& input_src = part.input_src[l];
			#ifdef flagDEBUG
			int src_count = 0; // count enabled sources for debugging (1 is correct)
			int enabled_count = 0; // exactly one must be enabled
			#endif
			enabled_input_factories.SetCount(0);
			
			for (int m = 0; m < input_src.GetCount(); m++) {
				
				// Check if source is enabled
				/*const InputSource& src = input_src[m];
				int src_enabled_bit = GetBitCore(ci.factory, l, m);
				bool src_enabled = ReadBit(ci.value, src_enabled_bit);
				if (!src_enabled) continue;*/
				Panic("TODO"); // break first
				
				/*#ifdef flagDEBUG
				int bit = InputToEnabled(src_enabled_bit);
				int enabled_bit = GetBitEnabled(src.factory);
				ASSERT(bit == enabled_bit);
				ASSERT(ReadBit(ci.value, bit));
				#endif*/
				
				const InputSource& src = input_src[0];
				
				enabled_count++;
				enabled_input_factories.Add(src.factory);
				
				// Create unique combination for current input source factory
				/*CreateUniqueCombination(ci.value, src.factory, unique_slot_comb);
				ASSERT(unique_slot_comb.GetCount() == job_combination_bytes);*/
				int hash;
				Panic("TODO");
				
				int found_inputs = ConnectInput(l, src.output, ci, src.factory, hash);
				
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
				//LOG("Combination:");
				//LOG(GetCombinationString(ci.value));
				//DUMPC(job_queue);
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
}

int System::ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash) {
	const RegisterInput& input = combparts[ci.factory].inputs[input_id];
	
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
	/*for (int n = 0; n < job_queue.GetCount(); n++) {
		JobItem& src_ci = job_queue[n];
		
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
	}*/
	ASSERT_(is_one_input_only || input_count > 1 || input_dynamic || input_highprio, "Couldn't find multiple inputs");
	
	return input_count;
}

void System::CreateCore(CoreItem& ci) {
	ASSERT(ci.core == NULL);
	
	// Create core-object
	ci.core = System::GetCtrlFactories()[ci.factory].b();
	
	// Set attributes
	ci.core->base = this;
	ci.core->factory = ci.factory;
	ci.core->RefreshIO();
	ci.core->SetUnique(ci.unique);
	ci.core->SetSymbol(ci.sym);
	ci.core->SetTimeframe(ci.tf, GetPeriod(ci.tf));
	ci.core->LoadCache();
	
	// Connect object
	Panic("TODO");
	
	//ci.core->SetArguments(*args);
	
	// Initialize
	ci.core->InitAll();
}

int System::GetPathPriority(const Vector<int>& path) {
	Panic("TODO");
	return -1;
}

/*int System::GetBitCore(int struct_id, int fac_id, int input_id, int src_id) const {
	
	// Struct-id begin
	
	
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

int System::GetBitEnabled(int struct_id, int fac_id) const {
	Panic("TODO");
	return -1;
}*/


}
