#if 0

#include "Overlook.h"

























#if 0


AgentGroup::AgentGroup() {
	limit_factor = 0.01;
	group_input_width = 0;
	group_input_height = 0;
	mode = 0;
	enable_training = true;
	
	prev_equity = 0;
	prev_reward = 0;
	buf_count = 0;
	data_size = 0;
	signal_size = 0;
	act_iter = 0;
	main_tf = -1;
	main_tf_pos = -1;
	timeslot_tf = -1;
	timeslot_tf_pos = -1;
	current_submode = -1;
	symid_count = 0;
	timeslot_minutes = 0;
	timeslots = 0;
	prev_least_results = 0;
	random_loops = 0;
	realtime_count = 0;
	reset_optimizer = false;
	is_realtime = false;
	is_looping = false;
	sys = NULL;
	
	a0 = 0;
	t0 = 0;
	a1 = 0;
	t1 = 0;
}

AgentGroup::~AgentGroup() {
	StopGroup();
	StopAgents();
}

void AgentGroup::LoopAgentsToEnd(int submode, bool tail_only) {
	if (agents.IsEmpty()) return;
	is_looping = true;
	double prev_epsilon = agents[0].dqn.GetEpsilon();
	
	// Always 0 randomness in realtime
	if (is_realtime)
		SetEpsilon(0);
	
	for(int i = 0; i <= submode && i < tf_ids.GetCount(); i++) {
		Progress(i, submode+1, "Looping agents...");
		LoopAgentsToEndTf(i, tail_only);
	}
	
	SetEpsilon(prev_epsilon);
	is_looping = false;
}

void AgentGroup::LoopAgentsToEndTf(int tf_id, bool tail_only) {
	Vector<Agent*> agent_ptrs;
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& agent = agents[i];
		if (agent.tf_id == tf_id)
			agent_ptrs.Add(&agent);
	}
	
	if (agent_ptrs.IsEmpty()) return;
	
	Vector<Callback> mains;
	for(int i = 0; i < agent_ptrs.GetCount(); i++) {
		Agent& agent = *agent_ptrs[i];
		agent.RefreshTotalEpochs();
		if (!tail_only)
			agent.epoch_actual = 0;
		mains.Add(agent.MainCallback());
	}
	
	Agent& agent = *agent_ptrs[0];
	CoWork co;
	while (agent.epoch_actual < agent.epoch_total) {
		
		a1 = epoch_actual;
		t1 = agent.epoch_total;
		if (agent.epoch_actual % 100 == 0) {
			SubProgress(agent.epoch_actual, agent.epoch_total);
		}
		
		for(int i = 0; i < agent_ptrs.GetCount(); i++)
			co & mains[i];
		co.Finish();
		
		for(int j = 1; j < agent_ptrs.GetCount(); j++) {
			ASSERT(agent_ptrs[j]->epoch_actual == agent.epoch_actual);
		}
	}
	
	ASSERT(agent.epoch_actual == agent.epoch_total); // not epoch_actual==0 ...
}

void AgentGroup::LoopAgentsForRandomness(int submode) {
	if (agents.IsEmpty()) return;
	
	random_loops++;
	StopAgents();
	
	is_looping = true;
	
	for(int i = 0; i <= submode && i < tf_ids.GetCount(); i++) {
		LoopAgentsToEndTf(i, false);
	}
	
	is_looping = false;
	
	StartAgentsFast(submode);
}

void AgentGroup::SetMode(int i) {
	
	// Only the change of mode matters
	if (i == mode) return;
	
	
	// Stop previous mode
	watchdog.Kill();
	if      (mode == MODE_TRAINING) {
		StopAgents();
	}
	else if (mode == MODE_WEIGHTS) {
		StopGroup();
	}
	else if (mode == MODE_REAL) {
		is_realtime = false;
	}
	
	// Start new mode
	mode = i;
	if (enable_training) {
		
		// If modes cannot be started, they are ready
		
		if (mode == MODE_TRAINING) {
			int started = StartAgents(GetAgentSubMode());
			if (!started) {
				// Change to next mode
				mode++;
			}
			else {
				watchdog.Set(1000, THISBACK(CheckAgentSubMode));
			}
		}
		
		if (mode == MODE_WEIGHTS) {
			bool started = StartGroup();
			if (!started) {
				// Change to next mode
				mode++;
			}
		}
		
		if (mode == MODE_REAL) {
			is_realtime = true;
		}
	}
}

