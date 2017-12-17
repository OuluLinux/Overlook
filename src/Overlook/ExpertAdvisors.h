#ifndef _Overlook_ExpertAdvisors_h_
#define _Overlook_ExpertAdvisors_h_

namespace Overlook {
using namespace Upp;

#define DEBUG_BUFFERS 1

class DqnAdvisor : public Core {
	
	
protected:
	friend class WeekSlotAdvisor;
	
	
	static const int INPUT_PERIOD	= 5;
	static const int INPUT_COUNT	= 4*4;
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	
	enum {ACTION_LONG, ACTION_SHORT, ACTION_IDLE, ACTION_COUNT};
	typedef DQNTrainer<ACTION_COUNT, INPUT_PERIOD * INPUT_COUNT, 100> DQN;
	
	
	// Persistent
	Vector<DQN::DQItem>			data;
	DQN							dqn_trainer;
	Vector<double>				dqntraining_pts;
	OnlineAverage1				change_av;
	int							opt_counter			= 0;
	int							rflist_iter			= 0;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	
	// Temp
	VectorBool					full_mask;
	ForestArea					area;
	ConstBuffer*				open_buf			= NULL;
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
	void SetRealArea();
	void RefreshOutputBuffers();
	void RefreshMain();
	void RefreshAll();
	void RefreshAction(int data_pos);
	void RefreshReward(int data_pos);
	int  GetAction(DQN::DQItem& before, int cursor);
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	static const int main_graphs = 3;
	static const int buffer_count = main_graphs;
	
	double GetSpreadPoint() const {return spread_point;}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>()
			% In<StrongForce>()
			% In<WilliamsPercentRange>()
			% In<ValueChange>(&FilterFunction)
			% In<WilliamsPercentRange>(&FilterFunction)
			% Out(buffer_count, buffer_count)
			% Out(0, 0)
			% Mem(data)
			% Mem(dqn_trainer)
			% Mem(dqntraining_pts)
			% Mem(opt_counter)
			% Mem(dqn_round)
			% Mem(dqn_pt_cursor);
	}
	
	static bool FilterFunction(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;

		static int strong_sym;
		if (strong_sym == 0) strong_sym = ::Overlook::GetSystem().GetStrongSymbol();
		
		return out_sym == strong_sym;
	}
	
};


}

#endif
