#include "Overlook.h"

namespace Overlook {

Color RainbowColor(double progress);

DecisionTreeNode::DecisionTreeNode() {
	gain = 0.0;
	qt = NULL;
	column = -1;
	target_value = -1;
	target_value_count = 0;
	subset_column = -1;
	subset_column_value = -1;
	dataset_size = 0;
	error = 0.0;
}

String DecisionTreeNode::LineString(int depth) const {
	String out;
	for(int i = 0; i < depth; i++)
		out << "    ";
	if (column == -1) {out << "<invalid>\n"; return out;}
	if (subset_column != -1) {
		out << "(Subset: \"" << qt->columns[subset_column].name << "\" value=" << subset_column_value << "),\t";
	}
	out << "\"" << qt->columns[column].name << "\" column=" << column << " gain=" << gain << " dataset_size=" << dataset_size << " target_value=" << target_value << " error=" << error << "\n";
	for(int i = 0; i < nodes.GetCount(); i++)
		out << nodes[i].LineString(depth+1);
	return out;
}



QueryTable::QueryTable() {
	bytes = 0;
	bits = 0;
	target_count = 0;
	test = PRUNE_REDUCEERROR;
	z = 0.67; // equal to a confidence level of 50%
	//z = 0.69; // equal to a confidence level of 75%
	//z = 1.96; // equal to a confidence level of 95%
	underfit_limit = 0.666;
	overfit_size_limit = 2;
}

void QueryTable::GetDecisionTree(int target_id, DecisionTreeNode& root, int row_count) {
	bool has_validation_set = test == PRUNE_REDUCEERROR;
	
	Vector<int> dataset, validation_dataset;
	for(int i = 0; i < row_count; i++) {
		if (has_validation_set && Random(10) == 1)
			validation_dataset.Add(i);
		else
			dataset.Add(i);
	}
	Index<int> used_cols;
	GetDecisionTree(target_id, root, root, dataset, used_cols, validation_dataset);
	
	// Post-prune leafs that has higher information gain than root
	if (test == PRUNE_ERROREST)
		PruneErrorEstimation(target_id, root);
	else if (test == PRUNE_REDUCEERROR)
		PruneReducedError(target_id, root, validation_dataset);
	else
		Panic("Invalid test type");
	
	//ASSERT_(!root.nodes.IsEmpty(), "Oh no, all leafs were pruned!");
}


void QueryTable::GetDecisionTree(int target_id, const DecisionTreeNode& root, DecisionTreeNode& node, const Vector<int>& dataset, Index<int>& used_columns, const Vector<int>& validation_dataset) {
	
	// Prepare function
	ASSERT(!columns.IsEmpty());
	ASSERT(!dataset.IsEmpty());
	
	// Find all unique target values, and count them
	VectorMap<int, int> target_count;
	for(int i = 0; i < dataset.GetCount(); i++) {
		int row = dataset[i];
		int t = Get(row, target_id);
		target_count.GetAdd(t, 0)++;
	}
	
	// Count total (some values could have been skipped as invalid)
	double total = 0;
	for(int i = 0; i < target_count.GetCount(); i++)
		total += target_count[i];
	
	// Calculate entropy
	double entropy = 0;
	for(int i = 0; i < target_count.GetCount(); i++)  {
		double part = target_count[i] * 1.0 / total;
		entropy -= part * log2((double)part);
	}
	
	// Loop over all columns, which are not target columns (columns between targets and forecasts)
	int processed_columns = 0;
	double max_gain = -DBL_MAX;
	int max_id = -1, max_value = -1, max_value_count;
	for(int i = this->target_count; i < columns.GetCount(); i++) {
		
		// Skip columns (predictors) which have been added to the decision tree already
		if (used_columns.Find(i) != -1) continue;
		processed_columns++;
		
		// Vector for unique values in the column (predictor categories)
		VectorMap<int, VectorMap<int, int> > pred_counts;
		
		double pred_total = 0;
		for(int j = 0; j < dataset.GetCount(); j++) {
			// Get column value and target value
			int row = dataset[j];
			int c = Get(row, i);
			int t = Get(row, target_id);
			
			// Get map by column value
			VectorMap<int, int>& pred_target_count = pred_counts.GetAdd(c);
			
			// Increase target count with this column value by one
			pred_target_count.GetAdd(t, 0)++;
			
			pred_total += 1;
		}
		
		// Loop over all unique values in the column
		double pred_entropy = 0;
		for(int j = 0; j < pred_counts.GetCount(); j++) {
			const VectorMap<int, int>& pred_target_count = pred_counts[j];
			
			// Sum total value count in the table with this unique predictor category
			int pred_cat_total = 0;
			for(int k = 0; k < pred_target_count.GetCount(); k++)
				pred_cat_total += pred_target_count[k];
			
			// Calculate entropy for predictor category (unique column value)
			double pred_cat_entropy = 0;
			for(int k = 0; k < pred_target_count.GetCount(); k++) {
				double part = pred_target_count[k] * 1.0 / pred_cat_total;
				pred_cat_entropy -= part * log2((double)part);
			}
			double col_entropy = (double)pred_cat_total / pred_total * pred_cat_entropy;
			pred_entropy += col_entropy;
			
		}
		
		// Calculate information gain value with this predictor in the current dataset
		double gain = entropy - pred_entropy;
		
		// Find the maximum information gain predictor
		if (gain > max_gain) {
			
			// Get most common value
			VectorMap<int, int> target_values;
			for(int j = 0; j < pred_counts.GetCount(); j++) {
				const VectorMap<int, int>& pred_target_count = pred_counts[j];
				for(int k = 0; k < pred_target_count.GetCount(); k++)
					target_values.GetAdd(pred_target_count.GetKey(k), 0) += pred_target_count[k];
			}
			SortByValue(target_values, StdGreater<int>());
			
			max_gain = gain;
			max_id = i;
			max_value = target_values.GetKey(0);
			max_value_count = target_values[0];
		}
	}
	
	// Return with invalid value
	if (max_id == -1)
		return;
	ASSERT(processed_columns > 0);
	
	// Set current node column id
	ASSERT(max_id != -1);
	node.column = max_id;
	node.gain = max_gain;
	node.qt = this;
	node.dataset_size = dataset.GetCount();
	node.target_value = max_value;
	node.target_value_count = max_value_count;
	ASSERT(max_value != -1);
	
	// Calculate error (http://www.saedsayad.com/decision_tree_overfitting.htm)
	double f = 1.0 - (double)node.target_value_count / (double)node.dataset_size;
	double N = node.dataset_size;
	double a = f + z*z / (2.0*N) + z * sqrt(f/N - f*f/N + z*z/(4.0*N*N));
	double b = 1.0 + z*z/N;
	node.error = a / b;
	
	// Return when there is no information gain at all
	if (max_gain <= 0)
		return;
	
	used_columns.Add(max_id);
	
	// Find all unique value datasets
	VectorMap<int, Vector<int> > pred_ids;
	for(int i = 0; i < dataset.GetCount(); i++) {
		int row = dataset[i];
		int c = Get(row, max_id);
		pred_ids.GetAdd(c).Add(row);
	}
	
	// Loop all subsets of data with unique column value (predictor category)
	for(int i = 0; i < pred_ids.GetCount(); i++) {
		const Vector<int>& subset = pred_ids[i];
		
		DecisionTreeNode& sub = node.nodes.Add();
		sub.qt = this;
		sub.subset_column = max_id;
		sub.subset_column_value = pred_ids.GetKey(i);
		GetDecisionTree(target_id, root, sub, subset, used_columns, validation_dataset);
		if (sub.column == -1)
			node.nodes.Pop();
	}
	
	// Remove added column id
	used_columns.Pop();
	
}


double QueryTable::GetValidationError(int target_id, const DecisionTreeNode& root, const Vector<int>& validation_dataset) {
	const Column& tcol = columns[target_id];
	
	double err_sum = 0;
	for(int i = 0; i < validation_dataset.GetCount(); i++) {
		int row = validation_dataset[i];
		
		int t = Get(row, target_id);
		int p = Predict(root, row, target_id);
		
		double err = fabs((double)t - (double)p) / ((double)tcol.max_value - 1.0);
		err_sum += err;
	}
	double err = err_sum / validation_dataset.GetCount();
	return err;
}


void QueryTable::PruneErrorEstimation(int target_id, DecisionTreeNode& root) {
	if (root.nodes.IsEmpty()) return;
	
	// Avoid underfitting! Do not prune if error percentage is small enough!
	double data_error = 1.0 - (double)root.target_value_count / (double)root.dataset_size;
	bool avoid_underfitting = data_error < underfit_limit;
	
	// Calculate combined error rate of leafs
	double leaf_error_sum = 0;
	if (!avoid_underfitting) {
		double dataset_total = 0.0;
		for(int i = 0; i < root.nodes.GetCount(); i++) {
			const DecisionTreeNode& n = root.nodes[i];
			double size = n.dataset_size;
			double leaf_error = size * n.error;
			leaf_error_sum += leaf_error;
			dataset_total += size;
		}
		leaf_error_sum /= dataset_total;
	}
	
	// "The error rate at the parent node is 0.46 and since the error rate for its children (0.51)
	//  increases with the split, we do not want to keep the children."
	//   - http://www.saedsayad.com/decision_tree_overfitting.htm
	if (!avoid_underfitting && leaf_error_sum > root.error || root.dataset_size <= overfit_size_limit) {
		root.nodes.Clear();
	} else {
		for(int i = 0; i < root.nodes.GetCount(); i++)
			PruneErrorEstimation(target_id, root.nodes[i]);
	}
}

void QueryTable::PruneReducedError(int target_id, DecisionTreeNode& root, const Vector<int>& validation_dataset) {
	if (root.nodes.IsEmpty()) return;
	
	// Calculate correctly predicted value count
	// Get subset for nodes
	double node_error = 0;
	double leaf_error_sum = 0;
	Vector<Vector<int> > subsets;
	int predicted_value_count = 0;
	Vector<Tuple2<int, int> > substats;
	substats.SetCount(root.nodes.GetCount(), Tuple2<int, int>(0,0));
	subsets.SetCount(root.nodes.GetCount());
	ASSERT(root.target_value != -1);
	for(int i = 0; i < validation_dataset.GetCount(); i++) {
		int row = validation_dataset[i];
		
		// Check matching target value
		int t = Get(row, target_id);
		if (t == root.target_value)
			predicted_value_count++;
		
		// Add value to specific subset
		int c = Get(row, root.column);
		for(int j = 0; j < root.nodes.GetCount(); j++) {
			const DecisionTreeNode& sub = root.nodes[j];
			if (c == sub.subset_column_value) {
				Tuple2<int, int>& stats = substats[j];
				subsets[j].Add(row);
				ASSERT(sub.target_value != -1);
				if (t == sub.target_value) stats.a++;
				stats.b++;
				break;
			}
		}
	}
	
	// Avoid underfitting! Do not prune if error percentage is small enough!
	double f = 1.0 - (double)predicted_value_count / (double)validation_dataset.GetCount();
	bool avoid_underfitting = f < underfit_limit;
	
	if (!avoid_underfitting) {
		// Calculate error (http://www.saedsayad.com/decision_tree_overfitting.htm)
		double N = validation_dataset.GetCount();
		double a = f + z*z / (2.0*N) + z * sqrt(f/N - f*f/N + z*z/(4.0*N*N));
		double b = 1.0 + z*z/N;
		node_error = a / b;
		
		
		// Calculate combined error rate of leafs
		double dataset_total = 0.0;
		for(int i = 0; i < root.nodes.GetCount(); i++) {
			const Tuple2<int, int>& stats = substats[i];
			
			// Calculate error (http://www.saedsayad.com/decision_tree_overfitting.htm)
			double f = 1.0 - (double)stats.a / (double)stats.b;
			double N = stats.b;
			double a = f + z*z / (2.0*N) + z * sqrt(f/N - f*f/N + z*z/(4.0*N*N));
			double b = 1.0 + z*z/N;
			double subnode_error = a / b;
			
			double leaf_error = stats.b * subnode_error;
			leaf_error_sum += leaf_error;
			dataset_total += stats.b;
		}
		leaf_error_sum /= dataset_total;
	}
	
	// "The error rate at the parent node is 0.46 and since the error rate for its children (0.51)
	//  increases with the split, we do not want to keep the children."
	//   - http://www.saedsayad.com/decision_tree_overfitting.htm
	if (!avoid_underfitting && leaf_error_sum > node_error || root.dataset_size <= overfit_size_limit) {
		root.nodes.Clear();
	} else {
		for(int i = 0; i < root.nodes.GetCount(); i++)
			PruneReducedError(target_id, root.nodes[i], subsets[i]);
	}
}








#define PREV_STEPS 8
#define PREV_LEN 8
#define MAXTIMESTEP 6

QueryTableForecaster::QueryTableForecaster() {
	
}

void QueryTableForecaster::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1);
	SetCoreMaximum(+1);
	SetBufferColor(0, Green);
	SetBufferLineWidth(0, 2);
	
	
	// Add targets
	qt.AddColumn("Next change", PREV_STEPS);
	qt.EndTargets();
	
	// Add previous values
	for(int i = 1; i <= PREV_LEN; i++) {
		qt.AddColumn("Cur diff -" + IntStr(i), PREV_STEPS);
		if (i > 1) qt.AddColumn("Change -" + IntStr(i), PREV_STEPS);
	}
	
	// Add constants columns
	//  Note: adding month and day causes underfitting
	qt.AddColumn("Wday",			7);
	qt.AddColumn("Hour",			24);
	qt.AddColumn("5-min",			12);
	
	// Add columns from inputs
	for(int i = 0; i < optional_inputs.GetCount(); i++) {
		Input& indi_input = optional_inputs[i];
		if (indi_input.sources.IsEmpty()) continue;
		ASSERT(indi_input.sources.GetCount() == 1);
		
		Panic("TODO");
	}
}

