#ifndef _Overlook_ExpertAdvisors_h_
#define _Overlook_ExpertAdvisors_h_

namespace Overlook {
using namespace Upp;

#define DEBUG_BUFFERS 1

class DqnAdvisor : public Core {
	
protected:
	friend class WeekSlotAdvisor;
	
	
	static const int PERIOD_COUNT		= 8;
	static const int SRC_COUNT			= 8;
	static const int STRONGSRC_COUNT	= 4;
	static const int INPUT_PERIOD		= 1;
	static const int INPUT_COUNT		= 2 * SRC_COUNT * PERIOD_COUNT;
	
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
	int							opt_counter			= 0;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	
	// Temp
	VectorBool					full_mask;
	ForestArea					area;
	ConstBuffer*				open_buf			= NULL;
	DQN::MatType				tmp_before_state, tmp_after_state;
	Vector<ConstBuffer*>		bufs;
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
	void LoadState(DQN::MatType& state, int cursor);
	
public:
	typedef DqnAdvisor CLASSNAME;
	DqnAdvisor();
	
	virtual void Init();
	
	static const int main_graphs = 3;
	static const int buffer_count = main_graphs;
	
	double GetSpreadPoint() const {return spread_point;}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>();
		
		for(int i = 0; i < PERIOD_COUNT; i++) {
			reg % In<ScissorChannelOscillator>(&Args)
				% In<WilliamsPercentRange>(&Args)
				% In<Momentum>(&Args)
				% In<RelativeStrengthIndex>(&Args)
				% In<ScissorChannelOscillator>(&FilterFunction, &Args)
				% In<WilliamsPercentRange>(&FilterFunction, &Args)
				% In<Momentum>(&FilterFunction, &Args)
				% In<RelativeStrengthIndex>(&FilterFunction, &Args);
		}
		
		reg % Out(buffer_count, buffer_count)
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
	
	static void Args(int input, FactoryDeclaration& decl, const Vector<int>& args) {
		int pshift = (input - 1) / SRC_COUNT;
		//int type   = (input - 1) % SRC_COUNT;
		int period = (1 << (2 + pshift));

		decl.AddArg(period);
	}

};


}

#endif
