/*

                                      Foreword
                                      ========
This is what I do best. It's not about being Robin Hood, but about classifying markets when
socialists want you to classify plants.


                                    Introduction
                                    ============

System optimizer searches for better investing algorithm pipelines with genetic optimizer.
Pipeline is formed from one compact integer vector, which includes 3 features: time-slots, market
symbol baskets and the decision maker tree-structure of template classes. Known long period,
such as one week, is splitted into time-slots, which has dynamic begin and end. Those time-slots
has importance number, and for every time-position the first matching time-slot is searched.
Baskets can be formed from all available investing instruments. They can be formed in different
ways and new methods can always be added. The most basic method is to allow genetic optimizer
to decide what market tickers (symbols) are used. One other way is to find correlation groups,
which will smooth the fast changes, but they will also have the risk of one symbol detaching
from the group.

New instrument is created based on time-slots and baskets. Their change of price can be summed
to one vector. That way, useless calculation of separate items in the basket can be avoided.
The instrument is then processed with the template tree-structured pipeline, which gives
the long/short/idle signal. The tree-structure is tested in different time-scales from longest
timeframe to shortest, because of the longest have usually higher probability of success and it
overcomes costs easier. Every node of the tree-structure also takes the slower timeframe
instance of the node as input to make proxying of the slower signal possible.

Every template-node takes arguments from the genetic optimizer, and those arguments are
being optimized of course. Template class has 5 arguments: learning template (decision tree vs
neural network vs similarity), priority (moment vs probable target), target (heuristic vs
scheduled), reason (match past vs match more probable), level (immediate values vs derived
indicator values). Given arguments affects the dataset, which is used for learning. A data
source matches to some value range in some argument. The set of sources is difficult to find
and it requires insights to the web effect of market instruments and to the reasons behind
changes of prices. With previous hints you might find some of them, but some sources are also
based on methods found in the literacy since 80's. Some simplest methods are just hiding in
broad daylight.

<TODO a chapter about template sources>


                             Implementation description
                             ==========================

Genetic optimizer is embedded in the QueryTable class, and InitGeneticOptimizer prepares
columns in the QueryTable instance. Target value is the profit encoded to 0-65536 range
integer. Currently, the pipeline has a tree-structure and it is limited to 2-leaf-nodes and 3
steps having maximum size of sum(x, 0, 2, 2^x). The size can be changed, but currently there is
no need to make it any other. The fixed size allows the fixed size of rows and matching
bit-ranges in every row. That makes the processing faster and the algorithm less complex.

Genetic optimizer is based on differential evolution algorithm. Only difference to exactly
original algorithm is that population and generation concepts are not being used. The system
optimizer rather uses the top values of the table as population and allows multi-threading by
generating more than one candidate to the queue.
I consider it to be similar enough to the original and studied algorithm. Basically, it takes the
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

Row is evaluated by processing it's components in correct order and by taking the output value
from the last component. The row is usually called pipeline for that reason here. The whole tree
can be in the writing moment 1+2+2*2. It can be masked based on recursive dependencies of the
root node. GetCoreQueue doesn't have separate unmasking part, but it first unmasks only the
root and then recursively unmasks all it's dependencies while also increasing the queue.
One template might have traditional indicators as inputs, which also must be added to the queue
This might cause duplicate items in the queue, which must be removed at the end.

Traditional indicator arguments are in the argument space of the structural Template node. Due to
the fixed bit-ranges in the row, there must be fixed size of maximum indicators and arguments.
Also, the range of row values must be converted to the range of the indicator argument. Indicators must be
enabled with boolean switches rather than 'how many' indicators can be used, so that the genetic mutation
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

const int max_depth = 2;
const int max_sources = 2;
const int max_timeslots = 8;

void MaskBits(Vector<byte>& vec, int bit_begin, int bit_count) {
	int byt = bit_begin / 8;
	int bit = bit_begin % 8;
	byte* v = vec.Begin() + byt;
	for(int i = 0; i < bit_count; i++) {
		bool b = (1 << i);
		if (!b)		*v &= ~(1 << bit);
		else		*v |=  (1 << bit);
		bit++;
		if (bit == 8) {
			bit = 0;
			v++;
		}
	}
}

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

void System::SetBasketCount(int i) {
	ASSERT(i >= 0 && data.GetCount());
	data.SetCount(structural_begin + i);
	for(int i = structural_begin; i < data.GetCount(); i++) {
		data[i].SetCount(periods.GetCount());
		for(int j = 0; j < periods.GetCount(); j++)
			data[i][j].SetCount(regs.GetCount());
	}
}

inline uint32 Pow2(int exp) {
	uint32 r = 1;
	r <<= exp;
	return r;
}

int System::GetPathPriority(const Vector<int>& path) {
	ASSERT(max_sources == 2);
	int priority = 3;
	for(int i = 0; i < path.GetCount(); i++) {
		int j = path[i];
		ASSERT(j >= 0);
		
		// Structural node
		if (j < 1000) {
			ASSERT(j < max_sources);
			ASSERT(i <= max_depth);
			int sub_nodes = 0;
			for(int k = i+1; k <= max_depth; k++) {
				int e = max_depth - k;
				ASSERT(e >= 0);
				sub_nodes += Pow2(e);
			}
			priority += j * sub_nodes;
			if (path.GetCount() == i+1)
				break;
			priority++;
		}
		// Traditional indicator (0 - DataBridge, 1 - ValueChange, 2 - others)
		else if (i == path.GetCount()-1)
			return Upp::min(2, j - 1000);
	}
	return priority;
}

int System::GetEnabledColumn(const Vector<int>& path) {
	ASSERT(max_sources == 2);
	int col = structural_begin;
	for(int i = 0; i < path.GetCount(); i++) {
		int j = path[i];
		ASSERT(j >= 0);
		
		// Structural node
		if (j < 1000) {
			ASSERT(j < max_sources);
			ASSERT(i <= max_depth);
			int sub_nodes = 0;
			for(int k = i+1; k <= max_depth; k++) {
				int e = max_depth - k;
				ASSERT(e >= 0);
				sub_nodes += Pow2(e);
			}
			col += (1 + j * sub_nodes) * slot_args;
			if (path.GetCount() == i+1)
				break;
		}
		// Traditional indicator
		else {
			ASSERT(ma_id != -1);
			j -= 1000 + ma_id;
			if (j < 0 || j >= traditional_indicators)
				return -1;
			col += traditional_enabled_cols[j];
			
			// Dependencies of a traditional indicators doesn't have argument values in the
			// combination row. So, they don't have location (column) in the combination
			// row.
			if (i < path.GetCount()-1)
				return -1;
		}
	}
	ASSERT(col >= 0 && col < table.GetColumnCount());
	return col;
}

void System::MaskPath(const Vector<byte>& src, const Vector<int>& path, Vector<byte>& dst) const {
	Vector<byte> mask;
	mask.SetCount(table.GetRowBytes(), 0);
	
	// Mask vector
	ASSERT(max_sources == 2);
	int col = structural_begin;
	for(int i = 0; i < path.GetCount(); i++) {
		int j = path[i];
		ASSERT(j >= 0);
		
		// Structural node
		if (j < 1000) {
			ASSERT(j < max_sources);
			ASSERT(i <= max_depth);
			int sub_nodes = 0;
			for(int k = i+1; k <= max_depth; k++) {
				int e = max_depth - k;
				ASSERT(e >= 0);
				sub_nodes += Pow2(e);
			}
			col += j * sub_nodes * slot_args;
			if (path.GetCount() == i+1) {
				MaskBits(mask, col, sub_nodes * slot_args);
				break;
			}
			col += slot_args;
		}
		// Traditional indicator
		else {
			j -= 1000;
			col += traditional_enabled_cols[j];
			MaskBits(mask, col, traditional_col_length[j]);
			break; // all maskable is masked
		}
	}
	
	// AND operate vector
	dst.SetCount(mask.GetCount());
	for(int i = 0; i < mask.GetCount(); i++) {
		dst[i] = mask[i] & src[i];
	}
}

void System::InitGeneticOptimizer() {
	ASSERT(table.GetColumnCount() == 0);
	ASSERT(traditional_enabled_cols.GetCount() == 0);
	
	// Target value
	table.AddColumn("Result", 65536);
	table.EndTargets();
	
	
	
	for(int i = 0; i < max_timeslots; i++) {
		String slot_desc = "Time #" + IntStr(i);
		
		// Timeslot columns
		table.AddColumn(slot_desc + " timeslot method", 3);
		
		// Method #1, wday/hour
		table.AddColumn(slot_desc + " wdayhour begin", 24*7);
		table.AddColumn(slot_desc + " wdayhour length", 24);
		
		// Method #2, hour of day
		table.AddColumn(slot_desc + " hour begin", 24);
		table.AddColumn(slot_desc + " hour length", 24);
		
		// Method #3, always (no arguments)
		
		
		// Basket columns
		table.AddColumn(slot_desc + " basket method", 3);
		
		// Method #1 group id (priority increasing from highest=0)
		table.AddColumn(slot_desc + " time-pos group", 16);
		
		// Method #2, group id (priority increasing from highest=0)
		table.AddColumn(slot_desc + " all-time group", 16);
		
		// Method #3, symbol enabled bits
		for(int j = 0; j < symbols.GetCount(); j++)
			table.AddColumn(slot_desc + " sym" + IntStr(j), 3);
		
	}
	structural_begin = table.GetColumnCount();
	
	
	
	
	
	// Columns for structure
	structural_columns = 0;
	for(int i = 0; i <= max_depth; i++)
		structural_columns += pow(2, i);
	template_id = Find<Template>();
	const Vector<ArgType>& template_args = regs[template_id].args;
	template_arg_count = template_args.GetCount();
	
	// Columns for traditional indicator arguments
	ma_id = Find<MovingAverage>();
	int psych_id = Find<Psychological>();
	ASSERT_(ma_id != -1, "MovingAverage is not registered");
	ASSERT_(psych_id != -1, "Psychological is not registered");
	traditional_indicators = psych_id - ma_id + 1;
	traditional_arg_count = 0;
	for(int j = 0; j < traditional_indicators; j++) {
		int len = regs[ma_id + j].args.GetCount();
		traditional_enabled_cols.Add(1 + template_arg_count + j + traditional_arg_count);
		traditional_col_length.Add(len);
		traditional_arg_count += len;
	}
	
	// Enable slot, template arguments, enable trad. indicators, arguments for trad. indicators
	slot_args = 1 + template_arg_count + traditional_indicators + traditional_arg_count;
	
	// Priorities for structural elements are simplified to be:
	// - 3 for traditional indicators (DataBridge, ValueChange, other derivatives)
	// - 'structural_columns' for the template-tree
	structural_priorities = 3 + structural_columns;
	
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
			const FactoryRegister& part = regs[factory];
			const FactoryRegister& reg = regs[factory];
			const String& title = GetCtrlFactories()[factory].a;
			String desc = slot_desc + " " + title + " enabled";
			
			// Is traditional indicator important
			table.AddColumn(desc, 16);
			
			// Arguments for a traditional indicator
			for(int k = 0; k < reg.args.GetCount(); k++) {
				const ArgType& arg = reg.args[k];
				String desc = slot_desc + " " + title + " " + desc;
				int max_value = arg.max - arg.min + 1;
				table.AddColumn(desc, max_value);
			}
		}
	}
	
	
	// Test path->column function
	{
		Vector<int> path;
		int col = structural_begin;
		ASSERT(GetEnabledColumn(path) == col);
		col += slot_args;
		for(int i = 0; i < max_sources; i++) {
			path.Add(i);
			int tgt = GetEnabledColumn(path);
			ASSERT(tgt == col);
			col += slot_args;
			for(int j = 0; j < max_sources; j++) {
				path.Add(j);
				int tgt = GetEnabledColumn(path);
				ASSERT(tgt == col);
				col += slot_args;
				path.Pop();
			}
			path.Pop();
		}
		int col_count = table.GetColumnCount();
		ASSERT(col == col_count);
	}
}

void System::InitDataset() {
	
	// Create totally random work queue to avoid possibly wrong local fitting
	pl_queue.SetCount(max_queue);
	for(int i = 0; i < max_queue; i++) {
		PipelineItem& pi = pl_queue[i];
		pi.value.SetCount(table.GetRowBytes(), 0);
		for(int j = 1; j < table.GetColumnCount(); j++) {
			int max_value = table.GetColumn(j).max_value;
			int value = max_value > 1 ? Random(max_value) : 0;
			table.Set0(j, value, pi.value);
		}
	}
}

void System::RefreshPipeline() {
	const int target_col = 0;
	
	
	// Don't fill queue until it is small enough. Calculating decision tree is demanding.
	if (pl_queue.GetCount() > min_queue) return;
	
	
	// Sort table by descending result value: greatest at first
	// This also determines what is considered to be 'best' by the differential evolution
	table.Sort(target_col, true);
	const int table_rows = Upp::min(1000, table.GetCount());
	if (table_rows < 2)
		return;
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
				
				// Add the combination to the working queue
				pi->priority = i;
				pl_queue_lock.Enter();
				pl_queue.Add(pi.Detach());
				pl_queue_lock.Leave();
			}
		}
	}
}

void System::GetBasketArgs(PipelineItem& pi) {
	ASSERT(pi.sym != -1);
	Vector<int>& args = basket_args.GetAdd(pi.sym);
	CombineHash ch;
	args.SetCount(structural_begin-1);
	for(int i = 1; i < structural_begin; i++) {
		int j = table.Get0(i, pi.value);
		args[i-1] = j;
		ch << j << 1;
	}
	basket_hashes.GetAdd(pi.sym) = ch;
}

int System::GetCoreQueue(PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, Index<int>* tf_ids, int thread_id) {
	if (pi.sym == -1) {
		pi.sym = basket_sym_begin + thread_id;
		GetBasketArgs(pi);
	}
	int sym = pi.sym;
	Index<int> sym_ids;
	sym_ids.Add(sym);
	
	const int tf_count = GetPeriodCount();
	for (int tf = tf_count -1; tf >= 0; tf--) {
		
		// Skip disallowed tfs if allowed tf-list is given
		if (tf_ids && tf_ids->Find(tf) == -1)
			continue;
		
		// Columns for structure
		Vector<int> path;
		path.Add(0);
		GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
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
	
	// Remove duplicates
	Panic("TODO");
	
	return 0;
}

int System::GetCoreQueue(Vector<int>& path, const PipelineItem& pi, Vector<Ptr<CoreItem> >& ci_queue, int tf, const Index<int>& sym_ids) {
	ASSERT(table.GetColumnCount() > 0);
	ASSERT(ma_id != -1);
	
	int sub_pos = path.Top();
	bool is_template = sub_pos < 1000;
	int factory = is_template ? template_id : path.Top() - 1000;
	const int tf_count = GetPeriodCount();
	const int sym_count = GetTotalSymbolCount();
	
	
	// Template objects
	Vector<int> input_hashes;
	if (is_template) {
		ASSERT(sub_pos >= 0 && sub_pos < max_sources);
		
		// Columns for structure
		for(int i = 0; i < max_sources; i++) {
			
			// Check if sub-node is enabled
			path.Add(i);
			int col = GetEnabledColumn(path);
			if (!table.Get0(col, pi.value)) {
				path.Pop();
				continue;
			}
			int h = GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
			path.Pop();
			
			input_hashes.Add(h);
		}
		
		for(int i = 0; i < traditional_indicators; i++) {
			
			// Check if sub-node is enabled
			path.Add(1000 + ma_id + i);
			int col = GetEnabledColumn(path);
			if (!table.Get0(col, pi.value)) {
				path.Pop();
				continue;
			}
			int h = GetCoreQueue(path, pi, ci_queue, tf, sym_ids);
			path.Pop();
			
			input_hashes.Add(h);
		}
		Panic("TODO check input_hashes position matching... (connecting doesn't work at all with this yet)"); 
	}
	// Traditional indicator
	else {
		// Loop inputs of the factory
		const FactoryRegister& reg = regs[factory];
		
		// Connect input sources
		// Loop all inputs of the custom core-class
		Index<int> sub_sym_ids;
		for (int l = 0; l < reg.in.GetCount(); l++) {
			const RegisterInput& input = reg.in[l];
			FilterFunction fn = (FilterFunction)input.data;
			
			// Get all symbols what input requires
			sub_sym_ids.Clear();
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				int in_sym = sym_ids[i];
				
				for(int j = 0; j < GetTotalSymbolCount(); j++) {
					if (fn(this, in_sym, -1, j, -1))
						sub_sym_ids.FindAdd(j);
				}
			}
			
			int h = 0;
			if (!sub_sym_ids.IsEmpty()) {
				path.Add(1000 + input.factory);
				h = GetCoreQueue(path, pi, ci_queue, tf, sub_sym_ids);
				path.Pop();
			}
			
			input_hashes.Add(h);
		}
	}
	
	// Get the unique hash for core item
	int hash = 0;
	String unique;
	Vector<int> args;
	if (is_template) {
		// Mask this and all dependencies in the pipeline combination
		ASSERT(!pi.value.IsEmpty());
		Vector<byte> comb;
		MaskPath(pi.value, path, comb);
		
		// Get unique string and trim it
		String unique_long = HexVector(comb);
		for (int j = unique_long.GetCount()-1; j >= 0; j--) {
			if (unique_long[j] != '0') {
				unique = unique_long.Left(j+1);
				break;
			}
		}
		ASSERT(!unique.IsEmpty());
		hash = GetHash(comb);
	}
	else {
		CombineHash ch;
		
		int arg_col = GetEnabledColumn(path);
		if (arg_col != -1) {
			arg_col++; // enabled column
			
			const FactoryRegister& reg = regs[factory];
			for(int i = 0; i < reg.args.GetCount(); i++) {
				const ArgType& arg = reg.args[i];
				
				int value = arg.min + table.Get0(arg_col, pi.value);
				ASSERT(value >= arg.min && value <= arg.max);
				args.Add(value);
				
				ch << value << 1;
				
				arg_col++;
			}
			
			hash = ch;
		}
		// Unique string can be same for all traditional indicators
		unique = "src";
	}
	
	
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		
		// Get CoreItem
		CoreItem& ci = data[sym][tf][factory].GetAdd(hash);
		
		// Init object if it was just created
		if (ci.sym == -1) {
			int path_priority = GetPathPriority(path);
			ASSERT(path_priority >= 0 && path_priority < structural_priorities);
			
			ci.sym = sym;
			ci.tf = tf;
			ci.priority = // lower value is more important
				
				// Slowest tf is most important in this system.
				((tf_count-1-tf) * structural_priorities +
				
				// Structural column might require all symbols, so it is more important than symbol.
				// Also, root is the column 0 and it must be processed last.
				path_priority * sym_count) +
				
				// Lower symbols must be processed before higher, because cores are allowed to
				// require lower id symbols in the same timeframe and same structural column.
				sym;
			
			ci.factory = factory;
			ci.unique = unique;
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
	for (int l = 0; l < part.in.GetCount(); l++) {
		const RegisterInput& input = part.in[l];
		ConnectInput(l, 0, ci, input.factory, ci.input_hashes[l]);
	}
}

void System::ConnectInput(int input_id, int output_id, CoreItem& ci, int factory, int hash) {
	Vector<int> symlist, tflist;
	const RegisterInput& input = regs[ci.factory].in[input_id];
	const int sym_count = GetTotalSymbolCount();
	const int tf_count = GetPeriodCount();
	
	
	FilterFunction fn = (FilterFunction)input.data;
	ASSERT(fn);
	
	
	// Filter timeframes
	for(int i = tf_count-1; i >= ci.tf; i--) {
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
	
	for(int i = 0; i < symlist.GetCount(); i++) {
		int sym = symlist[i];
		
		for(int j = 0; j < tflist.GetCount(); j++) {
			int tf = tflist[j];
			
			
			CoreItem& src_ci = data[sym][tf][factory].GetAdd(hash);
			ASSERT_(src_ci.sym != -1, "Source CoreItem was not yet initialized");
			
			ASSERT_(src_ci.priority <= ci.priority, "Source didn't have higher priority than current");
			
			
			// Source found
			ci.AddInput(input_id, src_ci.sym, src_ci.tf, src_ci, output_id);
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
	c.factory = ci.factory;
	c.RefreshIO();
	c.SetUnique(ci.unique);
	c.SetSymbol(ci.sym);
	c.SetTimeframe(ci.tf, GetPeriod(ci.tf));
	c.LoadCache();
	
	// Connect object
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
	if (ci.args.GetCount() == arg.keys.GetCount()) {
		for(int i = 0; i < arg.keys.GetCount(); i++)
			arg.args[i] = ci.args[i];
		arg.SetStoring();
		c.IO(arg);
	}
	
	// Initialize
	c.InitAll();
}

}
