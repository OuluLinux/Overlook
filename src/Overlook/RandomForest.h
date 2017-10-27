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
};

struct Option : Moveable<Option> {
	int max_depth = 4;
	bool type = false; // weaker type
	int tries_count = 10;
	int tree_count = 100;
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
	void SetCount(int count) {
		for(int i = 0; i < src.GetCount(); i++)
			src[i].SetCount(count);
		labels.SetCount(count);
		this->count = count;
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
	                 indeces into data of the instances that should be payed attention to. Everything not in the list
	                 should be ignored. This is done for efficiency. The function should return a model where you store
	                 variables. (i.e. model = {}; model.myvar = 5;) This will be passed to test_function.
	options.test_function is a function with signature "funtion myWeakTest(inst, model)" where inst is 1D array specifying an example,
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

};


// Misc utility functions
double Entropy(const VectorBool& labels, const VectorBool& ix);



struct BufferRandomForest {
	RandomForest forest;
	VectorBool predicted_label;
	Option options;
	double train_accuracy = 0.0, test_accuracy = 0.0;
	
	BufferRandomForest();
	void SetInputCount(int i);
	void Process(const ConstBufferSource& bufs, const VectorBool& real_label, const VectorBool& mask, int test0_end, int test1_end);
	
};

}

#endif
