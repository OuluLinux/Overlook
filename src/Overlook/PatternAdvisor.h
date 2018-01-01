#ifndef _Overlook_PatternAdvisor_h_
#define _Overlook_PatternAdvisor_h_

namespace Overlook {
using namespace Upp;

#define DEBUG_BUFFERS 1

class PatternAdvisor : public Core {
	
protected:
	friend class AccountAdvisor;
	
	
	static const int trend_max = 4;
	static const int break_max = 8;
	static const int row_size = (2 * 3 * trend_max) + trend_max + 2 * break_max + 4;
	static const int num_states = ADVISOR_PERIOD * row_size;
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	
	typedef DQNTrainer<ADVISOR_PERIOD, num_states, 100> DQN;
	
	
	// Persistent
	Vector<DQN::DQVector>		data;
	DQN							dqn_trainer;
	Vector<double>				dqntraining_pts;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	
	// Temp
	VectorBool					input_data;
	DQN::MatType				tmp_before_state, tmp_after_state;
	ConstBuffer					*open_buf = NULL, *low_buf = NULL, *high_buf = NULL, *volume_buf = NULL;
	int							prev_counted		= 0;
	int							data_count			= 0;
	int							main_count			= 0;
	int							main_visible		= 0;
	double						spread_point		= 0.0;
	bool						once				= true;
	#ifdef flagDEBUG
	int							dqn_max_rounds		= 5000;
	#else
	int							dqn_max_rounds		= 1000000;
	#endif
	
protected:
	virtual void Start();
	
	bool TrainingDQNBegin();
	bool TrainingDQNIterator();
	bool TrainingDQNEnd();
	bool TrainingDQNInspect();
	void RunMain();
	void RefreshOutputBuffers();
	void RefreshMain();
	void RefreshAll();
	void RefreshAction(int data_pos);
	void RefreshReward(int data_pos);
	void LoadState(DQN::MatType& state, int cursor);
	
public:
	typedef PatternAdvisor CLASSNAME;
	PatternAdvisor();
	
	virtual void Init();
	
	double GetSpreadPoint() const {return spread_point;}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% Out(ADVISOR_PERIOD, ADVISOR_PERIOD)
			% Mem(data)
			% Mem(dqn_trainer)
			% Mem(dqntraining_pts)
			% Mem(dqn_round)
			% Mem(dqn_pt_cursor);
	}
	
};


}

#endif
