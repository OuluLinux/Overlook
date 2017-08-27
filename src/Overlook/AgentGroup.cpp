#include "Overlook.h"

namespace Overlook {

SingleFixedSimBroker::SingleFixedSimBroker() {
	
	
	begin_equity = 10000;
}

void SingleFixedSimBroker::Reset() PARALLEL {
	equity = begin_equity;
	balance = begin_equity;
	order.is_open = false;
	order_count = 0;
	profit_sum = 0.0;
	loss_sum = 0.0;
}

float SingleFixedSimBroker::RealtimeBid(const Snapshot& snap, int sym_id) const PARALLEL {
	return snap.open[sym_id] - spread_points;
}

float SingleFixedSimBroker::RealtimeAsk(const Snapshot& snap, int sym_id) const PARALLEL {
	return snap.open[sym_id];
}

void SingleFixedSimBroker::OrderSend(int type, float volume, float price) PARALLEL {
	ASSERT(!order.is_open);
	order.type = type;
	order.volume = volume;
	order.open = price;
	order.is_open = true;
	order_count++;
}

void SingleFixedSimBroker::OrderClose(const Snapshot& snap) PARALLEL {
	ASSERT(order.is_open);
	order.is_open = false;
	double profit = GetCloseProfit(snap);
	balance += profit;
	equity = balance;
	if (profit > 0) profit_sum += profit;
	else            loss_sum   -= profit;
}


double SingleFixedSimBroker::GetCloseProfit(const Snapshot& snap) const PARALLEL {
	const FixedOrder& o = order;
	
	// NOTE: only for forex. Check SimBroker for other symbols too
	
	double volume = 100000 * o.volume; // lotsize * volume
	
	float close;
	if (o.type == OP_BUY)
		close = RealtimeBid(snap, sym_id);
	else if (o.type == OP_SELL)
		close = RealtimeAsk(snap, sym_id);
	else
		return 0.0;
	
	// Some time ranges can be invalid, and they have same price constantly...
	// Don't let them affect. This is the easiest and safest way to avoid that problem.
	// In normal and meaningful environment, open and close price is never the same.
	if (o.open == close)
		return 0.0;
	
	
	if (o.type == OP_BUY) {
		double change = volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeBid(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeAsk(snap, proxy_id);
		return change;
	}
	else if (o.type == OP_SELL) {
		double change = -1.0 * volume * (close / o.open - 1.0);
		if (proxy_base_mul == 0) return change;
		
		if      (proxy_base_mul == +1)
			change /= RealtimeAsk(snap, proxy_id);
		else //if (proxy_base_mul == -1)
			change *= RealtimeBid(snap, proxy_id);
		return change;
	}
	else return 0.0;
}

void SingleFixedSimBroker::Cycle(int signal, const Snapshot& snap) PARALLEL {
	ASSERT(sym_id >= 0 && sym_id < SYM_COUNT);
	
	FixedOrder& o = order;
	
	// Close order
	if (o.is_open) {
		if (o.type == OP_BUY) {
			if (signal <= 0)
				OrderClose(snap);
			else
				return;
		}
		else if (o.type == OP_SELL) {
			if (signal >= 0)
				OrderClose(snap);
			else
				return;
		}
	}
	
	
	if      (signal > 0)
		OrderSend(OP_BUY,  0.01f, RealtimeAsk(snap, sym_id));
	else if (signal < 0)
		OrderSend(OP_SELL, 0.01f, RealtimeBid(snap, sym_id));
	
}

void SingleFixedSimBroker::RefreshOrders(const Snapshot& snap) PARALLEL {
	FixedOrder& o = order;
	
	if (o.is_open) {
		equity = balance + GetCloseProfit(snap);
	} else {
		equity = balance;
	}
}









TraineeBase::TraineeBase() {
	
}

void TraineeBase::Create() {
	for(int i = 0; i < AGENT_RESULT_COUNT; i++)
		result[i] = 0.0f;
	result_cursor = 0;
}














Agent::Agent() {
	
}

void Agent::Create() {
	
	dqn.Reset();
	
	TraineeBase::Create();
	
}

void Agent::Init() {
	
	broker.sym_id = sym_id;
	broker.begin_equity = begin_equity;
	broker.spread_points = spread_points;
	broker.proxy_id = proxy_id;
	broker.proxy_base_mul = proxy_base_mul;
	
	ResetEpoch();
	
}

void Agent::ResetEpoch() {
	if (broker.order_count > 0) {
		last_drawdown = broker.GetDrawdown();
		if (broker.equity > best_result)
			best_result = broker.equity;
		result[result_cursor] = broker.equity;
		result_cursor = (result_cursor + 1) % AGENT_RESULT_COUNT;
		result_count++;
	}
	
	broker.Reset();
	
	prev_equity = broker.AccountEquity();
	signal = 0;
	timestep_actual = 0;
	timestep_total = 1;
	cursor = 1;
}

void Agent::Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL {
	if (timestep_actual <= 0) {
		
		broker.RefreshOrders(cur_snap);
		double equity = broker.AccountEquity();
		double reward = equity - prev_equity;
		
		
		Backward(cur_snap, reward);
		
		
		if (broker.equity < 0.25 * broker.begin_equity) broker.Reset();
		prev_equity = broker.equity;
		
		
		Forward(cur_snap, prev_snap);
		
	}
	
	// LOG("Agent " << id << ": " << cursor << ", " << signal << ", " << timestep_actual << "/" << timestep_total);
	
	WriteSignal(cur_snap);
}

void Agent::Forward(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL {
	
	// Input values
	// - time_values
	// - all data from snapshot
	// - previous signal
	// - account change sensor
	float input_array[AGENT_STATES];
	int cursor = 0;
	
	
	// time_values
	input_array[cursor++] = cur_snap.year_timesensor;
	input_array[cursor++] = cur_snap.week_timesensor;
	
	
	// sensor data for current tf
	int sensor_cursor = 0;
	for(int i = 0; i < GROUP_SENSOR_SIZE; i++)
		input_array[cursor++] = cur_snap.sensor[sensor_cursor++];
	
	
	// all previous signals from same tf agents
	int signal_cursor = group_id * GROUP_SIGNAL_SIZE;
	if (!(signal_cursor >= 0 && signal_cursor + GROUP_SIGNAL_SIZE <= SIGNAL_SIZE)) {
		LOG((signal_cursor + GROUP_SIGNAL_SIZE) << " <= " << SIGNAL_SIZE);
	}
	int as = AGENT_STATES;
	int begin = signal_cursor;
	int end = signal_cursor + GROUP_SIGNAL_SIZE;
	int ssize = SIGNAL_SIZE;
	ASSERT(signal_cursor >= 0 && signal_cursor + GROUP_SIGNAL_SIZE <= SIGNAL_SIZE);
	for(int i = 0; i < GROUP_SIGNAL_SIZE; i++)
		input_array[cursor++] = prev_snap.signal[signal_cursor++];
	
	
	ASSERT(cursor == AGENT_STATES);
	
	
	int action = dqn.Act(input_array);
	ASSERT(action >= 0 && action < AGENT_ACTIONCOUNT);
    
    
    // Convert action to simple signal
    
	// Long/Short
	if (action < 20) {
		int exp = action / 2;
		bool neg = exp % 2; // 0,+1,-1,-2,+2,+4,-4 ...
		bool dir = (action % 2) != neg;
		signal = dir ? -1 : +1;
		timestep_total = 1 << exp;
	}
	
	// Idle
	else {
		action -= 20;
		signal = 0;
		timestep_total = 1 << action;
	}
	
	timestep_actual = timestep_total;
	
	
	// Set signal to broker
	broker.Cycle(signal, cur_snap);
	
	
}


void Agent::WriteSignal(Snapshot& cur_snap) PARALLEL {
	
	if (timestep_actual < 0) timestep_actual = 0;
	float timestep_sensor = 0.75 - 0.75 * timestep_actual / timestep_total;
	
	
	// Write latest average to the group values
	float pos, neg, idl;
	if (signal == 0) {
		pos = 1.0;
		neg = 1.0;
		idl = timestep_sensor;
	}
	else if (signal > 0) {
		pos = timestep_sensor;
		neg = 1.0;
		idl = 1.0;
	}
	else {
		pos = 1.0;
		neg = timestep_sensor;
		idl = 1.0;
	}
	
	
	int group_begin = group_id * GROUP_SIGNAL_SIZE;
	int sigpos = group_begin + sym_id * SIGNAL_SENSORS;
	ASSERT(sigpos >= 0 && sigpos+2 < SIGNAL_SIZE);
	cur_snap.signal[sigpos + 0] = pos;
	cur_snap.signal[sigpos + 1] = neg;
	cur_snap.signal[sigpos + 2] = idl;
	
}

void Agent::Backward(Snapshot& snap, double reward) PARALLEL {
	
	// pass to brain for learning
	if (is_training && cursor > 1)
		dqn.Learn(reward);
	
	reward_sum += reward;
	
	if (iter % 50 == 0) {
		average_reward = reward_sum / 50;
		reward_sum = 0;
	}
	
	iter++;
}





















AgentGroup::AgentGroup(System* sys) : sys(sys) {
	running = false;
	stopped = true;
	group_count = 2;
	
	allowed_symbols.Add("AUDCAD");
	allowed_symbols.Add("AUDJPY");
	allowed_symbols.Add("AUDNZD");
	allowed_symbols.Add("AUDUSD");
	allowed_symbols.Add("CADJPY");
	allowed_symbols.Add("EURAUD");
	allowed_symbols.Add("EURCAD");
	allowed_symbols.Add("EURGBP");
	allowed_symbols.Add("EURJPY");
	allowed_symbols.Add("EURNZD");
	allowed_symbols.Add("EURUSD");
	allowed_symbols.Add("GBPAUD");
	allowed_symbols.Add("GBPCAD");
	allowed_symbols.Add("GBPJPY");
	allowed_symbols.Add("GBPNZD");
	allowed_symbols.Add("GBPUSD");
	allowed_symbols.Add("NZDCAD");
	allowed_symbols.Add("NZDJPY");
	allowed_symbols.Add("NZDUSD");
	allowed_symbols.Add("USDCAD");
	allowed_symbols.Add("USDJPY");

}

AgentGroup::~AgentGroup() {
	Stop();
	StoreThis();
}

void AgentGroup::Init() {
	ASSERT(sys);
	
	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);
	
	ManagerLoader& loader = GetManagerLoader();
	Thread::Start(THISBACK(InitThread));
	loader.Run();
}

void AgentGroup::InitThread() {
	Progress(0, 6, "Refreshing work queue");
	MetaTrader& mt = GetMetaTrader();
	for(int i = 0; i < mt.GetSymbolCount(); i++) {
		const Symbol& sym = mt.GetSymbol(i);
		if (sym.IsForex() && allowed_symbols.Find(sym.name.Left(6)) != -1)
			sym_ids.Add(i);
	}
	sym_count = sym_ids.GetCount();
	ASSERT(sym_count == SYM_COUNT);
	main_tf = sys->FindPeriod(1);
	ASSERT(main_tf != -1);
	
	int stoch_id = sys->Find<StochasticOscillator>();
	int osma_id = sys->Find<OsMA>();
	ASSERT(stoch_id != -1);
	ASSERT(osma_id != -1);
	
	indi_ids.Clear();
	indi_ids.Add().Set(osma_id).AddArg(5).AddArg(5*2).AddArg(5);
	indi_ids.Add().Set(osma_id).AddArg(15).AddArg(15*2).AddArg(15);
	indi_ids.Add().Set(osma_id).AddArg(240).AddArg(240*2).AddArg(240);
	indi_ids.Add().Set(osma_id).AddArg(1440).AddArg(1440*2).AddArg(1440);
	indi_ids.Add().Set(stoch_id).AddArg(5);
	indi_ids.Add().Set(stoch_id).AddArg(15);
	indi_ids.Add().Set(stoch_id).AddArg(240);
	indi_ids.Add().Set(stoch_id).AddArg(1440);
	
	RefreshWorkQueue();
	
	
	Progress(1, 6, "Refreshing data source and indicators");
	ProcessWorkQueue();
	ProcessDataBridgeQueue();
	
	
	Progress(2, 6, "Refreshing pointers of data sources");
	ResetValueBuffers();
	
	
	Progress(3, 6, "Reseting snapshots");
	RefreshSnapshots();
	
	
	Progress(5, 6, "Initializing agents and joiners");
	if (agents.GetCount() == 0)
		CreateAgents();
	for(int i = 0; i < agents.GetCount(); i++) {
		Agent& a = agents[i];
		
		const Price& askbid = mt.GetAskBid()[a.sym__];
		double ask = askbid.ask;
		double bid = askbid.bid;
		const Symbol& symbol = mt.GetSymbol(a.sym__);
		if (symbol.proxy_id != -1) {
			int j = sym_ids.Find(symbol.proxy_id);
			ASSERT(j != -1);
			a.proxy_id = j;
			a.proxy_base_mul = symbol.base_mul;
		} else {
			a.proxy_id = -1;
			a.proxy_base_mul = 0;
		}
		a.begin_equity = mt.AccountEquity();
		a.spread_points = ask - bid;
		ASSERT(a.spread_points > 0.0);
		
		a.Init();
	}
	
	
	agent_equities.SetCount(agents.GetCount() * snaps.GetCount(), 0.0);
	int agent_equities_mem = agent_equities.GetCount() * sizeof(double);
	
	max_memory_size = GetAmpDeviceMemory();
	agents_total_size = agents.GetCount() * sizeof(Agent);
	memory_for_snapshots = max_memory_size - agents_total_size - agent_equities_mem;
	snaps_per_phase = memory_for_snapshots / sizeof(Snapshot) * 8 / 10;
	snap_phase_count = snaps.GetCount() / snaps_per_phase;
	if (snaps.GetCount() % snaps_per_phase != 0) snap_phase_count++;
	snap_phase_id = 0,
	
	
	Progress(6, 6, "Complete");
}

void AgentGroup::Start() {
	Stop();
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Main));
}

