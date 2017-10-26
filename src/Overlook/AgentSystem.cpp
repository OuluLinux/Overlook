#if 0

#include "Overlook.h"

namespace Overlook {

bool reset_signals;
bool reset_amps;
bool reset_fuse;

AgentSystem::AgentSystem(System* sys) : sys(sys) {
	running = false;
	stopped = true;
	
	// Instrument, spread pips

#ifndef flagHAVE_ALLSYM

	allowed_symbols.Add("EURUSD", 3); // 0.26
	allowed_symbols.Add("GBPUSD", 3); // 0.23
	allowed_symbols.Add("USDJPY", 3); // 0.27
	allowed_symbols.Add("USDCAD", 3); // 0.24
	allowed_symbols.Add("EURJPY", 3); // 0.23
	allowed_symbols.Add("EURCHF", 3); // 0.26
	
#else

	// also AUD,NZD,CHF included
	allowed_symbols.Add("EURUSD",  3); // lts
	allowed_symbols.Add("GBPUSD",  3); // lts
	allowed_symbols.Add("USDJPY",  3); // lts
	allowed_symbols.Add("USDCHF",  3); // lts
	allowed_symbols.Add("USDCAD",  3); // lts
	allowed_symbols.Add("AUDUSD",  3); // lts
	allowed_symbols.Add("NZDUSD",  3); // lts
	allowed_symbols.Add("EURJPY",  3); // lts
	allowed_symbols.Add("EURCHF",  3); // lts
	allowed_symbols.Add("EURGBP",  3); // lts
	allowed_symbols.Add("AUDCAD", 10); // lts
	allowed_symbols.Add("AUDJPY", 10); // lts
	allowed_symbols.Add("CADJPY", 10); // lts
	allowed_symbols.Add("CHFJPY", 10); // lts
	//allowed_symbols.Add("NZDCAD", 10);
	//allowed_symbols.Add("NZDCHF", 10);
	allowed_symbols.Add("EURAUD",  7); // lts
	allowed_symbols.Add("GBPCHF",  7); // lts
	allowed_symbols.Add("GBPJPY",  7); // lts
	allowed_symbols.Add("AUDNZD", 12); // lts
	allowed_symbols.Add("EURCAD", 12); // lts
	//allowed_symbols.Add("GBPAUD", 12);
	//allowed_symbols.Add("GBPCAD", 12);
	//allowed_symbols.Add("GBPNZD", 12);

#endif


	// SKIP because of no long-term data available:
	//  - AUDCHF
	//  - CADCHF
	//  - EURNZD
	//  - GBPJPY
	//  - NZDJPY

	ASSERT(allowed_symbols.GetCount() == SYM_COUNT);

	created = GetSysTime();
}

AgentSystem::~AgentSystem() {
	Stop();
}

void AgentSystem::Init() {
	ASSERT(sys);

	free_margin_level = FMLEVEL;

	WhenInfo  << Proxy(sys->WhenInfo);
	WhenError << Proxy(sys->WhenError);

	ManagerLoader& loader = GetManagerLoader();
	Thread::Start(THISBACK(InitThread));
	loader.Run();
}

void AgentSystem::InitThread() {
	Progress(0, 6, "Refreshing work queue");
	MetaTrader& mt = GetMetaTrader();
	const Vector<Price>& askbid = mt._GetAskBid();
	String not_found;

	for (int j = 0; j < allowed_symbols.GetCount(); j++) {
		const String& allowed_sym = allowed_symbols.GetKey(j);
		bool found = false;

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& sym = mt.GetSymbol(i);

			if (sym.IsForex() && (sym.name.Left(6)) == allowed_sym) {
				double base_spread = 1000.0 * (askbid[i].ask / askbid[i].bid - 1.0);

				if (base_spread >= 0.5) {
					Cout() << "Warning! Too much spread: " << sym.name << " (" << base_spread << ")" << "\n";
				}

				sym_ids.Add(i);

				spread_points.Add(allowed_symbols[j] * sym.point);
				found = true;
				break;
			}
		}

		if (!found)
			not_found << allowed_sym << " ";
	}

	sym_count = sym_ids.GetCount();

	ASSERTUSER_(sym_count == SYM_COUNT, "All required forex instruments weren't shown in the mt4: " + not_found);

	main_tf = sys->FindPeriod(1);
	ASSERT(main_tf != -1);

	begin_equity = mt.AccountEquity();

	int stoch_id = sys->Find<StochasticOscillator>();
	int osma_id = sys->Find<OsMA>();
	int volav_id = sys->Find<VolatilityAverage>();
	ASSERT(stoch_id != -1);
	ASSERT(osma_id != -1);
	ASSERT(volav_id != -1);

