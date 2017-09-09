#include "Overlook.h"

namespace Overlook {

AgentSignal::AgentSignal() {
	
}

void AgentSignal::Create() {
	dqn.Reset();
	iter = 0;
}

void AgentSignal::ResetEpoch() {
	if (broker.order_count > 0) {
		result_equity.Add(broker.equity);
		result_drawdown.Add(broker.GetDrawdown());
	}
	broker.Reset();
	signal = 0;
	prev_equity = broker.AccountEquity();
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
	
	cursor_sigbegin = 0;
	for(int i = 0; i < SIGSENS_COUNT; i++)
		epoch_av[i].Clear();
}

void AgentSignal::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap  = snaps[cursor - 0];
	Snapshot& prev_snap = snaps[cursor - 1];
	timestep_actual--;
	if (timestep_actual <= 0) {
		broker.RefreshOrders(cur_snap);
		double equity = broker.AccountEquity();
		double reward = equity - prev_equity;
		reward /= timestep_total;
		Backward(reward);
		if (broker.equity < 0.25 * broker.begin_equity)
			broker.Reset();
		prev_equity = broker.equity;
		Forward(cur_snap, prev_snap);
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.AccountEquity();
	cursor++;
}

void AgentSignal::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	
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
		input_array[cursor++] = prev_snap.GetSignalSensorUnsafe(group_id, i);
	
	
	ASSERT(cursor == SIGNAL_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < SIGNAL_ACTIONCOUNT);
	
	
	// Convert action to simple signal
	int prev_signal = signal;
	if (action < SIGNAL_POS_FWDSTEPS) {
		signal = +1;
		timestep_total = 1 << (SIGNAL_FWDSTEP_BEGIN + action);
	}
	else {
		signal = -1;
		action -= SIGNAL_POS_FWDSTEPS;
		timestep_total = 1 << (SIGNAL_FWDSTEP_BEGIN + action);
	}
	
	cur_snap.SetSignalOutput(group_id, sym_id, signal);
	
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
	double prev_signals[AMP_SENSORS];
	
	if (signal == 0) {
		prev_signals[0] = timestep_sensor;
		prev_signals[1] = 1.0;
		prev_signals[2] = 1.0;
	}
	else if (signal > 0) {
		prev_signals[0] = 1.0;
		prev_signals[1] = timestep_sensor;
		prev_signals[2] = 1.0;
	}
	else {
		prev_signals[0] = 1.0;
		prev_signals[1] = 1.0;
		prev_signals[2] = timestep_sensor;
	}
	
	if (signal == 0 || cursor == cursor_sigbegin) {
		for(int i = 0; i < SIGSENS_COUNT*2; i++)
			prev_signals[3 + i] = 1.0;
	} else {
		const int periods[SIGSENS_COUNT] = SIGSENS_PERIODS;
		for(int i = 0; i < SIGSENS_COUNT; i++) {
			const Snapshot& snap  = snaps[Upp::max(cursor_sigbegin, cursor - periods[i])];
			double change  = (cur_snap.GetOpen(sym_id) / snap.GetOpen(sym_id)  - 1.0) * (1.0 / periods[i]);
			change *= signal;
			epoch_av[i].Add(fabs(change));
			double sensor = change / (epoch_av[i].mean * 3.0);
			if (sensor >= 0.0) {
				prev_signals[3 + i * 2 + 0] = 1.0 - Upp::max(0.0, Upp::min(+1.0, +sensor));
				prev_signals[3 + i * 2 + 1] = 1.0;
			} else {
				prev_signals[3 + i * 2 + 0] = 1.0;
				prev_signals[3 + i * 2 + 1] = 1.0 - Upp::max(0.0, Upp::min(+1.0, -sensor));
			}
		}
	}
	
	
	for(int i = 0; i < AMP_SENSORS; i++)
		cur_snap.SetSignalSensor(group_id, sym_id, i, prev_signals[i]);
}

void AgentSignal::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && timestep_total > 0)
		dqn.Learn(reward);
	
	reward_sum += reward;
	
	if (iter % 50 == 0) {
		average_reward = reward_sum / 50;
		reward_sum = 0;
	}
	
	iter++;
}

















AgentAmp::AgentAmp() {
	
}

void AgentAmp::Create() {
	dqn.Reset();
	iter = 0;
}