void AgentGroup::Stop() {
	running = false;
	while (stopped != true) Sleep(100);
}

void AgentGroup::SetAgentsTraining(bool b) {
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].is_training = b;
}

void AgentGroup::Main() {
	if (agents.GetCount() == 0 || sym_ids.IsEmpty() || indi_ids.IsEmpty()) return;
	
	RefreshSnapshots();
	
	#ifdef HAVE_SYSTEM_AMP
	Vector<Snapshot> snaps;
	#endif
	
	while (running) {
		if (phase == PHASE_SEEKSNAPS) {
			#ifdef HAVE_SYSTEM_AMP
			// Seek different snapshot dataset, because GPU memory is limited.
			if (snap_phase_id >= snap_phase_count)
				snap_phase_id = 0;
			
			int snap_begin = snap_phase_id * snaps_per_phase;
			int snap_end = Upp::min(snap_begin + snaps_per_phase, this->snaps.GetCount());
			int snap_count = snap_end - snap_begin;
			ASSERT(snap_count > 0);
			snaps.SetCount(snap_count);
			memcpy(snaps.Begin(), this->snaps.Begin() + snap_begin, snap_count * sizeof(Snapshot));
			snap_phase_id++;
			#endif
			
			phase = PHASE_TRAINING;
		}
		else if (phase == PHASE_TRAINING) {
			SetAgentsTraining(true);
			
			
			int snap_count = snaps.GetCount();
			int agent_count = agents.GetCount();
			array_view<Snapshot, 1>  snap_view(snap_count, snaps.Begin());
			array_view<Agent, 1> agents_view(agent_count, agents.Begin());
			array_view<double, 1> equities_view(agent_equities.GetCount(), agent_equities.Begin());
			
			
			TimeStop ts;
			int64 total_elapsed = 0;
			for (int64 iter = 0; phase == PHASE_TRAINING && running; iter++) {
				
				// Change snapshot area, if needed, sometimes
				if (iter & (1 << 14)) {
					total_elapsed += ts.Elapsed();
					ts.Reset();
					iter = 0;
					if (total_elapsed > 10*60*1000) {
						phase = PHASE_SEEKSNAPS;
						break;
					}
				}
				
				parallel_for_each(agents_view.extent, [=](index<1> idx) PARALLEL
			    {
			        int agent_id = idx[0];
			        Agent& agent = agents_view[idx];
			        int equities_begin = agent.id * snap_count;
			        
			        if (agent.cursor == 0)
						agent.cursor++;
			        
			        for(int i = 0; i < 1000; i++) {
			            Snapshot& cur_snap  = snap_view[agent.cursor - 0];
						Snapshot& prev_snap = snap_view[agent.cursor - 1];
						
						agent.timestep_actual--;
						
						agent.Main(cur_snap, prev_snap);
						
						// Get some diagnostic stats
						equities_view[equities_begin + agent.cursor] = agent.broker.AccountEquity();
						agent.cursor++;
						
						// Close all order at the end
						if (agent.cursor >= snap_count) {
							agent.ResetEpoch();
						}
			        }
			    });
			}
			
			agents_view.synchronize();
			equities_view.synchronize();
			
			
			SetAgentsTraining(false);
			
		}
		else if (phase == PHASE_WEIGHTS) {
			
			
			// Weight single group (21 pairs)
			
		}
		else if (phase == PHASE_FINAL) {
			
			
			// Weight group joiners to final output
			
		}
		else if (phase == PHASE_UPDATE) {
			
			RefreshSnapshots();
			
			// Updates latest snapshot signals
			
		}
		else if (phase == PHASE_REAL) {
			
			
			// Updates broker
			
		}
		else if (phase == PHASE_WAIT) {
			
			
			// Changes to PHASE_UPDATE when time to update
			
		}
		else Sleep(100);
	}
	
	
	stopped = true;
}

