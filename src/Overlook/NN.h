#ifndef _Overlook_NN_h_
#define _Overlook_NN_h_

namespace Overlook {

class NetNN : public NNCore {
	
	static const int input_length = 10;
	static const int max_martingale = 4;
	static const int input_count = input_length + max_martingale * 2 + 1;
	static const int pip_step = 10;
	static const int spread_pips = 3;
	
	enum {ACT_DOUBLE_BUY, ACT_DOUBLE_SELL, ACT_WAIT, ACT_COLLECT, ACT_COUNT};
	
	
	// Persistent
	Vector<int> tick_pos;
	Vector<bool> signals;
	int tick_counted = 0;
	
	
	// Temporary
	struct Data {
		Vector<double> sensors;
		Vector<double> posv, negv;
		double collected_reward = 0;
		double equity = 0;
		int iter_pos = 0;
		int doublelen = 0;
		int multiplier = 1;
	};
	CoreList cl_sym;
	Data d[2];
	
	
	void RefreshTicks();
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Brain& brain);
	virtual void Iterate(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf);
	virtual void Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		
	}
	
};


class CombineNN : public NNCore {
	
	// Temporary
	CoreList cl_sym;
	
	
public:
	virtual void Init();
	virtual void InitNN(ConvNet::Brain& brain);
	virtual void Iterate(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf) {}
	virtual void Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output);
	virtual void FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted);
	virtual void Input(InNN& in) {
		System& sys = GetSystem();
		System::NetSetting& net = sys.GetNet(0);
		for(int i = 0; i < net.symbol_ids.GetCount(); i++)
			in.Add<NetNN>(net.symbol_ids.GetKey(i), 0);
	}
	virtual void SerializeNN(Stream& s) {}
	
};

}

#endif
