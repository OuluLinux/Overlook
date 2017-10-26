#ifndef _RandomForest_Forest_h_
#define _RandomForest_Forest_h_

namespace RandomForest {

struct Pair : Moveable<Pair> {
	double value[2];
	
	Pair() {}
	Pair(double x, double y) {value[0] = x; value[1] = y;}
	
	const double& operator[] (int i) const {return value[i];}
	double& operator[] (int i) {return value[i];}
	int GetCount() const {return 2;}
};

struct Model : Moveable<Model> {
	double thr = 0.0;
	int ri = 0;
	
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
	Vector<Pair> data;
	Vector<bool> labels;
};


// Represents a single decision tree
struct DecisionTree {
	Vector<Model> models;
	Vector<int> leaf_positives, leaf_negatives;
	int max_depth = 4;
	Model (*train_function)(const Vector<Pair>& data, const Vector<bool>& labels, const Vector<int>& ix, const Option& options);
	bool (*test_function)(const Pair&, const Model&);
	
	
	
	void Train(const Vector<Pair>& data, const Vector<bool>& labels, const Option& options);

	// Returns probability that example inst is 1.
	double PredictOne(const Pair& inst);
};

struct RandomForest {
	Array<DecisionTree> trees;
	int tree_count = 100;
	int max_depth = 4;
	int tries_count = 10;
	
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

	void Train(const Vector<Pair>& data, const Vector<bool>& labels, const Option& options);

	/*
	inst is a 1D array of length D of an example.
	returns the probability of label 1, i.e. a number in range [0, 1]
	*/

	double PredictOne(const Pair& inst);

	// convenience function. Here, data is NxD array.
	// returns probabilities of being 1 for all data in an array.
	Vector<double> Predict(const Vector<Pair>& data);

};

// returns model
Model DecisionStumpTrain(const Vector<Pair>& data, const Vector<bool>& labels, const Vector<int>& ix, const Option& options);

// returns a decision for a single data instance
bool DecisionStumpTest(const Pair& inst, const Model& model);

// returns model. Code duplication with DecisionStumpTrain :(
Model Decision2DStumpTrain(const Vector<Pair>& data, const Vector<bool>& labels, const Vector<int>& ix, const Option& options);

// returns label for a single data instance
bool Decision2DStumpTest(const Pair& inst, const Model& mode);

// Misc utility functions
double Entropy(const Vector<bool>& labels, const Vector<int>& ix);

}

#endif
