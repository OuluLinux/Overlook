#include "Overlook.h"

namespace Overlook {

AgentGroup::AgentGroup() {
	sys = NULL;
	group_input_height = 0;
	data_size = 0;
	signal_size = 0;
	act_iter = 0;
	mode = 0;
	main_tf = -1;
	main_tf_pos = -1;
	symid_count = 0;
	
	limit_factor = 0.01;
	fmlevel = 0.90;
	buf_count = 0;
	enable_training = true;
	sig_freeze = true;
	reset_optimizer = false;
	accum_signal = false;
	allow_realtime = false;
	is_looping = false;
}

AgentGroup::~AgentGroup() {
	Stop();
}

bool AgentGroup::PutLatest(Brokerage& broker) {
	if (!allow_realtime) return false;
	
	WhenInfo("Refreshing snapshots");
	RefreshSnapshots();
	
	Time time = GetMetaTrader().GetTime();
	int shift = sys->GetShiftFromTimeTf(time, main_tf);
	if (shift != train_pos_all.Top()) {
		WhenError(Format("Current shift doesn't match the lastest snapshot shift (%d != %d)", shift, train_pos_all.Top()));
		return false;
	}
	
	WhenInfo("Looping agents until latest snapshot");
	LoopAgentsToEnd();
	Snapshot& shift_snap = snaps[train_pos_all.GetCount()-1];
	
	// Set probability for random actions to 0
	Forward(shift_snap, broker);
	
	WhenInfo("Refreshing broker data");
	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);
	if (mt) {
		mt->Data();
	} else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}
	broker.SetLimitFactor(limit_factor);
	
	WhenInfo("Updating orders");
	broker.SignalOrders(true);
	broker.RefreshLimits();
	
	return true;
}

void AgentGroup::LoopAgentsToEnd() {
	if (agents.IsEmpty()) return;
	is_looping = true;
	double prev_epsilon = agents[0].dqn.GetEpsilon();
	SetEpsilon(0);
	CoWork co;
	co.SetPoolSize(Upp::max(1, CPU_Cores() - 2));
	for(int i = 0; i < agents.GetCount(); i++) {
		co & THISBACK1(LoopAgentToEnd, i);
	}
	co.Finish();
	SetEpsilon(prev_epsilon);
	is_looping = false;
}

void AgentGroup::LoopAgentToEnd(int i) {
	Agent& agent = agents[i];
	agent.RefreshTotalEpochs();
	agent.epoch_actual = 0;
	while (agent.epoch_actual < agent.epoch_total) {
		agent.Main();
	}
	ASSERT(agent.epoch_actual == agent.epoch_total); // not epoch_actual==0 ...
}

void AgentGroup::Progress(int actual, int total, String desc) {
	a0 = actual;
	t0 = total;
	prog_desc = desc;
	WhenProgress(actual, total, desc);
	SubProgress(0, 1);
}

void AgentGroup::SubProgress(int actual, int total) {
	a1 = actual;
	t1 = total;
	WhenSubProgress(actual, total);
}

void AgentGroup::SetEpsilon(double d) {
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].dqn.SetEpsilon(d);
}

void AgentGroup::SetMode(int i) {
	
	// Only the change of mode matters
	if (i == mode) return;
	
	
	// Stop previous mode
	if      (mode == 0) {
		StopAgents();
	}
	else if (mode == 1) {
		StopGroup();
	}
	else if (mode == 2) {
		allow_realtime = false;
	}
	
	// Start new mode
	mode = i;
	if (enable_training) {
		if      (mode == 0) {
			StartAgents();
		}
		else if (mode == 1) {
			StartGroup();
		}
		else if (mode == 2) {
			allow_realtime = true;
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
			if (!j)		a.agent_input_height = 1 +     sym_ids.GetCount() * buf_count + sym_ids.GetCount() * 2 * 2;
			else		a.agent_input_height = 1 + 2 * sym_ids.GetCount() * buf_count + sym_ids.GetCount() * 2 * 2 * 2;
			a.Create(a.agent_input_width, a.agent_input_height);
		}
	}
}

void AgentGroup::Create(int width, int height) {
	go.SetArrayCount(width);
	go.SetCount(height);
	go.SetPopulation(100);
	go.SetMaxGenerations(1000);
	
	
	int sensors = symid_count + timeslots;
	ASSERT(height == sensors);
	
	for(int i = 0; i < symid_count; i++)
		go.Set(i, 0, +10, 1, "Agent #" + IntStr(i) + " weight");
	
	for(int i = 0; i < timeslots; i++)
		go.Set(symid_count + i, 0.70, 0.99, 0.01, "Timeslot #" + IntStr(i) + " free-margin level");
	
	
	go.UseLimits();
	go.Init();
}

