#ifndef _Overlook_DQN_h_
#define _Overlook_DQN_h_

#include <random>

namespace Overlook {

class RandomGaussian {
	std::default_random_engine generator;
	std::normal_distribution<double> distribution;
	
public:

	// weight normalization is done to equalize the output
	// variance of every neuron, otherwise neurons with a lot
	// of incoming connections have outputs of larger variance
	RandomGaussian(int length) : distribution(0, sqrt(1.0 / (double)(length))) {
		generator.seed(Random(1000000000));
	}
	double Get() {return distribution(generator);}
	operator double() {return distribution(generator);}
	
};

template <class T> // Workaround for GCC bug - specialization needed...
T& SingleRandomGaussianLock() {
	static T o;
	return o;
}

inline RandomGaussian& GetRandomGaussian(int length) {
	SpinLock& lock = SingleRandomGaussianLock<SpinLock>(); // workaround
	ArrayMap<int, RandomGaussian>& rands = Single<ArrayMap<int, RandomGaussian> >();
	lock.Enter();
	int i = rands.Find(length);
	RandomGaussian* r;
	if (i == -1) {
		r = &rands.Add(length, new RandomGaussian(length));
	} else {
		r = &rands[i];
	}
	lock.Leave();
	return *r;
}


#define COPY(type, dst, src, count) memcpy(dst, src, sizeof(type) * count)
#define ZERO(dst, count) {for(int i = 0; i < count; i++) dst[i] = 0;}
#define SET(dst, count, value) {for(int i = 0; i < count; i++) dst[i] = value;}
#define RAND(dst, count) {RandomGaussian& rand = GetRandomGaussian(count);	for (int i = 0; i < count; i++) {dst.Set(i, rand);}}











template <int width, int height>
class Mat : Moveable<Mat<width, height> > {
	static const int length = width * height;
	
	double weight_gradients[length];
	double weights[length];
	
	typedef Mat<width, height> MatType;
	
protected:
	inline int GetPos(int x, int y) const {
		ASSERT(x >= 0 && y >= 0 && x < width && y < height);
		return (width * y) + x;
	}
	inline int Pos(int i) const {
		ASSERT(i >= 0 && i < length);
		return i;
	}
	
public:

	
	Mat() {}
	Mat(MatType& vol) {COPY(double, this->weights, vol.weights, length); ZERO(weight_gradients, length);}
	Mat(const double weights[length]) {COPY(double, this->weights, weights, length); ZERO(weight_gradients, length);}
	Mat(const MatType& o) {*this = o;}
	Mat(double default_value) {SET(weights, length, default_value); ZERO(weight_gradients, length);}
	Mat& Init() {RAND(weights, length); ZERO(weight_gradients, length); return *this;}
	Mat& Init(const double weights[length]) {COPY(double, this->weights, weights, length); ZERO(weight_gradients, length); return *this;}
	Mat& Init(double default_value) {SET(weights, length, default_value); ZERO(weight_gradients, length); return *this;}
	
	void Serialize(Stream& s) {
		if (s.IsStoring()) {
			s.Put(weights,				sizeof(double) * length);
			s.Put(weight_gradients,		sizeof(double) * length);
		}
		else if (s.IsLoading()) {
			s.Get(weights,				sizeof(double) * length);
			s.Get(weight_gradients,		sizeof(double) * length);
		}
	}
	
	Mat& operator=(const MatType& src) {COPY(double, this->weights, src.weights, length); COPY(double, this->weight_gradients, src.weight_gradients, length); return *this;}
	
	const double* GetWeights()   const {return weights;}
	const double* GetGradients() const {return weight_gradients;}
	
