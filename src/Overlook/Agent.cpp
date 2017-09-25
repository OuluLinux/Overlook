#include "Overlook.h"

namespace Overlook {



AgentFilter::AgentFilter() {
	
}

void AgentFilter::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % iter;
}

void AgentFilter::Create() {
	dqn.Reset();
	iter = 0;
	result_equity.Clear();
	result_drawdown.Clear();
	rewards.Clear();
}

void AgentFilter::ResetEpoch() {
	if (cursor > 1) {
		result_equity.Add(all_reward_sum);
		double posneg_sum = pos_reward_sum + neg_reward_sum;
		double dd = posneg_sum > 0.0 ? (neg_reward_sum / posneg_sum * 100.0) : 100.0;
		result_drawdown.Add(dd);
	}
	
	signal = 0;
	lower_output_signal = 0;
	
	timestep_actual = 0;
	timestep_total = -1;
	fwd_cursor = 0;
	cursor = 1;
	skip_learn = true;
	
	all_reward_sum = 0;
	pos_reward_sum = 0;
	neg_reward_sum = 0;
	
	change_av.Clear();
	
	// Reduce trainer sampling interval in case of very large data to get uniform sampling.
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentFilter::Main(Vector<Snapshot>& snaps) {
	ASSERT(level >= 0 && level < FILTER_COUNT);
	Snapshot& cur_snap = snaps[cursor - 0];			// Get the snapshot in current data reading position
	timestep_actual--;								// Expect main to move 1 time-step forward every time
	lower_output_signal = level == 0 ? 1 : cur_snap.GetFilterOutput(agent->group_id, level-1, agent->sym_id);
	// Do new actions when timestep_total has been moved, or when source input has zeroed.
	if (timestep_actual <= 0 || lower_output_signal == 0) {
		int sym_id = agent->sym_id;
		Snapshot& fwd_snap = snaps[fwd_cursor];		// Snapshot of previous Forward call
		Snapshot& prev_snap = snaps[cursor - 1];	// Previous snapshot is used as input also. Next is unknown.
		double reward;
		if (skip_learn) {
			reward = 0.0;
		} else {
			double change = fabs(cur_snap.GetOpen(sym_id) / fwd_snap.GetOpen(sym_id) - 1.0);
			int timesteps = cursor - fwd_cursor;
			change /= timesteps;					// Get average change per timestep
			change_av.Add(change);					// Get online average of change of value
			if (signal == 0) {
				reward = 0.0;
			} else {
				// Give reward for higher or lower than average.
				bool active_lower = agent->group_active_lower[level];
				reward = !active_lower ?
						(change - change_av.mean) * +1000.0 :
						(change - change_av.mean) * -1000.0;
				if (reward >= 0)
					pos_reward_sum += reward;
				else
					neg_reward_sum -= reward;
				all_reward_sum += reward;
			}
		}
		Backward(reward);							// Learn the reward
		Forward(cur_snap, prev_snap);				// Do new actions
		fwd_cursor = cursor;
	}
	Write(cur_snap);								// Always write sensors to the common table
	
	equity[cursor] = all_reward_sum;				// Get some equity data for visualization
	
	cursor++;										// Move 1 time-step forward also with data reading cursor
}

void AgentFilter::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	ASSERT(level >= 0 && level < FILTER_COUNT);
	int group_id = agent->group_id, sym_id = agent->sym_id, group_step = agent->group_step;
	
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		timestep_total = 1;
		signal = 0;
	}
	else {
		skip_learn = false;

		// Input values
		// - time_values
		// - input sensors
		// - previous signals
		int cursor = 0;
		double input_array[FILTER_STATES];
		
		
		// time_values
		input_array[cursor++] = cur_snap.GetYearSensor();
		input_array[cursor++] = cur_snap.GetWeekSensor();
		input_array[cursor++] = cur_snap.GetDaySensor();
		
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		// all previous outputs from the same phase
		for(int i = 0; i < FILTER_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = prev_snap.GetFilterSensorUnsafe(j, level, i);
		
		ASSERT(cursor == FILTER_STATES);
		
		
		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < FILTER_ACTIONCOUNT);
		ASSERT(FILTER_ACTIONCOUNT == (FILTER_POS_FWDSTEPS + FILTER_ZERO_FWDSTEPS));
		
		int timefwd_step;
		if (action < FILTER_POS_FWDSTEPS) {
			timefwd_step = action;
			sig_mul = +1;
		}
		else {
			timefwd_step = action - FILTER_POS_FWDSTEPS;
			sig_mul = 0;
		}
		
		timestep_total = 1 << (FILTER_FWDSTEP_BEGIN + group_step + timefwd_step + (FILTER_COUNT-1-level));
		signal = sig_mul;
		
		// Important: only 0-signal can have a random timestep. +/- signal requires alignment.
		// Reason: Signal and amp gives too short spikes because of mis-alignment.
		//         Filter +1 9-steps, at 8-step amp puts 10x, and at 9-step it's zeroed. Huge loss.
		if (agent->is_training && signal == 0)
			timestep_total += Random(RANDOM_TIMESTEPS);
		
	}
	
	timestep_actual = timestep_total;
}