void AgentGroup::Init() {
	ASSERT(sys);
	ASSERT(!tf_ids.IsEmpty());
	
	symid_count = sym_ids.GetCount() *  tf_ids.GetCount();
	
	main_tf = -1;
	main_tf_pos = -1;
	int main_tf_period = INT_MAX;
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int period = sys->GetPeriod(tf_ids[i]);
		if (period < main_tf_period) {
			main_tf_period = period;
			main_tf = tf_ids[i];
			main_tf_pos = i;
		}
	}
	tf_id = main_tf_pos;
	tf = main_tf;
	
	fastest_period_mins = sys->GetPeriod(main_tf) * sys->GetBasePeriod() / 60;
	int wdaymins = 5 * 24 * 60;
	timeslots = Upp::max(1, wdaymins / fastest_period_mins);
	
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
	
	data_size = 1 + symid_count * buf_count;
	signal_size = sym_ids.GetCount() * 2 * 2 * tf_ids.GetCount();
	total_size = data_size + signal_size;
	ASSERT(total_size > 0 && total_size < 200000);
	
	Progress(3, 6, "Reseting snapshots");
	RefreshSnapshots();
	
	group_input_width  = 1;
	group_input_height = symid_count + timeslots;
	
	Progress(4, 6, "Initializing group trainee");
	group = this;
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
	
	Progress(0, 1, "Complete");
	
	save_epoch = false;
}

void AgentGroup::Start() {
	int m = mode;
	mode = -1;
	SetMode(m);
}

void AgentGroup::StartGroup() {
	if (main_id != -1) return;
	act_iter = 0;
	main_id = sys->AddTaskBusy(THISBACK(Main));
}

void AgentGroup::StartAgents() {
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		ASSERT(a.group);
		a.Start();
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
	Array<Order> orders;
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
		data_looped_once = true;
		epoch_actual = 0;
	}
	
	if (reset_optimizer) {
		reset_optimizer = false;
		seq_results.Clear();
		Create(group_input_width, group_input_height);
	}
	
	if (epoch_actual == 0) {
		if (!allow_realtime) {
			
			// Loop agents to the end if program boots directly to group optimizer.
			bool all_looped_once = true;
			for(int i = 0; i < agents.GetCount(); i++)
				if (!agents[i].data_looped_once)
					all_looped_once = false;
			if (!all_looped_once)
				LoopAgentsToEnd();
			
		}
		prev_equity = broker.GetInitialBalance();
		prev_reward = 0.0;
		
		if (!allow_realtime)
			go.Start();
	}
	
	
	// Do some action
	Action();
	
	if (!allow_realtime && (end_of_epoch || broker.AccountEquity() < 0.3 * begin_equity)) {
		double energy = broker.AccountEquity() - broker.GetInitialBalance();
		go.Stop(energy);
		epoch_actual = 0;
	}
	
	
	// Store sequences periodically
	if ((act_iter % 1000) == 0 && last_store.Elapsed() > 60 * 60 * 1000) {
		StoreThis();
		last_store.Reset();
	}
	act_iter++;
	at_main = false;
}