void AgentAmp::ResetEpoch() {
	if (broker.order_count > 0) {
		result_equity.Add(broker.equity);
		result_drawdown.Add(broker.GetDrawdown());
	}
	broker.Reset();
	signal = 0;
	prev_input_signal = 0;
	prev_equity = broker.PartialEquity();
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
	
	cursor_sigbegin = 0;
	for(int i = 0; i < SIGSENS_COUNT; i++)
		epoch_av[i].Clear();
}

void AgentAmp::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap = snaps[cursor - 0];
	timestep_actual--;
	if (timestep_actual <= 0 || cur_snap.GetSignalOutput(agent->group_id, agent->sym_id) != prev_input_signal) {
		Snapshot& prev_snap = snaps[cursor - 1];
		broker.RefreshOrders(cur_snap);
		double equity = broker.PartialEquity();
		double reward = equity - prev_equity;
		reward /= timestep_total;
		reward /= broker.equity / broker.begin_equity;
		Backward(reward);
		if (broker.equity < 0.25 * broker.begin_equity)
			broker.Reset();
		prev_equity = broker.PartialEquity();
		Forward(cur_snap, prev_snap);
	}
	Write(cur_snap, snaps);
	equity[cursor] = broker.PartialEquity();
	cursor++;
}

void AgentAmp::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	
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
		input_array[cursor++] = cur_snap.GetSignalSensorUnsafe(group_id, i);
	
	
	// all previous outputs from the same phase
	for(int i = 0; i < AMP_GROUP_SIZE; i++)
		input_array[cursor++] = prev_snap.GetAmpSensorUnsafe(group_id, i);
	
	ASSERT(cursor == AMP_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < AMP_ACTIONCOUNT);
	
	
	const int maxscale_steps = AMP_MAXSCALES;
	const int timefwd_steps = AMP_FWDSTEPS;
	ASSERT(AMP_ACTIONCOUNT == (maxscale_steps * timefwd_steps));
	
	int maxscale_step	= action % maxscale_steps;			action /= maxscale_steps;
	int timefwd_step	= action % timefwd_steps;			action /= timefwd_steps;
	
	prev_signals[0]		= 1.0 * maxscale_step / maxscale_steps;
	prev_signals[1]		= 1.0 * timefwd_step  / timefwd_steps;
	
	int prev_signal		= signal;
	int maxscale		= 1 + maxscale_step * 2;
	timestep_total		= 1 << (timefwd_step + AMP_FWDSTEP_BEGIN);
	prev_input_signal	= cur_snap.GetSignalOutput(group_id, sym_id);
	
	if (prev_input_signal == 0) {
		timestep_total = 1;
		signal = 0;
	}
	else {
		signal = prev_input_signal * maxscale;
	}
	
	// Export value.
	cur_snap.SetAmpOutput(group_id, sym_id, signal);
	timestep_actual = timestep_total;
	if (prev_signal != signal)
		cursor_sigbegin = cursor;
	
	
	// Following accepts previous iteration values.
	// Rest of the function does NOT affect the exported value.
	// Copying signals back from snapshot does NOT affect realtime quality.
	// Invalid accumulation in the broker does NOT affect following forward calls in the forward-only mode.
	// This is not skipped in the forward-only mode, because the equity graph is being drawn for visualization.
	// Broker signals may NOT be read in realtime, because it can be reseted in a unsuccessful try.
	
	for(int i = 0; i < SYM_COUNT; i++)
		broker.SetSignal(i, cur_snap.GetAmpOutput(group_id, i));
	
	bool succ = broker.Cycle(cur_snap);
	
	if (!succ)
		broker.Reset();
}