void AgentFilter::Write(Snapshot& cur_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	ASSERT(level >= 0 && level < FILTER_COUNT);
	cur_snap.SetFilterOutput(group_id, level, sym_id, signal);
	
	if (timestep_actual < 0) timestep_actual = 0;
	double timestep_sensor = 0.75 - 0.75 * timestep_actual / timestep_total;
	double prev_signals[FILTER_SENSORS];
	
	if (sig_mul == 0) {
		prev_signals[0] = timestep_sensor;
		prev_signals[1] = 1.0;
	}
	else {
		prev_signals[0] = 1.0;
		prev_signals[1] = timestep_sensor;
	}
	
	for(int i = 0; i < FILTER_SENSORS; i++)
		cur_snap.SetFilterSensor(group_id, level, sym_id, i, prev_signals[i]);
}

void AgentFilter::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && !skip_learn) {
		dqn.Learn(reward);
		iter++;
		
		reward_sum += reward;
		reward_count++;
		
		if (reward_count > REWARD_AV_PERIOD) {
			rewards.Add(reward_sum / reward_count);
			reward_count = 0;
			reward_sum = 0.0;
		}
	}
}

















AgentSignal::AgentSignal() {
	
}

void AgentSignal::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % iter % deep_iter % extra_timesteps;
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
}

