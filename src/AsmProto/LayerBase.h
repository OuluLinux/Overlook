#ifndef _AsmProto_LayerBase_h_
#define _AsmProto_LayerBase_h_

enum {
	TYPE_VOLAT=1,
	TYPE_POLAR,
	TYPE_CORR,
	TYPE_MUX
};

struct IOData {
	Vector<double> in, out;
	
	void SetCount(int in, int out) {this->in.SetCount(in, 0); this->out.SetCount(out, 0);}
	void SetCount(const IOData& io) {in.SetCount(io.in.GetCount()); out.SetCount(io.out.GetCount());}
	Vector<double>& operator[](int i) {
		switch (i) {
			case 0: return in;
			case 1: return out;
			default: Panic("Invalid op"); return in;
		}
	}
};

class LayerBase {
	static const int DQN_ITERS = 10000;
	
	
	
protected:
	friend class Net;
	
	IOData* fwd_data = NULL;
	IOData  bwd_data;
	IOData  dqn_data, tmp;
	
	ConvNet::Session ses;
	ConvNet::DQNAgent dqn;
	int iter = 0;
	int type = 0;
	
	int depth = 0;
	
	double ratio = 0;
	
	Net* net = NULL;
	
	
public:
	typedef LayerBase CLASSNAME;
	LayerBase();
	
	void Backward(IOData& data);
	int  Forward(bool learn);
	
	String ToString(int indent=0);
	String GetTypeString();
	
	
	
	void BackwardVolatility(IOData& data);
	void BackwardPolarity(IOData& data);
	void BackwardCorrelation(IOData& data);
	void BackwardMux(IOData& data);
	
	int  ForwardVolatility(bool learn);
	int  ForwardPolarity(bool learn);
	int  ForwardCorrelation(bool learn);
	int  ForwardMux(bool learn);
	
};

#endif
