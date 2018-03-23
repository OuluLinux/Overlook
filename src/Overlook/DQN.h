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


//speed up the memcpy process!
template <class T>
void copy_linear(T* write, const T* read, unsigned int size) {
	int* w4 = (int*)write;
	int* r4 = (int*)read;
	unsigned int scan=0;
	
	ASSERT(size % sizeof(int) == 0);
	
	while (size >= 4) {
		w4[scan] = r4[scan];
		size -= 4;
		scan++;
	}
}

#define COPY(type, dst, src, count) copy_linear(dst, src, sizeof(type) * count)
#define ZERO(dst, count) {for(int i = 0; i < count; i++) dst[i] = 0;}
#define SET(dst, count, value) {for(int i = 0; i < count; i++) dst[i] = value;}
#define RAND(dst, count) {RandomGaussian& rand = GetRandomGaussian(count);	for (int i = 0; i < count; i++) {dst.Set(i, rand);}}











template <class T, int width, int height>
class Mat : Moveable<Mat<T, width, height> > {
	
	
protected:
	friend class System;
	
	static const int length = width * height;
	
	T weight_gradients[length];
	T weights[length];
	
	typedef Mat<T, width, height> MatType;
	
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
	Mat(MatType& vol) {COPY(T, this->weights, vol.weights, length); ZERO(weight_gradients, length);}
	Mat(const T weights[length]) {COPY(T, this->weights, weights, length); ZERO(weight_gradients, length);}
	Mat(const MatType& o) {*this = o;}
	Mat(T default_value) {SET(weights, length, default_value); ZERO(weight_gradients, length);}
	Mat& Init() {RAND(weights, length); ZERO(weight_gradients, length); return *this;}
	Mat& Init(const T weights[length]) {COPY(T, this->weights, weights, length); ZERO(weight_gradients, length); return *this;}
	Mat& Init(T default_value) {SET(weights, length, default_value); ZERO(weight_gradients, length); return *this;}
	
	void Serialize(Stream& s) {
		if (s.IsStoring()) {
			s.Put(weights,				sizeof(T) * length);
			s.Put(weight_gradients,		sizeof(T) * length);
		}
		else if (s.IsLoading()) {
			s.Get(weights,				sizeof(T) * length);
			s.Get(weight_gradients,		sizeof(T) * length);
		}
	}
	
	Mat& operator=(const MatType& src) {COPY(T, this->weights, src.weights, length); COPY(T, this->weight_gradients, src.weight_gradients, length); return *this;}
	
	const T* GetWeights()   const {return weights;}
	const T* GetGradients() const {return weight_gradients;}
	
	void Add(int i, T v) {weights[Pos(i)] += v;}
	void Add(int x, int y, T v) {weights[GetPos(x,y)] += v;}
	void AddFrom(const MatType& volume) {for (int i = 0; i < length; i++) {weights[i] += volume.weights[i];}}
	void AddFromScaled(const Mat& volume, T a) {for (int i = 0; i < length; i++) {weights[i] += a * volume.weights[i];}}
	void AddGradient(int x, int y, T v) {weight_gradients[GetPos(x,y)] += v;}
	void AddGradient(int i, T v) {weight_gradients[Pos(i)] += v;}
	void AddGradientFrom(const Mat& volume) {for (int i = 0; i < length; i++) {weight_gradients[i] += volume.weight_gradients[i];}}
	T Get(int x, int y) const {return weights[GetPos(x, y)];}
	T GetGradient(int x, int y) const {return weight_gradients[GetPos(x, y)];}
	void Set(int x, int y, T v) {weights[GetPos(x, y)] = v;}
	void SetConst(T c) {for (int i = 0; i < length; i++) {weights[i] = c;}}
	void SetConstGradient(T c) {for (int i = 0; i < length; i++) {weight_gradients[i] = c;}}
	void SetGradient(int x, int y, T v) {weight_gradients[GetPos(x, y)] = v;}
	T Get(int i) const {return weights[Pos(i)];}
	void Set(int i, T v) {weights[Pos(i)] = v;}
	T GetGradient(int i) const {return weight_gradients[Pos(i)];}
	void SetGradient(int i, T v) {weight_gradients[Pos(i)] = v;}
	void ZeroGradients() {for (int i = 0; i < length; i++) {weight_gradients[i] = 0;}}
	
	int GetWidth() const {return width;}
	int GetHeight() const {return height;}
	int GetLength() const {return length;}
	
