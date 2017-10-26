#include "RandomForest.h"

namespace RandomForest {

void RandomForest::Train(const Vector<Pair>& data, const Vector<bool>& labels, const Option& options) {
	tree_count = options.tree_count;
	
	// initialize many trees and train them all independently
	trees.SetCount(0);
	trees.SetCount(tree_count);
	
	for (int i = 0; i < tree_count; i++) {
		trees[i].Train(data, labels, options);
	}
}

/*
inst is a 1D array of length D of an example.
returns the probability of label 1, i.e. a number in range [0, 1]
*/
double RandomForest::PredictOne(const Pair& inst) {

	// have each tree predict and average out all votes
	double dec = 0;

	for (int i = 0; i < tree_count; i++) {
		dec += trees[i].PredictOne(inst);
	}

	dec /= tree_count;

	return dec;
}

// convenience function. Here, data is NxD array.
// returns probabilities of being 1 for all data in an array.
Vector<double> RandomForest::Predict(const Vector<Pair>& data) {

	Vector<double> probabilities;
	probabilities.SetCount(data.GetCount());

	for (int i = 0; i < data.GetCount(); i++) {
		probabilities[i] = PredictOne(data[i]);
	}

	return probabilities;
}


void DecisionTree::Train(const Vector<Pair>& data, const Vector<bool>& labels, const Option& options) {
	max_depth = options.max_depth;
	bool weakType = options.type;
	
	train_function = Decision2DStumpTrain;
	test_function = Decision2DStumpTest;
	
	if (weakType == 0) {
		train_function = DecisionStumpTrain;
		test_function = DecisionStumpTest;
	}

	if (weakType == 1) {
		train_function = Decision2DStumpTrain;
		test_function = Decision2DStumpTest;
	}

	// initialize various helper variables
	int64 numInternals = (1 << max_depth) - 1;
	int64 numNodes = (1 << (max_depth + 1)) - 1;

	Vector<Vector<int> > ixs;
	
	ixs.SetCount(numNodes + 1);

	for (int i = 1;i < ixs.GetCount(); i++)
		ixs[i].SetCount(0);

	ixs[0].SetCount(labels.GetCount());

	for (int i = 0; i < labels.GetCount(); i++)
		ixs[0][i] = i; // root node starts out with all nodes as relevant

	models.SetCount(numInternals);

	// train
	for (int n = 0; n < numInternals; n++) {

		// few base cases
		Vector<int>& ixhere = ixs[n];

		if (ixhere.GetCount() == 0) {
			continue;
		}

		if (ixhere.GetCount() == 1) {
			ixs[n*2+1].SetCount(1);
			ixs[n*2+1][0] = ixhere[0];    // arbitrary send it down left
			continue;
		}

		// learn a weak model on relevant data for this node
		Option opt;
		Model model = train_function(data, labels, ixhere, opt);
		models[n] = model; // back it up model

		// split the data according to the learned model
		Vector<int>& ixleft = ixs[n*2+1];
		Vector<int>& ixright = ixs[n*2+2];

		for (int i = 0; i < ixhere.GetCount(); i++) {
			int label = test_function(data[ixhere[i]], model);

			if (label == 1)
				ixleft.Add(ixhere[i]);
			else
				ixright.Add(ixhere[i]);
		}
	}

	// compute data distributions at the leafs
	leaf_positives.SetCount(numNodes);
	leaf_negatives.SetCount(numNodes);

	for (int n = numInternals; n < numNodes; n++) {
		int numones = 0;

		for (int i = 0; i < ixs[n].GetCount(); i++) {
			if (labels[ixs[n][i]] == true)
				numones += 1;
		}

		leaf_positives[n] = numones;
		leaf_negatives[n] = ixs[n].GetCount() - numones;
	}
}

// returns probability that example inst is 1.
double DecisionTree::PredictOne(const Pair& inst) {
	int n = 0;

	for (int i = 0; i < max_depth; i++) {
		int dir = test_function(inst, models[n]);

		if (dir == 1)
			n = n * 2 + 1; // descend left
		else
			n = n * 2 + 2; // descend right
	}

	return (leaf_positives[n] + 0.5) / (leaf_negatives[n] + 1.0); // bayesian smoothing!
}

// returns model
Model DecisionStumpTrain(const Vector<Pair>& data, const Vector<bool>& labels, const Vector<int>& ix, const Option& options) {
	
	int numtries = options.tries_count;

	// choose a dimension at random and pick a best split
	int ri = Random(data[0].GetCount());
	int N = ix.GetCount();

	// evaluate class entropy of incoming data
	double H = Entropy(labels, ix);
	double bestGain = 0;
	double bestThr = 0;

	for (int i = 0; i < numtries; i++) {

		// pick a random splitting threshold
		int ix1 = ix[Random(N)];
		int ix2 = ix[Random(N)];

		while (ix2 == ix1)
			ix2 = ix[Random(N)]; // enforce distinctness of ix2

		double a = Randomf();
		double thr = data[ix1][ri] * a + data[ix2][ri] * (1 - a);

		// measure information gain we'd get from split with thr
		double l1 = 1, r1 = 1, lm1 = 1, rm1 = 1; //counts for Left and label 1, right and label 1, left and minus 1, right and minus 1

		for (int j = 0;j < ix.GetCount(); j++) {
			if (data[ix[j]][ri] < thr) {
				if (labels[ix[j]] == true)
					l1++;
				else
					lm1++;
			}
			else {
				if (labels[ix[j]] == true)
					r1++;
				else
					rm1++;
			}
		}

		double t = l1 + lm1;  // normalize the counts to obtain probability estimates

		l1 = l1 / t;
		lm1 = lm1 / t;
		t = r1 + rm1;
		r1 = r1 / t;
		rm1 = rm1 / t;

		double LH = -l1 * log(l1) - lm1 * log(lm1); // left and right entropy
		double RH = -r1 * log(r1) - rm1 * log(rm1);

		double informationGain = H - LH - RH;
		//console.log("Considering split %f, entropy %f -> %f, %f. Gain %f", thr, H, LH, RH, informationGain);

		if (informationGain > bestGain || i == 0) {
			bestGain = informationGain;
			bestThr = thr;
		}
	}

	Model model;
	model.thr = bestThr;
	model.ri = ri;
	return model;
}

// returns a decision for a single data instance
bool DecisionStumpTest(const Pair& inst, const Model& model) {
	return inst[model.ri] < model.thr;
}

// returns model. Code duplication with DecisionStumpTrain :(
Model Decision2DStumpTrain(const Vector<Pair>& data, const Vector<bool>& labels, const Vector<int>& ix, const Option& options) {
	
	int numtries = options.tries_count;

	// choose a dimension at random and pick a best split
	int N = ix.GetCount();
	int ri1 = 0;
	int ri2 = 1;

	if (data[0].GetCount() > 2) {
		// more than 2D data. Pick 2 random dimensions
		ri1 = Random(data[0].GetCount());
		ri2 = Random(data[0].GetCount());

		while (ri2 == ri1)
			ri2 = Random(data[0].GetCount()); // must be distinct!
	}

	// evaluate class entropy of incoming data
	double H = Entropy(labels, ix);
	double bestGain = 0;
	double bestw1, bestw2, bestthr;
	
	Vector<double> dots;
	dots.SetCount(ix.GetCount(), 0.0);

	for (int i = 0; i < numtries; i++) {

		// pick random line parameters
		double alpha = Randomf() * 2 * M_PI;
		double w1 = cos(alpha);
		double w2 = sin(alpha);

		// project data on this line and get the dot products

		for (int j = 0; j < ix.GetCount(); j++) {
			dots[j] = w1 * data[ix[j]][ri1] + w2 * data[ix[j]][ri2];
		}

		// we are in a tricky situation because data dot product distribution
		// can be skewed. So we don't want to select just randomly between
		// min and max. But we also don't want to sort as that is too expensive
		// let's pick two random points and make the threshold be somewhere between them.
		// for skewed datasets, the selected points will with relatively high likelihood
		// be in the high-desnity regions, so the thresholds will make sense
		int ix1 = Random(N);
		int ix2 = Random(N);

		while (ix2 == ix1)
			ix2 = Random(N); // enforce distinctness of ix2

		double a = Randomf();
		double dotthr = dots[ix1] * a + dots[ix2] * (1 - a);

		// measure information gain we'd get from split with thr
		double l1 = 1, r1 = 1, lm1 = 1, rm1 = 1; //counts for Left and label 1, right and label 1, left and minus 1, right and minus 1

		for (int j = 0;j < ix.GetCount(); j++) {
			if (dots[j] < dotthr) {
				if (labels[ix[j]] == true)
					l1++;
				else
					lm1++;
			}

			else {
				if (labels[ix[j]] == true)
					r1++;
				else
					rm1++;
			}
		}

		double t = l1 + lm1;

		l1 = l1 / t;
		lm1 = lm1 / t;
		t = r1 + rm1;
		r1 = r1 / t;
		rm1 = rm1 / t;

		double LH = -l1 * log(l1) - lm1 * log(lm1); // left and right entropy
		double RH = -r1 * log(r1) - rm1 * log(rm1);

		double informationGain = H - LH - RH;
		//console.log("Considering split %f, entropy %f -> %f, %f. Gain %f", thr, H, LH, RH, informationGain);

		if (informationGain > bestGain || i == 0) {
			bestGain = informationGain;
			bestw1 = w1;
			bestw2 = w2;
			bestthr = dotthr;
		}
	}

	Model model;
	model.w1 = bestw1;
	model.w2 = bestw2;
	model.dotthr = bestthr;
	return model;
}

// returns label for a single data instance
bool Decision2DStumpTest(const Pair& inst, const Model& model) {
	return inst[0]*model.w1 + inst[1]*model.w2 < model.dotthr;
}

// Misc utility functions
double Entropy(const Vector<bool>& labels, const Vector<int>& ix) {
	int N = ix.GetCount();
	double p = 0.0;

	for (int i = 0; i < N; i++) {
		if (labels[ix[i]] == true)
			p += 1;
	}

	p = (1 + p) / (N + 2); // let's be bayesian about this
	double q = (1 + N - p) / (N + 2);
	return (-p * log(p) - q * log(q));
}

}
