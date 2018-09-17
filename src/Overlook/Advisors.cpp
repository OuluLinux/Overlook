#include "Overlook.h"

namespace Overlook {

SimpleBB::SimpleBB() {
	
}

void SimpleBB::Tick() {
	AdvisorSystem& as = GetAdvisorSystem();
	
	bool finish = broker.AccountBalance() >= init_balance * 1.03;
	
	if (as.GetCursor() < wait_cursor && !finish)
		return;
	
	Vector<Vector<Vector<AdvisorInput> > >& inputs = as.GetInputs();
	
	int active_sym = -1;
	double active_mean = 0.0;
	bool active_signal;
	
	for(int i = 0; i < inputs.GetCount(); i++) {
		auto& sym = inputs[i];
		auto& tf = sym[4];
		for(int j = 0; j < tf.GetCount(); j++) {
			AdvisorInput& ai = tf[j];
			if (ai.IsTriggered()) {
				bool signal = ai.GetSignal();
				double mean = ai.GetMean();
				if (mean > active_mean) {
					active_mean = mean;
					active_sym = i;
					active_signal = signal;
				}
			}
			if (ai.IsInverseTriggered()) {
				bool signal = ai.GetInverseSignal();
				double mean = -ai.GetMean();
				if (mean > active_mean) {
					active_mean = mean;
					active_sym = i;
					active_signal = signal;
				}
			}
		}
	}
	
	for (int i = 2; i < 4 && active_sym >= 0; i++) {
		int fast_active_sym = -1;
		double fast_active_mean = 0.0;
		bool fast_active_signal;
		
		auto& sym = inputs[active_sym];
		auto& tf = sym[i];
		for(int j = 0; j < tf.GetCount(); j++) {
			AdvisorInput& ai = tf[j];
			if (ai.IsTriggered()) {
				bool signal = ai.GetSignal();
				double mean = ai.GetMean();
				if (mean > fast_active_mean) {
					fast_active_mean = mean;
					fast_active_sym = active_sym;
					fast_active_signal = signal;
				}
			}
			if (ai.IsInverseTriggered()) {
				bool signal = ai.GetInverseSignal();
				double mean = -ai.GetMean();
				if (mean > fast_active_mean) {
					fast_active_mean = mean;
					fast_active_sym = active_sym;
					fast_active_signal = signal;
				}
			}
		}
		if (fast_active_sym == -1 || fast_active_signal != active_signal)
			active_sym = -1;
	}
	
	
	SetSignal(active_sym, active_signal);
	
	if (active_sym >= 0) {
		wait_cursor = as.GetCursor() + 60;
		init_balance = broker.AccountBalance();
	}
}

}
