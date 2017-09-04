#include "Overlook.h"

namespace Overlook {

bool reset_agents;
bool reset_joiners;

AgentGroup::AgentGroup(System* sys) : sys(sys) {
	running = false;
	stopped = true;
	
	allowed_symbols.Add("AUDUSD");
	allowed_symbols.Add("EURAUD");
	allowed_symbols.Add("EURGBP");
	allowed_symbols.Add("EURJPY");
	allowed_symbols.Add("EURUSD");
	allowed_symbols.Add("GBPJPY");
	allowed_symbols.Add("GBPUSD");
	allowed_symbols.Add("NZDUSD");
	allowed_symbols.Add("USDCAD");
	allowed_symbols.Add("USDJPY");
	
	created = GetSysTime();
}

AgentGroup::~AgentGroup() {
	Stop();
}

void AgentGroup::Init() {
	ASSERT(sys);
	
	agent_epsilon = 0.02;
	joiner_epsilon = 0.02;
	joiner_fmlevel = 0.6;
	
	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);
	
	ManagerLoader& loader = GetManagerLoader();
	Thread::Start(THISBACK(InitThread));
	loader.Run();
}

void AgentGroup::InitThread() {
	Progress(0, 6, "Refreshing work queue");
	MetaTrader& mt = GetMetaTrader();
	const Vector<Price>& askbid = mt._GetAskBid();
	for(int j = 0; j < allowed_symbols.GetCount(); j++) {
		const String& allowed_sym = allowed_symbols[j];
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& sym = mt.GetSymbol(i);
			if (sym.IsForex() && (sym.name.Left(6)) == allowed_sym) {
				double base_spread = 1000.0 * (askbid[i].ask / askbid[i].bid - 1.0);
				if (base_spread >= 0.5) {
					LOG("Warning! Too much spread: " << sym.name << " (" << base_spread << ")");
				}
				sym_ids.Add(i);
				break;
			}
		}
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
	indi_ids.Add().Set(osma_id).AddArg(60).AddArg(60*2).AddArg(60);
	indi_ids.Add().Set(osma_id).AddArg(240).AddArg(240*2).AddArg(240);
	indi_ids.Add().Set(osma_id).AddArg(1440).AddArg(1440*2).AddArg(1440);
	indi_ids.Add().Set(stoch_id).AddArg(5);
	indi_ids.Add().Set(stoch_id).AddArg(15);
	indi_ids.Add().Set(stoch_id).AddArg(60);
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
	else {
		for(int i = 0; i < agents.GetCount(); i++) {
			Agent& a = agents[i];
			a.sym = sym_ids[a.sym_id];
		}
	}
	if (reset_agents) {
		phase = Upp::min(phase, (int)PHASE_TRAINING);
		for(int i = 0; i < agents.GetCount(); i++)
			agents[i].Create();
	}
	for(int i = 0; i < agents.GetCount(); i++)
		InitAgent(agents[i]);
	ASSERT(agents.GetCount() == TRAINEE_COUNT);
	
	if (joiners.GetCount() == 0)
		CreateJoiners();
	
	if (reset_joiners) {
		phase = Upp::min(phase, (int)PHASE_JOINER);
		for(int i = 0; i < joiners.GetCount(); i++)
			joiners[i].Create();
	}
	
	for(int i = 0; i < joiners.GetCount(); i++)
		InitJoiner(joiners[i]);
	ASSERT(joiners.GetCount() == TRAINEE_COUNT);
	
	
	agent_equities.SetCount(agents.GetCount() * snaps.GetCount(), 0.0);
	agent_equities_count = snaps.GetCount();
	joiner_equities.SetCount(joiners.GetCount() * snaps.GetCount(), 0.0);
	joiner_equities_count = snaps.GetCount();
	
	SetFreeMarginLevel(0.6);
	
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

void AgentGroup::SetJoinersTraining(bool b) {
	for(int i = 0; i < joiners.GetCount(); i++)
		joiners[i].is_training = b;
}

void AgentGroup::Main() {
	if (agents.GetCount() == 0 || joiners.GetCount() == 0 || sym_ids.IsEmpty() || indi_ids.IsEmpty()) return;
	
	RefreshSnapshots();
	
	while (running) {
		if (phase == PHASE_TRAINING) {
			
			TrainAgents();
			
		}
		else if (phase == PHASE_JOINER) {
			
			TrainJoiners();
			
		}
		else if (phase == PHASE_REAL) {
			
			MainReal();
			Sleep(1000);
			
		}
		else Sleep(100);
	}
	
	
	stopped = true;
}

void AgentGroup::TrainAgents() {
	sys->WhenPushTask("Agent training");
	
	RefreshAgentEpsilon();
	if (GetAverageAgentIterations() >= 1.0)
		LoopAgentSignals(true);
	SetAgentsTraining(true);
	
	int snap_count = snaps.GetCount();
	agent_equities.SetCount(agents.GetCount() * snap_count, 0.0);
	agent_equities_count = snap_count;
	
	
	TimeStop ts;
	int64 total_elapsed = 0;
	for (int64 iter = 0; phase == PHASE_TRAINING && running; iter++) {
		
		// Change snapshot area, if needed, sometimes
		if (iter > 100) {
			RefreshAgentEpsilon();
			total_elapsed += ts.Elapsed();
			ts.Reset();
			iter = 0;
			if (total_elapsed > 5*60*1000) {
				sys->WhenPopTask();
				return; // call TrainAgents again to UpdateAmpSnaps safely
			}
			
			// Change phase to joiner eventually
			if (GetAverageAgentIterations() >= AGENT_PHASE_ITER_LIMIT) {
				phase = PHASE_JOINER;
				StoreThis();
				break;
			}
		}
		
		CoWork co;
		for(int i = 0; i < agents.GetCount(); i++) co & [=] {
			Agent& agent = agents[i];
	        int equities_begin = agent.id * snap_count;
	        
	        // Check cursor
	        if (agent.cursor <= 0 || agent.cursor >= snap_count)
				agent.ResetEpoch();
			
	        for(int k = 0; k < 100; k++) {
	            Snapshot& cur_snap  = snaps[agent.cursor - 0];
				Snapshot& prev_snap = snaps[agent.cursor - 1];
				
				agent.timestep_actual--;
				
				agent.Main(cur_snap, prev_snap);
				
				// Get some diagnostic stats
				int j = equities_begin + agent.cursor;
				if (j >= 0 && j < agent_equities.GetCount())
					agent_equities[j] = agent.broker.AccountEquity();
				agent.cursor++;
				
				// Close all order at the end
				if (agent.cursor >= snap_count) {
					agent.ResetEpoch();
				}
	        }
	    };
	    co.Finish();
	}
	
	SetAgentsTraining(false);
	
	sys->WhenPopTask();
}

void AgentGroup::LoopAgentSignals(bool from_begin) {
	sys->WhenPushTask("Loop agent signals");
	
	int snap_count = snaps.GetCount();
	agent_equities.SetCount(agents.GetCount() * snap_count, 0.0);
	agent_equities_count = snap_count;
	
	
	SetAgentsTraining(false);
	
	if (from_begin) {
		for(int i = 0; i < agents.GetCount(); i++)
			agents[i].ResetEpoch();
		
		for(int i = 1; i < snap_count && running; i++) {
			CoWork co;
			for(int j = 0; j < agents.GetCount(); j++) co & [=] {
		        Agent& agent = agents[j];
				Snapshot& cur_snap  = snaps[agent.cursor - 0];
				Snapshot& prev_snap = snaps[agent.cursor - 1];
				
				agent.timestep_actual--;
				agent.Main(cur_snap, prev_snap);
				agent_equities[agent.id * snaps.GetCount() + agent.cursor] = agent.broker.AccountEquity();
				agent.cursor++;
		    };
		    co.Finish();
		}
	}
	else {
		while (running) {
			bool run_any = false;
			for(int i = 0; i < agents.GetCount(); i++) {
				if (agents[i].cursor < snap_count) {
					run_any = true;
					break;
				}
			}
			if (!run_any)
				break;
				
			CoWork co;
			for(int i = 0; i < agents.GetCount(); i++) co & [=] {
		        Agent& agent = agents[i];
				if (agent.cursor >= snaps.GetCount()) {
					sys->WhenPopTask();
		            return;
				}
		        Snapshot& cur_snap  = snaps[agent.cursor - 0];
				Snapshot& prev_snap = snaps[agent.cursor - 1];
				
				agent.timestep_actual--;
				agent.Main(cur_snap, prev_snap);
				agent_equities[agent.id * snaps.GetCount() + agent.cursor] = agent.broker.AccountEquity();
				agent.cursor++;
		    };
		    co.Finish();
		}
	}
	
	sys->WhenPopTask();
}

void AgentGroup::TrainJoiners() {
	sys->WhenPushTask("Train joiners");
	
	RefreshAgentEpsilon();
	RefreshJoinerEpsilon();
	LoopAgentSignals(true);
	if (GetAverageJoinerIterations() >= 1.0)
		LoopJoinerSignals(true);
	SetAgentsTraining(false);
	SetJoinersTraining(true);
	
	int snap_count = snaps.GetCount();
	joiner_equities.SetCount(joiners.GetCount() * snap_count, 0.0);
	joiner_equities_count = snap_count;
	
	
	double prev_aviter = GetAverageJoinerIterations();
	
	TimeStop ts;
	int64 total_elapsed = 0;
	for (int64 iter = 0; phase == PHASE_JOINER && running; iter++) {
		
		// Randomize input  values
		double aviter = GetAverageJoinerIterations();
		if (aviter >= prev_aviter + 50000.0) {
			LoopAgentSignals(true);
			LoopJoinerSignals(true);
			SetJoinersTraining(true);
			prev_aviter = aviter;
		}
		
		// Change snapshot area, if needed, sometimes
		if (iter > 100) {
			RefreshJoinerEpsilon();
			total_elapsed += ts.Elapsed();
			ts.Reset();
			iter = 0;
			if (total_elapsed > 5*60*1000) {
				sys->WhenPopTask();
				return; // call TrainJoiners again to UpdateAmpSnaps safely
			}
			
			// Change phase to real mode eventually
			if (aviter >= JOINER_PHASE_ITER_LIMIT) {
				phase = PHASE_REAL;
				StoreThis();
				break;
			}
		}
		
		
		CoWork co;
		for(int i = 0; i < joiners.GetCount(); i++) co & [=] {
	        Joiner& joiner = joiners[i];
	        int snap_count = snaps.GetCount();
	        int equities_begin = joiner.id * snap_count;
	        
	        // Check cursor
	        if (joiner.cursor <= 0 || joiner.cursor >= snap_count)
				joiner.ResetEpoch();
			
	        for(int i = 0; i < 100; i++) {
				joiner.timestep_actual--;
				
		        joiner.Main(snaps);
				
				// Get some diagnostic stats
				joiner_equities[equities_begin + joiner.cursor] = joiner.broker.PartialEquity();
				joiner.cursor++;
				
				// Close all order at the end
				if (joiner.cursor >= snap_count) {
					joiner.ResetEpoch();
				}
	        }
	    };
	    co.Finish();
	}
	
	SetJoinersTraining(false);
	
	sys->WhenPopTask();
}

void AgentGroup::LoopJoinerSignals(bool from_begin) {
	sys->WhenPushTask("Loop joiner signals");
	
	int snap_count = snaps.GetCount();
	joiner_equities.SetCount(joiners.GetCount() * snap_count, 0.0);
	joiner_equities_count = snap_count;
	SetJoinersTraining(false);
	
	
	if (from_begin) {
		for(int i = 0; i < joiners.GetCount(); i++)
			joiners[i].ResetEpoch();
		
		for(int i = 1; i < snap_count && running; i++) {
			CoWork co;
			for(int j = 0; j < joiners.GetCount(); j++) co & [=] {
		        Joiner& joiner = joiners[j];
				Snapshot& cur_snap  = snaps[joiner.cursor - 0];
				Snapshot& prev_snap = snaps[joiner.cursor - 1];
				
				joiner.timestep_actual--;
				joiner.Main(snaps);
				joiner_equities[joiner.id * snaps.GetCount() + joiner.cursor] = joiner.broker.PartialEquity();
				joiner.cursor++;
		    };
		    co.Finish();
		}
	}
	else {
		
		for (int iter = 0; running; iter++) {
			bool run_any = false;
			for(int i = 0; i < joiners.GetCount(); i++) {
				if (joiners[i].cursor < snap_count) {
					run_any = true;
					break;
				}
			}
			if (!run_any)
				break;
			
			CoWork co;
			for(int i = 0; i < joiners.GetCount(); i++) co & [=] {
		        Joiner& joiner = joiners[i];
				if (joiner.cursor >= snaps.GetCount()) {
					sys->WhenPopTask();
		            return;
				}
		        Snapshot& cur_snap  = snaps[joiner.cursor - 0];
				Snapshot& prev_snap = snaps[joiner.cursor - 1];
				
				joiner.timestep_actual--;
				joiner.Main(snaps);
				joiner_equities[joiner.id * snaps.GetCount() + joiner.cursor] = joiner.broker.PartialEquity();
				joiner.cursor++;
		    };
		    co.Finish();
		}
	}
	
	sys->WhenPopTask();
}

void AgentGroup::MainReal() {
	sys->WhenPushTask("Real");
	
	MetaTrader& mt = GetMetaTrader();
    Time time = mt.GetTime();
	int wday = DayOfWeek(time);
	int shift = sys->GetShiftFromTimeTf(time, main_tf);
	
	
	// Loop agents and joiners without random events (epsilon = 0)
	SetAgentEpsilon(0.0);
	SetJoinerEpsilon(0.0);
	if (prev_shift <= 0) {
		LoopAgentSignals(true);
		LoopJoinerSignals(true);
	}
	SetAgentsTraining(false);
	SetJoinersTraining(false);
	
	
	if (prev_shift != shift) {
		if (wday == 0 || wday == 6) {
			// Do nothing
			prev_shift = shift;
		} else {
			sys->WhenInfo("Shift changed");
			

			// Updates latest snapshot and signals
			sys->SetEnd(mt.GetTime());
			RefreshSnapshots();
			LoopAgentSignals(false);
			LoopJoinerSignals(false);
			
			int last_snap_shift = snaps.Top().shift;
			if (shift != last_snap_shift) {
				WhenError(Format("Current shift doesn't match the lastest snapshot shift (%d != %d)", shift, last_snap_shift));
				sys->WhenPopTask();
				return;
			}
			
			
			// Forced askbid data download
			DataBridgeCommon& common = GetDataBridgeCommon();
			common.DownloadAskBid();
			common.RefreshAskBidData(true);
			
			
			// Refresh databridges
			ProcessDataBridgeQueue();
			
			
			// Reset signals
			if (realtime_count == 0) {
				for(int i = 0; i < mt.GetSymbolCount(); i++)
					mt.SetSignal(i, 0);
			}
			realtime_count++;
			
			
			// Use best group to set broker signals
			WhenInfo("Looping agents until latest snapshot");
			bool succ = PutLatest(mt, snaps);
			
			
			// Print info
			String sigstr = "Signals ";
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				if (i) sigstr << ",";
				sigstr << mt.GetSignal(sym_ids[i]);
			}
			WhenInfo(sigstr);
			
			
			// Notify about successful signals
			if (succ) {
				prev_shift = shift;
				
				sys->WhenRealtimeUpdate();
			}
		}
	}
	
	// Check for market closing (weekend and holidays)
	else {
		Time after_hour = time + 60*60;
		int wday_after_hour = DayOfWeek(after_hour);
		if (wday == 5 && wday_after_hour == 6) {
			sys->WhenInfo("Closing all orders before market break");
			for(int i = 0; i < mt.GetSymbolCount(); i++) {
				mt.SetSignal(i, 0);
				mt.SetSignalFreeze(i, false);
			}
			mt.SignalOrders(true);
		}
		
		if (wday != 0 && wday != 6 && last_datagather.Elapsed() >= 1*60*1000) {
			Data();
			last_datagather.Reset();
		}
	}
	
	sys->WhenPopTask();
}