void AgentSignal::ResetEpoch() {
	if (broker.order_count > 0) {
		result_equity.Add(broker.equity);
		result_drawdown.Add(broker.GetDrawdown() * 100.0);
	}
	broker.Reset();
	signal = 0;
	prev_equity = broker.AccountEquity();
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
	prev_equity_cursor = 0;
	skip_learn = true;
	
	cursor_sigbegin = 0;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentSignal::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap  = snaps[cursor - 0];
	Snapshot& prev_snap = snaps[cursor - 1];
	timestep_actual--;
	lower_output_signal = cur_snap.GetFilterOutput(agent->group_id, FILTER_COUNT-1, agent->sym_id);
	if (timestep_actual <= 0 || lower_output_signal == 0) {
		if (!skip_learn) {
			broker.RefreshOrders(cur_snap);
			double equity = broker.AccountEquity();
			int timesteps = cursor - prev_equity_cursor;
			double reward = (equity / prev_equity - 1.0) * 1000.0 / timesteps;
			Backward(reward);
			if (broker.equity < 0.25 * broker.begin_equity)
				broker.Reset();
		}
		prev_equity = broker.AccountEquity();
		prev_equity_cursor = cursor;
		Forward(cur_snap, prev_snap);
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.AccountEquity();
	cursor++;
}

void AgentSignal::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id, group_step = agent->group_step;
	int prev_signal = signal;
	
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		timestep_total = 1;
		signal = 0;
	} else {
		skip_learn = false;
		
		// Input values
		// - time_values
		// - input sensors
		// - previous signals
		int cursor = 0;
		double input_array[SIGNAL_STATES];
		
		
		// time_values
		input_array[cursor++] = cur_snap.GetYearSensor();
		input_array[cursor++] = cur_snap.GetWeekSensor();
		input_array[cursor++] = cur_snap.GetDaySensor();
		
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		
		// all previous outputs from the same phase
		for(int i = 0; i < SIGNAL_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = prev_snap.GetSignalSensorUnsafe(j, i);
		
		
		ASSERT(cursor == SIGNAL_STATES);
		
		
		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < SIGNAL_ACTIONCOUNT);
		
		if (action < SIGNAL_POS_FWDSTEPS) {
			signal = +1;
			timestep_total = 1 << (SIGNAL_FWDSTEP_BEGIN + group_step + action + extra_timesteps);
		}
		else {
			signal = -1;
			action -= SIGNAL_POS_FWDSTEPS;
			timestep_total = 1 << (SIGNAL_FWDSTEP_BEGIN + group_step + action + extra_timesteps);
		}
	}
	
	
	timestep_actual = timestep_total;
	if (prev_signal != signal)
		cursor_sigbegin = cursor;
	
	
	// Set signal to broker
	broker.Cycle(signal, cur_snap);
	
	
}

void AgentSignal::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetSignalOutput(group_id, sym_id, signal);
	
	if (timestep_actual < 0) timestep_actual = 0;
	double timestep_sensor = 0.75 - 0.75 * timestep_actual / timestep_total;
	double prev_signals[SIGNAL_SENSORS];
	
	if (signal == 0) {
		prev_signals[0] = 1.0;
		prev_signals[1] = 1.0;
	}
	else if (signal > 0) {
		prev_signals[0] = timestep_sensor;
		prev_signals[1] = 1.0;
	}
	else {
		prev_signals[0] = 1.0;
		prev_signals[1] = timestep_sensor;
	}
	
	for(int i = 0; i < SIGNAL_SENSORS; i++)
		cur_snap.SetSignalSensor(group_id, sym_id, i, prev_signals[i]);
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
	
}

void AgentAmp::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % iter % deep_iter % extra_timesteps;
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
}

