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
	if (column == -1) {
		out << "<invalid>\n";
		return out;
	}
	if (subset_column != -1) {
		out << "(Subset: \"" << qt->columns[subset_column].name
		    << "\" value=" << subset_column_value << "),\t";
	}
	out << "\"" << qt->columns[column].name << "\" column=" << column << " gain=" << gain
	    << " dataset_size=" << dataset_size << " target_value=" << target_value << " error="
	    << error << "\n";
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
	scale = 0.7;
	probability = 0.9;
}

void QueryTable::Serialize(Stream& s) {
	s % data % columns % target_name % z % underfit_limit % overfit_size_limit
	  % bytes % bits % target_count % test;
}

int QueryTable::Get(int row, int col) const {
	return Get0(col, data[row]);
}

void QueryTable::Set(int row, int col, int value) {
	Set0(col, value, data[row]);
}

int QueryTable::Get0(int col, const Vector<byte>& vec) const {
	int out = 0;
	const Column& pred = columns[col];
	int byt = pred.bit_begin / 8;
	int bit = pred.bit_begin % 8;
	typedef const byte ConstByte;
	ConstByte* v = vec.Begin() + byt;
	for(int i = 0; i < pred.size; i++) {
		bool b = *v & (1 << bit);
		if (b)
			out |= 1 << i;
		bit++;
		if (bit == 8) {
			bit = 0;
			v++;
		}
	}
	return out;
}

void QueryTable::Set0(int col, int value, Vector<byte>& vec) {
	const Column& pred = columns[col];
	ASSERT(value >= 0 && value < pred.max_value);
	int byt = pred.bit_begin / 8;
	int bit = pred.bit_begin % 8;
	byte* v = vec.Begin() + byt;
	for(int i = 0; i < pred.size; i++) {
		bool b = value & (1 << i);
		if (!b)		*v &= ~(1ULL << bit);
		else		*v |=  (1ULL << bit);
		bit++;
		if (bit == 8) {
			bit = 0;
			v++;
		}
	}
}

void QueryTable::SetCount(int i) {
	int old_count = data.GetCount();
	if (old_count == i) return;
	data.SetCount(i);
	for(int i = old_count; i < data.GetCount(); i++) data[i].SetCount(bytes, 0);
}

QueryTable::Column& QueryTable::AddColumn(const String& name, int max_value) {
	int bits = MaxBits(max_value-1);
	Column& p = columns.Add();
	p.name = name;
	p.max_value = max_value;
	p.bit_begin = this->bits;
	p.bit_end = p.bit_begin + bits;
	p.size = bits;
	this->bits += bits;
	this->bytes = this->bits / 8;
	if (this->bits % 8 != 0) this->bytes++;
	return p;
}

int QueryTable::Predict(const DecisionTreeNode& root, int row, int target_col) const {
	return Predict(root, data[row], target_col);
}

int QueryTable::Predict(const DecisionTreeNode& root, const Vector<byte>& row, int target_col) const {
	ConstDecisionTreeNode* n = &root;
	for (;;) {
		int col = n->column;
		int c = Get0(col, row);
		bool found = false;
		for(int i = 0; i < n->nodes.GetCount(); i++) {
			ConstDecisionTreeNode& sub = n->nodes[i];
			if (sub.subset_column_value == c) {
				found = true;
				n = &sub;
			}
		}
		if (!found || n->column == -1) {
			ASSERT(n->target_value != -1);
			return n->target_value;
		}
	}
	Panic("Invalid prediction");
	return -1;
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

void QueryTable::ClearQuery() {
	query.SetCount(bytes);
	for(int i = 0; i < bytes; i++)
		query[i] = 0;
}

void QueryTable::SetQuery(int column, int value) {
	Set0(column, value, query);
}

double QueryTable::QueryAverage(int target) {
	typedef const byte ConstByte;
	OnlineVariance var;
	
	// Get the beginning offset in a row in bits and bytes
	int offset_bit = columns[target_count].bit_begin;
	byte offset_mask = 0;
	for(int i = offset_bit % 8; i < 8; i++)
		offset_mask |= 1 << i;
	int offset_byt = offset_bit / 8;
	
	// Prepare fast query checking variables
	int check_begin = offset_byt + 1;
	ConstByte* query_begin = query.Begin() + offset_byt;
	ConstByte first_query_byte = *query_begin & offset_mask;
	query_begin++;
	
	// Loop all data and use rows with equal column values to the query row.
	for(int i = 0; i < data.GetCount(); i++) {
		const Vector<byte>& row = data[i];
		
		// Compare first possibly incomplete byte
		ConstByte* cur = row.begin() + offset_byt;
		if ((*cur & offset_mask) != first_query_byte)
			continue;
		cur++;
		
		// Compare remaining bytes
		ConstByte* query = query_begin;
		bool equal = true;
		for(int j = check_begin; j < bytes; j++) {
			if (*cur != *query) {
				equal = false;
				break;
			}
			cur++;
			query++;
		}
		if (!equal)
			continue;
		
		// Add target value to the mean average calculator
		int t = Get0(target, row);
		var.AddResult(t);
		
	}
	
	// Return mean average
	return var.GetMean();
}

void QueryTable::Sort(int column, bool descending) {
	sort_column = column;
	sort_descending = descending;
	Upp::Sort(data, *this);
}

bool QueryTable::operator()(const Vector<byte>& a, const Vector<byte>& b) const {
	int av = Get0(sort_column, a);
	int bv = Get0(sort_column, b);
	if (sort_descending == false)
		return av < bv;
	else
		return av > bv;
}

void QueryTable::Evolve(int best_row, int candidate, Vector<byte>& output_row) {
	ASSERT(data.GetCount() >= 3);
	int sample_row1, sample_row2;
	do {sample_row1 = Random(data.GetCount());} while (sample_row1 == candidate);
	do {sample_row2 = Random(data.GetCount());} while (sample_row2 == sample_row1 || sample_row2 == candidate);
	
	const Vector<byte>& best_solution	= data[best_row];
	const Vector<byte>& src				= data[candidate];
	const Vector<byte>& sample1			= data[sample_row1];
	const Vector<byte>& sample2			= data[sample_row2];
	
	output_row <<= src;
	
	int n = Random(columns.GetCount());
	for(int i = 0; i < columns.GetCount() && Randomf() < probability; i++) {
		int best		= Get0(n, best_solution);
		int value1		= Get0(n, sample1);
		int value2		= Get0(n, sample2);
		int int_value   = (int)(best + scale * (value1 - value2));
		Set0(n, int_value, output_row);
		n = (n + 1) % columns.GetCount();
	}
}

}