Joiner* AgentGroup::GetBestJoiner(int sym_id) {
	double best_dd = 100;
	int best_i = 0;
	for(int i = 0; i < joiners.GetCount(); i++) {
		Joiner& j = joiners[i];
		if (j.sym_id != sym_id) continue;
		double dd = j.last_drawdown;
		if (dd < best_dd) {
			best_dd = dd;
			best_i = i;
		}
	}
	return &joiners[best_i];
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
	last_store.Reset();
}

void AgentGroup::Serialize(Stream& s) {
	s % agents % joiners % indi_ids % created % phase;
}

void AgentGroup::RefreshSnapshots() {
	sys->WhenPushTask("Refreshing snapshots");
	
	ASSERT(buf_count != 0);
	ASSERT(sym_ids.GetCount() != 0);
	
	TimeStop ts;
	ProcessWorkQueue();
	
	int total_bars = sys->GetCountTf(main_tf);
	int bars = total_bars - data_begin;
	
	
	ASSERT(bars > 0);
	snaps.Reserve(bars + (60 - (bars % 60)));
	for(; counted_bars < bars; counted_bars++) {
		int shift = counted_bars + data_begin;
		Time t = sys->GetTimeTf(main_tf, shift);
		int wday = DayOfWeek(t);
		
		// Skip weekend
		if (wday == 0 || wday == 6) continue;
		
		Seek(snaps.Add(), shift);
	}
	ASSERT(snaps.GetCount() > 0);
	
	LOG("Refreshing snapshots took " << ts.ToString());
	sys->WhenPopTask();
}