void AgentGroup::LoadThis() {
	LoadFromFile(*this,	ConfigFile("agentgroup.bin"));
}

void AgentGroup::StoreThis() {
	Time t = GetSysTime();
	String file = ConfigFile("agentgroup.bin");
	bool rem_bak = false;
	if (FileExists(file)) {
		FileMove(file, file + ".bak");
		rem_bak = true;
	}
	StoreToFile(*this,	file);
	if (rem_bak)
		DeleteFile(file + ".bak");
}

void AgentGroup::Serialize(Stream& s) {
	/*TraineeBase::Serialize(s);
	s % go % agents % tf_limit % tf_ids % sym_ids % created % name % param_str
	  % fmlevel % limit_factor
	  % group_input_width % group_input_height
	  % mode
	  % enable_training;*/
}

void AgentGroup::SetEpsilon(double d) {
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].dqn.SetEpsilon(d);
}

void AgentGroup::RefreshSnapshots() {
	ASSERT(buf_count != 0);
	ASSERT(sym_ids.GetCount() != 0);
	
	TimeStop ts;
	ProcessWorkQueue();
	
	int snap_count = sys->GetCountTf(main_tf) - data_begin;
	ASSERT(snap_count > 0);
	snaps.Reserve(snap_count + (60 - (snap_count % 60)));
	for(int i = snaps.GetCount(); i < snap_count; i++)
		Seek(snaps.Add(), i + data_begin);
	
	LOG("Refreshing snapshots took " << ts.ToString());
}

