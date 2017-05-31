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
	
	struct Column : Moveable<Column> {
		Column() : bit_begin(0), bit_end(0), max_value(0) {}
		int bit_begin, bit_end, size, max_value;
		String name;
	};
	Vector<Vector<byte> > data;
	Vector<Column> columns;
	String target_name;
	double z;
	double underfit_limit;
	int overfit_size_limit;
	int bytes, bits;
	int target_count;
	int test;
	
	enum {PRUNE_ERROREST, PRUNE_REDUCEERROR};
	
public:
	
	QueryTable();
	
	int Get(int row, int col) const {
		int out = 0;
		const Vector<byte>& vec = data[row];
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
	
	void Set(int row, int col, int value) {
		Vector<byte>& vec = data[row];
		const Column& pred = columns[col];
		ASSERT(value >= 0 && value < pred.max_value);
		int byt = pred.bit_begin / 8;
		int bit = pred.bit_begin % 8;
		byte* v = vec.Begin() + byt;
		for(int i = 0; i < pred.size; i++) {
			bool b = value & (1 << i);
			if (!b)		*v &= ~(1 << bit);
			else		*v |=  (1 << bit);
			bit++;
			if (bit == 8) {
				bit = 0;
				v++;
			}
		}
	}
	
	int Predict(const DecisionTreeNode& root, int row, int target_col) const {
		ConstDecisionTreeNode* n = &root;
		for (;;) {
			int col = n->column;
			int c = Get(row, col);
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
	
	int GetCount() const {return data.GetCount();}
	
	void Reserve(int i) {data.Reserve(i);}
	void SetCount(int i) {
		int old_count = data.GetCount();
		data.SetCount(i);
		for(int i = old_count; i < data.GetCount(); i++) data[i].SetCount(bytes, 0);
	}
	
	Column& AddColumn(const String& name, int max_value) {
		int bits = MaxBits(max_value-1);
		Column& p = columns.Add();
		p.name = name;
		p.max_value = max_value;
		p.bit_begin = this->bits;
		p.bit_end = p.bit_begin + bits;
		p.size = bits;
		this->bits += bits;
		this->bytes = this->bits / 8;
		if (this->bits % 8 != 0) this->bits++;
		return p;
	}
	
	void PruneErrorEstimation(int target_id, DecisionTreeNode& root);
	void PruneReducedError(int target_id, DecisionTreeNode& root, const Vector<int>& validation_dataset);
	double GetValidationError(int target_id, const DecisionTreeNode& root, const Vector<int>& validation_dataset);
	void GetDecisionTree(int target_id, DecisionTreeNode& root, int row_count);
	void GetDecisionTree(int target_id, const DecisionTreeNode& root, DecisionTreeNode& node, const Vector<int>& dataset, Index<int>& used_columns, const Vector<int>& validation_dataset);
	void EndTargets() {target_count = columns.GetCount();}
};


class QueryTableForecaster : public Core {
	QueryTable qt;
	
public:
	typedef QueryTableForecaster CLASSNAME;
	QueryTableForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(SourcePhase, RealValue, SymTf)
			% In(IndiPhase, RealChangeValue, SymTf)
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastChangeValue, SymTf, 1, 1);
	}
};


class QueryTableHugeForecaster : public Core {
	Array<DecisionTreeNode> tree;
	QueryTable qt;
	int corr_period;
	
public:
	typedef QueryTableHugeForecaster CLASSNAME;
	QueryTableHugeForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % InDynamic(SourcePhase, RealValue, &FilterFunction)
			% InDynamic(IndiPhase, RealChangeValue, &FilterFunction)
			% InDynamic(IndiPhase, CorrelationValue, &FilterFunctionSym)
			//% InOptional(IndiPhase, RealIndicatorValue, SymTf)
			% Out(ForecastPhase, ForecastChangeValue, SymTf, 6, 6)
			% Arg("Correlation period", corr_period);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return out_tf >= in_tf;
		return true;
	}
	static bool FilterFunctionSym(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return out_tf >= in_tf;
		return out_sym == in_sym;
	}
};


class QueryTableMetaForecaster : public Core {
	
public:
	typedef QueryTableMetaForecaster CLASSNAME;
	QueryTableMetaForecaster();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf)
			% In(ForecastPhase, ForecastChangeValue, Sym)
			% In(IndiPhase, ForecastChannelValue, Sym)
			% InOptional(ForecastPhase, ForecastChangeValue, Sym)
			% InOptional(IndiPhase, IndicatorValue, Sym)
			% Out(ForecastCombPhase, ForecastChangeValue, SymTf);
	}
};


class QueryTableAgent : public Core {
	
public:
	typedef QueryTableAgent CLASSNAME;
	QueryTableAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, SymTf);
		reg % In(ForecastCombPhase, ForecastChangeValue, SymTf);
		reg % In(IndiPhase, ForecastChannelValue, SymTf);
		reg % In(IndiPhase, IdealOrderSignal, SymTf);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableHugeAgent : public Core {
	
public:
	typedef QueryTableHugeAgent CLASSNAME;
	QueryTableHugeAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(ForecastCombPhase, ForecastChangeValue, All);
		reg % In(IndiPhase, ForecastChannelValue, All);
		reg % In(IndiPhase, IdealOrderSignal, All);
		reg % Out(AgentPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableMetaAgent : public Core {
	
public:
	typedef QueryTableMetaAgent CLASSNAME;
	QueryTableMetaAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, Sym);
		///reg % In(AgentPhase, ForecastOrderSignal, Sym);
		//reg % InOptional(AgentPhase, ForecastOrderSignal, Sym);
		reg % Out(AgentCombPhase, ForecastOrderSignal, SymTf);
	}
};


class QueryTableDoubleAgent : public Core {
	
public:
	typedef QueryTableDoubleAgent CLASSNAME;
	QueryTableDoubleAgent();
	
	virtual void Init();
	virtual void Start();
	virtual void IO(ValueRegister& reg) {
		reg % In(IndiPhase, RealChangeValue, All);
		reg % In(AgentCombPhase, ForecastOrderSignal, All);
		reg % Out(AgentCombPhase, ForecastOrderSignal, All);
	}
};


}


#endif