void QueryTableForecaster::Start() {
	int bars = GetBars();
	int counted = GetCounted();
	int tf = GetTf();
	BaseSystem& bs = GetBaseSystem();
	ValueChange& bc = *Get<ValueChange>();
	double max_change = bc.GetMedianMax() * 2;
	double min_change = bc.GetMedianMin() * 2;
	double diff = max_change - min_change;
	
	bars -= 1;
	
	qt.Reserve(bars);
	
	TimeStop ts;
	VectorMap<int, int> in_qt;
	in_qt.Reserve(bars - counted);
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+1);
		
		// Only add valid data to avoid underfitting
		double open = Open(i);
		double close = Open(i+1);
		if (open == close) continue;
		
		// Add row
		int row = qt.GetCount();
		in_qt.Add(i, row);
		qt.SetCount(row+1);
		
		// Get some time values in binary format (starts from 0)
		Time t = bs.GetTimeTf(tf, i);
		int month = t.month-1;
		int day = t.day-1;
		int hour = t.hour;
		int minute = t.minute;
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		int pos = 0;
		
		// Add target value after one timestep
		double change = (open != 0.0 ? close / open - 1.0 : 0.0);
		const int div = PREV_STEPS;
		double step = diff / div;
		int v = (change - min_change) / step;
		if (v < 0)
			v = 0;
		if (v >= div)
			v = div -1;
		qt.Set(row, pos++, v);
		
		// Add previous changes in value
		double prev = open;
		for(int j = 1; j <= PREV_LEN; j++) {
			int k = Upp::max(0, i - j);
			double cur = Open(k);
			
			double change1 = cur != 0.0 ? open / cur - 1.0 : 0.0;
			int v = (change1 - min_change) / step;
			if (v < 0) v = 0;
			if (v >= div) v = div -1;
			qt.Set(row, pos++, v);
			
			if (j > 1) {
				double change2 = cur != 0.0 ? prev / cur - 1.0 : 0.0;
				v = (change2 - min_change) / step;
				if (v < 0) v = 0;
				if (v >= div) v = div -1;
				qt.Set(row, pos++, v);
			}
			
			prev = cur;
		}
		
		// Add constant time values
		qt.Set(row, pos++, dow);
		qt.Set(row, pos++, hour);
		qt.Set(row, pos++, minute / 5);
		
		
		// TODO: add indicators
		
	}
	
	// Create decision tree
	DecisionTreeNode tree;
	qt.GetDecisionTree(0, tree, qt.GetCount());
	
	// Draw error oscillator
	Buffer& buf = GetBuffer(0);
	for (int i = counted; i < bars; i++) {
		
		// Skip invalid values, which weren't added to the query-table
		int j = in_qt.Find(i);
		if (j == -1) {
			buf.Set(i, -1);
			continue;
		}
		int row = in_qt[j];
		
		// Get prediction and correct value
		double predicted = qt.Predict(tree, row, 0);
		double correct = qt.Get(row, 0);
		double diff = fabs(predicted - correct) / PREV_STEPS;
		buf.Set(i, diff);
	}
}















