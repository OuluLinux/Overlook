#ifndef _Overlook_Advisors_h_
#define _Overlook_Advisors_h_

namespace Overlook {

class DqnAdvisor : public Core {
	
	struct TrainingCtrl : public JobCtrl {
		TimeCallback tc;
		typedef TrainingCtrl CLASSNAME;
		TrainingCtrl() {tc.Set(1000/60, THISBACK(Refresh0));}
		void Refresh0() {if (IsVisible()) Refresh(); tc.Set(1000/60, THISBACK(Refresh0));}
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	static const int sym_count = 4;
	
	
	static const int have_othersyms = 0;
	static const int do_test = 1;
	static const int acts_per_step = 4;
	
	static const int window_count = 2;
	static const int level_side = 10;
	static const int level_count = 1 + level_side*2;
	static const int input_length = 3*level_count*window_count;
	static const int input_size = 5 + (input_length * (1 + (have_othersyms*(sym_count-1))));
	static const int output_size = 3;
	
	enum {ACTION_UP, ACTION_DOWN, ACTION_IDLE};
	
	// Persistent
	ConvNet::DQNAgent dqn;
	Vector<double> training_pts;
	Vector<int> cursors;
	double total = 0;
	double reward_sum = 0;
	double state_speed = 0.0, state_est, state_orderopen;
	int round = 0;
	int prev_counted = 0;
	bool state_orderisopen, state_ordertype;
	
	
	// Temporary
	TimeStop save_elapsed;
	Vector<double> tmp_mat;
	Vector<int> level_dist, level_len, level_dir;
	double point = 0.0001;
	int max_rounds = 0;
	int pos = 0;
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
	void ResetState(int pos);
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	~DqnAdvisor() {StoreCache();}
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Lbl(1)
			% Mem(dqn)
			% Mem(training_pts)
			% Mem(cursors)
			% Mem(total)
			% Mem(reward_sum)
			% Mem(state_speed)
			% Mem(state_est)
			% Mem(state_orderopen)
			% Mem(round)
			% Mem(prev_counted)
			% Mem(state_orderisopen)
			% Mem(state_ordertype)
			;
	}
	
	
	static bool Filter(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		else {
			if (in_sym == out_sym)
				return true;
			
			if (!have_othersyms)
				return false;
			else {
				String sym = GetSystem().GetSymbol(out_sym);
				return
					sym == "EURJPY" || sym == "EURUSD" || sym == "GBPUSD" || sym == "USDJPY";
			}
		}
	}
};





}

#endif
