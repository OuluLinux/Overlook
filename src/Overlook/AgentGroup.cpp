#include "Overlook.h"

namespace Overlook {

SingleFixedSimBroker::SingleFixedSimBroker() {
	
	
	
}

void SingleFixedSimBroker::Cycle(Snapshot& snap) PARALLEL {
	
	
	
}

void SingleFixedSimBroker::Reset() PARALLEL {
	
	
	
}

void SingleFixedSimBroker::RefreshOrders(Snapshot& snap) PARALLEL {
	
	
	
}















Agent::Agent() {
	sym_id = 0;
	sym = 0;
	proxy_sym = 0;
	input_count = 0;
	
	
}

void Agent::Create() {
	
	
	
}

void Agent::Init() {
	
	
	
}

void Agent::Main(Snapshot& cur_snap, Snapshot& prev_snap) PARALLEL {
	if (broker.order_count == 0) {
		prev_equity = broker.AccountEquity();
	}
	
	broker.RefreshOrders(cur_snap);
	double equity = broker.AccountEquity();
	double reward = equity - prev_equity;
	
	Backward(cur_snap, reward);
	
	
	Forward(cur_snap);
	
	
	broker.Cycle(cur_snap);
}

void Agent::Forward(Snapshot& snap) PARALLEL {
	
	
	
}

void Agent::Backward(Snapshot& snap, double reward) PARALLEL {
	
	
	
}

void Agent::CloseAll(Snapshot& snap) PARALLEL {
	
	
	
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
	data_size   = sym_count * buf_count * 2;
	signal_size = sym_count * 2;
	ASSERT_(data_size   == SENSOR_SIZE, "Fixed value for now");
	ASSERT_(signal_size == SIGNAL_SIZE, "Fixed value for now");
	
	
	Progress(3, 6, "Reseting snapshots");
	RefreshSnapshots();
	
	
	Progress(5, 6, "Initializing agents and joiners");
	if (agents.GetCount() == 0)
		CreateAgents();
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].Init();
	
	
	agent_equities.SetCount(agents.GetCount() * snaps.GetCount(), 0.0);
	int agent_equities_mem = agent_equities.GetCount() * sizeof(float);
	
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

void AgentGroup::Main() {
	if (agents.GetCount() == 0 || sym_ids.IsEmpty() || indi_ids.IsEmpty()) return;
	
	RefreshSnapshots();
	
	Vector<Snapshot> snaps;
	
	
	while (running) {
		if (phase == PHASE_SEEKSNAPS) {
			
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
			phase = PHASE_TRAINING;
		}
		else if (phase == PHASE_TRAINING) {
			int snap_count = snaps.GetCount();
			int agent_count = agents.GetCount();
			array_view<Snapshot, 1>  snap_view(snap_count, snaps.Begin());
			array_view<Agent, 1> agents_view(agent_count, agents.Begin());
			array_view<float, 1> equities_view(agent_equities.GetCount(), agent_equities.Begin());
			
			
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
			        
			        if (agent.cursor == 0)
						agent.cursor++;
			        
			        for(int i = 0; i < 100; i++) {
			            Snapshot& cur_snap  = snap_view[agent.cursor - 0];
						Snapshot& prev_snap = snap_view[agent.cursor - 1];
						int cursor_begin = agent.cursor;
						
						agent.Main(cur_snap, prev_snap);
						
						// Get some diagnostic stats
						int agent_cursor_begin = agent_id * snap_count;
						int cursor_end = agent.cursor < snap_count ? agent.cursor : snap_count;
						float equity = agent.broker.AccountEquity();
						for(int j = cursor_begin; j < cursor_end; j++) {
							equities_view[agent_cursor_begin + j] = equity;
						}
						
						// Close all order at the end
						if (agent.cursor >= snap_count) {
							agent.cursor = 1;
							agent.broker.Reset();
						}
			        }
			    });
			}
			
			agents_view.synchronize();
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
	LoadFromFile(*this,	ConfigFile(name + ".agrp"));
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
	snap.year_time = (month * 31.0 + day) / 372.0;
	snap.week_time = ((wday * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0);
	
	
	// Refresh values (tf / sym / value)
	int k = 0;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		Vector<ConstBuffer*>& indi_buffers = value_buffers[i];
		for(int j = 0; j < buf_count; j++) {
			ConstBuffer& src = *indi_buffers[j];
			double d = src.GetUnsafe(shift);
			double pos, neg;
			if (d > 0) {
				pos = d;
				neg = 0;
			} else {
				pos = 0;
				neg = -d;
			}
			snap.sensor[k++] = pos;
			snap.sensor[k++] = neg;
		}
	}
	ASSERT(k == SENSOR_SIZE);
	
	return true;
}

void AgentGroup::CreateAgents() {
	ASSERT(buf_count > 0);
	ASSERT(sym_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	agents.SetCount(sym_count);
	for(int group_id = 0; group_id < group_count; group_id++) {
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			const Symbol& sym = mt.GetSymbol(sym_ids[i]);
			Agent& a = agents[i];
			a.sym_id = i;
			a.sym = sym_ids[i];
			a.proxy_sym = sym.proxy_id;
			a.input_count = 2 + SENSOR_SIZE + SIGNAL_SIZE;
			a.group_id = group_id;
			a.Create();
		}
	}
}

}