void AgentAmp::Write(Snapshot& cur_snap, const Vector<Snapshot>& snaps) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetAmpOutput(group_id, sym_id, signal);
	
	if (timestep_actual < 0) timestep_actual = 0;
	prev_signals[2] = 0.75 - 0.75 * timestep_actual / timestep_total;
	
	if (signal == 0 || cursor == cursor_sigbegin) {
		for(int i = 0; i < SIGSENS_COUNT*2; i++)
			prev_signals[3 + i] = 1.0;
	} else {
		const int periods[SIGSENS_COUNT] = SIGSENS_PERIODS;
		for(int i = 0; i < SIGSENS_COUNT; i++) {
			const Snapshot& snap  = snaps[Upp::max(cursor_sigbegin, cursor - periods[i])];
			double change  = (cur_snap.GetOpen(sym_id) / snap.GetOpen(sym_id)  - 1.0) * (1.0 / periods[i]);
			change *= signal;
			epoch_av[i].Add(fabs(change));
			double sensor = change / (epoch_av[i].mean * 3.0);
			if (sensor >= 0.0) {
				prev_signals[3 + i * 2 + 0] = 1.0 - Upp::max(0.0, Upp::min(+1.0, +sensor));
				prev_signals[3 + i * 2 + 1] = 1.0;
			} else {
				prev_signals[3 + i * 2 + 0] = 1.0;
				prev_signals[3 + i * 2 + 1] = 1.0 - Upp::max(0.0, Upp::min(+1.0, -sensor));
			}
		}
	}
	
	
	for(int i = 0; i < AMP_SENSORS; i++)
		cur_snap.SetAmpSensor(group_id, sym_id, i, prev_signals[i]);
}

void AgentAmp::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && timestep_total > 0)
		dqn.Learn(reward);
	
	reward_sum += reward;
	
	if (iter % 50 == 0) {
		average_reward = reward_sum / 50;
		reward_sum = 0;
	}
	
	iter++;
}

















AgentFuse::AgentFuse() {
	
}

void AgentFuse::Create() {
	dqn.Reset();
	iter = 0;
}

void AgentFuse::ResetEpoch() {
	if (broker.order_count > 0) {
		result_equity.Add(broker.equity);
		result_drawdown.Add(broker.GetDrawdown());
	}
	broker.Reset();
	signal = 0;
	prev_input_signal = 0;
	prev_equity = broker.AccountEquity();
	
	timestep_actual = 0;
	timestep_total = -1;
	cursor = 1;
}

void AgentFuse::Main(Vector<Snapshot>& snaps) {
	Snapshot& cur_snap = snaps[cursor - 0];
	timestep_actual--;
	if (timestep_actual <= 0 || cur_snap.GetAmpOutput(agent->group_id, agent->sym_id) != prev_input_signal) {
		Snapshot& prev_snap = snaps[cursor - 1];
		broker.RefreshOrders(cur_snap);
		double equity = broker.PartialEquity();
		double reward = equity - prev_equity;
		reward /= timestep_total;
		reward /= broker.equity / broker.begin_equity;
		Backward(reward);
		if (broker.equity < 0.25 * broker.begin_equity)
			broker.Reset();
		prev_equity = broker.PartialEquity();
		Forward(cur_snap, prev_snap);
	}
	Write(cur_snap);
	equity[cursor] = broker.PartialEquity();
	cursor++;
}

void AgentFuse::Forward(Snapshot& cur_snap, Snapshot& prev_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	
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
		input_array[cursor++] = cur_snap.GetSignalSensorUnsafe(group_id, i);
	
	
	// all current amp-signals from same snapshot
	for(int i = 0; i < AMP_GROUP_SIZE; i++)
		input_array[cursor++] = cur_snap.GetAmpSensorUnsafe(group_id, i);
	
	
	// all previous outputs from the same phase
	for(int i = 0; i < FUSE_GROUP_SIZE; i++)
		input_array[cursor++] = prev_snap.GetFuseSensorUnsafe(group_id, i);
	
	ASSERT(cursor == FUSE_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < FUSE_ACTIONCOUNT);
	ASSERT(FUSE_ACTIONCOUNT == (FUSE_POS_FWDSTEPS + FUSE_ZERO_FWDSTEPS + FUSE_NEG_FWDSTEPS));
	
	int timefwd_step;
	if (action < FUSE_POS_FWDSTEPS) {
		timefwd_step = action;
		sig_mul = +1;
	}
	else if (action < (FUSE_POS_FWDSTEPS + FUSE_ZERO_FWDSTEPS)) {
		timefwd_step = action - FUSE_POS_FWDSTEPS;
		sig_mul = 0;
	}
	else {
		timefwd_step = action - (FUSE_POS_FWDSTEPS + FUSE_ZERO_FWDSTEPS);
		sig_mul = -1;
	}
	
	
	timestep_total		= 1 << (timefwd_step + FUSE_FWDSTEP_BEGIN);
	prev_input_signal	= cur_snap.GetAmpOutput(group_id, sym_id);
	
	if (prev_input_signal == 0) {
		timestep_total = 1;
		signal = 0;
	}
	else {
		signal = prev_input_signal * sig_mul;
	}
	
	// Export value.
	cur_snap.SetFuseOutput(group_id, sym_id, signal);
	timestep_actual = timestep_total;
	
	
	// Following accepts previous iteration values.
	// Rest of the function does NOT affect the exported value.
	// Copying signals back from snapshot does NOT affect realtime quality.
	// Invalid accumulation in the broker does NOT affect following forward calls in the forward-only mode.
	// This is not skipped in the forward-only mode, because the equity graph is being drawn for visualization.
	// Broker signals may NOT be read in realtime, because it can be reseted in a unsuccessful try.
	
	for(int i = 0; i < SYM_COUNT; i++)
		broker.SetSignal(i, cur_snap.GetFuseOutput(group_id, i));
	
	bool succ = broker.Cycle(cur_snap);
	
	if (!succ)
		broker.Reset();
}