	indi_ids.Clear();
	#ifndef flagFAST
	indi_ids.Add().Set(volav_id).AddArg(5);
	indi_ids.Add().Set(volav_id).AddArg(15);
	indi_ids.Add().Set(volav_id).AddArg(60);
	indi_ids.Add().Set(volav_id).AddArg(240);
	indi_ids.Add().Set(volav_id).AddArg(1440);
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
	#else
	for(int i = 0; i < 5; i++) {
		// tfs: 4, 8, 16, 32, 64
		int period = MEASURE_PERIOD(i * 2 - 3);
		indi_ids.Add().Set(volav_id).AddArg(period);
		indi_ids.Add().Set(osma_id).AddArg(period).AddArg(period*2).AddArg(period);
		indi_ids.Add().Set(stoch_id).AddArg(period);
	}
	#endif

	RefreshWorkQueue();


	Progress(1, 6, "Refreshing data source and indicators");
	ProcessWorkQueue();
	ProcessDataBridgeQueue();
	ProcessLabelQueue();


	Progress(2, 6, "Refreshing pointers of data sources");
	ResetValueBuffers();


	Progress(3, 6, "Reseting snapshots");
	RefreshSnapshots();


	Progress(5, 6, "Initializing agents");
	InitBrokerValues();

	if (groups.GetCount() == 0) {
		CreateAgents();
	}
	else {
		for (int i = 0; i < groups.GetCount(); i++) {
			AgentGroup& ag = groups[i];
			ASSERT(ag.agents.GetCount() == SYM_COUNT);

			for (int j = 0; j < sym_ids.GetCount(); j++)
				ag.agents[j].sym = sym_ids[ag.agents[j].sym_id];
		}
	}

	if (reset_signals) {
		phase = Upp::min(phase, (int)PHASE_SIGNAL_TRAINING);

		for (int i = 0; i < groups.GetCount(); i++)
			for (int j = 0; j < sym_ids.GetCount(); j++)
				groups[i].agents[j].sig.DeepCreate();

		reset_amps = true;
	}

	if (reset_amps) {
		phase = Upp::min(phase, (int)PHASE_AMP_TRAINING);

		for (int i = 0; i < groups.GetCount(); i++)
			for (int j = 0; j < sym_ids.GetCount(); j++)
				groups[i].agents[j].amp.DeepCreate();
		
		reset_fuse = true;
	}

	if (reset_fuse) {
		phase = Upp::min(phase, (int)PHASE_FUSE_TRAINING);
		fuse.DeepCreate();
	}
	
	InitAgents();

	Progress(6, 6, "Complete");
}

void AgentSystem::ResetAgents() {
	groups.Clear();
	CreateAgents();
	InitAgents();
}

void AgentSystem::InitAgents() {
	for (int i = 0; i < GROUP_COUNT; i++) {
		AgentGroup& ag = groups[i];
		ag.sys = this;

		for (int j = 0; j < SYM_COUNT; j++) {
			Agent& a = ag.agents[j];
			a.group = &ag;
			a.Init();
		}
	}
	fuse.sys = this;
	fuse.Init();
	
	SetFreeMarginLevel(FMLEVEL);
}

void AgentSystem::Start() {
	Stop();
	running = true;
	stopped = false;
	Thread::Start(THISBACK(Main));
}

void AgentSystem::Stop() {
	running = false;

	while (stopped != true)
		Sleep(100);
}

void AgentSystem::SetAgentsTraining(bool b) {
	for (int i = 0; i < groups.GetCount(); i++)
		for (int j = 0; j < groups[i].agents.GetCount(); j++)
			groups[i].agents[j].is_training = b;
	fuse.is_training = b;
}

void AgentSystem::SetSingleFixedBroker(int sym_id, SingleFixedSimBroker& broker) {
	broker.sym_id			= sym_id;
	broker.begin_equity		= Upp::max(10000.0, begin_equity);
	broker.spread_points	= spread_points[sym_id];
	broker.proxy_id			= proxy_id[sym_id];
	broker.proxy_base_mul	= proxy_base_mul[sym_id];
}

void AgentSystem::SetFixedBroker(int sym_id, FixedSimBroker& broker) {
	for (int i = 0; i < SYM_COUNT; i++) {
		broker.spread_points[i] = spread_points[i];
		broker.proxy_id[i] = proxy_id[i];
		broker.proxy_base_mul[i] = proxy_base_mul[i];
	}

	broker.begin_equity = Upp::max(10000.0, begin_equity);
	broker.leverage = leverage;
	broker.free_margin_level = free_margin_level;
	broker.part_sym_id = sym_id;
}

void AgentSystem::Main() {
	if (groups.GetCount() == 0 || sym_ids.IsEmpty() || indi_ids.IsEmpty())
		return;

	RefreshSnapshots();

	while (running) {
		ReduceExperienceMemory(phase);

		if (phase < PHASE_FUSE_TRAINING) {
			TrainAgents(phase);
		}
		else
		if (phase == PHASE_FUSE_TRAINING) {
			TrainFuse();
		}
		else
		if (phase == PHASE_REAL) {
			MainReal();
			Sleep(1000);
		}
		else
			Sleep(100);
	}


	stopped = true;
}