void AgentGroup::Progress(int actual, int total, String desc) {
	ManagerLoader& loader = GetManagerLoader();
	loader.PostProgress(actual, total, desc);
	loader.PostSubProgress(0, 1);
}

void AgentGroup::SubProgress(int actual, int total) {
	ManagerLoader& loader = GetManagerLoader();
	loader.PostSubProgress(actual, total);
}

void AgentGroup::ResetValueBuffers() {
	
	// Get total count of output buffers in the indicator list
	VectorMap<unsigned, int> bufout_ids;
	int buf_id = 0;
	for(int i = 0; i < indi_ids.GetCount(); i++) {
		FactoryDeclaration& decl = indi_ids[i];
		const FactoryRegister& reg = sys->GetRegs()[decl.factory];
		for(int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);
		bufout_ids.Add(decl.GetHashValue(), buf_id);
		buf_id += reg.out[0].visible;
	}
	buf_count = buf_id;
	ASSERT(buf_count);
	
	
	// Get DataBridge core pointer for easy reading
	databridge_cores.SetCount(0);
	databridge_cores.SetCount(sys->GetSymbolCount(), NULL);
	int factory = sys->Find<DataBridge>();
	for(int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];
		if (ci.factory != factory)
			continue;
		databridge_cores[ci.sym] = &*ci.core;
	}
	
	
	// Reserve zeroed memory for output buffer pointer vector
	value_buffers.Clear();
	value_buffers.SetCount(sym_ids.GetCount());
	for(int i = 0; i < sym_ids.GetCount(); i++)
		value_buffers[i].SetCount(buf_count, NULL);
	
	
	// Get output buffer pointer vector
	int total_bufs = 0;
	data_begin = 0;
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.GetOutput(0);
		
		int sym_id = sym_ids.Find(ci.sym);
		if (sym_id == -1)
			continue;
		if (ci.tf != main_tf)
			continue;
		
		DataBridge* db = dynamic_cast<DataBridge*>(&*ci.core);
		if (db) data_begin = Upp::max(data_begin, db->GetDataBegin());
		
		Vector<ConstBuffer*>& indi_buffers = value_buffers[sym_id];
		
		const FactoryRegister& reg = sys->GetRegs()[ci.factory];
		
		FactoryDeclaration decl;
		decl.Set(ci.factory);
		for(int j = 0; j < ci.args.GetCount(); j++) decl.AddArg(ci.args[j]);
		for(int j = decl.arg_count; j < reg.args.GetCount(); j++) decl.AddArg(reg.args[j].def);
		unsigned hash = decl.GetHashValue();
		
		int buf_begin_id = bufout_ids.Find(hash);
		if (buf_begin_id == -1)
			continue;
		int buf_begin = bufout_ids[buf_begin_id];
		
		for (int l = 0; l < reg.out[0].visible; l++) {
			int buf_pos = buf_begin + l;
			ConstBuffer*& bufptr = indi_buffers[buf_pos];
			ASSERT_(bufptr == NULL, "Duplicate work item");
			bufptr = &output.buffers[l];
			total_bufs++;
		}
	}
	int expected_total = sym_ids.GetCount() * buf_count;
	ASSERT_(total_bufs == expected_total, "Some items are missing in the work queue");
}