void AgentFuse::Write(Snapshot& cur_snap) {
	int group_id = agent->group_id, sym_id = agent->sym_id;
	cur_snap.SetFuseOutput(group_id, sym_id, signal);
	
	if (timestep_actual < 0) timestep_actual = 0;
	double timestep_sensor = 0.75 - 0.75 * timestep_actual / timestep_total;;
	double prev_signals[FUSE_SENSORS];
	
	if (sig_mul == 0) {
		prev_signals[0] = timestep_sensor;
		prev_signals[1] = 1.0;
		prev_signals[2] = 1.0;
	}
	else if (sig_mul == 1) {
		prev_signals[0] = 1.0;
		prev_signals[1] = timestep_sensor;
		prev_signals[2] = 1.0;
	}
	else {
		prev_signals[0] = 1.0;
		prev_signals[1] = 1.0;
		prev_signals[2] = timestep_sensor;
	}
	
	for(int i = 0; i < FUSE_SENSORS; i++)
		cur_snap.SetFuseSensor(group_id, sym_id, i, prev_signals[i]);
}

void AgentFuse::Backward(double reward) {
	
	// pass to brain for learning
	if (agent->is_training && timestep_total > 0)
		dqn.Learn(reward);
	
	reward_sum += reward;
	
	if (iter % 50 == 0) {
		average_reward = reward_sum / 50;
		reward_sum = 0;
	}
	
	iter++;
}

















Agent::Agent() {
	sig.agent	= this;
	amp.agent	= this;
	fuse.agent	= this;
}

void Agent::CreateAll() {
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
	
	ResetEpochAll();
}

void Agent::RefreshSnapEquities() {
	int snap_count = group->sys->snaps.GetCount();
	sig.equity.SetCount(snap_count, 0);
	amp.equity.SetCount(snap_count, 0);
	fuse.equity.SetCount(snap_count, 0);
}

void Agent::ResetEpochAll() {
	sig.ResetEpoch();
	amp.ResetEpoch();
	fuse.ResetEpoch();
}

void Agent::ResetEpoch(int phase) {
	switch (phase) {
		case 0: sig.ResetEpoch(); break;
		case 1: amp.ResetEpoch(); break;
		case 2: fuse.ResetEpoch(); break;
	}
}

void Agent::Main(int phase, Vector<Snapshot>& snaps) {
	switch (phase) {
		case 0: sig.Main(snaps); break;
		case 1: amp.Main(snaps); break;
		case 2: fuse.Main(snaps); break;
	}
}

int Agent::GetCursor(int phase) const {
	switch (phase) {
		case 0: return sig.cursor;
		case 1: return amp.cursor;
		case 2: return fuse.cursor;
	}
	return 0;
}

double Agent::GetLastDrawdown(int phase) const {
	switch (phase) {
		case 0: return sig.GetLastDrawdown();
		case 1: return amp.GetLastDrawdown();
		case 2: return fuse.GetLastDrawdown();
	}
	return 0;
}

double Agent::GetLastResult(int phase) const {
	switch (phase) {
		case 0: return sig.GetLastResult();
		case 1: return amp.GetLastResult();
		case 2: return fuse.GetLastResult();
	}
	return 0;
}

void Agent::Serialize(Stream& s) {
	s % sig % amp % fuse % sym_id % sym % group_id;
}

}