	int GetMaxColumn() const {
		T max = -DBL_MAX;
		int pos = -1;
		for(int i = 0; i < length; i++) {
			T d = weights[i];
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
		T r = Randomf();
		T x = 0.0;
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
	typedef Mat<double, WIDTH, HEIGHT> MatType;
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
	
	void Serialize(Stream& s) {s % output;}
	
};

template <int WIDTH, int HEIGHT>
struct RecurrentMul {
	typedef Mat<double, WIDTH, HEIGHT> MatType;
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
	
	void Serialize(Stream& s) {s % output;}
	
};

template <int WIDTH, int HEIGHT>
struct RecurrentAdd {
	typedef Mat<double, WIDTH, HEIGHT> MatType;
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
	
	void Serialize(Stream& s) {s % output;}
	
};






template <int WIDTH, int HEIGHT>
struct DQItem : Moveable<DQItem<WIDTH, HEIGHT> > {
	typedef Mat<double, WIDTH, HEIGHT> MatType;
	
	MatType before_state, after_state;
	double after_reward = 0.0;
	int before_action = 0;
	
	void Serialize(Stream& s) {s % before_state % after_state % after_reward % before_action;}
};

template <int ACTIONS>
struct DQVector : Moveable<DQVector<ACTIONS> > {
	double weight[ACTIONS];
	double correct[ACTIONS];
	
	void Serialize(Stream& s) {
		if (s.IsLoading()) {
			s.Get(weight,  sizeof(double) * ACTIONS);
			s.Get(correct, sizeof(double) * ACTIONS);
		}
		else if (s.IsStoring()) {
			s.Put(weight,  sizeof(double) * ACTIONS);
			s.Put(correct, sizeof(double) * ACTIONS);
		}
	}
};




// return Mat but filled with random numbers from gaussian
template <class T>
void RandMat(double mu, double std, T& m) {
	m.Init(0.0);
	
	std::default_random_engine generator;
	std::normal_distribution<double> distribution(mu, std);
	generator.seed(Random(INT_MAX));
	
	for (int i = 0; i < m.GetLength(); i++)
		m.Set(i, distribution(generator));
}


template <class MatType>
void UpdateMat(MatType& m, double alpha) {
	// updates in place
	for (int i = 0; i < m.GetLength(); i++) {
		double d = m.GetGradient(i);
		m.Add(i, -alpha * d);
		m.SetGradient(i, 0);
	}
}



template <int num_actions, int num_states, int num_hidden_units = 100>
class DQNTrainer {
	
public:
	static const int INPUT_SIZE = num_states;
	static const int OUTPUT_SIZE = num_actions;
	
	typedef Mat<double, 1, num_states> MatType;
	
	struct Data {
	
		Mat<double, num_states, num_hidden_units>			W1;
		Mat<double, 1, num_hidden_units>					b1;
		Mat<double, num_hidden_units, num_actions>			W2;
		Mat<double, 1, num_actions>							b2;
		
		// width = agent-width * agent-height, height = W1.height
		RecurrentMul<1, num_hidden_units>					mul1;
		RecurrentAdd<1, num_hidden_units>					add1;
		RecurrentTanh<1, num_hidden_units>					tanh;
		
		// width = tanh.width, height = W2.height
		RecurrentMul<1, num_actions>						mul2;
		RecurrentAdd<1, num_actions>						add2;
	};
	
	Data data;
	
	typedef  Mat<double, 1, num_actions>				FwdOut;
	typedef  DQItem<1, num_states>						DQItemType;
	typedef  DQVector<num_actions>						DQVectorType;
	
	
protected:
	double epsilon, alpha, tderror_clamp;
	double tderror;
	double gamma;
	
	
	
public:

	DQNTrainer() {
		Init();
	}
	
	void Init() {
		gamma = 0.99;	// future reward discount factor
		epsilon = 0.2;	// for epsilon-greedy policy
		alpha = 0.005;	// value function learning rate
		tderror_clamp = 1.0;
		tderror = 0;
		
		Reset();
	}
	
	void   SetAlpha(double r) {alpha = r;}
	int    GetAlpha() const {return alpha;}
	
	void   SetGamma(double d) {gamma = d;}
	double GetGamma() const {return gamma;}
	
	double GetTDError() const {return tderror;}
	
	void   SetEpsilon(double e) {epsilon = e;}
	double GetEpsilon() const {return epsilon;}
	
	void Reset() {
		RandMat(0, 0.01, data.W1);
		data.b1.Init(0.0);
		RandMat(0, 0.01, data.W2);
		data.b2.Init(0.0);
		
		tderror = 0; // for visualization only...
	}
	
	FwdOut& Forward(MatType& input) {
		// Original:
		//		var a1mat = G.Add(G.Mul(net.W1, s), net.b1);
		//		var h1mat = G.Tanh(a1mat);
		//		var a2mat = G.Add(G.Mul(net.W2, h1mat), net.b2);
		data.mul1.Forward(data.W1,			input);
		data.add1.Forward(data.mul1.output,	data.b1);
		data.tanh.Forward(data.add1.output);
		data.mul2.Forward(data.W2,			data.tanh.output);
		data.add2.Forward(data.mul2.output,	data.b2);
		
		return data.add2.output;
	}
	
	void Backward(MatType& input) {
		data.add2.Backward(data.mul2.output,	data.b2);
		data.mul2.Backward(data.W2,				data.tanh.output);
		data.tanh.Backward(data.add1.output);
		data.add1.Backward(data.mul1.output,	data.b1);
		data.mul1.Backward(data.W1,				input);
	}
	
	int Act(MatType& before_state) {
		
		// epsilon greedy policy
		int action;
		if (epsilon > 0.0 && Randomf() < epsilon) {
			action = Random(num_actions);
		} else {
			// greedy wrt Q function
			FwdOut& amat = Forward(before_state);
			action = amat.GetMaxColumn(); // returns index of argmax action
		}
		
		return action;
	}
	
	void Evaluate(MatType& before_state, DQVectorType& out) {
		
		// greedy wrt Q function
		FwdOut& amat = Forward(before_state);
		
		// epsilon greedy policy
		if (epsilon > 0.0 && Randomf() < epsilon) {
			for(int i = 0; i < num_actions; i++)
				out.weight[i] = amat.Get(Random(num_actions));
		} else {
			for(int i = 0; i < num_actions; i++)
				out.weight[i] = amat.Get(i);
		}
	}
	
	void Evaluate(MatType& before_state, double* buf, int size) {
		ASSERT(size == num_actions);
		
		// greedy wrt Q function
		FwdOut& amat = Forward(before_state);
		
		for(int i = 0; i < num_actions; i++)
			buf[i] = amat.Get(i);
	}
	
	double Learn(DQItemType& item) {
		return Learn(item.before_state, item.before_action, item.after_reward, item.after_state);
	}
	
	double Learn(MatType& s0, int a0, double reward0, MatType& s1) {
		
		// compute the target Q value
		double qmax = reward0;
		if (gamma > 0.0) {
			FwdOut& tmat = Forward(s1);
			qmax += gamma * tmat.Get(tmat.GetMaxColumn());
		}
		
		// now predict
		FwdOut& pred = Forward(s0);
		
		double tderror = pred.Get(a0) - qmax;
		double clamp = tderror_clamp;
		double abs_tderror = tderror >= 0.0 ? +tderror : -tderror;
		if (abs_tderror > clamp) {
			// huber loss to robustify
			if (tderror > clamp)
				tderror = +clamp;
			else
				tderror = -clamp;
		}
		pred.SetGradient(a0, tderror);
		Backward(s0); // compute gradients on net params
		
		// update net
		UpdateMat(data.W1, alpha);
		UpdateMat(data.b1, alpha);
		UpdateMat(data.W2, alpha);
		UpdateMat(data.b2, alpha);
		
		return tderror;
	}
	
	double Learn(MatType& s0, double correct[num_actions]) {
		
		// now predict
		FwdOut& pred = Forward(s0);
		
		double av_tderror = 0.0;
		for (int i = 0; i < num_actions; i++) {
			double tderror = pred.Get(i) - correct[i];
			double clamp = tderror_clamp;
			double abs_tderror = tderror >= 0.0 ? +tderror : -tderror;
			if (abs_tderror > clamp) {
				// huber loss to robustify
				if (tderror > clamp)
					tderror = +clamp;
				else
					tderror = -clamp;
			}
			pred.SetGradient(i, tderror);
			
			av_tderror += abs_tderror;
		}
		av_tderror /= num_actions;
		
		Backward(s0); // compute gradients on net params
		
		// update net
		UpdateMat(data.W1, alpha);
		UpdateMat(data.b1, alpha);
		UpdateMat(data.W2, alpha);
		UpdateMat(data.b2, alpha);
		
		return av_tderror;
	}
	
	
	
	void Serialize(Stream& s) {
		s % data.W1 % data.b1 % data.W2 % data.b2 % data.mul1 % data.add1 % data.tanh % data.mul2 % data.add2
		  % epsilon % alpha % tderror_clamp % gamma % tderror;
	}
};

}

#endif
