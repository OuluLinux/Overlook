#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

namespace Overlook {

/*
class DqnAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int other_syms = 8;
	
	
	
	static const int input_length = 30;
	static const int input_size = input_length * 1;
	static const int output_size = 2;
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
	void DumpTest();
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&Filter)
			% Out(1, 1)
			% Out(0, 0)
			% Mem(dqn)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(round)
			% Mem(prev_counted);
	}
	
	
	static bool Filter(void* basesystem, bool match_tf, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (match_tf)
			return in_tf == out_tf;
		else {
			if (in_sym == out_sym)
				return true;
			
			String sym = GetSystem().GetSymbol(out_sym);
			return
				sym == "EURJPY" || sym == "EURUSD" || sym == "GBPUSD" || sym == "USDCAD" ||
				sym == "USDJPY" || sym == "USDCHF" || sym == "AUDUSD" || sym == "EURAUD";
		}
	}
};
*/
}

#endif
