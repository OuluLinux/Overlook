#ifndef _Overlook_AdvisorBase_h_
#define _Overlook_AdvisorBase_h_

namespace Overlook {

/*

	AdvisorBase
	 - construct with main & visible buffers
	 - have last main buffer as (possibly hidden) oscillator with range -1 +1

*/

class AdvisorBase : public Core {
	
	static const int INPUT_PERIOD = 5;
	
	struct TrainingRFCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	struct TrainingDQNCtrl : public JobCtrl {
		Vector<Point> polyline;
		virtual void Paint(Draw& w);
	};
	
	enum {ACTION_LONG, ACTION_SHORT, ACTION_IDLE, ACTION_COUNT};
	typedef DQNTrainer<ACTION_COUNT, INPUT_PERIOD*2, 100> DQN;
	
	typedef Tuple<RandomForestMemory, VectorBool> RF;
	
	
protected:
	
	static const int RF_COUNT = 0;
	static const int DQN_SIG_COUNT = 3;
	
	// Persistent
	Array<RF>					rflist;
	Vector<DQN::DQItem>			data;
	BufferRandomForest			rf_trainer;
	DQN							dqn_trainer;
	Vector<double>				training_pts;
	Vector<double>				dqntraining_pts;
	OnlineAverage1				change_av;
	int							opt_counter			= 0;
	int							rflist_iter			= 0;
	int							dqn_round			= 0;
	int							dqn_pt_cursor		= 0;
	
	
	// Temp
	VectorBool					full_mask;
	One<RF>						training_rf;
	ForestArea					area;
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
	bool TrainingRFBegin();
	bool TrainingRFIterator();
	bool TrainingRFEnd();
	bool TrainingRFInspect();
	bool TrainingDQNBegin();
	bool TrainingDQNIterator();
	bool TrainingDQNEnd();
	bool TrainingDQNInspect();
	void RefreshOutputBuffers();
	void RefreshAction(int cursor);
	void RefreshReward(int cursor);
	int  GetAction(DQN::DQItem& before, int cursor);
	void RunMainRF();
	void RunMainDQN();
	void RefreshAll();
	void RefreshMain();
	void LoadState(DQN::MatType& state, int cursor);
	
	void BaseIO(ValueRegister& reg, FilterFunction filter_fn=SymTfFilter) {
		
		const int buffer_count			= main_count + RF_COUNT + DQN_SIG_COUNT;
		#if DEBUG_BUFFERS
		const int visible_buffer_count	= main_count + RF_COUNT + DQN_SIG_COUNT;
		#else
		const int visible_buffer_count	= main_visible;
		#endif
		
		reg % In<DataBridge>(filter_fn)
			% Out(buffer_count, visible_buffer_count)
			% Mem(rflist)
			% Mem(data)
			% Mem(rf_trainer)
			% Mem(dqn_trainer)
			% Mem(training_pts)
			% Mem(dqntraining_pts)
			% Mem(change_av)
			% Mem(opt_counter)
			% Mem(rflist_iter)
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
