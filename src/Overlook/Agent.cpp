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
	target_id = 0;
	
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
	
	lower_output_signal = 0;
	if (cur_snap.IsResultClusterPredictedTarget(sym_id, group_id, target_id))
		lower_output_signal = group_signal;
	ASSERT(group_signal != 0);
		
	change_sum += fabs(cur_snap.GetChange(sym_id));
	change_count++;
	
	if (lower_output_signal != prev_lower_output_signal) {
		if (!skip_learn) {
			Snapshot& fwd_snap = snaps[fwd_cursor];		// Snapshot of previous Forward call
			double reward = 0.0;
			if (signal == +1)
				reward = +1.0 * ((cur_snap.GetOpen(sym_id) - agent->spread_points) / fwd_snap.GetOpen(sym_id) - 1.0);
			else if (signal == -1)
				reward = -1.0 * (cur_snap.GetOpen(sym_id) / (fwd_snap.GetOpen(sym_id) - agent->spread_points) - 1.0);
			if (change_sum > 0.0)	reward /= change_sum;
			else					reward = 0.0;
			if (reward >= 0)		pos_reward_sum += reward;
			else					neg_reward_sum -= reward;
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
		
		#ifdef flagSKIP_IDLEACT
		signal = lower_output_signal;
		target_id = action;
		ASSERT(target_id >= 0 && target_id < TARGET_COUNT);
		#else
		if (!action) {
			signal = 0;
			target_id = 0;
		}
		else {
			signal = lower_output_signal;
			target_id = action - 1;
			ASSERT(target_id >= 0 && target_id < TARGET_COUNT);
		}
		#endif
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
			if (change_sum > 0.0)	reward /= change_sum;
			else					reward = 0.0;
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
	
	group->sys->SetFixedBroker(sym_id, amp.broker);
	
	spread_points = group->sys->spread_points[sym_id];
	
	sig.group_signal = group->sys->result_centroids[group_id].change > 0 ? +1 : -1;
	
	RefreshSnapEquities();
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

bool Agent::IsTrained(int phase) const {
	if (is_fail)
		return true;
	if (phase < 0)
		return false;
	if ((phase == PHASE_SIGNAL_TRAINING && sig.iter >= SIGNAL_PHASE_ITER_LIMIT) ||
		(phase == PHASE_AMP_TRAINING    && amp.iter >= AMP_PHASE_ITER_LIMIT))
		return true;
	return false;
}





























AgentFuse::AgentFuse() {
	action_counts.SetCount(FUSE_ACTIONCOUNT, 0);
}

void AgentFuse::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % action_counts % av_trigger % iter % deep_iter;
}

void AgentFuse::DeepCreate() {
	deep_iter = 0;
	Create();
}

void AgentFuse::Init() {
	if (iter == 0) Create();
	for(int i = 0; i < FUSE_BROKERCOUNT; i++)
		sys->SetFixedBroker(-1, broker[i]);
	sys->SetFixedBroker(-1, test_broker);
	RefreshSnapEquities();
	ResetEpoch();
	for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
		broker[i].free_margin_level = 0.90;
		for(int j = 0; j < FUSE_AVCOUNT; j++) {
			triggers[i * FUSE_AVCOUNT + j].SetPeriod(MEASURE_PERIOD(FUSE_TRIGGERPERIOD + j));
		}
	}
	dqn.SetExperienceCountMax(10000);
}

void AgentFuse::RefreshSnapEquities() {
	int snap_count = sys->snaps.GetCount();
	equity.SetCount(snap_count, 0);
}

void AgentFuse::Create() {
	dqn.Reset();
	iter = 0;
	result_equity.Clear();
	result_drawdown.Clear();
	rewards.Clear();
	av_trigger.Clear();
	for(int i = 0; i < action_counts.GetCount(); i++)
		action_counts[i] = 0;
}

