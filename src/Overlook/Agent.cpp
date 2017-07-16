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
		
		if (epoch_actual == 0) {
			accum_buf = 0;
		}
		
		// Do some action
		Action();
		
	}
	
	not_stopped--;
}

void Agent::Forward(Snapshot& snap, Brokerage& broker, Snapshot* next_snap) {
	ASSERT(group_id != -1);
	
	// Input values
	// - time_values
	// - all data from snapshot
	// - 'accum_buf'
	// - account change sensor
	ASSERT(snap.values.GetCount() == group->input_height);
	int action = dqn.Act(snap.values);
	
    
    // Convert action to simple signal
    int signal;
	if      (action == ACT_NOACT)  signal =  0;
	else if (action == ACT_INCSIG) signal = +1;
	else if (action == ACT_DECSIG) signal = -1;
	else if (action == ACT_RESETSIG) {signal = 0; accum_buf = 0;}
	else Panic("Invalid action");
	
	accum_buf += signal;
	if      (accum_buf > +20) accum_buf = +20;
	else if (accum_buf < -20) accum_buf = -20;
	
	if (accum_buf < 0) signal = -1;
	else if (accum_buf > 0) signal = +1;
	else signal = 0;
	
	if (next_snap) {
		
		// Write latest average to the group values
		int i = group->GetSignalPos(group_id);
		double d = accum_buf / 20.0;
		ASSERT(d >= -1.0 && d <= 1.0);
		if (d >= 0) {
			next_snap->values[i++] = 1.0 - d;
			next_snap->values[i++] = 1.0;
		} else {
			next_snap->values[i++] = 1.0;
			next_snap->values[i++] = 1.0 + d;
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