void AgentGroup::CreateAgents() {
	ASSERT(buf_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	agents.SetCount(symid_count);
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		for(int j = 0; j < tf_ids.GetCount(); j++) {
			int group_id = i + j * sym_ids.GetCount();
			const Symbol& sym = mt.GetSymbol(sym_ids[i]);
			Agent& a = agents[group_id];
			a.group = this;
			a.sym_id = i;
			a.tf_id = j;
			a.sym = sym_ids[i];
			a.tf = tf_ids[j];
			a.group_id = group_id;
			a.proxy_sym = sym.proxy_id;
			a.agent_input_width  = 1;
			a.agent_input_height = 2 + sym_ids.GetCount() * buf_count + (j+1) * (sym_ids.GetCount() * 2 * 2);
			a.has_timesteps = (sys->GetPeriod(a.tf) * sys->GetBasePeriod() / 60) < (4*60);
			a.Create(a.agent_input_width, a.agent_input_height);
		}
	}
}

void AgentGroup::Create(int width, int height) {
	/*go.SetArrayCount(width);
	go.SetCount(height);
	go.SetPopulation(100);
	go.SetMaxGenerations(50);
	
	
	int sensors = symid_count + timeslots;
	ASSERT(height == sensors);
	
	for(int i = 0; i < symid_count; i++)
		go.Set(i, 0, +10, 1, "Agent #" + IntStr(i) + " weight");
	
	for(int i = 0; i < timeslots; i++)
		go.Set(symid_count + i, 0.70, 0.99, 0.01, "Timeslot #" + IntStr(i) + " free-margin level");
	
	
	go.UseLimits();
	go.Init();*/
}

void AgentGroup::Init() {
	ASSERT(sys);
	ASSERT(!tf_ids.IsEmpty());
	
	tf_limit.SetCount(tf_ids.GetCount(), 0.07);
	
	symid_count = sym_ids.GetCount() *  tf_ids.GetCount();
	
	main_tf = -1;
	main_tf_pos = -1;
	timeslot_tf = -1;
	timeslot_tf_pos = -1;
	int main_tf_period = INT_MAX;
	int timeslot_tf_period = INT_MAX;
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int period = sys->GetPeriod(tf_ids[i]);
		int period_mins = period * sys->GetBasePeriod() / 60;
		if (period < main_tf_period) {
			main_tf_period = period;
			main_tf = tf_ids[i];
			main_tf_pos = i;
		}
		
		// Timeslots shouldn't be less than 4 hours
		if (period < timeslot_tf_period && period_mins >= (4*60)) {
			timeslot_tf_period = period;
			timeslot_tf = tf_ids[i];
			timeslot_tf_pos = i;
		}
	}
	tf_id = main_tf_pos;
	tf = main_tf;
	
	if (timeslot_tf != -1)	timeslot_minutes = timeslot_tf_period * sys->GetBasePeriod() / 60;
	else					timeslot_minutes = 4 * 60;
	int wdaymins = 5 * 24 * 60;
	timeslots = Upp::max(1, wdaymins / timeslot_minutes);
	
	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);
	
	tf_minperiods.SetCount(tf_ids.GetCount(), 1);
	tf_periods.SetCount(tf_ids.GetCount(), 1);
	tf_types.SetCount(tf_ids.GetCount(), 0);
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int p = sys->GetPeriod(tf_ids[i]);
		int minperiod = p * sys->GetBasePeriod() / 60;
		int period = p / main_tf_period;
		tf_periods[i] = period;
		tf_minperiods[i] = minperiod;
		
		if      (minperiod <  1440)	tf_types[i] = 0;
		else if (minperiod == 1440)	tf_types[i] = 1;
		else						tf_types[i] = 2;
	}
	
	
	int indi;
	indi = sys->Find<Sensors>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	
	RefreshWorkQueue();
	
	Progress(1, 6, "Processing data");
	ProcessWorkQueue();
	ProcessDataBridgeQueue();
	
	Progress(2, 6, "Finding value buffers");
	ResetValueBuffers();
	
	data_size   = symid_count * buf_count;
	signal_size = symid_count * 2;
	
	Progress(3, 6, "Reseting snapshots");
	group_input_width  = 1;
	group_input_height = symid_count + timeslots;
	RefreshSnapshots();
	
	Progress(4, 6, "Initializing group trainee");
	/*group = this;
	if (go.GetCount() <= 0) {
		Create(group_input_width, group_input_height);
	}
	TraineeBase::Init();
	
	Progress(5, 6, "Initializing agents");
	if (agents.IsEmpty()) {
		CreateAgents();
	}
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		a.group = this;
		agents[i].Init();
	}
	
	Progress(0, 1, "Complete");*/
	
	save_epoch = false;
}