void AgentSystem::ReduceExperienceMemory(int phase) {
	for (int p = 0; p < phase; p++) {
		for (int i = 0; i < GROUP_COUNT; i++) {
			for (int j = 0; j < SYM_COUNT; j++) {
				switch (p) {

				case PHASE_SIGNAL_TRAINING:
					groups[i].agents[j].sig.dqn.ClearExperience();
					break;

				case PHASE_AMP_TRAINING:
					groups[i].agents[j].amp.dqn.ClearExperience();
					break;

				case PHASE_REAL:
					break;
				}
			}
		}
	}
}

void AgentSystem::RefreshSnapEquities() {
	for (int i = 0; i < groups.GetCount(); i++)
		for (int j = 0; j < groups[i].agents.GetCount(); j++)
			groups[i].agents[j].RefreshSnapEquities();
	fuse.RefreshSnapEquities();
}

double AgentSystem::GetPhaseIters(int phase) {
	if (phase == PHASE_SIGNAL_TRAINING)
		return GetAverageSignalIterations();

	if (phase == PHASE_AMP_TRAINING)
		return GetAverageAmpIterations();

	return 0;
}

void AgentSystem::TrainAgents(int phase) {
	sys->WhenPushTask("Agent " + String(phase == PHASE_SIGNAL_TRAINING ? "signal" : "amp"));

	// Update configurations
	RefreshSnapshots();
	RefreshSnapEquities();
	RefreshAgentEpsilon(phase);
	RefreshLearningRate(phase);

	for (int i = 0; i < phase; i++)
		LoopAgentSignals(i);
	
	SetAgentsTraining(true);
	
	
	// Create processing loop to ensure that every agent gets enough training
	typedef Tuple2<int, int> AgentPos;
	Vector<AgentPos> proc_agents;
	
	for (int i = 0; i < GROUP_COUNT; i++) {
		if (result_sectors[i].pnd.x.count == 0)
			continue;
		
		for (int j = 0; j < SYM_COUNT; j++) {
			Agent& agent = groups[i].agents[j];
			
			if (!agent.IsTrained(phase))
				proc_agents.Add(AgentPos(i, j));
		}
	}
	
	
	// Main loop
	int prev_av_iters = GetAverageIterations(phase);
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());

	for (int64 iter = 0; running; iter++) {

		if (iter % 30 == 0) {

			// Update some attributes as times go by
			RefreshAgentEpsilon(phase);
			RefreshLearningRate(phase);


			// Change snapshot area, if needed, sometimes
			int av_iters = GetAverageIterations(phase);

			if (av_iters - prev_av_iters >= BREAK_INTERVAL_ITERS) {
				StoreThis();
				break; // call TrainAgents again to RefreshSnapshots safely
			}


			// Change to the next phase eventually
			for (int i = 0; i < proc_agents.GetCount(); i++) {
				const AgentPos& apos = proc_agents[i];
				Agent& agent = groups[apos.a].agents[apos.b];

				if (agent.IsTrained(phase)) {
					proc_agents.Remove(i);
					i--;
				}
			}

			if (proc_agents.IsEmpty()) {
				this->phase++;
				StoreThis();
				break;
			}

		}


		// Run agents in CoWork threads
		// This is the main point of threading in the Overlook.
		// If GPGPU cannot be used without big changes at this point, it won't be used.
		// C++ AMP almost worked, but threads took too much time...

		for (int i = 0; i < proc_agents.GetCount(); i++) {
			const AgentPos& apos = proc_agents[i];
			int gid = apos.a;
			int aid = apos.b;
			
			co & [=] {
				Agent& agent = groups[gid].agents[aid];

				// Check cursor
				int cursor = agent.GetCursor(phase);

				if (cursor <= 0 || cursor >= snaps.GetCount())
					agent.ResetEpoch(phase);

				int64 end = agent.GetIter(phase) + 100;
				
				int begin_iter = agent.GetIter(phase);
				while (agent.GetIter(phase) < end && running) {
					agent.Main(phase, snaps);

					// Close all order at the end

					if (agent.GetCursor(phase) >= snaps.GetCount()) {
						agent.ResetEpoch(phase);
						if (begin_iter == agent.GetIter(phase)) {
							break;
						}
					}
				}
			};
		}

		co.Finish();
	}

	SetAgentsTraining(false);

	sys->WhenPopTask();
}

