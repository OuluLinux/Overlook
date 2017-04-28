#include "DataCore.h"

namespace DataCore {


HugeDQNAgent::HugeDQNAgent() {
	AddValue<double>("Reward");
	
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

void HugeDQNAgent::SetArguments(const VectorMap<String, Value>& args) {
	
}

void HugeDQNAgent::Init() {
	TimeVector& tv = GetTimeVector();
	
	AddDependency("/open", 1, 1);
	AddDependency("/forecaster", 1, 1);
	
	tf_count = tv.GetPeriodCount();
	max_shift = 8;
	sym_count = tv.GetSymbolCount();
	
	// Sensors total:
	// - data sensors = sym_count * tf_count *  max_shift
	// - current states per symbol = sym_count
	// - how many states are active (open orders)
	// - reward since last all-closed state
	// - max fraction of equity to use to open orders
	total = sym_count * tf_count *  max_shift + sym_count + 1 + 1 + 1;
	
	
	// Actions total:
	// - WAIT + INC_ACCOUNT_FACTOR + DEC_ACCOUNT_FACTOR + sym_count * [SHORT, LONG]
	actions_total = 3 + sym_count * 2;
		
	// My name is Bond, James Bond.
	agent.Init(1, total, actions_total);
	agent.Reset();
	
	action = 0;
	prev_action = 0;
	velocity.SetCount(sym_count, 0);
	
	
	max_velocity = 5;
	do_training = true;
	
	//broker.Init();
}

bool HugeDQNAgent::Process(const SlotProcessAttributes& attr) {
	
	if (attr.tf_id == 0 && attr.sym_id == 0) {
		
		// Check if position is useless for training
		/*double* open = src.GetValue<double>(0, 0, attr);
		double* prev = src.GetValue<double>(0, 1, attr);
		if (!prev || *prev == *open)
			return true;*/
		
		LOG(Format("HugeDQNAgent::Process sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()));
		Cout() << Format("HugeDQNAgent::Process sym=%d tf=%d pos=%d", attr.sym_id, attr.tf_id, attr.GetCounted()) << "\n";
		
		// Return reward value
		Backward(attr);
		
		// Get new action
		Forward(attr);
	}
	
	// Write signal to symbol's data-row
	double* out = GetValue<double>(0, attr);
	*out = broker.GetSignal(attr.sym_id);
	
	return do_training;
}


void HugeDQNAgent::Forward(const SlotProcessAttributes& attr) {
	const Slot& src = GetDependency(0);
	
	// Reserve memory
	input_array.SetCount(total);
	int pos = 0;
	
	// Write sensor values;
	for(int i = 0; i < sym_count; i++) {
		for(int j = 0; j < tf_count; j++) {
			double* prev = src.GetValue<double>(0, i, j, max_shift, attr);
			for(int k = 0; k < max_shift; k++) {
				double* open = src.GetValue<double>(0, i, j, max_shift-1-k, attr);
				if (prev) {
					double d = *open / *prev - 1.0;
					input_array[pos++] = d;
				} else {
					input_array[pos++] = 0;
				}
				prev = open;
			}
		}
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
	ASSERT(pos == total);
	
	
	// Loop actions until WAIT is received. In this way, the velocity vector can be fine tuned
	// as long as it is needed and single errorneous action is lightweight.
	int max_actions = sym_count * 3;
	for (int i = 0; i < max_actions; i++) {
		
		// Get action from agent
		prev_action = action;
		action = agent.Act(input_array);
		
		// WAIT action breaks loop
		if (action == 0) break;
		
		// Increase free margin level
		else if (action == 1) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() + 0.05);
			input_array[total-1] = broker.GetFreeMarginLevel();
		}
		
		// Decrease free margin level
		else if (action == 2) {
			broker.SetFreeMarginLevel(broker.GetFreeMarginLevel() - 0.05);
			input_array[total-1] = broker.GetFreeMarginLevel();
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
	}
	
	broker.Cycle();
}

void HugeDQNAgent::Backward(const SlotProcessAttributes& attr) {
	
	// in backward pass agent learns.
	// compute reward
	reward = broker.GetPreviousCycleChange();
	
	// pass to brain for learning
	agent.Learn(reward);
}



}