QueryTableHugeForecaster::QueryTableHugeForecaster() {
	corr_period = 4;
}

void QueryTableHugeForecaster::Init() {
	SetCoreSeparateWindow();
	SetCoreMinimum(-1);
	SetCoreMaximum(+1);
	for(int i = 0; i < 6; i++) {
		SetBufferColor(i, RainbowColor(i / 6.0));
		SetBufferLineWidth(i, 6-i);
	}
	
	// Add targets
	for(int i = 0; i < PREV_LEN; i++) {
		int len = pow(2, i);
		qt.AddColumn("Change +" + IntStr(len), PREV_STEPS);
	}
	qt.EndTargets();
	
	BaseSystem& bs = GetBaseSystem();
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount() - GetTimeframe();
	
	// Add previous values
	for (int sym = 0; sym < sym_count; sym++) {
		if (sym == GetSymbol()) continue; // skip this
		for (int tf = 0; tf < tf_count; tf++) {
			qt.AddColumn("Cur diff -1 (" + IntStr(sym) + ":" + IntStr(tf) + ")", PREV_STEPS);
			qt.AddColumn("Correlation (" + IntStr(sym) + ":" + IntStr(tf) + ")", PREV_STEPS);
		}
	}
	
	// Add constants columns
	//  Note: adding month and day causes underfitting
	qt.AddColumn("Wday",			7);
	qt.AddColumn("Hour",			24);
	qt.AddColumn("5-min",			12);
	
	// Add columns from inputs
	/*for (int sym = 0; sym < sym_count; sym++) {
		if (sym == GetSymbol()) continue; // skip this
		for (int tf = 0; tf < tf_count; tf++) {
			for(int i = 0; i < optional_inputs.GetCount(); i++) {
				Input& indi_input = optional_inputs[i];
				if (indi_input.sources.IsEmpty()) continue;
				ASSERT(indi_input.sources.GetCount() == 1);
				
				Panic("TODO");
			}
		}
	}*/
}