void AgentAmp::ResetEpoch() {
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
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
	prev_equity_cursor = 0;
	skip_learn = true;
	clean_epoch = true;
	
	cursor_sigbegin = 0;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentAmp::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap = snaps[cursor - 0];
	timestep_actual--;
	lower_output_signal = cur_snap.GetSignalOutput(agent->group_id, agent->sym_id);
	if (timestep_actual <= 0 || lower_output_signal != prev_lower_output_signal) {
		Snapshot& prev_snap = snaps[cursor - 1];
		if (!skip_learn) {
			broker.RefreshOrders(cur_snap);
			double equity = broker.AccountEquity();
			int timesteps = cursor - prev_equity_cursor;
			double reward = (equity / prev_equity - 1.0) * 1000.0 / timesteps;
			Backward(reward);
			if (equity < 0.25 * broker.begin_equity) {
				clean_epoch = false;
				broker.Reset();
			}
		}
		prev_equity = broker.AccountEquity();
		prev_equity_cursor = cursor;
		Forward(cur_snap, prev_snap);
		prev_lower_output_signal = lower_output_signal;
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.AccountEquity();
	cursor++;
}

void AgentAmp::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id, group_step = agent->group_step;
	int prev_signal = signal;
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		prev_signals[0]		= 1.0;
		prev_signals[1]		= 1.0;
		timestep_total = 1;
		signal = 0;
	}
	else {
		skip_learn = false;
		
		// Input values
		// - time_values
		// - input sensors
		// - previous signals
		int cursor = 0;
		double input_array[AMP_STATES];
		
		
		// time_values
		input_array[cursor++] = cur_snap.GetYearSensor();
		input_array[cursor++] = cur_snap.GetWeekSensor();
		input_array[cursor++] = cur_snap.GetDaySensor();
		
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		
		// all current signals from same snapshot
		for(int i = 0; i < SIGNAL_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = cur_snap.GetSignalSensorUnsafe(j, i);
		
		
		// all previous outputs from the same phase
		for(int i = 0; i < AMP_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = prev_snap.GetAmpSensorUnsafe(j, i);
		
		ASSERT(cursor == AMP_STATES);
		

		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < AMP_ACTIONCOUNT);
		
		const int maxscale_steps = AMP_MAXSCALES;
		const int timefwd_steps = AMP_FWDSTEPS;
		ASSERT(AMP_ACTIONCOUNT == (maxscale_steps * timefwd_steps));
		
		int maxscale_step	= action % maxscale_steps;
		int timefwd_step	= action / maxscale_steps;
		
		prev_signals[0]		= 1.0 * maxscale_step / maxscale_steps;
		prev_signals[1]		= 1.0 * timefwd_step  / timefwd_steps;
		
		int maxscale   = 1 + maxscale_step * AMP_MAXSCALE_MUL;
		timestep_total = 1 << (AMP_FWDSTEP_BEGIN + group_step + timefwd_step + extra_timesteps);
		
		signal = lower_output_signal * maxscale;
	}
	
	// Export value.
	timestep_actual = timestep_total;
	if (prev_signal != signal)
		cursor_sigbegin = cursor;
	
	
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
	
	if (timestep_actual < 0) timestep_actual = 0;
	prev_signals[2] = 0.75 - 0.75 * timestep_actual / timestep_total;
	
	for(int i = 0; i < AMP_SENSORS; i++)
		cur_snap.SetAmpSensor(group_id, sym_id, i, prev_signals[i]);
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

















AgentFuse::AgentFuse() {
	
}

void AgentFuse::Serialize(Stream& s) {
	s % dqn % result_equity % result_drawdown % rewards % iter % deep_iter % extra_timesteps;
}

void AgentFuse::DeepCreate() {
	deep_iter = 0;
	Create();
}

void AgentFuse::Create() {
	dqn.Reset();
	iter = 0;
	result_equity.Clear();
	result_drawdown.Clear();
	rewards.Clear();
}

void AgentFuse::ResetEpoch() {
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
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
	prev_equity_cursor = 0;
	skip_learn = true;
	clean_epoch = true;
	
	cursor_sigbegin = 0;
	
	if (prev_reset_iter > 0) {
		int iters = iter - prev_reset_iter;
		int add_exp_every = Upp::max(5, iters / dqn.GetExperienceCountMax());
		dqn.SetExperienceAddEvery(add_exp_every);
	}
	prev_reset_iter = iter;
}

void AgentFuse::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap = snaps[cursor - 0];
	timestep_actual--;
	lower_output_signal = cur_snap.GetAmpOutput(agent->group_id, agent->sym_id);
	if (timestep_actual <= 0 || lower_output_signal != prev_lower_output_signal) {
		Snapshot& prev_snap = snaps[cursor - 1];
		if (!skip_learn) {
			broker.RefreshOrders(cur_snap);
			double equity = broker.AccountEquity();
			int timesteps = cursor - prev_equity_cursor;
			double reward = (equity / prev_equity - 1.0) * 1000.0 / timesteps;
			Backward(reward);
			if (equity < 0.25 * broker.begin_equity) {
				clean_epoch = false;
				broker.Reset();
			}
		}
		prev_equity = broker.AccountEquity();
		prev_equity_cursor = cursor;
		Forward(cur_snap, prev_snap);
		prev_lower_output_signal = lower_output_signal;
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.AccountEquity();
	cursor++;
}

void AgentFuse::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id, group_step = agent->group_step;
	int prev_signal = signal;
	
	if (lower_output_signal == 0) {
		skip_learn = true;
		timestep_total = 1;
		signal = 0;
	}
	else {
		skip_learn = false;
		
		// Input values
		// - time_values
		// - input sensors
		// - previous signals
		int cursor = 0;
		double input_array[FUSE_STATES];
		
		
		// time_values
		input_array[cursor++] = cur_snap.GetYearSensor();
		input_array[cursor++] = cur_snap.GetWeekSensor();
		input_array[cursor++] = cur_snap.GetDaySensor();
		
		
		// sensors of value of current
		for(int i = 0; i < SENSOR_SIZE; i++)
			input_array[cursor++] = cur_snap.GetSensorUnsafe(i);
		
		
		// all current signals from same snapshot
		for(int i = 0; i < SIGNAL_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = cur_snap.GetSignalSensorUnsafe(j, i);
		
		
		// all current signals from same snapshot
		for(int i = 0; i < AMP_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = cur_snap.GetAmpSensorUnsafe(j, i);
		
		
		// all previous outputs from the same phase
		for(int i = 0; i < FUSE_GROUP_SIZE; i++)
			for(int j = 0; j < GROUP_COUNT; j++)
				input_array[cursor++] = prev_snap.GetFuseSensorUnsafe(j, i);
		
		ASSERT(cursor == FUSE_STATES);
		
		const int timefwd_steps = AMP_FWDSTEPS;
		
		int action = dqn.Act(input_array);
		ASSERT(action >= 0 && action < FUSE_ACTIONCOUNT);
		
		int is_enabled		= action % 2;
		int timefwd_step	= action / 2;
		
		timestep_total = 1 << (FUSE_FWDSTEP_BEGIN + group_step - 2 + timefwd_step + extra_timesteps);
		
		if (is_enabled)
			signal = lower_output_signal;
		else
			signal = 0;
	}
	
	// Export value.
	timestep_actual = timestep_total;
	if (prev_signal != signal)
		cursor_sigbegin = cursor;
	
	
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

void AgentFuse::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetFuseOutput(group_id, sym_id, signal);
	
	if (timestep_actual < 0) timestep_actual = 0;
	prev_signals[0] = 0.75 - 0.75 * timestep_actual / timestep_total;
	
	for(int i = 0; i < FUSE_SENSORS; i++)
		cur_snap.SetFuseSensor(group_id, sym_id, i, prev_signals[i]);
}

void AgentFuse::Backward(double reward) {
	
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
	for(int i = 0; i < FILTER_COUNT; i++) {
		filter[i].level = i;
		filter[i].agent = this;
	}
	sig.agent	= this;
	amp.agent	= this;
	fuse.agent	= this;
}

void Agent::CreateAll() {
	for(int i = 0; i < FILTER_COUNT; i++) {
		filter[i].Create();
	}
	sig.Create();
	amp.Create();
	fuse.Create();
}


void Agent::Init() {
	ASSERT(sym_id >= 0);
	
	group->sys->SetSingleFixedBroker(sym_id,	sig.broker);
	group->sys->SetFixedBroker(sym_id,			amp.broker);
	group->sys->SetFixedBroker(sym_id,			fuse.broker);
	
	RefreshSnapEquities();
	RefreshGroupSettings();
	ResetEpochAll();
	
	
}

void Agent::RefreshSnapEquities() {
	int snap_count = group->sys->snaps.GetCount();
	for(int i = 0; i < FILTER_COUNT; i++) {
		filter[i].equity.SetCount(snap_count, 0);
	}
	sig.equity.SetCount(snap_count, 0);
	amp.equity.SetCount(snap_count, 0);
	fuse.equity.SetCount(snap_count, 0);
}

void Agent::ResetEpochAll() {
	for(int i = 0; i < FILTER_COUNT; i++) {
		filter[i].ResetEpoch();
	}
	sig.ResetEpoch();
	amp.ResetEpoch();
	fuse.ResetEpoch();
}

void Agent::ResetEpoch(int phase) {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		sig.ResetEpoch(); break;
		case PHASE_AMP_TRAINING:		amp.ResetEpoch(); break;
		case PHASE_FUSE_TRAINING:		fuse.ResetEpoch(); break;
		default:						filter[phase].ResetEpoch(); break;
	}
}

void Agent::Main(int phase, Vector<Snapshot>& snaps) {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		sig.Main(snaps); break;
		case PHASE_AMP_TRAINING:		amp.Main(snaps); break;
		case PHASE_FUSE_TRAINING:		fuse.Main(snaps); break;
		default:						filter[phase].Main(snaps); break;
	}
}

