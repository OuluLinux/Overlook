#ifndef _Overlook_MainAdvisor_h_
#define _Overlook_MainAdvisor_h_

namespace Overlook {
using namespace Upp;

#define DEBUG_BUFFERS 1

class MainAdvisor : public Core {
	
protected:
	friend class AccountAdvisor;
	
	
	static const int PERIOD_COUNT		= 8;
	static const int SRC_COUNT			= 4;
	static const int SYMBOLBUF_COUNT	= SRC_COUNT * PERIOD_COUNT + 1;
	static const int INPUTBUF_COUNT		= (SYM_COUNT+1) * SYMBOLBUF_COUNT;
	static const int INPUT_PERIOD		= ADVISOR_PERIOD;
	static const int INPUT_SIZE			= INPUT_PERIOD * INPUTBUF_COUNT + 2;
	static const int OUTPUT_SIZE		= (SYM_COUNT + 1) * ADVISOR_PERIOD;
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	
	typedef DQNTrainer<OUTPUT_SIZE, INPUT_SIZE, 100> DQN;
	
	
	// Persistent
	Vector<DQN::DQVector>		data;
	DQN							dqn_trainer;
	Vector<double>				dqntraining_pts;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	
	// Temp
	ConstBuffer*				open_buf[SYM_COUNT+1];
	DQN::MatType				tmp_before_state, tmp_after_state;
	Vector<ConstBuffer*>		bufs;
	int							prev_counted		= 0;
	int							data_count			= 0;
	int							main_count			= 0;
	int							main_visible		= 0;
	double						spread_point[SYM_COUNT];
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
	typedef MainAdvisor CLASSNAME;
	MainAdvisor();
	
	virtual void Init();
	
	
	double GetSpreadPoint(int i) const {return spread_point[i];}
	
	virtual void IO(ValueRegister& reg) {
		reg % In<DataBridge>(&FilterFunction2);
		
		for(int i = 0; i < PERIOD_COUNT; i++) {
			reg % In<ScissorChannelOscillator>(&FilterFunction1, &Args)
				% In<WilliamsPercentRange>(&FilterFunction1, &Args)
				% In<Momentum>(&FilterFunction1, &Args)
				% In<RelativeStrengthIndex>(&FilterFunction1, &Args);
		}
		
		reg % In<ValueChange>(&FilterFunction1)
			% Out(1, 1)
			% Mem(data)
			% Mem(dqn_trainer)
			% Mem(dqntraining_pts)
			% Mem(dqn_round)
			% Mem(dqn_pt_cursor);
	}
	
	static bool FilterFunction1(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;

		static int strong_sym;
		if (strong_sym == 0) strong_sym = ::Overlook::GetSystem().GetStrongSymbol();
		
		return out_sym == strong_sym || ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
	}
	
	static bool FilterFunction2(void* basesystem, int in_sym, int in_tf, int out_sym, int out_tf) {
		if (in_sym == -1)
			return in_tf == out_tf;
		
		if (in_sym == out_sym) return true;
		
		static int strong_sym;
		if (strong_sym == 0) strong_sym = ::Overlook::GetSystem().GetStrongSymbol();
		
		return out_sym == strong_sym || ::Overlook::GetSystem().GetSymbolPriority(out_sym) < SYM_COUNT;
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
