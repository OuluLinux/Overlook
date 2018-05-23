#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

namespace Overlook {

class RecurrentAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int sign_max = 20;
	static const int input_size = sign_max * 2;
	
	// Persistent
	ConvNet::RecurrentSession ses;
	Vector<double> training_pts;
	Vector<int> cursors;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	
	
	// Temporary
	Vector<int> sequence;
	double point = 0.0001;
	int max_rounds = 0;
	bool once = false;
	bool do_test = false;
	
	
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	int ChangeToChar(double change);
	double CharToChange(int chr);
	
public:
	typedef RecurrentAdvisor CLASSNAME;
	RecurrentAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(1, 1)
			% Out(0, 0)
			% Mem(ses)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
};

class DqnAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int other_syms = 8;
	enum {ACTIONMODE_SIGN, ACTIONMODE_TREND, ACTIONMODE_WEIGHTED, ACTIONMODE_HACK};
	
	static const int have_normaldata = 1;
	static const int have_normalma = 0;
	static const int have_hurst = 0;
	static const int have_anomaly = 0;
	static const int have_othersyms = 0;
	static const int have_actionmode = ACTIONMODE_HACK;
	static const int do_test = 0;
	
	
	static const int input_length = 30;
	static const int input_size = (input_length * (have_normaldata + have_normalma + have_hurst + have_anomaly) * (1 + (have_othersyms*(other_syms-1))));
	static const int output_size = 2 * (have_actionmode != ACTIONMODE_WEIGHTED ? 1 : 3);
	typedef DQNTrainer<output_size, input_size> DQN;
	
	
	// Persistent
	DQN dqn;
	Vector<double> training_pts;
	Vector<int> cursors;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	
	
	// Temporary
	DQN::MatType tmp_mat;
	double point = 0.0001;
	int max_rounds = 0;
	bool once = true;
	
	void LoadInput(int pos);
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<Normalized>(&Filter)
			% In<HurstWindow>(&Filter)
			% In<Anomaly>(&Filter)
			% Out(1, 1)
			% Out(0, 0)
			% Mem(dqn)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
	
	static bool Filter(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		else {
			String sym = GetSystem().GetSymbol(out_sym);
			return
				sym == "EURJPY" || sym == "EURUSD" || sym == "GBPUSD" || sym == "USDCAD" ||
				sym == "USDJPY" || sym == "USDCHF" || sym == "AUDUSD" || sym == "EURAUD";
		}
	}
};



class DqnFastAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int MUL = 20;
	static const int input_length = 30;
	static const int input_size = input_length;
	static const int output_length = 10;
	static const int output_size = output_length*2;
	typedef DQNTrainer<output_size, input_size> DQN;
	
	// Persistent
	DQN dqn;
	Vector<double> training_pts;
	Vector<int> cursors;
	double total = 0;
	int round = 0;
	int prev_counted = 0;
	
	
	// Temporary
	DQN::MatType tmp_mat;
	double point = 0.0001;
	int max_rounds = 0;
	bool once = true;
	bool do_test = false;
	
	void LoadInput(int pos);
protected:
	virtual void Start();
	
	bool TrainingBegin();
	bool TrainingIterator();
	bool TrainingEnd();
	bool TrainingInspect();
	void RefreshAll();
	
public:
	typedef DqnFastAdvisor CLASSNAME;
	DqnFastAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<Normalized>()
			% Out(1, 1)
			% Out(0, 0)
			% Mem(dqn)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
};


class MultiDqnAdvisor : public Core {
	
protected:
	virtual void Start();
	
public:
	typedef MultiDqnAdvisor CLASSNAME;
	MultiDqnAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<DqnAdvisor>(&Filter0)
			% In<GridSignal>(&Filter1)
			% Out(1, 1)
			% Out(0, 0);
	}
	
	static bool Filter0(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1) {
			int period = GetSystem().GetPeriod(out_tf);
			return /*period == 60 || period == 240 ||*/ period == 1440;
		}
		else {
			return in_sym == out_sym;
		}
	}
	
	static bool Filter1(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1) {
			int period = GetSystem().GetPeriod(out_tf);
			return period == 1440;
		}
		else {
			return in_sym == out_sym;
		}
	}
};




}

#endif
