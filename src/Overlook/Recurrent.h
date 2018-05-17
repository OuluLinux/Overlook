#ifndef _Overlook_Recurrent_h_
#define _Overlook_Recurrent_h_

#include "Mat.h"

/*
	Recurrent.h is a C++ conversion of Recurrent.js of Andrej Karpathy.
	MIT License.

	Original source:	https://github.com/karpathy/reinforcejs (fork of recurrentjs)
*/

namespace ConvNet {

class RecurrentBase {
	
protected:
	RecurrentBase();
	RecurrentBase(Mat& input);
	RecurrentBase(Mat& input1, Mat& input2);
	
public:
	
	Mat *input1, *input2;
	Mat output;
	
	virtual ~RecurrentBase();
	virtual Mat& Forward() = 0;
	virtual void Backward() = 0;
	virtual String GetKey() const {return "base";}
	virtual int GetArgCount() const = 0;
	
	Mat& Forward(Mat& input);
	Mat& Forward(Mat& input1, Mat& input2);
	
	Mat& operator() (Mat& a) {return Forward(a);}
	Mat& operator() (Mat& a, Mat& b) {return Forward(a,b);}
	
};

class RecurrentRowPluck : public RecurrentBase {
	int* ix;
	
public:
	RecurrentRowPluck(int* i) {ix = i;}
	RecurrentRowPluck(int* i, Mat& in) : RecurrentBase(in) {ix = i;}
	~RecurrentRowPluck() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "RowPluck";}
	virtual int GetArgCount() const {return 0;}
	
};

class RecurrentTanh : public RecurrentBase {
	
public:
	RecurrentTanh() {}
	RecurrentTanh(Mat& in) : RecurrentBase(in) {};
	~RecurrentTanh() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Tanh";}
	virtual int GetArgCount() const {return 1;}
	
};

class RecurrentSigmoid : public RecurrentBase {
	
public:
	RecurrentSigmoid() {}
	RecurrentSigmoid(Mat& in) : RecurrentBase(in) {};
	~RecurrentSigmoid() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Sigmoid";}
	virtual int GetArgCount() const {return 1;}
	
};

class RecurrentRelu : public RecurrentBase {
	
public:
	RecurrentRelu() {}
	RecurrentRelu(Mat& in) : RecurrentBase(in) {};
	~RecurrentRelu() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Relu";}
	virtual int GetArgCount() const {return 1;}
	
};

class RecurrentMul : public RecurrentBase {
	
public:
	RecurrentMul() {}
	RecurrentMul(Mat& in1, Mat& in2) : RecurrentBase(in1, in2) {};
	~RecurrentMul() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Mul";}
	virtual int GetArgCount() const {return 2;}
	
};

class RecurrentAdd : public RecurrentBase {
	
public:
	RecurrentAdd() {}
	RecurrentAdd(Mat& in1, Mat& in2) : RecurrentBase(in1, in2) {};
	~RecurrentAdd() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Add";}
	virtual int GetArgCount() const {return 2;}
	
};

class RecurrentDot : public RecurrentBase {
	
public:
	RecurrentDot() {}
	RecurrentDot(Mat& in1, Mat& in2) : RecurrentBase(in1, in2) {};
	~RecurrentDot() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Dot";}
	virtual int GetArgCount() const {return 2;}
	
};

class RecurrentEltMul : public RecurrentBase {
	
public:
	RecurrentEltMul() {}
	RecurrentEltMul(Mat& in1, Mat& in2) : RecurrentBase(in1, in2) {};
	~RecurrentEltMul() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "EltMul";}
	virtual int GetArgCount() const {return 2;}
	
};

class RecurrentCopy : public RecurrentBase {
	
public:
	RecurrentCopy(Mat& in1, Mat& in2) : RecurrentBase(in1, in2) {};
	~RecurrentCopy() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "Copy";}
	virtual int GetArgCount() const {return 2;}
	
};