void AgentGroup::Progress(int actual, int total, String desc) {
	ManagerLoader& loader = GetManagerLoader();
	loader.PostProgress(actual, total, desc);
	loader.PostSubProgress(0, 1);
	
	if (actual < total) {
		if (actual > 0)
			sys->WhenPopTask();
		sys->WhenPushTask(desc);
	}
	else if (actual == total && actual > 0) {
		sys->WhenPopTask();
	}
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
		
		
		// Check that args match to declaration
		#ifdef flagDEBUG
		ArgChanger ac;
		ac.SetLoading();
		ci.core->IO(ac);
		ASSERT(ac.args.GetCount() >= ci.args.GetCount());
		for(int i = 0; i < ci.args.GetCount(); i++) {
			int a = ac.args[i];
			int b = ci.args[i];
			if (a != b) {
				LOG(Format("%d != %d", a, b));
			}
			ASSERT(a == b);
		}
		#endif
		
		
		int buf_begin_id = bufout_ids.Find(hash);
		if (buf_begin_id == -1)
			continue;
		
		int buf_begin = bufout_ids[buf_begin_id];
		
		//LOG(i << ": " << ci.factory << ", " << sym_id << ", " << (int64)hash << ", " << buf_begin);
		
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
	sys->WhenPushTask("Processing work queue");
	
	work_lock.Enter();
	
	for(int i = 0; i < work_queue.GetCount(); i++) {
		SubProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
	
	work_lock.Leave();
	
	sys->WhenPopTask();
}

void AgentGroup::ProcessDataBridgeQueue() {
	sys->WhenPushTask("Processing databridge work queue");
	
	work_lock.Enter();
	
	for(int i = 0; i < db_queue.GetCount(); i++) {
		SubProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}
	
	work_lock.Leave();
	
	sys->WhenPopTask();
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
	
	
	// Shift
	snap.shift = shift;
	
	
	// Time sensor
	snap.year_timesensor = (month * 31.0 + day) / 372.0;
	snap.week_timesensor = ((wday * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0);
	snap.day_timesensor  = (hour * 60 + minute) / (24.0 * 60.0);
	
	
	// Refresh values (tf / sym / value)
	int k = 0;
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		Vector<ConstBuffer*>& indi_buffers = value_buffers[i];
		for(int j = 0; j < buf_count; j++) {
			ConstBuffer& src = *indi_buffers[j];
			double d = src.GetUnsafe(shift);
			double pos, neg;
			if (d > 0) {
				pos = 1.0 - d;
				neg = 1.0;
			} else {
				pos = 1.0;
				neg = 1.0 + d;
			}
			snap.sensor[k++] = pos;
			snap.sensor[k++] = neg;
		}
		
		DataBridge& db = *dynamic_cast<DataBridge*>(databridge_cores[sym_ids[i]]);
		double open = db.GetBuffer(0).GetUnsafe(shift);
		snap.open[i] = open;
	}
	ASSERT(k == SENSOR_SIZE);
	
	
	// Reset signals
	for(int i = 0; i < AGENT_SIGNAL_SIZE; i++)
		snap.agent_signal[i] = 1.0;
	
	for(int i = 0; i < JOINER_SIGNAL_SIZE; i++)
		snap.joiner_signal[i] = 1.0;
	
	for(int i = 0; i < TRAINEE_COUNT; i++)
		snap.agent_broker_signal[i] = 0;
	
	for(int i = 0; i < TRAINEE_COUNT; i++)
		snap.joiner_broker_signal[i] = 0;
	
	return true;
}

void AgentGroup::CreateAgents() {
	ASSERT(buf_count > 0);
	ASSERT(sym_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	agents.SetCount(TRAINEE_COUNT);
	int j = 0;
	for(int group_id = 0; group_id < GROUP_COUNT; group_id++) {
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			const Symbol& sym = mt.GetSymbol(sym_ids[i]);
			Agent& a = agents[j];
			a.id = j;
			a.sym_id = i;
			a.sym = sym_ids[i];
			a.group_id = group_id;
			a.Create();
			j++;
		}
	}
	ASSERT(j == agents.GetCount());
}

void AgentGroup::CreateJoiners() {
	ASSERT(buf_count > 0);
	ASSERT(sym_count > 0);
	
	MetaTrader& mt = GetMetaTrader();
	joiners.SetCount(TRAINEE_COUNT);
	int k = 0;
	for(int group_id = 0; group_id < GROUP_COUNT; group_id++) {
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			const Symbol& sym = mt.GetSymbol(sym_ids[i]);
			Joiner& j = joiners[k];
			j.id = k;
			j.sym_id = i;
			j.sym = sym_ids[i];
			j.group_id = group_id;
			j.Create();
			k++;
		}
	}
	ASSERT(k == joiners.GetCount());
}

