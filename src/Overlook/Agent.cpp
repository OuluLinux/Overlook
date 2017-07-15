#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent() {
	proxy_sym = -1;
	sym = -1;
	group_id = -1;
	
	not_stopped = 0;
	group_count = 0;
	smooth_reward = 0.0;
	running = false;
}

Agent::~Agent() {
	Stop();
}

void Agent::Init() {
	TraineeBase::Init();
	
	broker.SetFixedVolume();
}

void Agent::Start() {
	if (running) return;
	running = true;
	not_stopped++;
	Thread::Start(THISBACK(Main));
}

void Agent::Stop() {
	if (!running) return;
	running = false;
	while (not_stopped) Sleep(100);
}

void Agent::Main() {
	ASSERT(group);
	epoch_actual = 0;
	
	while (running) {
		epoch_total = group->snaps.GetCount();
		
		// Do some action
		Action();
		
	}
	
	not_stopped--;
}

void Agent::Forward(Snapshot& snap, Brokerage& broker, Snapshot* next_snap) {
	ASSERT(group_id != -1);
	
	// Put time values to the input
	// - time_values
	// - all data from snapshot (or single symbol)
	// - all previous signals (or single sym)
	int action;
	if (!group->single_data && !group->single_signal) {
		// The simplest case is to just use the whole vector of the snapshot
		ASSERT(snap.values.GetCount() == group->input_height);
		action = dqn.Act(snap.values);
	}
	else {
		// Copy partial data to separate vector
		Panic("TODO");
		if (group->single_data) {
			
		}
		else {
			// in forward pass the agent simply behaves in the environment
			int ag_begin = group->GetSignalBegin();
			int ag_end   = group->GetSignalEnd();
			int ag_diff  = ag_end - ag_begin;
			
			
			// Copy values from data input (expect values to be in 0-1 range
			/*int j = 0;
			for(int i = 0; i < tf_ids.GetCount(); i++) {
				for(int k = 0; k < buf_count; k++) {
					double d = snap.values[i * buf_count + k];
					ASSERT(d >= 0.0 && d <= 1.0);
					input_array[j++] = d;
				}
			}*/
		}
		
		
		if (group->single_signal) {
			
		}
		else {
			
		}
		
		action = dqn.Act(input_array);
	}
	
    
    // Convert action to simple signal
    int signal;
	if      (action == ACT_NOACT)  signal =  0;
	else if (action == ACT_INCSIG) signal = +1;
	else if (action == ACT_DECSIG) signal = -1;
	else Panic("Invalid action");
	
	
	if (next_snap) {
		
		// Collect average of the value
		signal_average.Set(signal);
		signal_average.SeekNext();
	
	
		// Write latest average to the group values
		int ag_begin = group->GetSignalPos(group_id);
		int ag_end   = group->GetSignalPos(group_id + 1);
		ASSERT(ag_end - ag_begin == group->tf_ids.GetCount() * 2);
		for(int i = ag_begin, j = 0; i < ag_end; j++) {
			double d = signal_average.Get(j);
			ASSERT(d >= -1.0 && d <= 1.0);
			if (d >= 0) {
				next_snap->values[i++] = 1.0 - d;
				next_snap->values[i++] = 1.0;
			} else {
				next_snap->values[i++] = 1.0;
				next_snap->values[i++] = 1.0 + d;
			}
		}
	}
	
	
	// Set signal to broker
	if (!group->sig_freeze) {
		// Don't use signal freezing. Might cause unreasonable costs.
		broker.SetSignal(sym, signal);
		broker.SetSignalFreeze(sym, false);
	} else {
		// Set signal to broker, but freeze it if it's same than previously.
		int prev_signal = broker.GetSignal(sym);
		if (signal != prev_signal) {
			broker.SetSignal(sym, signal);
			broker.SetSignalFreeze(sym, false);
		} else {
			broker.SetSignalFreeze(sym, true);
		}
	}
	
}

void Agent::Backward(double reward) {
	
	// pass to brain for learning
	dqn.Learn(reward);
	
	smooth_reward += reward;
	
	if (iter % 50 == 0) {
		smooth_reward /= 50;
		WhenRewardAverage(smooth_reward);
		smooth_reward = 0;
	}
	iter++;
}

int Agent::GetAction(const Volume& fwd, int sym) const {
	int pos = sym * ACTIONCOUNT;
	double max_col = 0;
	double max_val = fwd.Get(pos);
	for(int i = 1; i < ACTIONCOUNT; i++) {
		double val = fwd.Get(pos + i);
		
		// Skip action with invalid values
		if (!IsFin(val))
			return ACT_NOACT;
		
		
		if (val > max_val) {
			max_val = val;
			max_col = i;
		}
	}
	return max_col;
}

void Agent::SetAskBid(SimBroker& sb, int pos) {
	ASSERT(sym != -1);
	{
		Core& core = *group->databridge_cores[sym];
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(sym, open.Get(pos));
	}
	if (proxy_sym != -1) {
		Core& core = *group->databridge_cores[proxy_sym];
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(proxy_sym, open.Get(pos));
	}
	sb.SetTime(group->sys->GetTimeTf(group->tf_ids.Top(), pos));
}

void Agent::Serialize(Stream& s) {
	TraineeBase::Serialize(s);
	s % sym % proxy_sym % group_id;
}

}
