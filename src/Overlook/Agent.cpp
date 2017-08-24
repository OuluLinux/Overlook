#if 0

#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent() {
	posreward = 0;
	negreward = 0;
	smooth_reward = 0;
	prev_equity = 0;
	group_count = 0;
	tf_step = 0;
	timestep = 0;
	has_yeartime = 0;
	is_training_iteration = 0;
	
	agent_id = -1;
	sym_id = -1;
	sym = -1;
	proxy_sym = -1;
	agent_input_width = 0;
	agent_input_height = 0;
	has_timesteps = false;
}

Agent::~Agent() {
	//Stop();
}

void Agent::RefreshTotalEpochs() {
	epoch_total = group->train_pos_all.GetCount();
}

void Agent::Create(int width, int height) {
	//dqn.Init(width, height, !has_timesteps ? ACTIONCOUNT : 1+2*5);
	dqn.Reset();
	
	//ASSERT(!group->param_str.IsEmpty());
	//dqn.LoadInitJSON(group->param_str);
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
	
	broker.SetFixedVolume();*/
}

void Agent::Start() {
	if (main_id != -1) return;
	//epoch_actual = 0;
	Panic("TODO");
	//main_id = group->sys->AddTaskBusy(THISBACK(Main));
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
	is_training_iteration = !group->is_looping && is_training;
	
	int ret_epoch_actual = epoch_actual + 1;
	
	RefreshTotalEpochs();
	if (epoch_total > 0) {
		if (epoch_actual == 0) {
			prev_equity = broker.AccountEquity();
			posreward = 0;
			negreward = 0;
			timestep = 0;
		}
		
		timestep--;
		bool skip_action = false;
		
		if (timestep > 0) {
			if (is_training_iteration) {
				double equity = broker.AccountEquity();
				int epoch_begin = epoch_actual;
				for(int i = 0; i < timestep; i++) {
					epoch_actual++;
					SeekActive();
				}
				timestep = 0;
				
				for(int i = epoch_begin; i <= epoch_actual && i < group->snaps.GetCount(); i++)
					thrd_equity[i] = equity;
				
				if (epoch_actual >= group->snaps.GetCount())
					epoch_actual  = group->snaps.GetCount() - 1;
			} else {
				epoch_actual++;
				skip_action = true;
				
				if (group->is_looping && epoch_actual > epoch_total - 10) {
					LOG("Skip sym=" << sym << " tf=" << tf << " : " << timestep << " (" << epoch_actual << " / " << epoch_total << ")");
				}
			}
		}
		
		if (!skip_action && group->is_looping && epoch_actual < group->snaps.GetCount()) {
			Snapshot& snap = group->snaps[epoch_actual];
			if (!snap.IsActive(tf_id)) {
				epoch_actual++;
				skip_action = true;
			}
		}
		
		if (!skip_action) {
			// Do some action
			if (epoch_actual < group->snaps.GetCount())
				Action();
			else if (!group->is_looping)
				epoch_actual = 0;
		}
	}
	
	if (group->is_looping)
		epoch_actual = ret_epoch_actual;
	
	at_main = false;
}

void Agent::Forward(Snapshot& snap, SimBroker& broker) {
	ASSERT(group_id != -1);
	
	// Input values
	// - time_values
	// - all data from snapshot
	// - previous signal
	// - account change sensor
	input_array.SetCount(agent_input_height);
	int cursor = 0;
	
	
	// time_values
	input_array[cursor++] = snap.year_timesensor;
	input_array[cursor++] = snap.wday_timesensor;
	
	
	// sensor data for current tf
	int block_size = group->sym_ids.GetCount() * group->buf_count;
	int all_block_count = tf_id + 1;
	int slower_block_count = tf_id;
	int slow_size = block_size * slower_block_count;
	memcpy(		input_array.Begin() + cursor,
				snap.sensors.Begin() + slow_size,
				block_size * sizeof(double));
	cursor += block_size;
	
	
	// all current signals from slower agents
	block_size = group->sym_ids.GetCount() * 2;
	slow_size = block_size * slower_block_count;
	memcpy(		input_array.Begin() + cursor,
				snap.signals.Begin(),
				slow_size * sizeof(double));
	cursor += slow_size;
	
	
	// all previous signals from same tf agents
	memcpy(		input_array.Begin() + cursor,
				snap.prev_signals.Begin() + slow_size,
				block_size * sizeof(double));
	cursor += block_size;
	
	
	// all previous rewards from same tf agents
	int all_size = block_size * all_block_count;
	memcpy(		input_array.Begin() + cursor,
				snap.prev_rewards.Begin(),
				all_size * sizeof(double));
	cursor += all_size;
	
	
	ASSERT(cursor == agent_input_height);
	int action = dqn.Act(input_array);
	
    
    // Convert action to simple signal
    int signal;
    timestep = 1;
    if (!has_timesteps) {
		if      (action == ACT_NOACT)  signal =  0;
		else if (action == ACT_INCSIG) signal = +1;
		else if (action == ACT_DECSIG) signal = -1;
		else Panic("Invalid action");
    } else {
		if      (action == ACT_NOACT)  signal =  0;
		else {
			action--;
			int exp = 2 + action / 2;
			bool neg = exp % 2; // 0,+1,-1,-2,+2,+4,-4
			bool dir = (action % 2) != neg;
			signal = dir ? -1 : +1;
			timestep = 1 << exp;
		}
    }
	
	
	// Write latest average to the group values
	double pos, neg;
	if (signal == 0) {
		pos = 1.0;
		neg = 1.0;
	}
	else if (signal > 0) {
		pos = 0.0;
		neg = 1.0;
	}
	else {
		pos = 1.0;
		neg = 0.0;
	}
	int sigpos = (tf_id * group->sym_ids.GetCount() + sym_id) * 2;
	snap.signals[sigpos + 0] = pos;
	snap.signals[sigpos + 1] = neg;
	
	snap.prev_rewards[sigpos + 0] = posreward;
	snap.prev_rewards[sigpos + 1] = negreward;
	
	
	// Value is copied from next snap to next new signal position.
	// Also, reward value is copied from this, because current timestep reward is unknown.
	int end = snap.shift + tf_step * timestep;
	for(int i = snap.id + 1; i < group->snaps.GetCount(); i++) {
		Snapshot& s = group->snaps[i];
		if (s.shift >= end) {
			if (s.shift == end) {
				s.prev_signals[sigpos + 0] = pos;
				s.prev_signals[sigpos + 1] = neg;
			}
			break;
		}
		s.signals[sigpos + 0] = pos;
		s.signals[sigpos + 1] = neg;
		s.prev_signals[sigpos + 0] = pos;
		s.prev_signals[sigpos + 1] = neg;
		s.prev_rewards[sigpos + 0] = posreward;
		s.prev_rewards[sigpos + 1] = negreward;
	}
	
	
	// Set signal to broker, but freeze it if it's same than previously.
	int prev_signal = broker.GetSignal(sym);
	if (signal != prev_signal) {
		broker.SetSignal(sym, signal);
		broker.SetSignalFreeze(sym, false);
	} else {
		broker.SetSignalFreeze(sym, signal != 0);
	}
}

void Agent::Backward(double reward) {
	
	
	// Get reward sensor values
	int sigpos = (tf_id * group->sym_ids.GetCount() + sym_id) * 2;
	
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
	
	
	// pass to brain for learning
	if (is_training_iteration)
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
	  % agent_input_width % agent_input_height
	  % has_timesteps;
}

}
#endif