double AgentGroup::GetAverageAgentDrawdown() {
	double dd = 0;
	for(int i = 0; i < agents.GetCount(); i++)
		dd += agents[i].last_drawdown;
	return dd / agents.GetCount();
}

double AgentGroup::GetAverageAgentIterations() {
	double dd = 0;
	for(int i = 0; i < agents.GetCount(); i++)
		dd += agents[i].iter;
	return dd / agents.GetCount();
}

double AgentGroup::GetAverageJoinerDrawdown() {
	double dd = 0;
	for(int i = 0; i < joiners.GetCount(); i++)
		dd += joiners[i].last_drawdown;
	return dd / joiners.GetCount();
}

double AgentGroup::GetAverageJoinerIterations() {
	double dd = 0;
	for(int i = 0; i < joiners.GetCount(); i++)
		dd += joiners[i].iter;
	return dd / joiners.GetCount();
}

double AgentGroup::GetAverageJoinerEpochs() {
	double dd = 0;
	for(int i = 0; i < joiners.GetCount(); i++)
		dd += joiners[i].result_count;
	return dd / joiners.GetCount();
}

void AgentGroup::RefreshAgentEpsilon() {
	double iters = GetAverageAgentIterations();
	int level = iters / AGENT_EPS_ITERS_STEP;
	if (level <= 0)
		agent_epsilon = 0.20;
	else if (level == 1)
		agent_epsilon = 0.05;
	else if (level == 2)
		agent_epsilon = 0.02;
	else if (level >= 3)
		agent_epsilon = 0.01;
	SetAgentEpsilon(agent_epsilon);
}

