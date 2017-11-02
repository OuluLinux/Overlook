#ifndef _Overlook_RandomForest_h_
#define _Overlook_RandomForest_h_

namespace Overlook {

struct ConstBufferSource {
	Vector<ConstBuffer*> bufs;
	
public:
	ConstBufferSource();
	
	void SetDepth(int i) {bufs.SetCount(i, NULL);}
	void SetSource(int i, ConstBuffer& buf) {bufs[i] = &buf;}
	int GetCount() const;
	int GetDepth() const;
	
	double Get(int pos, int depth) const;
	
};

class ConstBufferSourceIter {
	typedef const ConstBufferSource ConstConstBufferSource;
	ConstConstBufferSource* src;
	int cur = 0;
public:
	ConstBufferSourceIter(const ConstBufferSource& src);
	
	void operator++(int i);
	void Seek(int i);
	
	double operator[](int i) const;
	
};

struct Model : Moveable<Model> {
	double thr = 0.0;
	int ri1 = 0;
	int ri2 = 0;
	
	double w1 = 0.0;
	double w2 = 0.0;
	double dotthr = 0.0;
	
	void Serialize(Stream& s) {
		s % thr % ri1 % ri2 % w1 % w2 % dotthr;
	}
};

struct Option : Moveable<Option> {
	int max_depth = 4;
	bool type = false; // weaker type
	int tries_count = 10;
	int tree_count = 100;
	
	void Serialize(Stream& s) {
		s % max_depth % type % tries_count % tree_count;
	}
};

struct Data {
	Option options;
	ConstBufferSource data;
	VectorBool labels;
	Vector<Buffer> src;
	int count = 0;
	
	Data() {
		src.SetCount(2);
		data.SetDepth(2);
		for(int i = 0; i < src.GetCount(); i++)
			data.SetSource(i, src[i]);
	}
	void SetDepth(int i) {
		src.SetCount(i);
		data.SetDepth(i);
		for(int i = 0; i < src.GetCount(); i++)
			data.SetSource(i, src[i]);
		SetCount(count);
	}
	void SetCount(int count) {
		for(int i = 0; i < src.GetCount(); i++)
			src[i].SetCount(count);
		labels.SetCount(count);
		this->count = count;
	}
	void Set(int i, int j, double d) {
		src[j].Set(i, d);
	}
	void SetLabel(int i, bool label) {
		labels.Set(i, label);
	}
	void SetXY(int i, double x, double y, bool label) {
		src[0].Set(i, x);
		src[1].Set(i, y);
		labels.Set(i, label);
	}
	void AddXY(double x, double y, bool label) {
		src[0].Add(x);
		src[1].Add(y);
		labels.SetCount(labels.GetCount() + 1);
		labels.Set(count, label);
		count++;
	}
	int GetCount() const {return count;}
	
};


// Represents a single decision tree
struct DecisionTree {
	typedef Tuple2<int, double> Dot;
	Vector<Model> models;
	Vector<int> leaf_positives, leaf_negatives;
	Vector<Dot> dots;
	Vector<VectorBool> ixs;
	int max_depth = 4;
	int id = 0;
	
	// returns model. Code duplication with DecisionStumpTrain :(
	Model Decision2DStumpTrain(int id, const ConstBufferSource& data, const VectorBool& labels, const VectorBool& ix, const Option& options);
	
	// returns label for a single data instance
	bool Decision2DStumpTest(const ConstBufferSourceIter& iter, const Model& mode);
	
	
	void Train(const ConstBufferSource& data, const VectorBool& labels, const VectorBool& mask, const Option& options);

	// Returns probability that example inst is 1.
	double PredictOne(const ConstBufferSourceIter& iter);
	
	void Serialize(Stream& s) {
		s % models % leaf_positives % leaf_negatives % dots % ixs % max_depth % id;
	}
};

struct RandomForest {
	Vector<double> probabilities;
	Array<DecisionTree> trees;
	int tree_count = 100;
	int max_depth = 4;
	int tries_count = 10;
	int id = 0;
	
	/*
	data is 2D array of size N x D of examples
	labels is a 1D array of labels (only -1 or 1 for now). In future will support multiclass or maybe even regression
	tree_count can be used to customize number of trees to train (default = 100)
	max_depth is the maximum depth of each tree in the forest (default = 4)
	tries_count is the number of random hypotheses generated at each node during training (default = 10)
	train_function is a function with signature "function myWeakTrain(data, labels, ix, options)". Here, ix is a list of
	                 indices into data of the instances that should be payed attention to. Everything not in the list
	                 should be ignored. This is done for efficiency. The function should return a model where you store
	                 variables. (i.e. model = {}; model.myvar = 5;) This will be passed to test_function.
	options.test_function is a function with signature "function myWeakTest(inst, model)" where inst is 1D array specifying an example,
	                 and model will be the same model that you return in options.train_function. For example, model.myvar will be 5.
	                 see DecisionStumpTrain() and DecisionStumpTest() downstairs for example.
	*/

	void Train(const ConstBufferSource& data, const VectorBool& labels, const VectorBool& mask, const Option& options);

	/*
	inst is a 1D array of length D of an example.
	returns the probability of label 1, i.e. a number in range [0, 1]
	*/

	double PredictOne(const ConstBufferSourceIter& iter);

	// convenience function. Here, data is NxD array.
	// returns probabilities of being 1 for all data in an array.
	void Predict(const ConstBufferSource& data, Vector<double>& probabilities);
	
	
	void Serialize(Stream& s) {
		s % probabilities % trees % tree_count % max_depth
		  % tries_count % id;
	}
};


// Misc utility functions
double Entropy(const VectorBool& labels, const VectorBool& ix);



struct ForestArea {
	int train_begin = 0, train_end = 0;
	int test0_begin = 0, test0_end = 0;
	int test1_begin = 0, test1_end = 0;
	
	void FillArea(int level, int data_count);
	void FillArea(int level);
};

struct RandomForestStat {
	double train_accuracy = 0.0, test0_accuracy = 0.0, test1_accuracy = 0.0;
	int train_total_count = 0, test0_total_count = 0, test1_total_count = 0;
	int train_correct_count = 0, test0_correct_count = 0, test1_correct_count = 0;
	
	void Serialize(Stream& s) {
		s % train_accuracy % test0_accuracy % test1_accuracy
		  % train_total_count % test0_total_count % test1_total_count
		  % train_correct_count % test0_correct_count % test1_correct_count;
	}
	void Print(ArrayCtrlPrinter& printer) const {
		printer.Add("train_total_count", train_total_count);
		printer.Add("test0_total_count", test0_total_count);
		printer.Add("test1_total_count", test1_total_count);
		printer.Add("train_correct_count", train_correct_count);
		printer.Add("test0_correct_count", test0_correct_count);
		printer.Add("test1_correct_count", test1_correct_count);
		printer.Add("train_accuracy", train_accuracy);
		printer.Add("test0_accuracy", test0_accuracy);
		printer.Add("test1_accuracy", test1_accuracy);
	}
};

struct BufferRandomForest {
	
	// Persistent
	RandomForest forest;
	Option options;
	
	// Temporary
	VectorBool predicted_label;
	Atomic locked;
	RandomForestStat stat;
	int cache_id = 0;
	bool use_cache = false;
	
	BufferRandomForest();
	void Process(int part_id, const ForestArea& area, const ConstBufferSource& bufs, const VectorBool& real_label, const VectorBool& mask);
	void SetCacheId(int id) {cache_id = id;}
	void UseCache(bool b) {use_cache = b;}
	void Serialize(Stream& s) {
		s % forest % options;
	}
};

}

#endif