void AgentSystem::TrainFuse() {
	sys->WhenPushTask("Fuse training");
	
	
	// Update configurations
	RefreshSnapshots();
	RefreshSnapEquities();
	SetSignalEpsilon(0.01);
	SetAmpEpsilon(0.01);

	for (int i = 0; i < phase; i++)
		LoopAgentSignals(i);

	SetAgentsTraining(true);
	
	int prev_av_iters = GetAverageIterations(phase);

	for (int64 iter = 0; running; iter++) {

		if (iter % 300 == 0) {

			// Update some attributes as times go by
			RefreshAgentEpsilon(phase);
			RefreshLearningRate(phase);
			
			
			// Change snapshot area, if needed, sometimes
			int av_iters = GetAverageIterations(phase);
			if (av_iters - prev_av_iters >= BREAK_INTERVAL_ITERS) {
				StoreThis();
				break; // call TrainAgents again to RefreshSnapshots safely
			}
			
			
			if (fuse.IsTrained()) {
				this->phase++;
				StoreThis();
				break;
			}
		}
		

		// Check cursor
		int cursor = fuse.GetCursor();

		if (cursor <= 0 || cursor >= snaps.GetCount())
			fuse.ResetEpoch();

		int64 end = fuse.GetIter() + 100;
		
		int begin_iter = fuse.GetIter();
		while (fuse.GetIter() < end && running) {
			fuse.Main(snaps);

			// Close all order at the end
			if (fuse.GetCursor() >= snaps.GetCount()) {
				fuse.ResetEpoch();
				if (begin_iter == fuse.GetIter())
					break;
			}
		}
		
	}
	
	SetAgentsTraining(false);
	
	sys->WhenPopTask();
}

void AgentSystem::LoopAgentSignals(int phase) {
	sys->WhenPushTask("Loop agent signals, phase " + IntStr(phase));

	// Prepare loop without training
	RefreshSnapEquities();
	SetAgentsTraining(false);


	// Process Agent::Main in CoWork threads
	if (phase < PHASE_FUSE_TRAINING) {
		for (int i = 0; i < GROUP_COUNT; i++)
			for (int j = 0; j < SYM_COUNT; j++)
				groups[i].agents[j].ResetEpoch(phase);
		
		CoWork co;
		co.SetPoolSize(GetUsedCpuCores());
		for (int i = 1; i < snaps.GetCount() && running; i++) {
			for (int j = 0; j < GROUP_COUNT; j++) {
				for (int k = 0; k < SYM_COUNT; k++) {
					co & [=] {
						groups[j].agents[k].Main(phase, snaps);
					};
				}
			}
	
			co.Finish();
		}
	}
	else if (phase == PHASE_FUSE_TRAINING) {
		fuse.ResetEpoch();
		for (int i = 1; i < snaps.GetCount() && running; i++)
			fuse.Main(snaps);
	}

	sys->WhenPopTask();
}

void AgentSystem::LoopAgentSignalsAll(bool from_begin) {
	sys->WhenPushTask("Loop agent signals");

	// Prepare loop without training
	RefreshSnapEquities();
	SetAgentsTraining(false);

	// Process Agent::Main in CoWork threads
	CoWork co;
	co.SetPoolSize(GetUsedCpuCores());

	if (from_begin) {
		for (int i = 0; i < GROUP_COUNT; i++)
			for (int j = 0; j < SYM_COUNT; j++)
				groups[i].agents[j].ResetEpochAll();
		fuse.ResetEpoch();
		
		for (int i = 1; i < snaps.GetCount() && running; i++) {
			for (int phase = 0; phase < PHASE_REAL; phase++) {
				if (phase < PHASE_FUSE_TRAINING) {
					for (int j = 0; j < GROUP_COUNT; j++) {
						for (int k = 0; k < SYM_COUNT; k++) {
							co & [=] {
								groups[j].agents[k].Main(phase, snaps);
							};
						}
					}
					co.Finish();
				}
				else if (phase == PHASE_FUSE_TRAINING) {
					fuse.Main(snaps);
				}
			}
		}
	}

	else {
		for (int phase = 0; phase < PHASE_REAL; phase++) {
			if (phase < PHASE_FUSE_TRAINING) {
				while (running) {
					bool run_any = false;
	
					for (int i = 0; i < GROUP_COUNT; i++) {
						for (int j = 0; j < SYM_COUNT; j++) {
							if (groups[i].agents[j].GetCursor(phase) < snaps.GetCount()) {
								run_any = true;
								break;
							}
						}
					}
	
					if (!run_any)
						break;
	
					for (int i = 0; i < GROUP_COUNT; i++) {
						for (int j = 0; j < SYM_COUNT; j++) {
							co & [=] {
								Agent& agent = groups[i].agents[j];
		
								if (agent.GetCursor(phase) < snaps.GetCount())
									agent.Main(phase, snaps);
							};
						}
					}
	
					co.Finish();
				}
			}
			else if (phase == PHASE_FUSE_TRAINING) {
				while (fuse.GetCursor() < snaps.GetCount() && running)
					fuse.Main(snaps);
			}
		}
	}

	sys->WhenPopTask();
}