void QueryTableHugeForecaster::Start() {
	int id = GetSymbol();
	int thistf = GetTf();
	int bars = GetBars();
	int counted = GetCounted();
	if (bars == counted)
		return;
	
	
	BaseSystem& bs = GetBaseSystem();
	int sym_count = bs.GetSymbolCount();
	int tf_count = bs.GetPeriodCount() - GetTimeframe();
	
	// Get some useful values for all syms and tfs
	Vector<ConstBuffer*> bufs;
	Vector<double> max_changes, min_changes, diffs;
	bufs.SetCount(sym_count * tf_count, NULL);
	max_changes.SetCount(sym_count * tf_count, 0);
	min_changes.SetCount(sym_count * tf_count, 0);
	diffs.SetCount(sym_count * tf_count, 0);
	
	Vector<int> time_pos;
	time_pos.SetCount(tf_count, 0);
	
	for (int sym = 0; sym < sym_count; sym++) {
		for (int tf = 0; tf < tf_count; tf++) {
			int i = sym * tf_count + tf;
			bufs[i] = &GetInputBuffer(0, sym, tf + thistf, 0);
			ValueChange& bc = *dynamic_cast<ValueChange*>(GetInputCore(1, sym, tf + thistf));
			double max_change = bc.GetMedianMax() * 2;
			double min_change = bc.GetMedianMin() * 2;
			double diff = max_change - min_change;
			//LOG(Format("i=%d sym=%d tf=%d max=%f min=%f diff=%f", i, sym, tf, max_change, min_change, diff));
			max_changes[i] = max_change;
			min_changes[i] = min_change;
			diffs[i] = diff;
		}
	}
	
	int ii = id * tf_count;
	double max_change = max_changes[ii];
	double min_change = min_changes[ii];
	double diff       = diffs[ii];
	//LOG(Format("i=%d sym=%d tf=%d max=%f min=%f diff=%f", ii, id, 0, max_change, min_change, diff));
	
	int peek = pow(2, MAXTIMESTEP-1);
	bars -= peek;
	
	qt.Reserve(bars);
	
	TimeStop ts;
	VectorMap<int, int> in_qt;
	in_qt.Reserve(bars - counted);
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+peek);
		
		// Only add valid data to avoid underfitting
		double open = Open(i);
		double close = Open(i+1);
		if (open == close) continue;
		
		// Add row
		int row = qt.GetCount();
		in_qt.Add(i, row);
		qt.SetCount(row+1);
		
		// Get some time values in binary format (starts from 0)
		Time t = bs.GetTimeTf(thistf, i);
		int month = t.month-1;
		int day = t.day-1;
		int hour = t.hour;
		int minute = t.minute;
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		int pos = 0;
		time_pos[0] = i;
		for(int j = 1; j < tf_count; j++)
			time_pos[j] = bs.GetShiftTf(thistf, thistf + j, i);
		
		// Add target value after timestep
		for(int j = 0; j < MAXTIMESTEP; j++) {
			int len = pow(2, j);
			int k = i + len;
			double next = Open(k);
			double change = open != 0.0 ? next / open - 1.0 : 0.0;
			const int div = PREV_STEPS;
			double step = diff / div;
			int v = (change - min_change) / step;
			if (v < 0)
				v = 0;
			if (v >= div)
				v = div -1;
			qt.Set(row, pos++, v);
		}
		
		// Add previous changes in value
		for (int sym = 0, csym = 0; sym < sym_count; sym++) {
			if (sym == GetSymbol()) continue; // skip this
			for (int tf = 0; tf < tf_count; tf++) {
				int j = sym * tf_count + tf;
				ConstBuffer& buf = *bufs[j];
				int k = time_pos[tf];
				double open = k > 0 ? buf.GetUnsafe(k-1) : 0.0;
				double cur = buf.GetUnsafe(k);
				double min_change = min_changes[j];
				double diff = diffs[j];
				const int div = PREV_STEPS;
				double step = diff / div;
				
				// Difference to previous time-position
				double change = open != 0.0 ? cur / open - 1.0 : 0.0;
				int v = (change - min_change) / step;
				if (v < 0) v = 0;
				if (v >= div) v = div -1;
				qt.Set(row, pos++, v);
				
				// Correlation to the main symbol
				ConstBuffer& cbuf = GetInputBuffer(2, GetSymbol(), thistf + tf, csym);
				double corr = cbuf.GetUnsafe(k);
				v = (corr + 1.0) / 2.0 * PREV_STEPS;
				if (v == PREV_STEPS) v = PREV_STEPS - 1; // equal to 1.0 doesn't need own range
				ASSERT(v >= 0 && v < PREV_STEPS);
				qt.Set(row, pos++, v);
			}
			csym++;
		}
		
		// Add constant time values
		qt.Set(row, pos++, dow);
		qt.Set(row, pos++, hour);
		qt.Set(row, pos++, minute / 5);
		
		
		// TODO: add indicators
		
	}
	
	// Create decision trees
	tree.Clear();
	tree.SetCount(MAXTIMESTEP);
	for(int i = 0; i < tree.GetCount(); i++)
		qt.GetDecisionTree(i, tree[i], qt.GetCount());
	
	// Draw error oscillator
	for (int i = counted; i < bars; i++) {
		
		// Skip invalid values, which weren't added to the query-table
		int j = in_qt.Find(i);
		if (j == -1) {
			for(int j = 0; j < MAXTIMESTEP; j++)
				GetBuffer(j).Set(i, -1);
			continue;
		}
		int row = in_qt[j];
		
		// Get prediction and correct value
		for(int j = 0; j < MAXTIMESTEP; j++) {
			double predicted = qt.Predict(tree[j], row, j);
			double correct = qt.Get(row, j);
			double diff = fabs(predicted - correct) / PREV_STEPS;
			GetBuffer(j).Set(i, diff);
		}
	}
}


QueryTableMetaForecaster::QueryTableMetaForecaster() {
	
}

void QueryTableMetaForecaster::Init() {
	
}

void QueryTableMetaForecaster::Start() {
	
	Panic("TODO");
}


QueryTableAgent::QueryTableAgent() {
	
}

void QueryTableAgent::Init() {
	
}

void QueryTableAgent::Start() {
	
	Panic("TODO");
}


QueryTableHugeAgent::QueryTableHugeAgent() {
	
}

void QueryTableHugeAgent::Init() {
	
}

void QueryTableHugeAgent::Start() {
	
	Panic("TODO");
}


QueryTableMetaAgent::QueryTableMetaAgent() {
	
}

void QueryTableMetaAgent::Init() {
	
}

void QueryTableMetaAgent::Start() {
	
	Panic("TODO");
}


QueryTableDoubleAgent::QueryTableDoubleAgent() {
	
}

void QueryTableDoubleAgent::Init() {
	
}

void QueryTableDoubleAgent::Start() {
	
	Panic("TODO");
}

}