	void Add(int i, double v) {weights[Pos(i)] += v;}
	void Add(int x, int y, double v) {weights[GetPos(x,y)] += v;}
	void AddFrom(const MatType& volume) {for (int i = 0; i < length; i++) {weights[i] += volume.weights[i];}}
	void AddFromScaled(const Mat& volume, double a) {for (int i = 0; i < length; i++) {weights[i] += a * volume.weights[i];}}
	void AddGradient(int x, int y, double v) {weight_gradients[GetPos(x,y)] += v;}
	void AddGradient(int i, double v) {weight_gradients[Pos(i)] += v;}
	void AddGradientFrom(const Mat& volume) {for (int i = 0; i < length; i++) {weight_gradients[i] += volume.weight_gradients[i];}}
	double Get(int x, int y) const {return weights[GetPos(x, y)];}
	double GetGradient(int x, int y) const {return weight_gradients[GetPos(x, y)];}
	void Set(int x, int y, double v) {weights[GetPos(x, y)] = v;}
	void SetConst(double c) {for (int i = 0; i < length; i++) {weights[i] = c;}}
	void SetConstGradient(double c) {for (int i = 0; i < length; i++) {weight_gradients[i] = c;}}
	void SetGradient(int x, int y, double v) {weight_gradients[GetPos(x, y)] = v;}
	double Get(int i) const {return weights[Pos(i)];}
	void Set(int i, double v) {weights[Pos(i)] = v;}
	double GetGradient(int i) const {return weight_gradients[Pos(i)];}
	void SetGradient(int i, double v) {weight_gradients[Pos(i)] = v;}
	void ZeroGradients() {for (int i = 0; i < length; i++) {weight_gradients[i] = 0;}}
	
	int GetWidth() const {return width;}
	int GetHeight() const {return height;}
	int GetLength() const {return length;}
	
	int GetMaxColumn() const {
		double max = -DBL_MAX;
		int pos = -1;
		for(int i = 0; i < length; i++) {
			double d = weights[i];
			if (i == 0 || d > max) {
				max = d;
				pos = i;
			}
		}
		return pos;
	}
	
	int GetSampledColumn() const {
		// sample argmax from w, assuming w are
		// probabilities that sum to one
		double r = Randomf();
		double x = 0.0;
		for(int i = 0; i < length; i++) {
			x += weights[i];
			if (x > r) {
				return i;
			}
		}
		return length - 1;
	}
};

















inline double sig(double x) {
	// helper function for computing sigmoid
	return 1.0 / (1.0 + exp(-x));
}

template <int WIDTH, int HEIGHT>
struct RecurrentTanh {
	typedef Mat<WIDTH, HEIGHT> MatType;
	static const int length = WIDTH * HEIGHT;
	MatType output;
	
	template <class T>
	MatType& Forward(const T& input) {
		ASSERT(input.GetWidth() == WIDTH && input.GetHeight() == HEIGHT);
		// tanh nonlinearity
		output.Init(0.0);
		for (int i = 0; i < length; i++) {
			output.Set(i, tanh(input.Get(i)));
		}
		
		return output;
	}
	
	template <class T>
	void Backward(T& input) {
		ASSERT(input.GetWidth() == WIDTH && input.GetHeight() == HEIGHT);
		for (int i = 0; i < length; i++) {
			// grad for z = Tanh(x) is (1 - z^2)
			double mwi = output.Get(i);
			double d = (1.0 - mwi * mwi) * output.GetGradient(i);
			input.AddGradient(i, d);
		}
	}
};

template <int WIDTH, int HEIGHT>
struct RecurrentMul {
	typedef Mat<WIDTH, HEIGHT> MatType;
	static const int length = WIDTH * HEIGHT;
	MatType output;
	
	template <class T1, class T2>
	MatType& Forward(const T1& input1, const T2& input2) {
		
		// multiply matrices input1 * input2
		ASSERT_(input1.GetWidth() == input2.GetHeight(), "matmul dimensions misaligned");
		
		int h = input1.GetHeight();
		int w = input2.GetWidth();
		ASSERT(WIDTH == w && HEIGHT == h);
		output.Init(0.0);
		
		// loop over rows of input1
		for (int i = 0; i < h; i++) {
			
			// loop over cols of input2
			for (int j = 0; j < w; j++) {
				
				// dot product loop
				double dot = 0.0;
				for (int k = 0; k < input1.GetWidth(); k++) {
					dot += input1.Get(k, i) * input2.Get(j, k);
				}
				output.Set(j, i, dot);
			}
		}
		return output;
	}
	
	template <class T1, class T2>
	void Backward(T1& input1, T2& input2) {
		
		// loop over rows of m1
		for (int i = 0; i < input1.GetHeight(); i++) {
			
			// loop over cols of m2
			for (int j = 0; j < input2.GetWidth(); j++) {
				
				// dot product loop
				for (int k = 0; k < input1.GetWidth(); k++) {
					double b = output.GetGradient(j, i);
					input1.AddGradient(k, i, input2.Get(j, k) * b);
					input2.AddGradient(j, k, input1.Get(k, i) * b);
				}
			}
		}
	}
};

template <int WIDTH, int HEIGHT>
struct RecurrentAdd {
	typedef Mat<WIDTH, HEIGHT> MatType;
	static const int length = WIDTH * HEIGHT;
	MatType output;
	