void AgentSystem::MainReal() {
	sys->WhenPushTask("Real");

	MetaTrader& mt		= GetMetaTrader();
	Time time			= mt.GetTime();
	int wday			= DayOfWeek(time);
	int shift			= sys->GetShiftFromTimeTf(time, main_tf);
	Time after_hour		= time + 60 * 60;
	int wday_after_hour	= DayOfWeek(after_hour);


	// Loop agents and joiners without random events (epsilon = 0)
	SetSignalEpsilon(0.0);
	SetAmpEpsilon(0.0);
	SetFuseEpsilon(0.0);

	if (prev_shift <= 0) {
		LoopAgentSignalsAll(true);
		prev_shift++; // never again
	}

	SetAgentsTraining(false);
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && time.hour < 4)) {
		// Do nothing
		prev_shift = shift;
		
		// Retrain agents at weekends
		if (wday == 0 && trigger_retrain) {
			trigger_retrain = false;
			phase = 0;
			ResetAgents();
		}
	}
	
	// Check for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_hour == 6) {
		sys->WhenInfo("Closing all orders before market break");

		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}

		mt.SignalOrders(true);
	}
	
	else if (prev_shift != shift) {
		sys->WhenInfo("Shift changed");
		
		// Train again at weekends
		trigger_retrain = true;
		

		// Updates latest snapshot and signals
		sys->SetEnd(mt.GetTime());
		ProcessWorkQueue();
		ProcessDataBridgeQueue();
		ProcessLabelQueue();
		RefreshSnapshots();
		LoopAgentSignalsAll(false);

		int last_snap_shift = snaps.Top().GetShift();

		if (shift != last_snap_shift) {
			WhenError(Format("Current shift doesn't match the lastest snapshot shift (%d != %d)", shift, last_snap_shift));
			sys->WhenPopTask();
			return;
		}


		// Forced askbid data download
		DataBridgeCommon& common = GetDataBridgeCommon();
		common.DownloadAskBid();
		common.RefreshAskBidData(true);


		// Refresh databridges again
		ProcessDataBridgeQueue();
		
		
		// Reset signals
		if (realtime_count == 0) {
			for (int i = 0; i < mt.GetSymbolCount(); i++)
				mt.SetSignal(i, 0);
		}
		mt.SetFreeMargin(FMLEVEL);
		realtime_count++;


		// Set broker signals
		WhenInfo("Looping agents until latest snapshot");
		bool succ = PutLatest(mt, snaps);


		// Print info
		String sigstr = "Signals ";

		for (int i = 0; i < sym_ids.GetCount(); i++) {
			if (i)
				sigstr << ",";

			sigstr << mt.GetSignal(sym_ids[i]);
		}

		WhenInfo(sigstr);


		// Notify about successful signals
		if (succ) {
			prev_shift = shift;

			sys->WhenRealtimeUpdate();
		}
	}
	
	
	if (wday != 0 && wday != 6 && last_datagather.Elapsed() >= 1*60*1000) {
		Data();
		last_datagather.Reset();
	}
	
	
	sys->WhenPopTask();
}

void AgentSystem::LoadThis() {
	if (FileExists(ConfigFile("agentgroup.bin.bak")))
		LoadFromFile(*this,	ConfigFile("agentgroup.bin.bak"));
	else
		LoadFromFile(*this,	ConfigFile("agentgroup.bin"));
}