void AgentGroup::RefreshJoinerEpsilon() {
	double iters = GetAverageJoinerIterations();
	int level = iters / JOINER_EPS_ITERS_STEP;
	if (level <= 0)
		joiner_epsilon = 0.20;
	else if (level == 1)
		joiner_epsilon = 0.05;
	else if (level == 2)
		joiner_epsilon = 0.02;
	else if (level >= 3)
		joiner_epsilon = 0.01;
	SetJoinerEpsilon(joiner_epsilon);
}

void AgentGroup::SetAgentEpsilon(double epsilon) {
	this->agent_epsilon = epsilon;
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].dqn.SetEpsilon(epsilon);
}

void AgentGroup::SetJoinerEpsilon(double epsilon) {
	this->joiner_epsilon = epsilon;
	for(int i = 0; i < joiners.GetCount(); i++)
		joiners[i].dqn.SetEpsilon(epsilon);
}

void AgentGroup::SetFreeMarginLevel(double fmlevel) {
	this->joiner_fmlevel = fmlevel;
	for(int i = 0; i < joiners.GetCount(); i++)
		joiners[i].SetFreeMarginLevel(fmlevel);
}

void AgentGroup::InitAgent(Agent& a) {
	MetaTrader& mt = GetMetaTrader();
	
	DataBridge* db = dynamic_cast<DataBridge*>(databridge_cores[a.sym]);
	
	const Symbol& symbol = mt.GetSymbol(a.sym);
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
	
	
	a.spread_points = db->GetAverageSpread() * db->GetPoint();
	ASSERT(a.spread_points > 0.0);
	
	a.Init();
}

