#include "Overlook.h"

namespace Overlook {

AgentGroup::AgentGroup() {
	sys = NULL;
	
	agent_input_width = 0;
	group_input_width = 0;
	agent_input_height = 0;
	group_input_height = 0;
	data_size = 0;
	signal_size = 0;
	act_iter = 0;
	mode = 0;
	main_tf = -1;
	main_tf_pos = -1;
	
	limit_factor = 0.01;
	global_free_margin_level = 0.90;
	buf_count = 0;
	enable_training = true;
	sig_freeze = true;
	reset_optimizer = false;
	accum_signal = false;
	allow_realtime = false;
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
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& agent = agents[i];
		agent.RefreshTotalEpochs();
		while (agent.epoch_actual < agent.epoch_total) {
			agent.Main();
		}
		ASSERT(agent.epoch_actual == agent.epoch_total); // not epoch_actual==0 ...
	}
	Snapshot& shift_snap = snaps[train_pos_all.GetCount()-1];
	Forward(shift_snap, broker, NULL);
	
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
	
	return true;
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

void AgentGroup::CreateAgents() {
	ASSERT(buf_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	agents.SetCount(sym_ids.GetCount());
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = mt.GetSymbol(sym_ids[i]);
		Agent& a = agents[i];
		a.group = this;
		a.sym = sym_ids[i];
		a.group_id = i;
		a.proxy_sym = sym.proxy_id;
		a.Create(agent_input_width, agent_input_height);
	}
}

void AgentGroup::Create(int width, int height) {
	go.SetArrayCount(width);
	go.SetCount(height);
	go.SetPopulation(10);
	go.SetMaxGenerations(100);
	
	
	int sensors = timeslots;
	ASSERT(height == sensors);
	for(int i = 0; i < sensors; i++) {
		go.Set(i, 0.75, 0.999, 0.001);
	}
	
	
	//go.UseLimits();
	go.Init();
}

void AgentGroup::InitThreads() {
	ASSERT(buf_count != 0);
	ASSERT(tf_ids.GetCount() != 0);
	ASSERT(sym_ids.GetCount() != 0);
	ASSERT(agent_input_width != 0);
	ASSERT(group_input_width != 0);
	
	GenerateSnapshots();
}

void AgentGroup::Init() {
	ASSERT(sys);
	
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
	
	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);
	
