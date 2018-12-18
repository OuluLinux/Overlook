#ifndef _Overlook_NN_h_
#define _Overlook_NN_h_

namespace Overlook {

class NetNN : public NNCore {
	
protected:
	friend class MartNN;
	
	static const int input_length = 10;
	static const int output_sym_count = 2;
	
	
	// Persistent
	Vector<char> sym_signals;
	
	
	// Temporary
	CoreList cl_net, cl_sym;
	int sym_count;
	
	
public:
	virtual void Init();
	virtual void InitNN(Data& data);
	virtual void Sample(Data& data);
	virtual void Start(Data& data, int pos, Vector<double>& output);
	virtual void FillVector(Data& data);
	virtual void Input(InNN& in) {
		
	}
	virtual void SerializeNN(Stream& s) {s % sym_signals;}
};

class IntPerfNN : public NNCore {
	
	
	static const int input_length = 10;
	
	
	// Persistent
	
	
	// Temporary
	CoreList cl_net, cl_sym;
	int sym_count;
	
	
public:
	virtual void Init();
	virtual void InitNN(Data& data);
	virtual void Sample(Data& data);
	virtual void Start(Data& data, int pos, Vector<double>& output);
	virtual void FillVector(Data& data);
	virtual void Input(InNN& in) {
		in.Add<NetNN>(GetTf());
	}
	virtual void SerializeNN(Stream& s) {}
	
};

class MultiTfNetNN : public NNCore {
	
	// Temporary
	Vector<double> tmp;
	CoreList cl_sym;
	
public:
	virtual void Init();
	virtual void InitNN(Data& data);
	virtual void Sample(Data& data);
	virtual void Start(Data& data, int pos, Vector<double>& output);
	virtual void FillVector(Data& data);
	virtual void Input(InNN& in) {
		in.Add<IntPerfNN>(GetTf()+0);
		in.Add<IntPerfNN>(GetTf()+1);
		in.Add<IntPerfNN>(GetTf()+2);
	}
};

class MartNN : public NNCore {
	
	enum {OPT_GROUPPERIOD, OPT_GROUPSTEP, OPT_GROUPAV,
		OPT_SIGLEN, OPT_MINSUM, OPT_MINLEN, OPT_MINFACT, OPT_MINGROUP, OPT_FINMINLEN,
		OPT_MAXMART, OPT_SL, OPT_TP, OPT_TRAILSL, OPT_TRAILTP, OPT_COUNT};
	
	static const int max_open_symbols = 3;
	
	struct Order : Moveable<Order> {
		int symbol = -1;
		int ticket;
		int type;
		double lots;
		double open;
		double highest_close, lowest_close;
	};
	
	struct MartData {
		Vector<Order> orders;
		Vector<Point> pattern;
		Vector<PatternDistance> distances;
		Vector<OnlineAverageWindow1> distance_averages;
		Vector<OnlineAverageWindow1> signal_pos_averages, signal_neg_averages;
		Vector<uint64> descriptors;
		Vector<int> symbol_group, symbol_factor;
		Vector<int> pos_signal_since_activation, neg_signal_since_activation;
		Vector<int> pos_final_since_activation, neg_final_since_activation;
		Vector<int> pos_group_signal, neg_group_signal;
		Vector<int> signals, prev_signals;
		Vector<int> group_size;
		Vector<bool> symbol_added, symbol_has_orders;
		
		int group_period, group_step, group_av;
		
		int signal_length, signal_minsum, signal_minlength;
		int signal_minfactor, signal_mingroupsize, signal_finalminlength;
		
		int stoploss, takeprofit, trailstoploss, trailtakeprofit;
		int max_martingale = 5;
		
		double equity, balance;
		int mult = 1;
		int pos = 0;
		int loss_count = 0;
		int history_orders_total = 0;
	};
	
	// Persistent
	Vector<double> best_trial[2];
	VectorBool op_hist;
	
	// Temporary
	MartData data[2];
	bool write_equity = false;
	
	CoreList cl_sym;
	
	void LoadTrial(const Vector<double>& trial, MartData& m);
	void ResetPattern(MartData& m);
	void ResetDistanceAverages(MartData& m);
	void ResetSignalAverages(MartData& m);
	void IterateOnce(Data& d, MartData& m, int begin, int end);
	void IterateGroups(MartData& m, int i);
	void IterateSignals(MartData& m, int i);
	void IterateLimits(MartData& m, int i);
	void SetSymbolLots(MartData& m, int sym, double lots);
	void CloseOrder(MartData& m, int i, double lots);
	void OpenOrder(MartData& m, int sym, int type, double lots);
	
public:
	virtual void Init();
	virtual void InitNN(Data& data);
	virtual void Sample(Data& data) {}
	virtual void Optimize(Data& data);
	virtual void Start(Data& data, int pos, Vector<double>& output);
	virtual void FillVector(Data& data);
	virtual void Input(InNN& in) {
		in.Add<NetNN>(GetTf());
	}
	virtual void SerializeNN(Stream& s) {s % op_hist % best_trial[0] % best_trial[1];}
	
};


class StatsNN : public NNCore {
	
	// Temporary
	Vector<double> tmp;
	CoreList cl_sym;
	
public:
	virtual void Init();
	virtual void InitNN(Data& data);
	virtual void Sample(Data& data) {}
	virtual void Start(Data& data, int pos, Vector<double>& output);
	virtual void Optimize(Data& data);
	virtual void FillVector(Data& data);
	virtual void Input(InNN& in) {
		in.Add<NetNN>(GetTf());
	}
};

}

#endif
