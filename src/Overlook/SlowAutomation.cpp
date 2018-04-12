#include "Overlook.h"

namespace Overlook {

void SlowAutomation::Serialize(Stream& s) {
	s
	  % open_buf
	  % time_buf
	  % brain
	  % point
	  % spread
	  % prev_sig_time
	  % sym % tf % period
	  % prev_sig
	  % brain_iters
	  % loadsource_pos
	  % loadsource_cursor
	  % not_first;
}

void SlowAutomation::LoadInput(Vector<double>& input, int pos) {
	double absmax_d = 0;
	double zero = open_buf[pos];
	for(int i = 0; i < input_length; i++) {
		double d = open_buf[--pos] - zero;
		if (d >= 0) {
			input[i * 2 + 0] = +d;
			input[i * 2 + 1] = 0;
		} else {
			input[i * 2 + 0] = 0;
			input[i * 2 + 1] = -d;
		}
		if (d < 0) d = -d;
		if (d > absmax_d) absmax_d = d;
	}
	for(int i = 0; i < input_count; i++) {
		double& d = input[i];
		d = 1.0 - d / absmax_d;
	}
}

double SlowAutomation::TestAction(int pos, int action) {
	if (action == ACTION_IDLE)
		return 0.0;
	
	double reward = 0.0;
	
	int open_time = time_buf[pos];
	double open = open_buf[pos];
	double close;
	
	int max_time = open_time + DEF_TIMELIMIT * 60;
	for(int i = pos; i < open_buf.GetCount(); i++) {
		int time = time_buf[i];
		close = open_buf[i];
		
		if (action == ACTION_LONG)
			reward = +(close / (open + spread) - 1.0) * 1000;
		else
			reward = -(close / (open - spread) - 1.0) * 1000;
		
		if (reward <= DEF_SL || reward >= DEF_TP)
			break;
		
		if (time >= max_time)
			break;
		
	}
	
	return reward;
}

void SlowAutomation::Evolve() {
	int& iters = this->brain_iters;
	
	const double max_alpha = 0.01;
	
	if (!iters) {
		// input_depth = input_count * 2 + output_count = 103
		String net_str =
			"[\n"
			"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":103},\n"
			"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
			"\t{\"type\":\"regression\", \"neuron_count\":3},\n"
			"\t{\"type\":\"sgd\", \"learning_rate\":0.001, \"momentum\":0.0, \"batch_size\":64, \"l2_decay\":0.01}\n"
			"]\n";
			
		brain.Init(input_count, output_count);
		brain.Reset();
		bool success = brain.MakeLayers(net_str);
		if (!success) Panic("Brain didn't initialize");
	}
	
	Vector<double> input_array;
	input_array.SetCount(input_count);
	
	int pos_count = open_buf.GetCount() - brain_rightoffset - brain_leftoffset;
	while (*running && iters < max_iters) {
		int pos = brain_leftoffset + iters % pos_count;
		
		LoadInput(input_array, pos);
		int action = brain.Forward(input_array);
		double reward = TestAction(pos, action);
		brain.Backward(reward);
		
		iters++;
	}
}

void SlowAutomation::GetOutputValues(bool& signal, int& level) {
	int pos = open_buf.GetCount() - 1;
	if (pos < 0) return;
	
	Vector<double> input_array;
	input_array.SetCount(input_count);
	LoadInput(input_array, pos);
	int action = brain.Forward(input_array);
	
	switch (action) {
		case ACTION_LONG:  signal = 0; level = 1; break;
		case ACTION_SHORT: signal = 1; level = 1; break;
		case ACTION_IDLE:  signal = 0; level = 0; break;
	}
}

}