	tf_periods.SetCount(tf_ids.GetCount(), 1);
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int period = sys->GetPeriod(tf_ids[i]) / main_tf_period;
		tf_periods[i] = period;
	}
	
	
	int indi;
	indi = sys->Find<Sensors>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	
	
	broker.SetCollecting(1000000.0);
	
	RefreshWorkQueue();
	
	Progress(1, 6, "Processing data");
	ProcessWorkQueue();
	ProcessDataBridgeQueue();
	
	Progress(2, 6, "Finding value buffers");
	ResetValueBuffers();
	
	data_size = 1 + sym_ids.GetCount() * tf_ids.GetCount() * buf_count;
	signal_size = sym_ids.GetCount() * 2 * 3;
	total_size = data_size + signal_size;
	agent_input_width  = 1;
	agent_input_height = GetSignalPos(sym_ids.GetCount());
	fastest_period_mins = sys->GetPeriod(main_tf) * sys->GetBasePeriod() / 60;
	int wdaymins = 5 * 24 * 60;
	timeslots = Upp::max(1, wdaymins / fastest_period_mins);
	group_input_width  = 1;
	group_input_height = timeslots;
	
	Progress(3, 6, "Reseting snapshots");
	InitThreads();
	
	Progress(4, 6, "Initializing group trainee");
	group = this;
	if (agents.IsEmpty()) {
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

void AgentGroup::Main() {
	ASSERT(!at_main);
	at_main = true;
	epoch_total = snaps.GetCount();
	if (epoch_actual >= epoch_total)
		epoch_actual = 0;
	
	if (reset_optimizer) {
		reset_optimizer = false;
		seq_results.Clear();
		Create(group_input_width, group_input_height);
	}
	
	if (!allow_realtime && epoch_actual == 0) {
		prev_reward = 0;
		go.Start();
	}
	
	// Do some action
	Action();
	
	if (!allow_realtime && (!epoch_actual == epoch_total-1 || broker.AccountEquity() < 0.4 * begin_equity)) {
		double energy = broker.GetCollected() + broker.AccountEquity() - broker.GetInitialBalance();
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

void AgentGroup::Forward(Snapshot& snap, Brokerage& broker, Snapshot* next_snap) {
	
	// Input values
	// - data values
	//		- time value
	//		- data sensors
	//		- 'accum_buf'
	// - free-margin-level
	// - active instruments total / maximum
	// - account change sensor
	// - instrument value / (0.1 * equity) or something
	
	int timeslot = (((snap.time_values[2] - 1) * 24 + snap.time_values[3]) * 60 + snap.time_values[4]) / fastest_period_mins;
	ASSERT(timeslot >= 0 && timeslot < timeslots);
	
	const Vector<double>& go_values = !allow_realtime ?
		 go.GetTrialSolution() :
		 go.GetBestSolution();
	ASSERT(go_values.GetCount() == group_input_height);
	
	
	// Additional sensor values
    for(int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
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
	
	// Set free-margin level
	/*double fmlevel = go_values[timeslot];
	if (fmlevel < 0.75)
		fmlevel = 0.75;
	else if (fmlevel > 0.99)
		fmlevel = 0.99;
	broker.SetFreeMarginLevel(fmlevel);
	global_free_margin_level = fmlevel;*/
	
	
}

void AgentGroup::Backward(double reward) {
	prev_reward = reward;
	
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
	  % global_free_margin_level % limit_factor
	  % agent_input_width % agent_input_height
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
	ASSERT(group_id >= 0 && group_id <= sym_ids.GetCount());
	return data_size + group_id * 2 * 3;
}

void AgentGroup::RefreshSnapshots() {
	ProcessWorkQueue();
	
	int pos = train_pos_all.Top()+1;
	int bars = sys->GetCountTf(main_tf);
	
	train_pos_all.Reserve(bars - pos);
	train_pos.SetCount(sym_ids.GetCount());
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
		if (tf_type < 2 && (wday == 0 || wday == 6))
			continue;
		
		//LOG("Agent::RefreshSnapshots: Creating snapshot at " << Format("%", t));
		One<Snapshot> snap;
		snap.Create();
		
		ResetSnapshot(*snap);
		Seek(*snap, i);
		
		snaps.Add(snap.Detach());
		
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
			if (any_match) {
				train_pos_all.Add(i);
				WhenInfo("Added snapshot: " + Format("%",t));
			}
		}
		else if (tf_type == 1) {
			if (wday == 0 || wday == 6)
				continue;
			int pospos = train_pos_all.GetCount();
			train_pos_all.Add(i);
			WhenInfo("Added snapshot: " + Format("%",t));
			for(int j = 0; j < sym_ids.GetCount(); j++)
				train_pos[j].Add(pospos);
		}
		else if (tf_type == 2) {
			int pospos = train_pos_all.GetCount();
			train_pos_all.Add(i);
			WhenInfo("Added snapshot: " + Format("%",t));
			for(int j = 0; j < sym_ids.GetCount(); j++)
				train_pos[j].Add(pospos);
		}
	}
}

void AgentGroup::GenerateSnapshots() {
	
	// Generate snapshots
	TimeStop ts;
	snaps.SetCount(train_pos_all.GetCount());
	for(int i = 0; i < train_pos_all.GetCount(); i++) {
		if (i % 87 == 0)
			SubProgress(i, train_pos_all.GetCount());
		Snapshot& snap = snaps[i];
		ResetSnapshot(snap);
		Seek(snap, train_pos_all[i]);
	}
	LOG("Generating snapshots took " << ts.ToString());
}

void AgentGroup::ResetSnapshot(Snapshot& snap) {
	ASSERT(!value_buffers.IsEmpty());
	
	// Reserve memory for values
	snap.values.SetCount(total_size, 0.0);
	snap.time_values.SetCount(5);
	
	// Seek to beginning
	Seek(snap, 0);
}

bool AgentGroup::Seek(Snapshot& snap, int shift) {
	int bars = sys->GetCountTf(main_tf);
	if (shift >= bars || shift < 0)
		return false;
	
	
	// Get some time values in binary format (starts from 0)
	Time t = sys->GetTimeTf(main_tf, shift);
	int month = t.month-1;
	int day = t.day-1;
	int hour = t.hour;
	int minute = t.minute;
	int dow = DayOfWeek(t);
	snap.time_values[0] = month;
	snap.time_values[1] = day;
	snap.time_values[2] = dow;
	snap.time_values[3] = hour;
	snap.time_values[4] = minute;
	snap.time = t;
	snap.added = GetSysTime();
	snap.shift = shift;
	
	snap.signals.SetCount(sym_ids.GetCount(), 0);
	
	// Time sensor
	int vpos = 0;
	double time_sensor = ((dow * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0);
	snap.values[vpos++] = time_sensor;
	
	
	// Refresh values (tf / sym / value)
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int pos = sys->GetShiftTf(main_tf, tf_ids[i], shift);
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			Vector<ConstBuffer*>& indi_buffers = value_buffers[j][i];
			for(int k = 0; k < buf_count; k++) {
				ConstBuffer& src = *indi_buffers[k];
				double d = src.GetUnsafe(pos);
				snap.values[vpos + (j * tf_ids.GetCount() + i) * buf_count + k] = d;
			}
		}
	}
	
	return true;
}

void AgentGroup::RefreshWorkQueue() {
	Index<int> sym_ids;
	sym_ids <<= this->sym_ids;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(i);
		if (sym.proxy_id == -1) continue;
		sym_ids.FindAdd(sym.proxy_id);
	}
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	// Get DataBridge work queue
	Index<int> db_tf_ids, db_indi_ids;
	db_tf_ids.Add(main_tf);
	db_indi_ids.Add(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, db_tf_ids, db_indi_ids);
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
	databridge_cores.SetCount(sys->GetSymbolCount(), NULL);
	int factory = sys->Find<DataBridge>();
	for(int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];
		if (ci.factory != factory) continue;
		databridge_cores[ci.sym] = &*ci.core;
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
	int expected_total = sym_ids.GetCount() * tf_ids.GetCount() * buf_count;
	ASSERT_(total_bufs == expected_total, "Some items are missing in the work queue");
	
	
	int pos = data_begins[main_tf_pos];
	int bars = sys->GetCountTf(main_tf);
	
	#ifdef flagDEBUG
	pos = Upp::max(pos, bars-1000);
	#endif
	
	train_pos_all.Reserve(bars - pos);
	train_pos.SetCount(sym_ids.GetCount());
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
	}
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
		if (!databridge_cores[i]) continue;
		Core& core = *databridge_cores[i];
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(core.GetSymbol(), open.Get(pos));
	}
	sb.SetTime(sys->GetTimeTf(main_tf, pos));
}

}
