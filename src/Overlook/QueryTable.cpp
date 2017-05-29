#include "Overlook.h"

namespace Overlook {

DecisionTreeNode::DecisionTreeNode() {
	gain = 0.0;
	qt = NULL;
	column = -1;
	target_value = -1;
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
	test = PRUNE_ERROREST;
	z = 0.67; // equal to a confidence level of 50%
	//z = 0.69; // equal to a confidence level of 75%
	//z = 1.96; // equal to a confidence level of 95%
}

void QueryTable::GetDecisionTree(int target_id, DecisionTreeNode& root, int row_count) {
	Vector<int> dataset, validation_dataset;
	for(int i = 0; i < row_count; i++) {
		if (Random(10) == 5)
			validation_dataset.Add(i);
		else
			dataset.Add(i);
	}
	Index<int> used_cols;
	GetDecisionTree(target_id, root, root, dataset, used_cols, validation_dataset);
	
	// Post-prune leafs that has higher information gain than root
	if (test == PRUNE_ERROREST)
		PruneErrorEstimation(target_id, root, validation_dataset);
	else if (test == PRUNE_CHI2)
		PruneChi2(target_id, root, validation_dataset);
	else
		Panic("Invalid test type");
	
	ASSERT_(!root.nodes.IsEmpty(), "Oh no, all leafs were pruned!");
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
	int max_id = -1, max_value = -1;
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
			VectorMap<int, int>& pred_target_count = pred_counts[j];
			
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
			
			// Get most common target value
			VectorMap<int, int> target_values;
			for(int j = 0; j < pred_counts.GetCount(); j++) {
				const VectorMap<int, int>& pred_target_count = pred_counts[j];
				for(int k = 0; k < pred_target_count.GetCount(); k++)
					target_values.GetAdd(pred_target_count.GetKey(k), 0) += pred_target_count[k];
			}
			SortByValue(target_values, StdGreater<int>());
			int target_value = target_values.GetKey(0);
			
			max_gain = gain;
			max_id = i;
			max_value = target_value;
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
	ASSERT(max_value != -1);
	
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


void QueryTable::PruneErrorEstimation(int target_id, const DecisionTreeNode& root, const Vector<int>& validation_dataset) {
	
	// Calculate error (http://www.saedsayad.com/decision_tree_overfitting.htm)
	double f = GetValidationError(target_id, root, validation_dataset);
	double N = validation_dataset.GetCount();
	double a = f + z*z / (2.0*N) + z * sqrt(f/N - f*f/N + z*z/(4.0*N*N));
	double b = 1.0 + z*z/N;
	root.error = a / b;
	
	double root_error = root.error;
	
	// Calculate combined error rate of leafs
	double leaf_error_sum = 0;
	double dataset_total = 0.0;
	for(int i = 0; i < root.nodes.GetCount(); i++) {
		const DecisionTreeNode& n = root.nodes[i];
		double size = n.dataset_size;
		double leaf_error = size * n.error;
		leaf_error_sum += leaf_error;
		dataset_total += size;
	}
	leaf_error_sum /= dataset_total;
	
	// "The error rate at the parent node is 0.46 and since the error rate for its children (0.51)
	//  increases with the split, we do not want to keep the children."
	//   - http://www.saedsayad.com/decision_tree_overfitting.htm
	if (leaf_error_sum > root_error) {
		root.nodes.Clear();
	} else {
		for(int i = 0; i < root.nodes.GetCount(); i++)
			PruneErrorEstimation(root.nodes[i]);
	}
}

void QueryTable::PruneChi2(int target_id, const DecisionTreeNode& root, const Vector<int>& validation_dataset) {
	// Calculate Chi^2 value and its probability
	VectorMap<int, VectorMap<int, int> > max_gain_pred_counts;*/
	
	Vector<int> cols;
	VectorMap<int,int> rows;
	int sum = 0;
	cols.SetCount(max_gain_pred_counts.GetCount(), 0);
	for(int i = 0; i < max_gain_pred_counts.GetCount(); i++) {
		const VectorMap<int, int>& vec = max_gain_pred_counts[i];
		int& col_sum = cols[i];
		for(int j = 0; j < vec.GetCount(); j++) {
			int t = vec.GetKey(j);
			int v = vec[j];
			rows.GetAdd(t,0) += v;
			col_sum += v;
			sum += v;
		}
	}
	double dsum = sum;
	double chi_sum = 0;
	for(int i = 0; i < cols.GetCount(); i++) {
		double c = cols[i];
		for(int j = 0; j < rows.GetCount(); j++) {
			double r = rows[j];
			double e = c * r / dsum;
			double n = max_gain_pred_counts[i].GetAdd(rows.GetKey(j), 0);
			double a = n - e;
			double X = a*a / e;
			chi_sum += X;
		}
	}
	double df = (cols.GetCount() - 1)*(rows.GetCount()-1);
	root.chi2 = sqrt(chi_sum / (dsum * sqrt(df)));

	
	double limit = 0.05;
	if (root.chi2 <= limit)
		root.nodes.Clear();
	else {
		for(int i = 0; i < root.nodes.GetCount(); i++) {
			PruneChi2(root.nodes[i]);
		}
	}
}















QueryTableForecaster::QueryTableForecaster() {
	
}

void QueryTableForecaster::Init() {
	
	// Add targets
	for(int i = 0; i < 3; i++) {
		int max_value = pow(2, 1+i);
		String change = IntStr(max_value) + "-Change ";
		for(int j = 0; j < 6; j++) {
			int max_value = pow(2, 1+j);
			String desc = change + IntStr(max_value) + "-Step";
			qt.AddColumn(desc, max_value);
		}
	}
	qt.EndTargets();
	
	// Add constants columns
	/*
	qt.AddColumn("Half-Year",		(2));
	qt.AddColumn("Quarter-Year",	(4));
	qt.AddColumn("1/6-Year",		(6));
	qt.AddColumn("Month",			(12));
	
	qt.AddColumn("1/2-Month",		(2));
	qt.AddColumn("1/4-Month",		(5));
	qt.AddColumn("Day",				(31));
	
	qt.AddColumn("Week-Begin",		(2));
	qt.AddColumn("Wday",			(7));
	qt.AddColumn("WdayHour",		(7*24));
	
	qt.AddColumn("1/2-day",			(2));
	qt.AddColumn("1/4-day",			(4));
	qt.AddColumn("1/8-day",			(8));
	qt.AddColumn("1/12-day",		(12));
	qt.AddColumn("Hour",			(24));
	
	qt.AddColumn("1/2-hour",		(2));
	qt.AddColumn("1/4-hour",		(4));
	qt.AddColumn("5-min",			(12));*/
	
	qt.AddColumn("Month",			(12));
	qt.AddColumn("Day",				(31));
	qt.AddColumn("Wday",			(7));
	qt.AddColumn("WdayHour",		(7*24));
	qt.AddColumn("Hour",			(24));
	qt.AddColumn("5-min",			(12));
	
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
	double max_change = bc.GetMax();
	double min_change = bc.GetMin();
	double diff = max_change - min_change;
	
	bars -= 32;
	
	qt.SetCount(bars);
	
	TimeStop ts;
	for (int i = counted; i < bars; i++) {
		SetSafetyLimit(i+32);
		
		Time t = bs.GetTimeTf(tf, i);
		int month = t.month-1;
		int day = t.day-1;
		int hour = t.hour;
		int minute = t.minute;
		int dow = DayOfWeek(t);
		int wdayhour = dow*24 + t.hour;
		double open = Open(i);
		
		int pos = 0;
		for(int j = 0; j < 3; j++) {
			int len = pow(2, j);
			int read_pos = i + len;
			double close = Open(read_pos);
			double change = (open != 0.0 ? close / open - 1.0 : 0.0);
			
			for(int k = 0; k < 6; k++) {
				int div = pow(2, k + 1);
				double step = diff / div;
				int v = (change - min_change) / step;
				if (v < 0)
					v = 0;
				if (v >= div)
					v = div -1;
				
				qt.Set(i, pos++, v);
			}
		}
		
		/*qt.Set(i, pos++, month / 6);
		qt.Set(i, pos++, month / 3);
		qt.Set(i, pos++, month / 2);
		qt.Set(i, pos++, month);
		
		qt.Set(i, pos++, Upp::min(1, day / 15));
		qt.Set(i, pos++, day / 7);
		qt.Set(i, pos++, day);
		
		qt.Set(i, pos++, dow >= 3);
		qt.Set(i, pos++, dow);
		qt.Set(i, pos++, wdayhour);
		
		qt.Set(i, pos++, hour / 12);
		qt.Set(i, pos++, hour / 6);
		qt.Set(i, pos++, hour / 3);
		qt.Set(i, pos++, hour / 2);
		qt.Set(i, pos++, hour);
		
		qt.Set(i, pos++, minute / 30);
		qt.Set(i, pos++, minute / 15);
		qt.Set(i, pos++, minute / 5);*/
		qt.Set(i, pos++, month);
		qt.Set(i, pos++, day);
		qt.Set(i, pos++, dow);
		qt.Set(i, pos++, wdayhour);
		qt.Set(i, pos++, hour);
		qt.Set(i, pos++, minute / 5);
		
	}
	
	LOG(ts.ToString());
	
	int target = 0;
	for(int i = 0; i < 3; i++) {
		for(int j = 0; j < 6; j++) {
			LOG("TARGET " << target);
			DecisionTreeNode tree;
			qt.GetDecisionTree(target++, tree, bars);
			LOG(tree.ToString());
			
		}
	}
	
	
	//int col = qt.GetLargestInfoGainPredictor(0);
	//DUMP(col);
	//DUMPC(qt.GetInfoGains());
}















QueryTableHugeForecaster::QueryTableHugeForecaster() {
	
}

void QueryTableHugeForecaster::Init() {
	
}

void QueryTableHugeForecaster::Start() {
	
	Panic("TODO");
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
