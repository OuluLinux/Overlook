#include "Agent.h"

namespace Agent {


void Core::Serialize(Stream& s) {
	s % bars % counted % buffers % labels;
	CoreSerializer cs;
	cs.stream = &s;
	IO(cs);
}

void Core::Refresh(bool run_start) {
	for(int i = 0; i < buffers.GetCount(); i++) {
		buffers[i].SetCount(bars);
	}
	for(int i = 0; i < labels.GetCount(); i++) {
		for(int j = 0; j < labels[i].buffers.GetCount(); j++) {
			labels[i].buffers[j].signal.SetCount(bars, false);
			labels[i].buffers[j].enabled.SetCount(bars, true);
		}
	}
	
	if (run_start) {
		Start();
		
		counted = bars;
	}
}

void Core::SetupBuffers() {
	OutputCounter out;
	IO(out);
	
	buffers.SetCount(out.out_sum);
	
	labels.SetCount(out.lbl_counts.GetCount());
	for(int i = 0; i < out.lbl_counts.GetCount(); i++)
		labels[i].buffers.SetCount(out.lbl_counts[i]);
	
}

}