void AgentFuse::ResetEpoch() {
	action_counts.SetCount(FUSE_ACTIONCOUNT, 0);
	
	if (test_broker.order_count > 0) {
		result_equity.Add(test_broker.equity);
		double dd = test_broker.GetDrawdown() * 100.0;
		if (!clean_epoch) dd = 100.0;
		result_drawdown.Add(dd);
	}
	test_broker.Reset();
	prev_test_equity = test_broker.AccountEquity();
	
	
	for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
		broker[i].Reset();
		prev_equity[i] = broker[i].AccountEquity();
	}
	
	for(int i = 0; i < FUSE_TRIGGERCOUNT; i++) {
		triggers[i].Clear();
	}
	
	cursor = 1;
	skip_learn = true;
	clean_epoch = true;
	active_broker = 0;
	trigger_ma = 0;
	prev_trigger_cursor = 0;
	
	reward_count = 0;
	reward_sum = 0.0;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentFuse::Main(Vector<Snapshot>& snaps) {
	if (cursor < 0 || cursor >= snaps.GetCount()) return;
	
	Snapshot& cur_snap = snaps[cursor - 0];
	
	
	for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
		FixedSimBroker& b = broker[i];
		b.RefreshOrders(cur_snap);
		double e = b.AccountEquity();
		for(int j = 0; j < FUSE_AVCOUNT; j++)
			triggers[i * FUSE_AVCOUNT + j].Add(e);
	}
	test_broker.RefreshOrders(cur_snap);
	
	
	if (IsTriggered()) {
		
		// Collect some stats about how often this is triggered
		if (cursor > prev_trigger_cursor)
			av_trigger.Add(cursor - prev_trigger_cursor);
		prev_trigger_cursor = cursor;
		
		
		double equity = test_broker.AccountEquity();
		if (!skip_learn) {
			double reward = (equity / prev_test_equity - 1.0) * 1000.0;
			
			Backward(reward);
			
			for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
				if (equity < 0.25 * test_broker.begin_equity) {
					clean_epoch = false;
					test_broker.Reset();
				}
			}
		}
		
		Snapshot& prev_snap = snaps[cursor - 1];
		Forward(cur_snap, prev_snap);
		
		
		prev_test_equity = equity;
	}
	
	
	for(int ddlevel = 0; ddlevel < 2; ddlevel++) {
		double ddlimit_begin, ddlimit_end;
		if (!ddlevel) {
			ddlimit_begin = 0.0;
			ddlimit_end   = 15.0;
		} else {
			ddlimit_begin = 15.0;
			ddlimit_end   = 40.0;
		}
		
		for(int i = 0; i < SYM_COUNT; i++) {
			double max_res = 0.0;
			int group_id = -1;
			int group_signal = 0;
			
			for (int j = 0; j < GROUP_COUNT; j++) {
				int signal = cur_snap.GetAmpOutput(j, i);
				if (signal || FUSE_ALLOWZEROSIGNAL) {
					AgentGroup& ag = sys->groups[j];
					Agent& agent = ag.agents[i];
					double res = agent.amp.GetLastResult();
					double dd = agent.amp.GetLastDrawdown();
					
					if (res > max_res && dd >= ddlimit_begin && dd < ddlimit_end) {
						max_res = res;
						group_id = j;
						group_signal = signal;
					}
				}
			}
			
			UpdateSignal(broker[ddlevel * 2 + 0], i, group_signal);
			UpdateSignal(broker[ddlevel * 2 + 1], i, -1 * group_signal);
		}
	}
	
	
	for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
		FixedSimBroker& b = broker[i];
		bool succ = b.Cycle(cur_snap);
		if (!succ) {
			b.Reset();
			for(int j = 0; j < FUSE_AVCOUNT; j++)
				triggers[i * FUSE_AVCOUNT + j].Clear();
		}
	}
	
	
	if (active_broker >= 0) {
		FixedSimBroker& src_broker = broker[active_broker];
		for(int i = 0; i < SYM_COUNT; i++)
			UpdateSignal(test_broker, i, src_broker.GetSignal(i));
	} else {
		for(int i = 0; i < SYM_COUNT; i++)
			UpdateSignal(test_broker, i, 0);
	}
	bool succ = test_broker.Cycle(cur_snap);
	if (!succ || test_broker.AccountEquity() >= 1.0e18) {
		test_broker.Reset();
		skip_learn = true;
	}
	
	
	Write(cur_snap, snaps);
	equity[cursor] = test_broker.AccountEquity();
	cursor++;
}