int Agent::GetCursor(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.cursor;
		case PHASE_AMP_TRAINING:		return amp.cursor;
		case PHASE_FUSE_TRAINING:		return fuse.cursor;
		default:						return filter[phase].cursor;
	}
	return 0;
}

double Agent::GetLastDrawdown(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.GetLastDrawdown();
		case PHASE_AMP_TRAINING:		return amp.GetLastDrawdown();
		case PHASE_FUSE_TRAINING:		return fuse.GetLastDrawdown();
		default:						return filter[phase].GetLastDrawdown();
	}
	return 0;
}

double Agent::GetLastResult(int phase) const {
	ASSERT(phase >= 0 && phase < PHASE_REAL);
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.GetLastResult();
		case PHASE_AMP_TRAINING:		return amp.GetLastResult();
		case PHASE_FUSE_TRAINING:		return fuse.GetLastResult();
		default: return filter[phase].GetLastResult();
	}
	return 0;
}

int64 Agent::GetIter(int phase) const {
	switch (phase) {
		case PHASE_SIGNAL_TRAINING:		return sig.iter;
		case PHASE_AMP_TRAINING:		return amp.iter;
		case PHASE_FUSE_TRAINING:		return fuse.iter;
		default:						return filter[phase].iter;
	}
	return 0;
}