void AgentGroup::RefreshWorkQueue() {
	
	// Add proxy symbols to the queue if any
	Index<int> tf_ids, sym_ids;
	tf_ids.Add(main_tf);
	sym_ids <<= this->sym_ids;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[i]);
		if (sym.proxy_id == -1) continue;
		sym_ids.FindAdd(sym.proxy_id);
	}
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	
	// Get DataBridge work queue
	Vector<FactoryDeclaration> db_indi_ids;
	db_indi_ids.Add().Set(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, tf_ids, db_indi_ids);
}

void AgentGroup::ProcessWorkQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		SubProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
	
	work_lock.Leave();
}

void AgentGroup::ProcessDataBridgeQueue() {
	work_lock.Enter();
	
	for(int i = 0; i < db_queue.GetCount(); i++) {
		SubProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}
	
	work_lock.Leave();
}

bool AgentGroup::Seek(Snapshot& snap, int shift) {
	
	// Check that shift is not too much
	ASSERT_(shift >= 0 && shift < sys->GetCountTf(main_tf), "Data position is not in data range");
	ASSERT_(shift >= data_begin, "Data position is before common starting point");
	
	
	// Get some time values in binary format (starts from 0)
	Time t = sys->GetTimeTf(main_tf, shift);
	int month = t.month-1;
	int day = t.day-1;
	int hour = t.hour;
	int minute = t.minute;
	int wday = DayOfWeek(t);
	
	
	// Time sensor
	snap.year_timesensor = (month * 31.0 + day) / 372.0;
	snap.week_timesensor = ((wday * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0);
	
	
	// Refresh values (tf / sym / value)
	int k = 0;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		Vector<ConstBuffer*>& indi_buffers = value_buffers[i];
		for(int j = 0; j < buf_count; j++) {
			ConstBuffer& src = *indi_buffers[j];
			double d = src.GetUnsafe(shift);
			double pos, neg;
			if (d > 0) {
				pos = +d;
				neg =  0;
			} else {
				pos =  0;
				neg = -d;
			}
			snap.sensor[k++] = pos;
			snap.sensor[k++] = neg;
		}
		
		DataBridge& db = *dynamic_cast<DataBridge*>(databridge_cores[sym_ids[i]]);
		double open = db.GetBuffer(0).GetUnsafe(shift);
		snap.open[i] = open;
	}
	
	
	
	ASSERT(k == SENSOR_SIZE);
	
	return true;
}

void AgentGroup::CreateAgents() {
	ASSERT(buf_count > 0);
	ASSERT(sym_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	agents.SetCount(sym_count * group_count);
	int j = 0;
	for(int group_id = 0; group_id < group_count; group_id++) {
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			const Symbol& sym = mt.GetSymbol(sym_ids[i]);
			Agent& a = agents[j];
			a.id = j;
			a.sym_id = i;
			a.sym__ = sym_ids[i];
			a.group_id = group_id;
			a.Create();
			j++;
		}
	}
	ASSERT(j == agents.GetCount());
}

}