void AgentGroup::Start() {
	int m = mode;
	mode = -1;
	SetMode(m);
}

bool AgentGroup::StartGroup() {
	if (main_id != -1)
		return false;
	
	// Check if optimizer has reached maximum rounds
	/*if (go.GetRound() >= go.GetMaxRounds())
		return false;*/
	
	act_iter = 0;
	main_id = sys->AddTaskBusy(THISBACK(Main));
	return true;
}

int AgentGroup::StartAgents(int submode) {
	FreezeAgents(submode);
	prev_least_results = -1;
	
	LoopAgentsToEnd(submode, false);
	
	return StartAgentsFast(submode);
}

int AgentGroup::StartAgentsFast(int submode) {
	int started = 0;
	current_submode = submode;
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		ASSERT(a.group);
		if (a.tf_id == submode) {
			a.Start();
			started++;
		}
	}
	return started;
}

void AgentGroup::FreezeAgents(int submode) {
	// Reset agent experience until submode and disable their training
	
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		ASSERT(a.group);
		if (a.tf_id >= submode) continue;
		
		a.is_training = false;
		
		//a.dqn.ClearExperience();
	}
}

void AgentGroup::Stop() {
	StopGroup();
	StopAgents();
	StoreThis();
}

void AgentGroup::StopGroup() {
	if (main_id == -1) return;
	sys->RemoveBusyTask(main_id);
	main_id = -1;
	while (at_main) Sleep(100);
}

void AgentGroup::StopAgents() {
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		a.Stop();
	}
}

void AgentGroup::Data() {
	if (last_datagather.Elapsed() < 5*60*1000)
		return;
	
	MetaTrader& mt = GetMetaTrader();
	Vector<Order> orders;
	Vector<int> signals;
	
	orders <<= mt.GetOpenOrders();
	signals <<= mt.GetSignals();
	
	mt.Data();
	
	int file_version = 1;
	double balance = mt.AccountBalance();
	double equity = mt.AccountEquity();
	Time time = mt.GetTime();
	
	FileAppend fout(ConfigFile(name + ".log"));
	int64 begin_pos = fout.GetSize();
	int size = 0;
	fout.Put(&size, sizeof(int));
	fout % file_version % balance % equity % time % signals % orders;
	int64 end_pos = fout.GetSize();
	size = end_pos - begin_pos - sizeof(int);
	fout.Seek(begin_pos);
	fout.Put(&size, sizeof(int));
	
	last_datagather.Reset();
}

void AgentGroup::Main() {
	ASSERT(!at_main);
	at_main = true;
	epoch_total = snaps.GetCount();
	if (epoch_actual >= epoch_total) {
		epoch_actual = 0;
	}
	
	if (reset_optimizer) {
		reset_optimizer = false;
		seq_results.Clear();
		Create(group_input_width, group_input_height);
	}
	/*
	if (epoch_actual == 0) {
		if (!is_realtime && !act_iter) {
			LoopAgentsToEnd(tf_ids.GetCount(), false);
		}
		prev_equity = broker.GetInitialBalance();
		prev_reward = 0.0;
		
		if (!is_realtime)
			go.Start();
	}*/
	
	
	// Do some action
	Action();
	/*
	if (!is_realtime && (end_of_epoch || broker.AccountEquity() < 0.3 * begin_equity)) {
		double energy = broker.AccountEquity() - broker.GetInitialBalance();
		go.Stop(energy);
		epoch_actual = 0;
	}
	*/
	
	// Store sequences periodically
	if ((act_iter % 1000) == 0 && last_store.Elapsed() > 60 * 60 * 1000) {
		StoreThis();
		last_store.Reset();
	}
	act_iter++;
	at_main = false;
}

