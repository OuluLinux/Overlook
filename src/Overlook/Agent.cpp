#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent() {
	cur_snap = NULL;
	proxy_sym = -1;
	sym_id = -1;
	sym = -1;
	
	group_count = 0;
	smooth_reward = 0.0;
	agent_input_width = 0;
	agent_input_height = 0;
}

Agent::~Agent() {
	Stop();
}

void Agent::RefreshTotalEpochs() {
	epoch_total = group->train_pos_all.GetCount();
}

void Agent::Create(int width, int height) {
	// Don't use ACT_RESETSIG if signal accumulation is not in use
	dqn.Init(width, height, ACTIONCOUNT);
	dqn.Reset();
	
	ASSERT(!group->param_str.IsEmpty());
	dqn.LoadInitJSON(group->param_str);
}

void Agent::Init() {
	ASSERT(agent_input_width != 0);
	
	DataBridge* db = dynamic_cast<DataBridge*>(group->databridge_cores[sym][tf]);
	// NO unique start, because early birds are trained with partly invalid data
	//		data_begin = group->sys->GetShiftTf(tf, group->main_tf, db->GetDataBegin());
	data_begin = group->sys->GetShiftTf(tf, group->main_tf, group->data_begins[tf_id]);
	
	int tf_period = group->sys->GetPeriod(tf);
	int maintf_period = group->sys->GetPeriod(group->main_tf);
	ASSERT(tf_period % maintf_period == 0);
	tf_step = tf_period / maintf_period;
	ASSERT(tf_step > 0);
	
	TraineeBase::Init();
	
	int tf_mins = tf_period * group->sys->GetBasePeriod() / 60;
	has_yeartime = tf_mins >= 7*24*60;
	
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
	if (at_main) return;
	at_main = true;
	RefreshTotalEpochs();
	if (epoch_total > 0) {
		if (epoch_actual == 0) {
			prev_equity = broker.AccountEquity();
		}
		
		// Do some action
		if (epoch_actual < group->snaps.GetCount())
			Action();
		else if (!group->is_looping)
			epoch_actual = 0;
	}
	at_main = false;
}

void Agent::Forward(Snapshot& snap, SimBroker& broker) {
	ASSERT(group_id != -1);
	this->cur_snap = &snap;
	
	// Input values
	// - time_values
	// - all data from snapshot
	// - previous signal
	// - account change sensor
	input_array.SetCount(agent_input_height);
	int cursor = 0;
	
	
	// time_values
	if (!has_yeartime)
		input_array[cursor] = snap.values[cursor];
	else
		input_array[cursor] = (snap.time_values[0] * 31.0 + snap.time_values[1]) / 372.0;
	cursor++;
	
	
	// all data from snapshot
	int block = group->sym_ids.GetCount() * group->buf_count;
	int block_count = tf_id + 1;
	int data_begin = 1; // after first time value
	memcpy(
		input_array.Begin() + cursor,
		snap.values.Begin() + data_begin,
		block * block_count * sizeof(double));
	cursor += block * block_count;
	
	
	// sensors
	block = group->sym_ids.GetCount() * 2 * 2;
	memcpy(
		input_array.Begin() + cursor,
		snap.values.Begin() + group->data_size,
		block * block_count * sizeof(double));
	cursor += block * block_count;
	
	ASSERT(cursor == agent_input_height);
	int action = dqn.Act(input_array);
	
    
    // Convert action to simple signal
    int signal;
	if      (action == ACT_NOACT)  signal =  0;
	else if (action == ACT_INCSIG) signal = +1;
	else if (action == ACT_DECSIG) signal = -1;
	else Panic("Invalid action");
	
    snap.signals[group_id] = signal;
	
	// Write latest average to the group values
	double d = signal;
	ASSERT(d >= -1.0 && d <= 1.0);
	double pos, neg;
	if (d >= 0) {
		pos = 1.0 - d;
		neg = 1.0;
	} else {
		pos = 1.0;
		neg = 1.0 + d;
	}
	
	
	// Value is copied from next snap to next new signal position.
	// Also, reward value is copied from this, because current timestep reward is unknown.
	int end = snap.shift + tf_step;
	int sigpos = group->GetSignalPos(group_id);
	double this_posreward = snap.values[sigpos + 2];
	double this_negreward = snap.values[sigpos + 3];
	for(int i = snap.id + 1; i < group->snaps.GetCount(); i++) {
		Snapshot& s = group->snaps[i];
		if (s.shift > end)
			break;
		int j = sigpos;
		s.values[j++] = pos;
		s.values[j++] = neg;
		s.values[j++] = this_posreward;
		s.values[j  ] = this_negreward;
	}
	
	
	// Set signal to broker, but freeze it if it's same than previously.
	if (group->sig_freeze) {
		int prev_signal = broker.GetSignal(sym);
		if (signal != prev_signal) {
			broker.SetSignal(sym, signal);
			broker.SetSignalFreeze(sym, false);
		} else {
			broker.SetSignalFreeze(sym, true);
		}
	} else {
		broker.SetSignal(sym, signal);
		broker.SetSignalFreeze(sym, false);
	}
	
}

void Agent::Backward(double reward) {
	// Write reward average to the next snapshot
	ASSERT(cur_snap);
	
	
	// Get reward sensor values
	int sigpos = group->GetSignalPos(group_id);
	int reward_pos = sigpos + 2;
	double posreward, negreward;
	if (reward > 0.0) {
		double max = reward_average.mean * 2.0;
		posreward = 1.0 - Upp::max(0.0, Upp::min(1.0, reward / max));
		negreward = 1.0;
	}
	else if (reward < 0.0) {
		double max = loss_average.mean * 2.0;
		posreward = 1.0;
		negreward = 1.0 - Upp::max(0.0, Upp::min(1.0, -reward / max));
	}
	else {
		posreward = 1.0;
		negreward = 1.0;
	}
	
	
	// Value is copied to next snap
	Snapshot& snap = *cur_snap;
	int end = snap.shift + tf_step;
	for(int i = snap.id + 1; i < group->snaps.GetCount(); i++) {
		Snapshot& s = group->snaps[i];
		if (s.shift < end)
			continue;
		int j = reward_pos;
		s.values[j++] = posreward;
		s.values[j  ] = negreward;
		break;
	}
	
	
	// pass to brain for learning
	if (!group->is_looping && is_training)
		dqn.Learn(reward);
	
	smooth_reward += reward;
	
	if (iter % 50 == 0) {
		smooth_reward /= 50;
		WhenRewardAverage(smooth_reward);
		smooth_reward = 0;
	}
	iter++;
}

void Agent::SetAskBid(SimBroker& sb, int pos) {
	if (tf != group->main_tf)
		pos = group->sys->GetShiftTf(group->main_tf, tf, pos);
	
	ASSERT(pos >= 0);
	ASSERT(sym != -1);
	{
		Core& core = *group->databridge_cores[sym][tf];
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(sym, open.Get(pos));
	}
	if (proxy_sym != -1) {
		Core* c = group->databridge_cores[proxy_sym][tf];
		ASSERT(c);
		Core& core = *c;
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(proxy_sym, open.Get(pos));
	}
	sb.SetTime(group->sys->GetTimeTf(tf, pos));
}

void Agent::Serialize(Stream& s) {
	TraineeBase::Serialize(s);
	s % dqn % sym_id % sym % proxy_sym
	  % agent_input_width % agent_input_height;
}

}
