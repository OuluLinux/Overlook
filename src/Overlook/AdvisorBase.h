#ifndef _Overlook_AdvisorBase_h_
#define _Overlook_AdvisorBase_h_

namespace Overlook {

/*

	AdvisorBase
	 - construct with main & visible buffers
	 - have last main buffer as (possibly hidden) oscillator with range -1 +1

*/

class AdvisorBase : public Core {
	
	static const int INPUT_PERIOD = ADVISOR_PERIOD;
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	typedef DQNTrainer<ADVISOR_PERIOD, INPUT_PERIOD*2, 100> DQN;
	
	
protected:
	
	// Persistent
	Vector<DQN::DQVector>			data;
	DQN							dqn_trainer;
	Vector<double>				dqntraining_pts;
	OnlineAverage1				change_av;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	// Temp
	VectorBool					full_mask;
	ConstBuffer*				open_buf			= NULL;
	DQN::MatType				tmp_before_state, tmp_after_state;
	int							prev_counted		= 0;
	int							data_count			= 0;
	int							main_count			= 0;
	int							main_visible		= 0;
	bool						once				= true;
	#ifdef flagDEBUG
	int							dqn_max_rounds		= 5000;
	#else
	int							dqn_max_rounds		= 1000000;
	#endif
	
	
	void BaseInit();
	void EnableJobs();
	bool TrainingDQNBegin();
	bool TrainingDQNIterator();
	bool TrainingDQNEnd();
	bool TrainingDQNInspect();
	void RefreshAction(int cursor);
	void RefreshReward(int cursor);
	void RunMain();
	void RefreshAll();
	void RefreshMain();
	void LoadState(DQN::MatType& state, int cursor);
	
	void BaseIO(ValueRegister& reg, FilterFunction filter_fn=SymTfFilter) {
		
		const int buffer_count			= main_count + ADVISOR_PERIOD;
		#if DEBUG_BUFFERS
		const int visible_buffer_count	= main_count + ADVISOR_PERIOD;
		#else
		const int visible_buffer_count	= main_visible;
		#endif
		
		reg % In<DataBridge>(filter_fn)
			% Out(buffer_count, visible_buffer_count)
			% Mem(data)
			% Mem(dqn_trainer)
			% Mem(dqntraining_pts)
			% Mem(change_av)
			% Mem(dqn_round)
			% Mem(dqn_pt_cursor);
	}
	
	
public:
	typedef AdvisorBase CLASSNAME;
	AdvisorBase(int main_count, int main_visible);
	
};

inline bool IsAdvisorBaseSymbol(int sym) {
	System& sys = GetSystem();
	return	sys.GetSymbolPriority(sym) < SYM_COUNT ||
			sys.GetStrongSymbol() == sym;
}

}

#endif
