#ifndef _Overlook_NN_h_
#define _Overlook_NN_h_

namespace Overlook {

class MainNN : public NNCore {
	
	static const int sym_count = 18;
	static const int input_length = 10;
	static const int pip_step = 30;
	static const int open_pip_step = 30;
	static const int spread_pips = 3;
	
	
	enum {ACT_BUY, ACT_SELL};
	
	// Temporary
	struct Data {
		ConvNet::Volume vol;
		double equity = 0;
		int iter_pos = 0;
	};
	CoreList cl_sym;
	Data d[2];
	
	
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Brain& brain);
	virtual void Iterate(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf);
	virtual void Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		
	}
	
};


}

#endif
