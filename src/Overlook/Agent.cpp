#include "Overlook.h"

namespace Overlook {

AgentSignal::AgentSignal() {
	action_counts.SetCount(SIGNAL_ACTIONCOUNT, 0);
}

void AgentSignal::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % action_counts % iter % deep_iter;
}

void AgentSignal::DeepCreate() {
	deep_iter = 0;
	Create();
}

void AgentSignal::Create() {
	dqn.Reset();
	iter = 0;
	result_equity.Clear();
	result_drawdown.Clear();
	rewards.Clear();
	for(int i = 0; i < action_counts.GetCount(); i++)
		action_counts[i] = 0;
}

void AgentSignal::ResetEpoch() {
	if (cursor > 1) {
		result_equity.Add(all_reward_sum);
		double posneg_sum = pos_reward_sum + neg_reward_sum;
		double dd = posneg_sum > 0.0 ? (neg_reward_sum / posneg_sum * 100.0) : 100.0;
		result_drawdown.Add(dd);
	}
	
	signal = 0;
	
	change_sum = 0;
	change_count = 0;
	
	fwd_cursor = 0;
	cursor = 1;
	skip_learn = true;
	
	all_reward_sum = 0;
	pos_reward_sum = 0;
	neg_reward_sum = 0;
	
	prev_lower_output_signal = 0;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentSignal::Main(Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	Snapshot& cur_snap  = snaps[cursor - 0];
	Snapshot& prev_snap = snaps[cursor - 1];
	
	lower_output_signal = 0; //cur_snap.GetFilterOutput(group_id, FILTER_COUNT-1, sym_id);
	if (cur_snap.IsResultClusterPredictedTarget(sym_id, group_id))
		lower_output_signal = group_signal;
	ASSERT(group_signal != 0);
		
	change_sum += fabs(cur_snap.GetChange(sym_id));
	change_count++;
	
	if (lower_output_signal != prev_lower_output_signal) {
		if (!skip_learn) {
			Snapshot& fwd_snap = snaps[fwd_cursor];		// Snapshot of previous Forward call
			double reward = signal * (cur_snap.GetOpen(sym_id) / fwd_snap.GetOpen(sym_id) - 1.0);
			reward /= change_sum;
			if (reward >= 0)	pos_reward_sum += reward;
			else				neg_reward_sum -= reward;
			all_reward_sum += reward;
			
			Backward(reward);
		}
		Forward(cur_snap, prev_snap);
		fwd_cursor = cursor;
		
		prev_lower_output_signal = lower_output_signal;
	}
	
	Write(cur_snap, snaps);
	
	equity[cursor] = all_reward_sum;
	
	cursor++;
}

void AgentSignal::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	int prev_signal = signal;
	
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		signal = 0;
	} else {
		skip_learn = false;
		
		int cursor = 0;
		double input_array[SIGNAL_STATES];
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		ASSERT(cursor == SIGNAL_STATES);
		
		
		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < SIGNAL_ACTIONCOUNT);
		action_counts[action]++;
		
		if (!action)
			signal = 0;
		else
			signal = lower_output_signal;
	}
	
	change_sum = 0.0;
	change_count = 0;
}

void AgentSignal::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetSignalOutput(group_id, sym_id, signal);
}

void AgentSignal::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && !skip_learn) {
		dqn.Learn(reward);
		iter++;
		deep_iter++;
		
		reward_sum += reward;
		reward_count++;
		
		if (reward_count > REWARD_AV_PERIOD) {
			rewards.Add(reward_sum / reward_count);
			reward_count = 0;
			reward_sum = 0.0;
		}
	}
}























AgentAmp::AgentAmp() {
	action_counts.SetCount(AMP_ACTIONCOUNT, 0);
}

void AgentAmp::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % action_counts % iter % deep_iter;
}

void AgentAmp::DeepCreate() {
	deep_iter = 0;
	Create();
}

void AgentAmp::Create() {
	dqn.Reset();
	iter = 0;
	result_equity.Clear();
	result_drawdown.Clear();
	rewards.Clear();
	for(int i = 0; i < action_counts.GetCount(); i++)
		action_counts[i] = 0;
}

void AgentAmp::ResetEpoch() {
	action_counts.SetCount(AMP_ACTIONCOUNT, 0);
	
	if (broker.order_count > 0) {
		result_equity.Add(broker.equity);
		double dd = broker.GetDrawdown() * 100.0;
		if (!clean_epoch) dd = 100.0;
		result_drawdown.Add(dd);
	}
	broker.Reset();
	signal = 0;
	lower_output_signal = 0;
	prev_lower_output_signal = 0;
	prev_equity = broker.AccountEquity();
	
	change_sum = 0;
	change_count = 0;
	
	cursor = 1;
	skip_learn = true;
	clean_epoch = true;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentAmp::Main(Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	Snapshot& cur_snap = snaps[cursor - 0];
	
	lower_output_signal = cur_snap.GetSignalOutput(group_id, sym_id);
	change_sum += fabs(cur_snap.GetChange(sym_id));
	change_count++;
	
	if (lower_output_signal != prev_lower_output_signal) {
		Snapshot& prev_snap = snaps[cursor - 1];
		if (!skip_learn) {
			broker.RefreshOrders(cur_snap);
			double equity = broker.AccountEquity();
			double reward = (equity / prev_equity - 1.0) * 1000.0;
			reward /= change_sum;
			Backward(reward);
			if (equity < 0.25 * broker.begin_equity) {
				clean_epoch = false;
				broker.Reset();
			}
		}
		prev_equity = broker.AccountEquity();
		Forward(cur_snap, prev_snap);
		prev_lower_output_signal = lower_output_signal;
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.AccountEquity();
	cursor++;
}

void AgentAmp::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	int prev_signal = signal;
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		signal = 0;
	}
	else {
		skip_learn = false;
		
		int cursor = 0;
		double input_array[AMP_STATES];
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		ASSERT(cursor == AMP_STATES);
		

		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < AMP_ACTIONCOUNT);
		action_counts[action]++;
		
		const int maxscale_steps = AMP_MAXSCALES;
		int maxscale_step = action % maxscale_steps;
		int maxscale = 1 + maxscale_step * AMP_MAXSCALE_MUL;
		
		signal = lower_output_signal * maxscale;
	}
	
	change_sum = 0.0;
	change_count = 0;
	
	
	// Following accepts previous iteration values.
	// Rest of the function does NOT affect the exported value.
	// Copying signals back from snapshot does NOT affect realtime quality.
	// Invalid accumulation in the broker does NOT affect following forward calls in the forward-only mode.
	// This is not skipped in the forward-only mode, because the equity graph is being drawn for visualization.
	// Broker signals may NOT be read in realtime, because it can be reseted in a unsuccessful try.
	
	broker.SetSignal(sym_id, signal);
	bool succ = broker.Cycle(cur_snap);
	
	if (!succ)
		broker.Reset();
}