	template <class T1, class T2>
	MatType& Forward(const T1& input1, const T2& input2) {
		ASSERT(input1.GetLength() == input2.GetLength());
		ASSERT(output.GetWidth() == input1.GetWidth() && output.GetHeight() == input1.GetHeight());
		
		output.Init(0.0);
		for (int i = 0; i < input1.GetLength(); i++) {
			output.Set(i, input1.Get(i) + input2.Get(i));
		}
		
		return output;
	}
	
	template <class T1, class T2>
	void Backward(T1& input1, T2& input2) {
		for (int i = 0; i < input1.GetLength(); i++) {
			input1.AddGradient(i, output.GetGradient(i));
			input2.AddGradient(i, output.GetGradient(i));
		}
	}
};






template <int WIDTH, int HEIGHT>
struct DQExperience : Moveable<DQExperience<WIDTH, HEIGHT> > {
	typedef Mat<WIDTH, HEIGHT> MatType;
	MatType state0, state1;
	int action0, action1;
	double reward0;
	
	void Set(MatType& state0, int action0, double reward0, MatType& state1, int action1) {
		this->state0 = state0;
		this->action0 = action0;
		this->reward0 = reward0;
		this->state1 = state1;
		this->action1 = action1;
	}
	void Serialize(Stream& s) {s % state0 % state1 % action0 % action1 % reward0;}
};





// return Mat but filled with random numbers from gaussian
template <class T>
void RandMat(double mu, double std, T& m) {
	/*m.Init(d, n, 0);
	
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(mu, std);
	generator.seed(Random(INT_MAX));
	
	for (int i = 0; i < m.GetLength(); i++)
		m.Set(i, distribution(generator));*/
	Panic("TODO");
}
/*
int SampleWeighted(Vector<double>& p) {
	ASSERT(!p.IsEmpty());
	double r = Randomf();
	double c = 0.0;
	for (int i = 0; i < p.GetCount(); i++) {
		c += p[i];
		if (c >= r)
			return i;
	}
	Panic("Invalid input vector");
	return 0;
}*/


template <class MatType>
void UpdateMat(MatType& m, double alpha) {
	// updates in place
	for (int i = 0; i < m.GetLength(); i++) {
		double d = m.GetGradient(i);
		m.Add(i, -alpha * d);
		m.SetGradient(i, 0);
	}
}



template <int width, int height, int num_actions, int num_states, int num_hidden_units = 100, int experience_size=10000>
class DQNAgent {
	
public:
	static const int length = width * height;
	typedef Mat<width, height> MatType;
	
	Mat<num_states, num_hidden_units>		W1;
	Mat<1, num_hidden_units>				b1;
	Mat<num_hidden_units, num_actions>		W2;
	Mat<1, num_actions>						b2;
	
	// width = agent-width * agent-height, height = W1.height
	RecurrentMul<length, num_hidden_units>	mul1;
	RecurrentAdd<length, num_hidden_units>	add1;
	RecurrentTanh<length, num_hidden_units>	tanh;
	
	// width = tanh.width, height = W2.height
	RecurrentMul<length, num_actions>		mul2;
	RecurrentAdd<length, num_actions>		add2;
	
	typedef  Mat<length, num_actions>		FwdOut;
	
protected:
	DQExperience<width, height> exp[experience_size]; // experience
	double gamma, epsilon, alpha, tderror_clamp;
	double tderror;
	int experience_add_every;
	int learning_steps_per_iteration;
	int expi;
	int nh;
	int ns;
	int na;
	int t;
	bool has_reward;
	
	MatType state;
	MatType state0, state1;
	int action0, action1;
	double reward0;
	
	
public:

	DQNAgent() {
		gamma = 0.9;	// future reward discount factor
		epsilon = 0.02;	// for epsilon-greedy policy
		alpha = 0.005;	// value function learning rate
		
		experience_add_every = 5; // number of time steps before we add another experience to replay memory
		learning_steps_per_iteration = 5;
		tderror_clamp = 1.0;
		
		expi = 0;
		t = 0;
		reward0 = 0;
		action0 = 0;
		action1 = 0;
		has_reward = false;
		tderror = 0;
	}
	
