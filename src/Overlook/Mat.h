#ifndef _Overlook_Mat_h_
#define _Overlook_Mat_h_

#include <random>

namespace ConvNet {

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

class Mat : Moveable<Mat> {
	Vector<double> weight_gradients;
	Vector<double> weights;

protected:
	
	int width;
	int height;
	int length;
	
public:

	
	Mat();
	Mat(int width, int height, Mat& vol);
	Mat(int width, int height, const Vector<double>& weights);
	Mat(const Mat& o) {*this = o;}
	Mat(int width, int height); // Mat will be filled with random numbers
	Mat(int width, int height, double default_value);
	Mat(const Vector<double>& weights);
	Mat& Init(const Mat& v, double default_value=0.0) {return Init(v.GetWidth(), v.GetHeight(), default_value);}
	Mat& Init(int width, int height); // Mat will be filled with random numbers
	Mat& Init(int width, int height, const Vector<double>& weights);
	Mat& Init(int width, int height, double default_value);
	
	~Mat();
	
	void Serialize(Stream& s) {s % weight_gradients % weights % width % height % length;}
	
	Mat& operator=(const Mat& src);
	
	const Vector<double>& GetWeights() const {return weights;}
	const Vector<double>& GetGradients() const {return weight_gradients;}
	
	void Add(int i, double v);
	void Add(int x, int y, double v);
	void AddFrom(const Mat& volume);
	void AddFromScaled(const Mat& volume, double a);
	void AddGradient(int x, int y, double v);
	void AddGradient(int i, double v);
	void AddGradientFrom(const Mat& volume);
	double Get(int x, int y) const;
	double GetGradient(int x, int y) const;
	void Set(int x, int y, double v);
	void SetConst(double c);
	void SetConstGradient(double c);
	void SetGradient(int x, int y, double v);
	double Get(int i) const;
	void Set(int i, double v);
	double GetGradient(int i) const;
	void SetGradient(int i, double v);
	void ZeroGradients();
	void Store(ValueMap& map) const;
	void Load(const ValueMap& map);
	
	
	int GetPos(int x, int y) const;
	int GetWidth()  const {return width;}
	int GetHeight() const {return height;}
	int GetLength() const {return length;}
	int GetMaxColumn() const;
	int GetSampledColumn() const;
	
};

}

#endif
