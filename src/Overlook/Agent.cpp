#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent() {
	next_snap = NULL;
	proxy_sym = -1;
	sym = -1;
	
	group_count = 0;
	smooth_reward = 0.0;
	accum_signal = false;
}

Agent::~Agent() {
	Stop();
}

void Agent::Create(int width, int height) {
	// Don't use ACT_RESETSIG if signal accumulation is not in use
	dqn.Init(width, height, accum_signal ? ACTIONCOUNT : ACTIONCOUNT-1);
	dqn.Reset();
	
	ASSERT(!group->param_str.IsEmpty());
	dqn.LoadInitJSON(group->param_str);
}

void Agent::Init() {
	TraineeBase::Init();
	
	broker.SetFixedVolume();
}

void Agent::Start() {
	if (main_id != -1) return;
	epoch_actual = 0;
	main_id = group->sys->AddTaskBusy(THISBACK(Main));
}

void Agent::Stop() {
	if (main_id == -1) return;
	group->sys->RemoveBusyTask(main_id);
	main_id = -1;
	while (at_main) Sleep(100);
}

void Agent::Main() {
	ASSERT(!at_main);
	at_main = true;
	epoch_total = group->train_pos[group_id].GetCount();
	if (epoch_total > 0) {
		if (epoch_actual == 0) {
			accum_buf = 0;
		}
		
		// Do some action
		Action();
	}
	at_main = false;
}

void Agent::Forward(Snapshot& snap, SimBroker& broker, Snapshot* next_snap) {
	ASSERT(group_id != -1);
	this->next_snap = next_snap;
	
	// Input values
	// - time_values
	// - all data from snapshot
	// - 'accum_buf'
	// - account change sensor
	ASSERT(snap.values.GetCount() == group->agent_input_height);
	int action = dqn.Act(snap.values);
	
    
    // Convert action to simple signal
    int signal;
	if      (action == ACT_NOACT)  signal =  0;
	else if (action == ACT_INCSIG) signal = +1;
	else if (action == ACT_DECSIG) signal = -1;
	else if (action == ACT_RESETSIG) {signal = 0; accum_buf = 0;}
	else Panic("Invalid action");
	
    if (accum_signal) {
		accum_buf += signal;
		if      (accum_buf > +20) accum_buf = +20;
		else if (accum_buf < -20) accum_buf = -20;
		
		if (accum_buf < 0) signal = -1;
		else if (accum_buf > 0) signal = +1;
		else signal = 0;
    }
	
	if (next_snap) {
		
		// Write latest average to the group values
		int i = group->GetSignalPos(group_id);
		double d = accum_signal ?
			accum_buf / 20.0 :
			signal;
		ASSERT(d >= -1.0 && d <= 1.0);
		if (d >= 0) {
			next_snap->values[i++] = 1.0 - d;
			next_snap->values[i++] = 1.0;
		} else {
			next_snap->values[i++] = 1.0;
			next_snap->values[i++] = 1.0 + d;
		}
		
		
		// Write instrument sensors
		double max = 10.0;
		d = 0.0;
		const Array<Order>& open_orders = broker.GetOpenOrders();
		for(int i = 0; i < open_orders.GetCount(); i++)
			d += open_orders[i].profit;
		if (d > 0.0) {
			next_snap->values[i++] = 1.0 - Upp::max(0.0, Upp::min(1.0, d / max));
			next_snap->values[i++] = 1.0;
		}
		else if (d < 0.0) {
			next_snap->values[i++] = 1.0;
			next_snap->values[i++] = 1.0 - Upp::max(0.0, Upp::min(1.0, -d / max));
		}
		else {
			next_snap->values[i++] = 1.0;
			next_snap->values[i++] = 1.0;
		}
	}
	
	
	// Set signal to broker, but freeze it if it's same than previously.
	int prev_signal = broker.GetSignal(sym);
	if (signal != prev_signal) {
		broker.SetSignal(sym, signal);
		broker.SetSignalFreeze(sym, false);
	} else {
		broker.SetSignalFreeze(sym, true);
	}
	
}

void Agent::Backward(double reward) {
	// Write reward average to the next snapshot
	if (next_snap) {
		double pos, neg;
		int reward_pos = group->GetSignalPos(group_id) + 4;
		if (reward > 0.0) {
			reward_average.Add(reward);
			double max = reward_average.mean * 2.0;
			next_snap->values[reward_pos + 0] = 1.0 - Upp::max(0.0, Upp::min(1.0, reward / max));
			next_snap->values[reward_pos + 1] = 1.0;
		}
		else if (reward < 0.0) {
			loss_average.Add(-reward);
			double max = loss_average.mean * 2.0;
			next_snap->values[reward_pos + 0] = 1.0;
			next_snap->values[reward_pos + 1] = 1.0 - Upp::max(0.0, Upp::min(1.0, -reward / max));
		}
		else {
			next_snap->values[reward_pos + 0] = 1.0;
			next_snap->values[reward_pos + 1] = 1.0;
		}
	}
	
	
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
	s % dqn % sym % proxy_sym % accum_signal;
}

}
