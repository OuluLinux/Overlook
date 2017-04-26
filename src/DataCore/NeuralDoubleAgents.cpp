#include "DataCore.h"

namespace DataCore {






MonaDoubleAgent::MonaDoubleAgent() {
	AddValue<char>("Signal");
	AddValue<double>("Reward");
	
	// Cheese need and goal.
	CHEESE_NEED = 1.0;
	CHEESE_GOAL = 0.5;
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			//"\"minimum\":-1,"
			//"\"maximum\":1,"
			//"\"point\":0.01,"
			
			"\"value0\":{"
				"\"label\":\"Reward\","
				"\"color\":\"64,128,192\","
				"\"style\":\"HISTOGRAM\","
				"\"line_width\":2,"
				"\"chr\":95,"
				"\"begin\":10,"
				"\"shift\":0"
			"}"
		"}"
	);
}

void MonaDoubleAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}

void MonaDoubleAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	AddDependency("/open");
	AddDependency("/metamona");
	
	tf_count = tv.GetPeriodCount();
	sym_count = tv.GetSymbolCount();
	
	// Sensors total:
	// - data sensors = sym_count
	// - current states per symbol = sym_count
	// - how many states are active (open orders)
	// - reward since last all-closed state
	// - max fraction of equity to use to open orders
	// - change in previous cycle (usually backward reward)
	total = sym_count + sym_count + 1 + 1 + 1 + 1;
	
	
	// Actions total:
	// - WAIT + INC_ACCOUNT_FACTOR + DEC_ACCOUNT_FACTOR + sym_count * [SHORT, LONG]
	actions_total = 3 + sym_count * 2;
	
	
	// Goal vector 0 .... 0.01, 0
	input_array.SetCount(0);
	input_array.SetCount(total, 0.0);
	input_array[total-3] = 0.01;
	
	// My, my, my, aye-aye, whoa!
	// M-m-m-my Mona
	// M-m-m-my Mona
	agent.MAX_MEDIATOR_LEVEL = 1;
	agent.Init(total, actions_total, 1);
	
	// Set a long second effect interval
	// for a higher level mediator.
	agent.SetEffectEventIntervals(1, 2);
	agent.SetEffectEventInterval(1, 0, 2, 0.5);
	agent.SetEffectEventInterval(1, 1, 10, 0.5);
	
	// Set need and goal for cheese.
	agent.SetNeed(0, CHEESE_NEED);
	agent.AddGoal(0, input_array, 0, CHEESE_GOAL);
	
	// Reset need.
    agent.SetNeed(0, CHEESE_NEED);
	agent.ClearResponseOverride();
	
	action = 0;
	prev_action = 0;
	velocity.SetCount(sym_count, 0);
	
	max_velocity = 5;
	do_training = true;
	
	//broker.Init();
}

bool MonaDoubleAgent::Process(const SlotProcessAttributes& attr) {
	const Slot& src = GetDependency(0);
	const Slot& metamona = GetDependency(1);
	Panic("TODO: check that channel predictor is followed strongly at this point");
	
	if (attr.tf_id == 0 && attr.sym_id == 0) {
		
		// Check if position is useless for training
		/*double* open = src.GetValue<double>(0, 0, attr);
		double* prev = src.GetValue<double>(0, 1, attr);
		if (!prev || *prev == *open)
			return true;*/
		
		LOG(Format("MonaDoubleAgent::Process sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
		
		// Get new action
		Forward(attr);
	}
	
	// Write signal
	char* sig = GetValue<char>(0, attr);
	*sig = velocity[attr.sym_id];
	
	// Write signal to symbol's data-row
	double* out = GetValue<double>(0, attr);
	*out = broker.GetSignal(attr.sym_id);
	
	return do_training;
}


void MonaDoubleAgent::Forward(const SlotProcessAttributes& attr) {
	const Slot& metamona = GetDependency(1);
	
	// Reserve memory
	input_array.SetCount(total);
	int pos = 0;
	
	// Write sensor values;
	for(int i = 0; i < sym_count; i++) {
		char* sig = metamona.GetValue<char>(0, i, attr.tf_id, 0, attr);
		input_array[pos++] = *sig;
	}
	
	
	// Write state values
	int state_begin = pos;
	for(int i = 0; i < sym_count; i++) {
		input_array[pos++] = (double)velocity[i] / (double)max_velocity;
	}
	
	
	// Write open orders and moving average of reward
	input_array[pos++] = broker.GetOpenOrderCount();
	input_array[pos++] = broker.GetWorkingMemoryChange();
	input_array[pos++] = broker.GetFreeMarginLevel();
	input_array[pos++] = broker.GetPreviousCycleChange(); // reward
	ASSERT(pos == total);
	
	
	// Loop actions until WAIT is received. In this way the velocity vector can be fine tuned
	// as long as it is needed and single errorneous action does not have too much weight.
	int max_actions = sym_count * 3;
	for (int i = 0; i < max_actions; i++) {
		
		// Get action from agent
		prev_action = action;
		action = agent.Cycle(input_array);
		
		// WAIT action breaks loop
		if (action == 0) break;
		
		// Increase free margin level
		else if (action == 1) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() + 0.05);
			input_array[total-2] = broker.GetFreeMarginLevel();
		}
		
		// Decrease free margin level
		else if (action == 2) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() - 0.05);
			input_array[total-2] = broker.GetFreeMarginLevel();
		}
		
		// Make change to signal
		else {
			action -= 3;
			int sym = action / 2;
			int act = action % 2;
			int& vel = velocity[sym];
			int acc = act == 0 ? +1 : -1;
			vel = Upp::max(Upp::min(vel + acc, +max_velocity), -max_velocity);
			double sig = (double)vel / (double)max_velocity;
			broker.SetSignal(sym, sig);
			
			// Write new value to the input_array for next agent.Act
			input_array[state_begin + sym] = sig;
		}
		
		
		// Check for goal state
		if (broker.GetOpenOrderCount() == 0 && broker.IsZeroSignal()) {
			
			// Set goal state, or at least as close as possible
			input_array.SetCount(0);
			input_array.SetCount(total, 0);
			input_array[total-3] = broker.GetWorkingMemoryChange();
			action = agent.Cycle(input_array);
			
			// Clear working memory.
		    agent.ClearWorkingMemory();
		    broker.ClearWorkingMemory();
		}
	}
	
	broker.Cycle();
}

}
