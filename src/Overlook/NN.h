#ifndef _Overlook_NN_h_
#define _Overlook_NN_h_

namespace Overlook {

class NetNN : public NNCore {
	
	static const int input_length = 10;
	
	
	// Temporary
	CoreList cl_net, cl_sym;
	int sym_count;
	
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Session& ses);
	virtual void Sample(ConvNet::Session& ses, bool is_realtime);
	virtual void Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		
	}
};

class MultiTfNetNN : public NNCore {
	
	// Temporary
	Vector<double> tmp;
	CoreList cl_sym;
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Session& ses);
	virtual void Sample(ConvNet::Session& ses, bool is_realtime);
	virtual void Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		in.Add<NetNN>(5);
		in.Add<NetNN>(6);
		in.Add<NetNN>(7);
	}
};


class IntPerfNN : public NNCore {
	
	enum {LBL_NEG, LBL_MID, LBL_POS};
	
	static const int input_length = 10;
	
	
	// Persistent
	VectorBool op_hist;
	
	
	// Temporary
	CoreList cl_sym;
	
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Session& ses);
	virtual void Sample(ConvNet::Session& ses, bool is_realtime);
	virtual void Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		in.Add<MultiTfNetNN>(5);
	}
	virtual void SerializeNN(Stream& s) {s % op_hist;}
	
};

}

#endif