void AgentAmp::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetAmpOutput(group_id, sym_id, signal);
}

void AgentAmp::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && !skip_learn) {
		dqn.Learn(reward);
		iter++;
		deep_iter++;
		
		reward_sum += reward;
		reward_count++;
		
		if (reward_count > REWARD_AV_PERIOD) {
			rewards.Add(reward_sum / reward_count);
			reward_count = 0;
			reward_sum = 0.0;
		}
	}
}


















Agent::Agent()
{
	sig.agent	= this;
	amp.agent	= this;
}

void Agent::CreateAll() {
	sig.Create();
	amp.Create();
}


void Agent::Init() {
	ASSERT(sym_id >= 0);
	
	group->sys->SetFixedBroker(sym_id,			amp.broker);
	
	spread_points = group->sys->spread_points[sym_id];
	
	sig.group_signal = group->sys->result_centroids[group_id].change > 0 ? +1 : -1;
	
	RefreshSnapEquities();
	RefreshGroupSettings();
	ResetEpochAll();
}

void Agent::RefreshSnapEquities() {
	int snap_count = group->sys->snaps.GetCount();
	sig.equity.SetCount(snap_count, 0);
	amp.equity.SetCount(snap_count, 0);
}

void Agent::ResetEpochAll() {
	sig.ResetEpoch();
	amp.ResetEpoch();
}

void Agent::ResetEpoch(int phase) {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		sig.ResetEpoch(); break;
		case PHASE_AMP_TRAINING:		amp.ResetEpoch(); break;
	}
}

void Agent::Main(int phase, Vector<Snapshot>& snaps) {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		sig.Main(snaps); break;
		case PHASE_AMP_TRAINING:		amp.Main(snaps); break;
	}
}

int Agent::GetCursor(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.cursor;
		case PHASE_AMP_TRAINING:		return amp.cursor;
	}
	return 0;
}

double Agent::GetLastDrawdown(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.GetLastDrawdown();
		case PHASE_AMP_TRAINING:		return amp.GetLastDrawdown();
	}
	return 0;
}

double Agent::GetLastResult(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.GetLastResult();
		case PHASE_AMP_TRAINING:		return amp.GetLastResult();
	}
	return 0;
}

int64 Agent::GetIter(int phase) const {
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.iter;
		case PHASE_AMP_TRAINING:		return amp.iter;
	}
	return 0;
}

void Agent::Serialize(Stream& s) {
	s % sig % amp % sym_id % sym % group_id;
}

void Agent::RefreshGroupSettings() {
	ASSERT(group_id != -1);
	// Lower group id has higher priority.
	// Filter splits the time range to half. The higher priority takes to more volatile half.
	// E.g. At group 0 & level 2: time range is 6 hours of 24 hours.
	//      It takes the higher than average part in every level (and not lower, thus false).
	// E.g. At group 3 & level 1: time range is 12 hours of 24 hours.
	//      It takes the better 12 hours at first split and worse 6 hours in the second split.
	//		It uses one step slower timesteps, because the worse part has less volatility.
	switch (group_id) {
		case 0:
			group_active_lower[0] = false;
			group_active_lower[1] = false;
			group_active_lower[2] = false;
			break;
		case 1:
			group_active_lower[0] = false;
			group_active_lower[1] = false;
			group_active_lower[2] = true;
			break;
		case 2:
			group_active_lower[0] = false;
			group_active_lower[1] = true;
			group_active_lower[2] = false;
			break;
		case 3:
			group_active_lower[0] = false;
			group_active_lower[1] = true;
			group_active_lower[2] = true;
			break;
		case 4:
			group_active_lower[0] = true;
			group_active_lower[1] = false;
			group_active_lower[2] = false;
			break;
		case 5:
			group_active_lower[0] = true;
			group_active_lower[1] = false;
			group_active_lower[2] = true;
			break;
		case 6:
			group_active_lower[0] = true;
			group_active_lower[1] = true;
			group_active_lower[2] = false;
			break;
		case 7:
			group_active_lower[0] = true;
			group_active_lower[1] = true;
			group_active_lower[2] = true;
			break;
	}
}

bool Agent::IsTrained(int phase) const {
	if (phase < 0) return false;
	if ((phase == PHASE_SIGNAL_TRAINING && sig.iter >= SIGNAL_PHASE_ITER_LIMIT) ||
		(phase == PHASE_AMP_TRAINING    && amp.iter >= AMP_PHASE_ITER_LIMIT)) {
		return true;
	}
	return false;
}

}