void AgentGroup::Forward(Snapshot& snap, Brokerage& broker) {
	
	int timeslot = (((snap.time_values[2] - 1) * 24 + snap.time_values[3]) * 60 + snap.time_values[4]) / fastest_period_mins;
	ASSERT(timeslot >= 0 && timeslot < timeslots);
	
	const Vector<double>& go_values = !allow_realtime ?
		 go.GetTrialSolution() :
		 go.GetBestSolution();
	ASSERT(go_values.GetCount() == group_input_height);
	
	
	fmlevel = Upp::min(0.99, Upp::max(0.70, go_values[symid_count + timeslot]));
	broker.SetFreeMarginLevel(fmlevel);
	
	symsignals.SetCount(sym_ids.GetCount());
	for(int i = 0; i < symsignals.GetCount(); i++) symsignals[i] = 0;
	
    for(int i = 0; i < agents.GetCount(); i++) {
		int sym_id = i % sym_ids.GetCount();
		int signal;
		const Agent& a = agents[i];
		
		
		// Very minimum requirement: positive drawdown
		#ifndef flagDEBUG
		if (a.last_drawdown >= 0.50)
			signal = 0;
		else
		#endif
		{
		    // Get signals from snapshots, where agents have wrote their latest signals.
		    signal = snap.signals[i];
		}
		
		
		int weight = Upp::min(10, Upp::max(0, (int)(go_values[i] + 0.5)));
		signal *= weight;
		
		symsignals[sym_id] += signal;
    }
		
	
	for(int i = 0; i < symsignals.GetCount(); i++) {
		int sym = sym_ids[i];
		int signal = symsignals[i];
		
		// Set signal to broker
		if (!sig_freeze) {
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
}

void AgentGroup::Backward(double reward) {
	double equity = broker.AccountEquity();
	double change = equity - prev_equity;
	reward = change / prev_equity;
	
	prev_reward = reward;
	
	prev_equity = equity;
	iter++;
}

void AgentGroup::StoreThis() {
	ASSERT(!name.IsEmpty());
	Time t = GetSysTime();
	String file = ConfigFile(name + ".agrp");
	bool rem_bak = false;
	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}
	StoreToFile(*this,	file);
	if (rem_bak)
		DeleteFile(file + ".bak");
}

void AgentGroup::LoadThis() {
	LoadFromFile(*this,	ConfigFile(name + ".agrp"));
}

void AgentGroup::Serialize(Stream& s) {
	TraineeBase::Serialize(s);
	s % go % agents % tf_ids % sym_ids % created % name % param_str
	  % fmlevel % limit_factor
	  % group_input_width % group_input_height
	  % mode % sig_freeze
	  % enable_training % accum_signal;
}

int AgentGroup::GetSignalBegin() const {
	return data_size;
}

int AgentGroup::GetSignalEnd() const {
	return total_size;
}

int AgentGroup::GetSignalPos(int group_id) const {
	ASSERT(group_id >= 0 && group_id <= symid_count);
	return data_size + group_id * 2 * 2;
}

void AgentGroup::RefreshSnapshots() {
	ASSERT(buf_count != 0);
	ASSERT(tf_ids.GetCount() != 0);
	ASSERT(sym_ids.GetCount() != 0);
	ASSERT(group_input_width != 0);
	
	TimeStop ts;
	ProcessWorkQueue();
	
	bool init = train_pos_all.IsEmpty();
	int pos = init ? 0 : train_pos_all.Top()+1;
	int bars = sys->GetCountTf(main_tf);
	int prev_pos_count = train_pos_all.GetCount();
	int agent_count = symid_count;
	
	train_pos_all.Reserve(bars - pos);
	
	int tf_mins = sys->GetPeriod(main_tf) * sys->GetBasePeriod() / 60;
	int tf_type;
	if (tf_mins < 24*60) tf_type = 0;
	else if (tf_mins == 24*60) tf_type = 1;
	else tf_type = 2;
	
	if (init) {
		// Add slower positions (from main_tf begin to backwards)
		for(int i = tf_ids.GetCount()-2; i >= 0; i--) {
			int tf_id = tf_ids[i];
			int prev_tf_id = tf_ids[i+1];
			int tf_bars = sys->GetCountTf(tf_id);
			int tf_end = sys->GetShiftTf(prev_tf_id, tf_id, 0);
			ASSERT(tf_end <= tf_bars && tf_end >= 0);
			for (int j = tf_end-1; j >= 0; j--) {
				// Position in main_tf
				int pos = sys->GetShiftTf(tf_id, main_tf, j);
				train_pos_all.Add(pos);
			}
		}
		Sort(train_pos_all, StdLess<int>());
	}
	
	for(int i = pos; i < bars; i++) {
		train_pos_all.Add(i);
	}
	
	for(int i = prev_pos_count; i < train_pos_all.GetCount(); i++) {
		#define SKIP {train_pos_all.Remove(i); i--; continue;}
		
		if (i) {ASSERT(train_pos_all[i-1] < train_pos_all[i]);}
		
		int pos = train_pos_all[i];
		int tf_id_count = tf_ids.GetCount();
		if (pos < 0) {
			// First tf_ids item is least likely to have negative pos due to longer time range
			tf_id_count = 0;
			for(int i = 0; i < tf_ids.GetCount(); i++) {
				int tf_pos = sys->GetShiftTf(main_tf, tf_ids[i], pos);
				if (tf_pos < 0)
					break;
				tf_id_count = i+1;
			}
		}
		
		
		Time t = sys->GetTimeTf(main_tf, pos);
		int wday = DayOfWeek(t);
		
		#undef SKIP
	}
	
	for(int i = prev_pos_count; i < train_pos_all.GetCount(); i++) {
		int pos = train_pos_all[i];
		if (i) {ASSERT(train_pos_all[i] > train_pos_all[i-1]);}
		
		One<Snapshot> snap;
		snap.Create();
		
		Seek(*snap, pos);
		
		// Remove those snapshots which aren't used at all for better perforfmance
		if (snap->tfs_used == 0) {
			train_pos_all.Remove(i);
			i--;
			continue;
		}
		
		snap->id = i;
		
		snaps.Add(snap.Detach());
	}
	
	LOG("Refreshing snapshots took " << ts.ToString());
}

void AgentGroup::ResetSnapshot(Snapshot& snap) {
	ASSERT(!value_buffers.IsEmpty());
	
	// Seek to beginning
	Seek(snap, 0);
}

bool AgentGroup::Seek(Snapshot& snap, int shift) {
	
	// Reserve memory for values
	ASSERT(total_size > 0 && total_size < 200000);
	snap.values.SetCount(total_size, 0.0);
	snap.time_values.SetCount(5);
	
	
	// Check that shift is not too much
	int bars = sys->GetCountTf(main_tf);
	if (shift >= bars)
		return false;
	
	
	// Get some time values in binary format (starts from 0)
	Time t = sys->GetTimeTf(main_tf, shift);
	int month = t.month-1;
	int day = t.day-1;
	int hour = t.hour;
	int minute = t.minute;
	int wday = DayOfWeek(t);
	snap.time_values[0] = month;
	snap.time_values[1] = day;
	snap.time_values[2] = wday;
	snap.time_values[3] = hour;
	snap.time_values[4] = minute;
	snap.time = t;
	snap.added = GetSysTime();
	snap.shift = shift;
	
	snap.signals.SetCount(symid_count, 0);
	
	// Time sensor
	int vpos = 0;
	double time_sensor = ((wday * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0);
	snap.values[vpos++] = time_sensor;
	
	
	// Refresh values (tf / sym / value)
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int pos = sys->GetShiftTf(main_tf, tf_ids[i], shift);
		if (pos >= 0) {
			for(int j = 0; j < sym_ids.GetCount(); j++) {
				Vector<ConstBuffer*>& indi_buffers = value_buffers[j][i];
				for(int k = 0; k < buf_count; k++) {
					ConstBuffer& src = *indi_buffers[k];
					double d = src.GetUnsafe(pos);
					snap.values[vpos + (i * sym_ids.GetCount() + j) * buf_count + k] = d;
				}
			}
		}
		else {
			for(int j = 0; j < sym_ids.GetCount(); j++)
				for(int k = 0; k < buf_count; k++)
					snap.values[vpos + (i * sym_ids.GetCount() + j) * buf_count + k] = 0.0;
		}
	}
	
	
	snap.tfmask = 0;
	snap.tfs_used = 0;
	
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int tf_type = tf_types[i];
		
		if      (tf_type == 0 && wday > 0 && wday < 6 && ((hour * 60 + minute) % tf_minperiods[i]) == 0) {
			int tf_shift = sys->GetShiftTf(main_tf, tf_ids[i], shift);
			if (tf_shift >= 0) {
				snap.tfs_used++;
				snap.SetActive(i);
			}
		}
		else if (tf_type == 1 && wday > 0 && wday < 6 && hour == 0 && minute == 0) {
			int tf_shift = sys->GetShiftTf(main_tf, tf_ids[i], shift);
			if (tf_shift >= 0) {
				snap.tfs_used++;
				snap.SetActive(i);
			}
		}
		else if (tf_type == 2 && wday == 0 && hour == 0 && minute == 0) {
			int tf_shift = sys->GetShiftTf(main_tf, tf_ids[i], shift);
			if (tf_shift >= 0) {
				snap.tfs_used++;
				snap.SetActive(i);
			}
		}
	}
	
	
	return true;
}