void AgentSystem::StoreThis() {
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

void AgentSystem::Serialize(Stream& s) {
	s % groups % indi_ids % result_centroids % indi_centroids
	  % result_sectors % indi_sectors % fuse
	  % created % phase
	  % initial_result_clustering % initial_indicator_clustering;
}

void AgentSystem::RefreshSnapshots() {
	sys->WhenPushTask("Refreshing snapshots");

	ASSERT(buf_count != 0);
	ASSERT(sym_ids.GetCount() != 0);

	TimeStop ts;
	ProcessWorkQueue();

	int total_bars = sys->GetCountTf(main_tf);
	int bars = total_bars - data_begin;

	if (bars == 0) {
		data_begin = total_bars - 10000;
		bars = total_bars - data_begin;
	}

	snaps.Reserve(bars + (60 - (bars % 60)));

	for (; counted_bars < bars; counted_bars++) {
		int shift = counted_bars + data_begin;
		Time t = sys->GetTimeTf(main_tf, shift);
		int wday = DayOfWeek(t);

		// Skip weekend

		if (wday == 0 || wday == 6)
			continue;

		Seek(snaps.Add(), shift);
	}

	ASSERT(snaps.GetCount() > 0);


	RefreshClusters();


	LOG("Refreshing snapshots took " << ts.ToString());
	sys->WhenPopTask();
}

void AgentSystem::Progress(int actual, int total, String desc) {
	Cout() << "\t" << actual << "/" << total << "\t" << desc << EOL;
	ManagerLoader& loader = GetManagerLoader();
	loader.PostProgress(actual, total, desc);
	loader.PostSubProgress(0, 1);

	if (actual < total) {
		if (actual > 0)
			sys->WhenPopTask();

		sys->WhenPushTask(desc);
	}

	else
	if (actual == total && actual > 0) {
		sys->WhenPopTask();
	}
}

void AgentSystem::SubProgress(int actual, int total) {
	ManagerLoader& loader = GetManagerLoader();
	loader.PostSubProgress(actual, total);
	Cout() << "\t\t" << actual << "/" << total << EOL;
}

void AgentSystem::ResetValueBuffers() {

	// Get total count of output buffers in the indicator list
	VectorMap<unsigned, int> bufout_ids;
	int buf_id = 0;

	for (int i = 0; i < indi_ids.GetCount(); i++) {
		FactoryDeclaration& decl = indi_ids[i];
		const FactoryRegister& reg = sys->GetRegs()[decl.factory];

		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);

		bufout_ids.Add(decl.GetHashValue(), buf_id);

		buf_id += reg.out[0].visible;
	}

	buf_count = buf_id;

	ASSERT(buf_count);


	// Get DataBridge core pointer for easy reading
	databridge_cores.SetCount(0);
	databridge_cores.SetCount(sys->GetSymbolCount(), NULL);
	open_buffers.SetCount(0);
	open_buffers.SetCount(sys->GetSymbolCount(), NULL);
	int factory = sys->Find<DataBridge>();
	for (int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];

		if (ci.factory != factory)
			continue;

		databridge_cores[ci.sym] = &*ci.core;
		open_buffers[ci.sym] = &ci.core->GetBuffer(0);
	}


	// Reserve zeroed memory for output buffer pointer vector
	value_buffers.Clear();

	value_buffers.SetCount(sym_ids.GetCount());

	for (int i = 0; i < sym_ids.GetCount(); i++)
		value_buffers[i].SetCount(buf_count, NULL);


	// Get output buffer pointer vector
	int bars = sys->GetCountTf(main_tf);
	int total_bufs = 0;
	data_begin = 0;

	for (int i = 0; i < work_queue.GetCount(); i++) {
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

		if (db) {
			int begin = db->GetDataBegin();
			int limit = bars - 10000;
			bool enough = begin < limit;
			ASSERTUSER_(enough, "Symbol " + GetMetaTrader().GetSymbol(ci.sym).name + " has no proper data.");

			if (begin > data_begin) {
				LOG("Limiting symbol " << GetMetaTrader().GetSymbol(ci.sym).name << " " << bars - begin);
				data_begin = begin;
			}
		}

		Vector<ConstBuffer*>& indi_buffers = value_buffers[sym_id];

		const FactoryRegister& reg = sys->GetRegs()[ci.factory];

		FactoryDeclaration decl;
		decl.Set(ci.factory);

		for (int j = 0; j < ci.args.GetCount(); j++)
			decl.AddArg(ci.args[j]);

		for (int j = decl.arg_count; j < reg.args.GetCount(); j++)
			decl.AddArg(reg.args[j].def);

		unsigned hash = decl.GetHashValue();


		// Check that args match to declaration
#ifdef flagDEBUG
		ArgChanger ac;

		ac.SetLoading();

		ci.core->IO(ac);

		ASSERT(ac.args.GetCount() >= ci.args.GetCount());

		for (int i = 0; i < ci.args.GetCount(); i++) {
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

void AgentSystem::RefreshWorkQueue() {

	// Add proxy symbols to the queue if any
	Index<int> tf_ids, sym_ids;
	tf_ids.Add(main_tf);
	sym_ids <<= this->sym_ids;

	for (int i = 0; i < sym_ids.GetCount(); i++) {
		const Symbol& sym = GetMetaTrader().GetSymbol(sym_ids[i]);

		if (sym.proxy_id == -1)
			continue;

		sym_ids.FindAdd(sym.proxy_id);
	}

	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);


	// Get DataBridge work queue
	Vector<FactoryDeclaration> db_indi_ids;
	db_indi_ids.Add().Set(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, tf_ids, db_indi_ids);
}

void AgentSystem::ProcessWorkQueue() {
	sys->WhenPushTask("Processing work queue");

	work_lock.Enter();

	for (int i = 0; i < work_queue.GetCount(); i++) {
		SubProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}

	work_lock.Leave();

	sys->WhenPopTask();
}

void AgentSystem::ProcessDataBridgeQueue() {
	sys->WhenPushTask("Processing databridge work queue");

	work_lock.Enter();

	for (int i = 0; i < db_queue.GetCount(); i++) {
		SubProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}

	work_lock.Leave();

	sys->WhenPopTask();
}

bool AgentSystem::Seek(Snapshot& snap, int shift) {

	// Check that shift is not too much
	ASSERT_(shift >= 0 && shift < sys->GetCountTf(main_tf), "Data position is not in data range");
	ASSERT_(shift >= data_begin, "Data position is before common starting point");


	// Get some time values in binary format (starts from 0)
	Time t = sys->GetTimeTf(main_tf, shift);
	int month = t.month - 1;
	int day = t.day - 1;
	int hour = t.hour;
	int minute = t.minute;
	int wday = DayOfWeek(t);


	// Shift
	snap.SetShift(shift);


	// Time sensor
	snap.SetTimeSensor(0, (month * 31.0 + day) / 372.0);
	snap.SetTimeSensor(1, ((wday * 24 + hour) * 60 + minute) / (7.0 * 24.0 * 60.0));
	snap.SetTimeSensor(2, (hour * 60 + minute) / (24.0 * 60.0));


	// Refresh values (tf / sym / value)
	int k = 0;

	for (int i = 0; i < sym_ids.GetCount(); i++) {
		Vector<ConstBuffer*>& indi_buffers = value_buffers[i];

		for (int j = 0; j < buf_count; j++) {
			ConstBuffer& src = *indi_buffers[j];
			double d = src.GetUnsafe(shift);
			d = (d + 1.0) / 2.0;
			snap.SetSensor(i, j, d);
		}

		DataBridge& db = *dynamic_cast<DataBridge*>(databridge_cores[sym_ids[i]]);

		ConstBuffer& buf = db.GetBuffer(0);
		double open = buf.GetUnsafe(shift);

		snap.SetOpen(i, open);

		if (shift > 0) {
			double prev = buf.GetUnsafe(shift - 1);
			double change = open / prev - 1.0;
			snap.SetChange(i, change);
		}
		else {
			snap.SetChange(i, 0.0);
		}
		
		for (int j = 0; j < MEASURE_PERIODCOUNT; j++) {
			int period = MEASURE_PERIOD(j);
			double d1, d2;
			double volat_sum = 0.0;
			d1 = open;

			for (int k = 1; k < period && k <= shift; k++) {
				d2 = buf.GetUnsafe(shift - k);
				double change = d1 / d2 - 1.0;
				d1 = d2;
				volat_sum += fabs(change);
			}

			double change = open / d1 - 1.0;

			snap.SetPeriodChange(i, j, change);
			snap.SetPeriodVolatility(i, j, volat_sum);
			
			int volat_int = volat_sum / VOLAT_DIV;
			int change_int = change / CHANGE_DIV;
			//CombineHash ch;
			//ch << volat_int << 1 << change_int << 1;
		}
	}


	// Reset signals
	snap.Reset();


	return true;
}

void AgentSystem::CreateAgents() {
	ASSERT(buf_count > 0);
	ASSERT(sym_count > 0);

	MetaTrader& mt = GetMetaTrader();
	groups.SetCount(GROUP_COUNT);

	for (int i = 0; i < GROUP_COUNT; i++) {
		groups[i].agents.SetCount(SYM_COUNT);

		for (int j = 0; j < sym_ids.GetCount(); j++) {
			const Symbol& sym = mt.GetSymbol(sym_ids[j]);
			Agent& a = groups[i].agents[j];
			a.group_id = i;
			a.sym_id = j;
			a.sym = sym_ids[j];
			a.CreateAll();
		}
	}
	
	fuse.Create();
}

double AgentSystem::GetAverageSignalDrawdown() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].sig.GetLastDrawdown();

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageSignalIterations() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].sig.iter;

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageSignalDeepIterations() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].sig.deep_iter;

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageAmpDrawdown() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].amp.GetLastDrawdown();

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageAmpIterations() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].amp.iter;

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageAmpDeepIterations() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].amp.deep_iter;

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageAmpEpochs() {
	double s = 0;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			s += groups[i].agents[j].amp.result_equity.GetCount();

	return s / TRAINEE_COUNT;
}