void AgentGroup::InitJoiner(Joiner& j) {
	MetaTrader& mt = GetMetaTrader();
	
	for(int i = 0; i < SYM_COUNT; i++) {
		int sym = sym_ids[i];
		
		DataBridge* db = dynamic_cast<DataBridge*>(databridge_cores[sym]);
		
		const Symbol& symbol = mt.GetSymbol(sym);
		if (symbol.proxy_id != -1) {
			int k = sym_ids.Find(symbol.proxy_id);
			ASSERT(k != -1);
			j.proxy_id[i] = k;
			j.proxy_base_mul[i] = symbol.base_mul;
		} else {
			j.proxy_id[i] = -1;
			j.proxy_base_mul[i] = 0;
		}
		j.spread_points[i] = db->GetAverageSpread() * db->GetPoint();
		ASSERT(j.spread_points[i] > 0.0);
	}
	
	j.begin_equity = mt.AccountEquity();
	j.leverage = 1000;
	
	j.Init();
}

bool AgentGroup::PutLatest(Brokerage& broker, Vector<Snapshot>& snaps) {
	System& sys = GetSystem();
	sys.WhenPushTask("Putting latest signals");
	
	
	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);
	if (mt) {
		mt->Data();
		broker.RefreshLimits();
	} else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}
	
	
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		Joiner& j = *GetBestJoiner(i);
		j.cursor = snaps.GetCount() - 1;
		j.Forward(snaps);
		int sym = sym_ids[i];
		int sig = j.signal;
		ASSERT(j.sym == sym);
		if (sig == broker.GetSignal(sym) && sig != 0)
			broker.SetSignalFreeze(sym, true);
		else {
			broker.SetSignal(sym, sig);
			broker.SetSignalFreeze(sym, false);
		}
	}
	
	broker.SetFreeMarginLevel(joiner_fmlevel);
	broker.SignalOrders(true);
	
	sys.WhenPopTask();
	
	return true;
}

void AgentGroup::Data() {
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

	FileAppend fout(ConfigFile("agentgroup.log"));
	int64 begin_pos = fout.GetSize();
	int size = 0;
	fout.Put(&size, sizeof(int));
	fout % file_version % balance % equity % time % signals % orders;
	int64 end_pos = fout.GetSize();
	size = end_pos - begin_pos - sizeof(int);
	fout.Seek(begin_pos);
	fout.Put(&size, sizeof(int));
}

}