void AgentGroup::Forward(Snapshot& snap, Brokerage& broker) {
	if (timeslot_minutes == -1)
		return;
	/*
	// (((wday - 1) * 24 + hour) * 60 + minute) / 'minutes in smallest period'
	int timeslot = (((snap.time_values[2] - 1) * 24 + snap.time_values[3]) * 60 + snap.time_values[4]) / timeslot_minutes;
	ASSERT(timeslot >= 0 && timeslot < timeslots);
	
	const Vector<double>& go_values = !is_realtime ?
		 go.GetTrialSolution() :
		 go.GetBestSolution();
	if (go_values.GetCount() != group_input_height) {
		DUMP(go_values.GetCount());
		DUMP(group_input_height);
	}
	ASSERT(go_values.GetCount() == group_input_height);
	
	
	fmlevel = Upp::min(0.99, Upp::max(0.70, go_values[symid_count + timeslot]));
	broker.SetFreeMarginLevel(fmlevel);
	
	symsignals.SetCount(sym_ids.GetCount());
	for(int i = 0; i < symsignals.GetCount(); i++) symsignals[i] = 0;
	
    for(int i = 0; i < agents.GetCount(); i++) {
		const Agent& a = agents[i];
		if (a.tf_id != main_tf_pos) continue;
		
		
		// Get signals from snapshots, where agents have wrote their latest signals.
		int sigpos = (a.tf_id * group->sym_ids.GetCount() + a.sym_id) * 2;
		double pos = snap.signals[sigpos + 0];
		double neg = snap.signals[sigpos + 1];
		int signal;
		if      (pos < 1.0)	signal = +1;
		else if (neg < 1.0)	signal = -1;
		else				signal =  0;
		
		
		int weight = Upp::min(10, Upp::max(0, (int)(go_values[i] + 0.5)));
		signal *= weight;
		
		symsignals[a.sym_id] += signal;
    }
	
	
	for(int i = 0; i < symsignals.GetCount(); i++) {
		int sym = sym_ids[i];
		int signal = symsignals[i];
		
		
		// Set signal to broker, but freeze it if it's same than previously.
		int prev_signal = broker.GetSignal(sym);
		if (signal != prev_signal) {
			broker.SetSignal(sym, signal);
			broker.SetSignalFreeze(sym, false);
		} else {
			broker.SetSignalFreeze(sym, signal != 0);
		}
    }*/
}

void AgentGroup::Backward(double reward) {
	double equity = broker.AccountEquity();
	double change = equity - prev_equity;
	reward = change / prev_equity;
	
	prev_reward = reward;
	
	prev_equity = equity;
	iter++;
}

void AgentGroup::SetTfLimit(int tf_id, double limit) {
	tf_limit[tf_id] = limit;
	watchdog.KillSet(1, THISBACK(CheckAgentSubMode));
}

double AgentGroup::GetTfDrawdown(int tf_id) {
	double dd = 0;
	int dd_div = 0;
	for(int i = 0; i < agents.GetCount(); i++) {
		const Agent& a = agents[i];
		if (a.tf_id != tf_id)
			continue;
		dd += a.last_drawdown;
		dd_div++;
	}
	if (!dd_div) return 1.0;
	return dd / dd_div;
}

int AgentGroup::GetAgentSubMode() {
	int submode = 0;
	
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		double dd = GetTfDrawdown(i);
		if (dd > tf_limit[i])
			break;
		submode = i+1;
	}
	
	return submode;
}

void AgentGroup::CheckAgentSubMode() {
	if (mode == MODE_TRAINING) {
		
		// Switch to train faster timeframe agents
		int submode = GetAgentSubMode();
		if (submode != current_submode) {
			StopAgents();
			
			// Remove agent-experience and store file
			FreezeAgents(submode);
			mode = submode < tf_ids.GetCount() ? MODE_TRAINING : MODE_WEIGHTS;
			StoreThis();
			
			mode = -1;
			current_submode = -1;
			SetMode(MODE_TRAINING);
		}
		
		// Randomize signals sometimes during training to not overfit single iteration
		else {
			int least_results = INT_MAX;
			for(int i = 0; i < agents.GetCount(); i++) {
				if (agents[i].tf_id == submode)
					least_results = Upp::min(least_results, agents[i].seq_results.GetCount());
			}
			if (least_results > prev_least_results+1) {
				LoopAgentsForRandomness(submode);
				prev_least_results = least_results;
			}
			
			watchdog.Set(1000, THISBACK(CheckAgentSubMode));
		}
	}
}

void AgentGroup::ResetSnapshot(Snapshot& snap) {
	ASSERT(!value_buffers.IsEmpty());
	
	// Seek to beginning
	Seek(snap, 0);
}

void AgentGroup::SetAskBid(SimBroker& sb, int pos) {
	for(int i = 0; i < databridge_cores.GetCount(); i++) {
		Core* cptr = databridge_cores[i][main_tf];
		if (!cptr) continue;
		Core& core = *cptr;
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(core.GetSymbol(), open.Get(pos));
	}
	sb.SetTime(sys->GetTimeTf(main_tf, pos));
}


#endif









#endif