void AgentGroup::RefreshWorkQueue() {
	Index<int> sym_ids;
	sym_ids <<= this->sym_ids;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[i]);
		if (sym.proxy_id == -1) continue;
		sym_ids.FindAdd(sym.proxy_id);
	}
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	// Get DataBridge work queue
	Index<int> db_indi_ids;
	db_indi_ids.Add(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, tf_ids, db_indi_ids);
}

void AgentGroup::ResetValueBuffers() {
	// Find value buffer ids
	VectorMap<int, int> bufout_ids;
	int buf_id = 0;
	for(int i = 0; i < indi_ids.GetCount(); i++) {
		bufout_ids.Add(indi_ids[i], buf_id);
		int indi = indi_ids[i];
		const FactoryRegister& reg = sys->GetRegs()[indi];
		buf_id += reg.out[0].visible;
	}
	buf_count = buf_id;
	ASSERT(buf_count);
	
	
	// Get DataBridge core pointer for easy reading
	databridge_cores.Clear();
	databridge_cores.SetCount(sys->GetSymbolCount());
	for(int i = 0; i < databridge_cores.GetCount(); i++)
		databridge_cores[i].SetCount(sys->GetPeriodCount(), NULL);
	int factory = sys->Find<DataBridge>();
	for(int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];
		if (ci.factory != factory) continue;
		databridge_cores[ci.sym][ci.tf] = &*ci.core;
	}
	
	
	// Reserve memory for value buffer vector
	value_buffers.Clear();
	value_buffers.SetCount(sym_ids.GetCount());
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		value_buffers[i].SetCount(tf_ids.GetCount());
		for(int j = 0; j < value_buffers[i].GetCount(); j++)
			value_buffers[i][j].SetCount(buf_count, NULL);
	}
	
	
	// Get value buffers
	int total_bufs = 0;
	data_begins.SetCount(tf_ids.GetCount(), 0);
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.GetOutput(0);
		
		int sym_id = sym_ids.Find(ci.sym);
		if (sym_id == -1) continue;
		int tf_id = tf_ids.Find(ci.tf);
		if (tf_id == -1) continue;
		
		DataBridge* db = dynamic_cast<DataBridge*>(&*ci.core);
		if (db) data_begins[tf_id] = Upp::max(data_begins[tf_id], db->GetDataBegin());
		
		Vector<ConstBuffer*>& indi_buffers = value_buffers[sym_id][tf_id];
		
		const FactoryRegister& reg = sys->GetRegs()[ci.factory];
		int buf_begin = bufout_ids.Find(ci.factory);
		if (buf_begin == -1) continue;
		buf_begin = bufout_ids[buf_begin];
		
		for (int l = 0; l < reg.out[0].visible; l++) {
			int buf_pos = buf_begin + l;
			ConstBuffer*& bufptr = indi_buffers[buf_pos];
			ASSERT_(bufptr == NULL, "Duplicate work item");
			bufptr = &output.buffers[l];
			total_bufs++;
		}
	}
	int expected_total = symid_count * buf_count;
	ASSERT_(total_bufs == expected_total, "Some items are missing in the work queue");
	
	
	data_begin = data_begins[main_tf_pos];
	
	
	
	//int bars = sys->GetCountTf(main_tf);
	//int group_size = symid_count;
	
	
	
	/*
	#ifdef flagDEBUG
	pos = Upp::max(pos, bars-1000);
	#endif
	
	train_pos_all.Reserve(bars - pos);
	train_pos.SetCount(group_size);
	for(int i = 0; i < train_pos.GetCount(); i++)
		train_pos[i].Reserve(bars - pos);
	
	int tf_mins = sys->GetPeriod(main_tf) * sys->GetBasePeriod() / 60;
	int tf_type;
	if (tf_mins < 24*60) tf_type = 0;
	else if (tf_mins == 24*60) tf_type = 1;
	else tf_type = 2;
	
	for(int i = pos; i < bars; i++) {
		Time t = sys->GetTimeTf(main_tf, i);
		int wday = DayOfWeek(t);
		if (tf_type == 0) {
			if (wday == 0 || wday == 6)
				continue;
			int pospos = train_pos_all.GetCount();
			bool any_match = false;
			for(int j = 0; j < sym_ids.GetCount(); j++) {
				const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[j]);
				if (sym.IsOpen(t)) {
					train_pos[j].Add(pospos);
					any_match = true;
				}
			}
			if (any_match)
				train_pos_all.Add(i);
		}
		else if (tf_type == 1) {
			if (wday == 0 || wday == 6)
				continue;
			int pospos = train_pos_all.GetCount();
			train_pos_all.Add(i);
			for(int j = 0; j < sym_ids.GetCount(); j++)
				train_pos[j].Add(pospos);
		}
		else if (tf_type == 2) {
			int pospos = train_pos_all.GetCount();
			train_pos_all.Add(i);
			for(int j = 0; j < sym_ids.GetCount(); j++)
				train_pos[j].Add(pospos);
		}
	}*/
}

void AgentGroup::ProcessWorkQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		//LOG(i << "/" << work_queue.GetCount());
		SubProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
	
	work_lock.Leave();
}

void AgentGroup::ProcessDataBridgeQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < db_queue.GetCount(); i++) {
		//LOG(i << "/" << db_queue.GetCount());
		SubProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}
	
	work_lock.Leave();
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

}
