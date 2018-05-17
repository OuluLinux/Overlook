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
	
	static const int input_length = 30;
	static const int input_size = input_length*2;
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
	bool once = false;
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
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
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

}

#endif