class RecurrentAddConst : public RecurrentBase {
	double d;
	
public:
	RecurrentAddConst(double d, Mat& in) : RecurrentBase(in), d(d) {};
	~RecurrentAddConst() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "AddConst";}
	virtual int GetArgCount() const {return 1;}
	
};

class RecurrentMulConst : public RecurrentBase {
	double d;
	
public:
	RecurrentMulConst(double d, Mat& in) : RecurrentBase(in), d(d) {};
	~RecurrentMulConst() {}
	virtual Mat& Forward();
	virtual void Backward();
	virtual String GetKey() const {return "AddConst";}
	virtual int GetArgCount() const {return 1;}
	
};






// Graph follows Net class, and allow easy pipeline creation.
// This is not very useful in complex problems.
class Graph {
	Vector<RecurrentBase*> layers;
	Vector<Mat*> extra_args;
	
public:
	Graph();
	~Graph();
	
	void Clear();
	Mat& Forward(Mat& input);
	void Backward();
	
	Mat& RowPluck(int* row);
	Mat& Tanh();
	Mat& Sigmoid();
	Mat& Relu();
	Mat& Mul(Mat& multiplier);
	Mat& Add(Mat& addition);
	Mat& Dot(Mat& v);
	Mat& EltMul(Mat& v);
	
	RecurrentBase& GetLayer(int i) {return *layers[i];}
	int GetCount() const {return layers.GetCount();}
	
};


// GraphTree allows custom connections between layers.
class GraphTree {
	Vector<RecurrentBase*> layers;
	
public:
	GraphTree();
	~GraphTree();
	
	void Clear();
	Mat& Forward();
	void Backward();
	
	Mat& RowPluck(int* row, Mat& in);
	Mat& Tanh(Mat& in);
	Mat& Sigmoid(Mat& in);
	Mat& Relu(Mat& in);
	Mat& Mul(Mat& in1, Mat& in2);
	Mat& Add(Mat& in1, Mat& in2);
	Mat& Dot(Mat& in1, Mat& in2);
	Mat& EltMul(Mat& in1, Mat& in2);
	Mat& Copy(Mat& src, Mat& dst);
	Mat& AddConstant(double d, Mat& in);
	Mat& MulConstant(double d, Mat& in);
	
	RecurrentBase& GetLayer(int i) {return *layers[i];}
	int GetCount() const {return layers.GetCount();}
	
	RecurrentBase& Top() {return *layers.Top();}
	
};




struct HighwayModel : Moveable<HighwayModel> {
	
	Mat noise_h[2];
	
	static int GetCount() {return 2;}
	
	Mat& GetMat(int i) {
		ASSERT(i >= 0 && i < 6);
		switch (i) {
			case 0: return noise_h[0];
			case 1: return noise_h[1];
			default: return noise_h[1];
		}
	}
};

struct LSTMModel : Moveable<LSTMModel> {
	
	Mat Wix, Wih, bi, Wfx, Wfh, bf, Wox, Woh, bo, Wcx, Wch, bc;
	
	static int GetCount() {return 12;}
	Mat& GetMat(int i) {
		ASSERT(i >= 0 && i < 12);
		switch (i) {
			case 0: return Wix;
			case 1: return Wih;
			case 2: return bi;
			case 3: return Wfx;
			case 4: return Wfh;
			case 5: return bf;
			case 6: return Wox;
			case 7: return Woh;
			case 8: return bo;
			case 9: return Wcx;
			case 10: return Wch;
			case 11: return bc;
			default: return bc;
		}
	}
};

struct RNNModel : Moveable<RNNModel> {
	
	Mat Wxh, Whh, bhh;
	
	static int GetCount() {return 3;}
	Mat& GetMat(int i) {
		ASSERT(i >= 0 && i < 3);
		switch (i) {
			case 0: return Wxh;
			case 1: return Whh;
			case 2: return bhh;
			default: return bhh;
		}
	}
};


void Softmax(const Mat& m, Mat& out);

}


#endif