double AgentSystem::GetAverageIterations(int phase) {
	switch (phase) {

	case PHASE_SIGNAL_TRAINING:
		return GetAverageSignalIterations();
		break;

	case PHASE_AMP_TRAINING:
		return GetAverageAmpIterations();
		break;

	case PHASE_FUSE_TRAINING:
		return GetAverageFuseIterations();
		break;
	}
	return 0;
}

double AgentSystem::GetAverageDeepIterations(int phase) {
	switch (phase) {

	case PHASE_SIGNAL_TRAINING:
		return GetAverageSignalDeepIterations();
		break;

	case PHASE_AMP_TRAINING:
		return GetAverageAmpDeepIterations();
		break;

	case PHASE_FUSE_TRAINING:
		return GetAverageFuseDeepIterations();
		break;

	default:
		return 0;
	}
}

void AgentSystem::RefreshAgentEpsilon(int phase) {
	if (phase == PHASE_FUSE_TRAINING) {
		int level = fuse.iter / FUSE_EPS_ITERS_STEP;

		if (level <= 0)
			fuse_epsilon = 0.20;
		else
		if (level == 1)
			fuse_epsilon = 0.05;
		else
		if (level == 2)
			fuse_epsilon = 0.02;
		else
		if (level >= 3)
			fuse_epsilon = 0.01;

		fuse.dqn.SetEpsilon(fuse_epsilon);
	}
	else {
		for (int i = 0; i < GROUP_COUNT; i++) {
			for (int j = 0; j < SYM_COUNT; j++) {
				Agent& a = groups[i].agents[j];
	
				if (phase == PHASE_SIGNAL_TRAINING) {
					int level = a.sig.iter / SIGNAL_EPS_ITERS_STEP;
	
					if (level <= 0)
						signal_epsilon = 0.20;
					else
					if (level == 1)
						signal_epsilon = 0.05;
					else
					if (level == 2)
						signal_epsilon = 0.02;
					else
					if (level >= 3)
						signal_epsilon = 0.01;
	
					a.sig.dqn.SetEpsilon(signal_epsilon);
				}
	
				else
				if (phase == PHASE_AMP_TRAINING) {
					int level = a.amp.iter / AMP_EPS_ITERS_STEP;
	
					if (level <= 0)
						amp_epsilon = 0.20;
					else
					if (level == 1)
						amp_epsilon = 0.05;
					else
					if (level == 2)
						amp_epsilon = 0.02;
					else
					if (level >= 3)
						amp_epsilon = 0.01;
	
					a.amp.dqn.SetEpsilon(amp_epsilon);
				}
			}
		}
	}
}

