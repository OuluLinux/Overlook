#include "Overlook.h"

namespace Overlook {

void RandomForest::Train(const ConstBufferSource& data, const VectorBool& labels, const VectorBool& mask, const Option& options) {
	tree_count = options.tree_count;
	
	// initialize many trees and train them all independently
	trees.SetCount(tree_count);
	
	train_depth = data.GetDepth();
	
	if (memory.IsEmpty()) memory.Create();
	
	memory->trees.SetCount(tree_count);
	
	train_success = true;
	for (int i = 0; i < tree_count; i++) {
		DecisionTree& tree = trees[i];
		tree.forest = this;
		tree.id = i;
		train_success &= tree.Train(data, labels, mask, options);
	}
	
	Chk();
}

void RandomForest::Chk() const {
	for (int i = 0; i < tree_count; i++) {
		const DecisionTree& tree = trees[i];
		const RandomForestMemoryTree& memtree = memory->trees[i];
		for(int j = 0; j < memtree.models.GetCount(); j++) {
			const Model& m = memtree.models[j];
			if (m.failed) continue;
			if (m.ri1 < 0 || m.ri1 >= train_depth || m.ri2 < 0 || m.ri2 >= train_depth)
				Panic("Invalid model");
		}
	}
}


/*
inst is a 1D array of length D of an example.
returns the probability of label 1, i.e. a number in range [0, 1]
*/
double RandomForest::PredictOne(const ConstBufferSourceIter& iter) {
	
	// have each tree predict and average out all votes
	double dec = 0;
	
	if (tree_count == -1 && memory->trees.GetCount()) tree_count = memory->trees.GetCount();
	ASSERT(memory->trees.GetCount() == tree_count);
	
	trees.SetCount(tree_count);
	for (int i = 0; i < tree_count; i++) {
		DecisionTree& tree = trees[i];
		tree.forest = this;
		tree.id = i;
		dec += tree.PredictOne(iter);
	}

	dec /= tree_count;
	
	return dec;
}

// convenience function. Here, data is NxD array.
// returns probabilities of being 1 for all data in an array.
void RandomForest::Predict(const ConstBufferSource& data, Vector<double>& probabilities) {
	probabilities.SetCount(data.GetCount());
	
	int cursor = 0;
	ConstBufferSourceIter iter(data, &cursor);
	for (; cursor < data.GetCount(); cursor++) {
		probabilities[cursor] = PredictOne(iter);
	}
}


bool DecisionTree::Train(const ConstBufferSource& data, const VectorBool& labels, const VectorBool& mask, const Option& options) {
	max_depth = options.max_depth;
	bool weakType = options.type;
	RandomForestMemoryTree& mem = forest->memory->trees[id];
	Vector<Model>& models		= mem.models;
	Vector<int>& leaf_positives	= mem.leaf_positives;
	Vector<int>& leaf_negatives	= mem.leaf_negatives;
	
	// initialize various helper variables
	int64 internal_count = (1 << max_depth) - 1;
	int64 node_count = (1 << (max_depth + 1)) - 1;
	models.SetCount(internal_count);
	for(int i = 0; i < models.GetCount(); i++)
		models[i].failed = true;
	int label_count = labels.GetCount();
	if (!label_count) {
		return false;
	}
	ixs.SetCount(node_count + 1);
	for (int i = 1; i < ixs.GetCount(); i++)
		ixs[i].SetCount(label_count).Zero();
	ixs[0] = mask; // root node starts out with all nodes as relevant
	ASSERT(mask.GetCount() > 0);
	
	// Zero trailing mask
	int label_mod = label_count % 64;
	if (label_mod > 0) {
		uint64* last = ixs[0].End()-1;
		for(int i = label_mod; i < 64; i++)
			*last = *last & (~(1ULL << i));
	}
	int N = ixs[0].PopCount();
	
	
	// train
	int cursor = 0;
	ConstBufferSourceIter iter(data, &cursor);
	for (int n = 0; n < internal_count; n++) {

		// few base cases
		VectorBool& ix = ixs[n];
		
		int popcount = ix.PopCount();
		
		if (popcount == 0) {
			models[n].failed = true;
			continue;
		}

		if (popcount == 1) {
			ixs[n*2+1] = ix;    // arbitrary send it down left
			models[n].failed = true;
			continue;
		}

		// learn a weak model on relevant data for this node
		Option opt;
		Model model = Decision2DStumpTrain(id * internal_count + n, data, labels, ix, opt);
		models[n] = model; // back it up model

		// split the data according to the learned model
		VectorBool& ixleft = ixs[n*2+1];
		VectorBool& ixright = ixs[n*2+2];
		
		ConstU64 *it = ix.Begin(), *end = ix.End();
		int loop_label_count = 0;
		for (uint64 j = 0; it != end; it++, j += 64) {
			if (*it == 0)
				continue;
			for(uint64 k = 0; k < 64; k++) {
				if (*it & (1ULL << k)) {
					cursor = j + k;
					ASSERT(loop_label_count++ < N);
					bool label = Decision2DStumpTest(iter, model);
					if (label)
						ixleft.Set(cursor, true);
					else
						ixright.Set(cursor, true);
				}
			}
		}
	}

	// compute data distributions at the leafs
	leaf_positives.SetCount(node_count);
	leaf_negatives.SetCount(node_count);
	
	
	for (int n = internal_count; n < node_count; n++) {
		VectorBool& ix = ixs[n];
		
		ASSERT(ix.GetCount() <= labels.GetCount());
		ConstU64 *it = ix.Begin(), *end = ix.End();
		ConstU64 *lit = labels.Begin(), *lend = labels.End();
		uint64 total = 0;
		uint64 numones = 0;
		for (; it != end; it++, lit++) {
			uint64 l = *it & *lit;
			numones += PopCount64(l);
			total += PopCount64(*it);
		}

		leaf_positives[n] = numones;
		leaf_negatives[n] = total - numones;
	}
	
	return true;
}

// returns probability that example inst is 1.
double DecisionTree::PredictOne(const ConstBufferSourceIter& iter) const {
	RandomForestMemoryTree& mem = forest->memory->trees[id];
	Vector<Model>& models		= mem.models;
	Vector<int>& leaf_positives	= mem.leaf_positives;
	Vector<int>& leaf_negatives	= mem.leaf_negatives;
	
	int n = 0;
	
	for (int i = 0; i < max_depth; i++) {
		int dir = Decision2DStumpTest(iter, models[n]);

		if (dir == 1)
			n = n * 2 + 1; // descend left
		else
			n = n * 2 + 2; // descend right
	}

	return (leaf_positives[n] + 0.5) / (leaf_negatives[n] + 1.0); // bayesian smoothing!
}


Model DecisionTree::Decision2DStumpTrain(int id, const ConstBufferSource& data, const VectorBool& labels, const VectorBool& ix, const Option& options) {
	int numtries = options.tries_count;
	
	// choose a dimension at random and pick a best split
	int N = ix.PopCount();
	int ri1 = 0;
	int ri2 = 1;
	
	ASSERT(data.GetDepth() >= 2);
	if (data.GetDepth() > 2) {
		int sum = 0;
		for(int i = 1; i < data.GetDepth(); i++)
			sum += i;
		id = id % sum;
		
		int i = 0;
		for (ri1 = 0; ri1 < data.GetDepth(); ri1++)
			for (ri2 = ri1+1; ri2 < data.GetDepth(); ri2++)
				if (i++ == id)
					goto match;
		match:
		ASSERT(ri1 >= 0 && ri1 < data.GetDepth());
		ASSERT(ri2 >= 0 && ri2 < data.GetDepth());
		ASSERT(i <= sum);
	}
	
	Model model;
	model.ri1 = ri1;
	model.ri2 = ri2;
	
	// evaluate class entropy of incoming data
	double H = Entropy(labels, ix);
	double best_gain = 0;
	double bestw1, bestw2, bestthr;
	
	if (dots.GetAlloc() < 128) dots.Reserve(128);
	dots.SetCount(N);
	if (dots.GetCount() != N) Panic("Memory error");
	for(int i = 0; i < dots.GetCount(); i++) dots[i] = Dot(0, 0.0);

	for (int i = 0; i < numtries; i++) {
		
		// pick random line parameters
		double alpha = Randomf() * 2 * M_PI;
		double w1 = cos(alpha);
		double w2 = sin(alpha);
		
		// project data on this line and get the dot products
		ConstU64 *it  = ix.Begin(), *end = ix.End();
		int d = 0;
		for (uint64 j = 0; it != end; it++, j += 64) {
			if (*it == 0)
				continue;
			for(uint64 k = 0; k < 64; k++) {
				if (*it & (1ULL << k)) {
					int pos = j + k;
					if (d >= N) break;
					dots[d++] = Dot(pos,
						w1 * data.Get(pos, ri1) +
						w2 * data.Get(pos, ri2));
				}
			}
		}
		ASSERT(d == N);
		
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
		double dotthr = dots[ix1].b * a + dots[ix2].b * (1 - a);

		// measure information gain we'd get from split with thr
		double l1 = 1, r1 = 1, lm1 = 1, rm1 = 1; //counts for Left and label 1, right and label 1, left and minus 1, right and minus 1

		for (int j = 0; j < dots.GetCount(); j++) {
			const Dot& dot = dots[j];
			
			if (dot.b < dotthr) {
				if (labels.Get(dot.a) == true)
					l1++;
				else
					lm1++;
			}

			else {
				if (labels.Get(dot.a) == true)
					r1++;
				else
					rm1++;
			}
		}

		double t = l1 + lm1;

		l1	= l1	/	t;
		lm1	= lm1	/	t;
		t	= r1	+	rm1;
		r1	= r1	/	t;
		rm1	= rm1	/	t;

		double LH = -l1 * log(l1) - lm1 * log(lm1); // left and right entropy
		double RH = -r1 * log(r1) - rm1 * log(rm1);

		double information_gain = H - LH - RH;
		//console.log("Considering split %f, entropy %f -> %f, %f. Gain %f", thr, H, LH, RH, information_gain);

		if (information_gain > best_gain || i == 0) {
			best_gain = information_gain;
			bestw1 = w1;
			bestw2 = w2;
			bestthr = dotthr;
		}
	}

	model.w1 = bestw1;
	model.w2 = bestw2;
	model.dotthr = bestthr;
	return model;
}

// returns label for a single data instance
bool DecisionTree::Decision2DStumpTest(const ConstBufferSourceIter& iter, const Model& model) const {
	if (model.failed) return false;
	return iter[model.ri1] * model.w1 + iter[model.ri2] * model.w2 < model.dotthr;
}

// Misc utility functions
double Entropy(const VectorBool& labels, const VectorBool& ix) {
	ASSERT(labels.GetCount() > 0);
	ASSERT(ix.GetCount() > 0);
	ConstU64 *it  = ix.Begin(), *end = ix.End();
	ConstU64 *lit = labels.Begin(), *lend = labels.End();
	int64 numones = 0;
	int64 N = 0;
	for (; it != end && lit != lend; it++, lit++) {
		uint64 l = *it & *lit;
		numones	+= PopCount64(l);
		N		+= PopCount64(*it);
	}
	double p = numones;

	p = (1 + p) / (N + 2); // let's be bayesian about this
	double q = (1 + N - p) / (N + 2);
	return (-p * log(p) - q * log(q));
}
















ConstBufferSource::ConstBufferSource() {
	
}

int ConstBufferSource::GetCount() const {
	return bufs[0]->GetCount();
}

int ConstBufferSource::GetDepth() const {
	return bufs.GetCount();
}

double ConstBufferSource::Get(int pos, int depth) const {
	return bufs[depth]->GetUnsafe(pos);
}














ConstBufferSourceIter::ConstBufferSourceIter(const ConstBufferSource& src, ConstInt* cursor_ptr) :
	src(&src), cursor_ptr(cursor_ptr)
{
	
}

double ConstBufferSourceIter::operator[](int i) const {
	const Vector<ConstBuffer*>& bufs = src->bufs;
	ASSERT(i >= 0 && i < bufs.GetCount());
	ConstBuffer* buf = bufs[i];
	ASSERT(buf != NULL);
	int cursor = *cursor_ptr;
	return buf->GetUnsafe(cursor);
}









BufferRandomForest::BufferRandomForest() {
	
}

void BufferRandomForest::Process(const ForestArea& area, const ConstBufferSource& bufs, const VectorBool& real_label, const VectorBool& mask) {
	ASSERT(mask.GetCount() > 0);
	
	VectorBool train_mask(mask);
	VectorBool test0_mask(mask);
	VectorBool test1_mask(mask);
	
	train_mask.LimitRight(area.train_begin);
	train_mask.LimitLeft(area.train_end);
	
	test0_mask.LimitRight(area.test0_begin);
	test0_mask.LimitLeft(area.test0_end);
	
	test1_mask.LimitRight(area.test1_begin);
	test1_mask.LimitLeft(area.test1_end);
	
	forest.Train(bufs, real_label, train_mask, options);
	
	int label_count = real_label.GetCount();
	predicted_label.SetCount(label_count).Zero();
	
	int cursor = 0;
	ConstBufferSourceIter iter(bufs, &cursor);
	
	int train_correct_count = 0, train_total_count = 0;
	int test0_correct_count = 0, test0_total_count = 0;
	int test1_correct_count = 0, test1_total_count = 0;
	
	ConstU64* it = mask.Begin();
	ConstU64* end = mask.End();
	for(int64 pos = 0; it != end; it++, pos += 64) {
		if (*it == 0)
			continue;
		cursor = pos;
		for(int j = 0; cursor < label_count && j < 64; cursor++, j++) {
			double prob = forest.PredictOne(iter);
			bool label = prob > 0.5;
			if (label)
				predicted_label.Set(cursor, true);
			
			if (*it & (1ULL < j)) {
				bool match = label == real_label.Get(cursor);
				
				if (cursor >= area.train_begin && cursor < area.train_end) {
					if (match)
						train_correct_count++;
					train_total_count++;
				}
				else if (cursor >= area.test0_begin && cursor < area.test0_end) {
					if (match)
						test0_correct_count++;
					test0_total_count++;
				}
				else if (cursor >= area.test1_begin && cursor < area.test1_end) {
					if (match)
						test1_correct_count++;
					test1_total_count++;
				}
			}
		}
	}
	stat.train_total_count = train_total_count;
	stat.test0_total_count = test0_total_count;
	stat.test1_total_count = test1_total_count;
	stat.train_correct_count = train_correct_count;
	stat.test0_correct_count = test0_correct_count;
	stat.test1_correct_count = test1_correct_count;
	stat.train_accuracy = train_total_count > 0 ? (double)train_correct_count / train_total_count : 0.0;
	stat.test0_accuracy = test0_total_count > 0 ? (double)test0_correct_count / test0_total_count : 0.0;
	stat.test1_accuracy = test1_total_count > 0 ? (double)test1_correct_count / test1_total_count : 0.0;
	
}

}