void AgentFuse::UpdateSignal(FixedSimBroker& broker, int sym_id, int signal) {
	if (signal == broker.GetSignal(sym_id) && signal != 0)
		broker.SetSignalFreeze(sym_id, true);
	else {
		broker.SetSignal(sym_id, signal);
		broker.SetSignalFreeze(sym_id, false);
	}
}

bool AgentFuse::IsTriggered() {
	if ((cursor - prev_trigger_cursor) > (MEASURE_PERIOD(FUSE_TRIGGERPERIOD + trigger_ma) / 3)) {
		int trigger_broker = Upp::max(0, active_broker);
		if (triggers[trigger_broker * FUSE_AVCOUNT + trigger_ma].IsTriggered())
			return true;
	}
	
	
	// Check for negative limit
	if (FUSE_TRIGGERLOSS && active_broker >= 0) {
		double change = test_broker.AccountEquity() / prev_test_equity - 1.0;
		if (change <= (-0.01 * FUSE_TRIGGERLOSS))
			return true;
	}
	
	return false;
}

void AgentFuse::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	skip_learn = false;
	
	double equity[FUSE_BROKERCOUNT];
	for(int i = 0; i < FUSE_BROKERCOUNT; i++)
		equity[i] = broker[i].AccountEquity();
	
	
	int cursor = 0;
	double input_array[FUSE_STATES];
	
	
	for(int j = 0; j < 1 + FUSE_AVCOUNT; j++) {
		double changes[FUSE_BROKERCOUNT];
		double max_change = 0.00001;
		
		
		if (j == 0) {
			for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
				double c = equity[i] / prev_equity[i] - 1.0;
				double ac = fabs(c);
				changes[i] = c;
				if (ac > max_change) max_change = ac;
			}
		} else {
			for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
				double c = triggers[i * FUSE_AVCOUNT + (j-1)].GetChange();
				double ac = fabs(c);
				changes[i] = c;
				if (ac > max_change) max_change = ac;
			}
		}
		
		
		for(int i = 0; i < FUSE_BROKERCOUNT; i++) {
			double c = changes[i];
			if (c >= 0) {
				input_array[cursor++] = 1.0 - c / max_change;
				input_array[cursor++] = 1.0;
			} else {
				input_array[cursor++] = 1.0;
				input_array[cursor++] = 1.0 + c / max_change;
			}
		}
	}
	
	ASSERT(cursor == FUSE_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < FUSE_ACTIONCOUNT);
	action_counts[action]++;
	
	trigger_ma    = action % FUSE_AVCOUNT;
	active_broker = action / FUSE_AVCOUNT;
	active_broker--;
	
	for(int i = 0; i < FUSE_BROKERCOUNT; i++)
		prev_equity[i] = equity[i];
}

void AgentFuse::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	for(int i = 0; i < SYM_COUNT; i++)
		cur_snap.SetFuseOutput(i, test_broker.GetSignal(i));
}

void AgentFuse::Backward(double reward) {
	
	// pass to brain for learning
	if (is_training && !skip_learn) {
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

int AgentFuse::GetCursor() const {
	return cursor;
}

int64 AgentFuse::GetIter() const {
	return iter;
}

bool AgentFuse::IsTrained() const {
	return iter >= FUSE_PHASE_ITER_LIMIT;
}

}