void AgentSystem::RefreshLearningRate(int phase) {
	double max_lrate = MAX_LEARNING_RATE;
	double min_lrate = MIN_LEARNING_RATE;
	double range = max_lrate - min_lrate;
	
	if (phase == PHASE_FUSE_TRAINING) {
		double prog = (double)fuse.iter / (double)FUSE_PHASE_ITER_LIMIT;
		double lrate = (1.0 - prog) * range + min_lrate;
		fuse.dqn.SetLearningRate(lrate);
	}
	else {
		for (int i = 0; i < GROUP_COUNT; i++) {
			for (int j = 0; j < SYM_COUNT; j++) {
				Agent& a = groups[i].agents[j];
	
				if (phase == PHASE_SIGNAL_TRAINING) {
					double prog = (double)a.sig.iter / (double)SIGNAL_PHASE_ITER_LIMIT;
					double lrate = (1.0 - prog) * range + min_lrate;
					a.sig.dqn.SetLearningRate(lrate);
				}
	
				else
				if (phase == PHASE_AMP_TRAINING) {
					double prog = (double)a.amp.iter / (double)AMP_PHASE_ITER_LIMIT;
					double lrate = (1.0 - prog) * range + min_lrate;
					a.amp.dqn.SetLearningRate(lrate);
				}
			}
		}
	}
}

void AgentSystem::SetSignalEpsilon(double epsilon) {
	this->signal_epsilon = epsilon;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			groups[i].agents[j].sig.dqn.SetEpsilon(epsilon);
}

void AgentSystem::SetAmpEpsilon(double epsilon) {
	this->amp_epsilon = epsilon;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			groups[i].agents[j].amp.dqn.SetEpsilon(epsilon);
}

void AgentSystem::SetFuseEpsilon(double epsilon) {
	this->fuse_epsilon = epsilon;
	fuse.dqn.SetEpsilon(epsilon);
}

void AgentSystem::SetFreeMarginLevel(double fmlevel) {
	this->free_margin_level = fmlevel;

	for (int i = 0; i < GROUP_COUNT; i++)
		for (int j = 0; j < SYM_COUNT; j++)
			groups[i].agents[j].SetFreeMarginLevel(fmlevel);
	
	fuse.test_broker.free_margin_level = fmlevel;
}

void AgentSystem::InitBrokerValues() {
	MetaTrader& mt = GetMetaTrader();

	proxy_id.SetCount(SYM_COUNT, 0);
	proxy_base_mul.SetCount(SYM_COUNT, 0);
	//spread_points.SetCount(SYM_COUNT, 0);

	for (int i = 0; i < SYM_COUNT; i++) {
		int sym = sym_ids[i];

		DataBridge* db = dynamic_cast<DataBridge*>(databridge_cores[sym]);

		const Symbol& symbol = mt.GetSymbol(sym);

		if (symbol.proxy_id != -1) {
			int k = sym_ids.Find(symbol.proxy_id);
			ASSERT(k != -1);
			proxy_id[i] = k;
			proxy_base_mul[i] = symbol.base_mul;
		}

		else {
			proxy_id[i] = -1;
			proxy_base_mul[i] = 0;
		}

		//spread_points[i] = db->GetAverageSpread() * db->GetPoint();
		//ASSERT(spread_points[i] > 0.0);
	}

	begin_equity = Upp::max(10000.0, mt.AccountEquity());

	leverage = 1000;
}

bool AgentSystem::PutLatest(Brokerage& broker, Vector<Snapshot>& snaps) {
	System& sys = GetSystem();
	sys.WhenPushTask("Putting latest signals");


	MetaTrader* mt = dynamic_cast<MetaTrader*>(&broker);

	if (mt) {
		mt->Data();
		broker.RefreshLimits();
	}

	else {
		SimBroker* sb = dynamic_cast<SimBroker*>(&broker);
		sb->RefreshOrders();
	}

	Snapshot& cur_snap = snaps.Top();
	for (int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		int sig = cur_snap.GetFuseOutput(i);

		if (sig == broker.GetSignal(sym) && sig != 0)
			broker.SetSignalFreeze(sym, true);
		else {
			broker.SetSignal(sym, sig);
			broker.SetSignalFreeze(sym, false);
		}
	}

	broker.SetFreeMarginLevel(free_margin_level);

	broker.SetFreeMarginScale((AMP_MAXSCALES - 1)*AMP_MAXSCALE_MUL * SYM_COUNT);
	broker.SignalOrders(true);

	sys.WhenPopTask();

	return true;
}

void AgentSystem::Data() {
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
#endif
