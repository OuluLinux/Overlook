#ifndef _Overlook_QueryTable_h_
#define _Overlook_QueryTable_h_

// Theory:
//  - http://www.saedsayad.com/decision_tree.htm
//  - http://www.saedsayad.com/decision_tree_overfitting.htm
//  - http://www.cs.bc.edu/~alvarez/ML/statPruning.html
//  - http://www3.nd.edu/~rjohns15/cse40647.sp14/www/content/lectures/24%20-%20Decision%20Trees%203.pdf

/*
	Notes:
	 - tree is pruned from the root, if you accidentaly use success percentage instead of failure percentage
*/

namespace Overlook {

inline int MaxBits(uint32 max_value) {
	uint32 lim = 0;
	for(int i = 0; i < 32; i++) {
		lim |= 1 << i;
		if (lim >= max_value)
			return i+1;
	}
	Panic("Invalid value");
}

class QueryTable;

class DecisionTreeNode : Moveable<DecisionTreeNode> {
	
protected:
	friend class QueryTable;
	
	Vector<DecisionTreeNode> nodes;
	QueryTable* qt;
	double gain;
	double error, chi2;
	int subset_column, subset_column_value;
	int column;
	int dataset_size, target_value, target_value_count;
	
public:
	DecisionTreeNode();
	
	String ToString() const {return LineString(0);}
	String LineString(int depth) const;
	
};

typedef const DecisionTreeNode ConstDecisionTreeNode;

class QueryTable {
	
protected:
	friend class DecisionTreeNode;
	
	int Get0(int col, const Vector<byte>& vec) const;
	void Set0(int col, int value, Vector<byte>& vec);
	
	struct Column : Moveable<Column> {
		Column() : bit_begin(0), bit_end(0), max_value(0) {}
		void Serialize(Stream& s) {s % bit_begin % bit_end % size % max_value % name;}
		int bit_begin, bit_end, size, max_value;
		String name;
	};
	Vector<Vector<byte> > data;
	Vector<Column> columns;
	String target_name;
	Vector<byte> query;
	double z;
	double underfit_limit;
	int overfit_size_limit;
	int bytes, bits;
	int target_count;
	int test;
	
	enum {PRUNE_ERROREST, PRUNE_REDUCEERROR};
	
public:
	
	QueryTable();
	void Serialize(Stream& s);
	int Get(int row, int col) const;
	void Set(int row, int col, int value);
	int Predict(const DecisionTreeNode& root, int row, int target_col) const;
	int Predict(const DecisionTreeNode& root, const Vector<byte>& row, int target_col) const;
	int GetCount() const {return data.GetCount();}
	void Reserve(int i) {data.Reserve(i);}
	void SetCount(int i);
	Column& AddColumn(const String& name, int max_value);
	void PruneErrorEstimation(int target_id, DecisionTreeNode& root);
	void PruneReducedError(int target_id, DecisionTreeNode& root, const Vector<int>& validation_dataset);
	double GetValidationError(int target_id, const DecisionTreeNode& root, const Vector<int>& validation_dataset);
	void GetDecisionTree(int target_id, DecisionTreeNode& root, int row_count);
	void GetDecisionTree(int target_id, const DecisionTreeNode& root, DecisionTreeNode& node, const Vector<int>& dataset, Index<int>& used_columns, const Vector<int>& validation_dataset);
	void EndTargets() {target_count = columns.GetCount();}
	void ClearQuery();
	void SetQuery(int column, int value);
	double QueryAverage(int target);
	int GetColumnCount() const {return columns.GetCount();}
	void Sort(int column, bool descending=false);
	void Evolve(int best_row, int candidate_row, Vector<byte>& output_row);
};


}


#endif