	void Reset() {
		RandMat(0, 0.01, W1);
		b1.Init(0.0);
		RandMat(0, 0.01, W2);
		b2.Init(0.0);
		
		expi = 0; // where to insert
		
		t = 0;
		
		reward0 = 0;
		action0 = 0;
		action1 = 0;
		has_reward = false;
		
		tderror = 0; // for visualization only...
	}
	
	FwdOut& Forward(MatType& input) {
		// Original:
		//		var a1mat = G.Add(G.Mul(net.W1, s), net.b1);
		//		var h1mat = G.Tanh(a1mat);
		//		var a2mat = G.Add(G.Mul(net.W2, h1mat), net.b2);
		mul1.Forward(W1,			input);
		add1.Forward(mul1.output,	b1);
		tanh.Forward(add1.output);
		mul2.Forward(W2,			tanh.output);
		add2.Forward(mul2.output,	b2);
		
		return add2.output;
	}
	
	int Act(const Vector<double>& slist) {
		
		// convert to a Mat column vector
		state.Init(slist);
		
		// epsilon greedy policy
		int action;
		if (Randomf() < epsilon) {
			action = Random(na);
		} else {
			// greedy wrt Q function
			//Mat& amat = ForwardQ(net, state);
			FwdOut& amat = Forward(state);
			action = amat.GetMaxColumn(); // returns index of argmax action
		}
		
		// shift state memory
		state0 = state1;
		action0 = action1;
		state1 = state;
		action1 = action;
		
		return action;
	}
	
	void Learn(double reward1) {
		Panic("TODO");
		/*
		// perform an update on Q function
		if (has_reward && alpha > 0 && state0.GetLength() > 0) {
			
			// learn from this tuple to get a sense of how "surprising" it is to the agent
			tderror = LearnFromTuple(state0, action0, reward0, state1, action1); // a measure of surprise
			
			// decide if we should keep this experience in the replay
			if (t % experience_add_every == 0) {
				if (exp.GetCount() == expi)
					exp.Add();
				ASSERT(state1.GetLength() > 0);
				exp[expi].Set(state0, action0, reward0, state1, action1);
				expi += 1;
				if (expi >= experience_size) { expi = 0; } // roll over when we run out
			}
			t += 1;
			
			if (!exp.IsEmpty()) {
				// sample some additional experience from replay memory and learn from it
				for (int k = 0; k < learning_steps_per_iteration; k++) {
					int ri = Random(exp.GetCount()); // TODO: priority sweeps?
					DQExperience& e = exp[ri];
					LearnFromTuple(e.state0, e.action0, e.reward0, e.state1, e.action1);
				}
			}
		}
		reward0 = reward1; // store for next update
		has_reward = true;*/
	}
	
	double LearnFromTuple(MatType& s0, int a0, double reward0, MatType& s1, int a1) {
		Panic("TODO");
		/*
		ASSERT(s0.GetLength() > 0);
		ASSERT(s1.GetLength() > 0);
		// want: Q(s,a) = r + gamma * max_a' Q(s',a')
		
		// compute the target Q value
		FwdOut& tmat = Forward(s1);
		double qmax = reward0 + gamma * tmat.Get(tmat.GetMaxColumn());
		
		// now predict
		FwdOut& pred = Forward(s0);
		
		double tderror = pred.Get(a0) - qmax;
		double clamp = tderror_clamp;
		if (fabs(tderror) > clamp) {  // huber loss to robustify
			if (tderror > clamp)
				tderror = +clamp;
			else
				tderror = -clamp;
		}
		pred.SetGradient(a0, tderror);
		G.Backward(); // compute gradients on net params
		
		// update net
		UpdateMat(W1, alpha);
		UpdateMat(b1, alpha);
		UpdateMat(W2, alpha);
		UpdateMat(b2, alpha);
		
		return tderror;
		*/
	}
	
	int GetExperienceWritePointer() const {return expi;}
	double GetTDError() const {return tderror;}
	double GetEpsilon() const {return epsilon;}
	
	int GetExperienceCount() const {return experience_size;}
	
	void SetEpsilon(double e) {epsilon = e;}
	
	
	void Serialize(Stream& s) {
		for(int i = 0; i < experience_size; i++)
			s % exp[i];
		s % gamma % epsilon % alpha % tderror_clamp % tderror % expi % t;
	}
};

}

#endif