void Agent::Serialize(Stream& s) {
	for(int i = 0; i < FILTER_COUNT; i++)
		s % filter[i];
	s % sig % amp % fuse % sym_id % sym % group_id;
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
			group_step = 0;
			break;
		case 1:
			group_active_lower[0] = false;
			group_active_lower[1] = false;
			group_active_lower[2] = true;
			group_step = 0;
			break;
		case 2:
			group_active_lower[0] = false;
			group_active_lower[1] = true;
			group_active_lower[2] = false;
			group_step = 1;
			break;
		case 3:
			group_active_lower[0] = false;
			group_active_lower[1] = true;
			group_active_lower[2] = true;
			group_step = 1;
			break;
		case 4:
			group_active_lower[0] = true;
			group_active_lower[1] = false;
			group_active_lower[2] = false;
			group_step = 1;
			break;
		case 5:
			group_active_lower[0] = true;
			group_active_lower[1] = false;
			group_active_lower[2] = true;
			group_step = 1;
			break;
		case 6:
			group_active_lower[0] = true;
			group_active_lower[1] = true;
			group_active_lower[2] = false;
			group_step = 2;
			break;
		case 7:
			group_active_lower[0] = true;
			group_active_lower[1] = true;
			group_active_lower[2] = true;
			group_step = 2;
			break;
	}
}

bool Agent::IsTrained(int phase) const {
	if (phase < 0) return false;
	if ((phase == PHASE_SIGNAL_TRAINING && sig.iter >= SIGNAL_PHASE_ITER_LIMIT) ||
		(phase == PHASE_AMP_TRAINING    && amp.iter >= AMP_PHASE_ITER_LIMIT)    ||
		(phase == PHASE_FUSE_TRAINING   && fuse.iter >= FUSE_PHASE_ITER_LIMIT)   ||
		(phase <  PHASE_SIGNAL_TRAINING && filter[phase].iter >= FILTER_PHASE_ITER_LIMIT)) {
		return true;
	}
	return false;
}

}
