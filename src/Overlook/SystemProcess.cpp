#include "Overlook.h"

namespace Overlook {

void System::StartMain() {
	StopMain();
	main_stopped = false;
	main_running = true;
	Thread::Start(THISBACK(MainLoop));
}

void System::StopMain() {
	main_running = false;
	while (!main_stopped || workers_started) Sleep(100);
}

void System::MainLoop() {
	
	// Allocate some non-persistent memory
	main_mem.SetCount(MEM_COUNT, 0);
	logic0.SetCount(GetCommonCount());
	logic1.SetCount(GetCommonCount());
	logic2.SetCount(GetCommonCount());
	workers_started = 0;
	for (int i = 0; i < REG_COUNT; i++) main_reg[i] = 0;
	int prev_step = -1;
	
	
	// Start workers
	int workers = Upp::min(GetCommonSymbolCount(), Upp::max(1, CPU_Cores() - 2));
	for (int i = 0; i < workers; i++) {
		Thread::Start(THISBACK1(Worker, i));
		workers_started++;
	}
	
	
	// Run main loop
	while (main_running && !Thread::IsShutdownThreads()) {
	
		
		Time t = GetMetaTrader().GetTime();
		#if SYS_M15
		int step = t.minute / 15;
		#elif SYS_H1
		int step = t.hour;
		#endif
		
		
		// Run actions by "instruction"
		dword& ins = main_reg[REG_INS];
		switch (ins) {
		
		// Wait until new data position
		case INS_WAIT_NEXTSTEP:
			
			if (prev_step == step) {
				Sleep(100);
				break;
			}
			prev_step = step;
			ins++;
			break;
		
		
		// Wait until logic is trained by worker threads
		case INS_WAIT_LOGICTRAINING:
			for (int l = 0; l < level_count; l++) {
				dword& is_training = main_reg[REG_LOGICTRAINING_L0_ISRUNNING + l];
				dword& is_trained = main_mem[MEM_TRAINED_L0 + l];
				if (is_training) {
					if (main_jobs.IsEmpty()) {
						is_training = false;
						is_trained = true;
						StoreAll();
					}
					else {
						Sleep(100);
						ins--;
						break;
					}
				}
			}
			ins++;
			break;
			
		
		// Refresh data-sources and indicators
		case INS_REFRESHINDI:
			RealizeMainWorkQueue();
			ProcessMainWorkQueue(true);
			//ProcessEnds();
			ins++;
			break;
			
			
		// Write boolean values from indicators to the main memory.
		// This refresh L0 & L1 input sources.
		case INS_INDIBITS:
			FillIndicatorBits();
			ins++;
			break;
			
			
		// Refresh persistent variables for training. Only once at first time.
		case INS_TRAINABLE:
			FillTrainableBits();
			ins++;
			break;
			
		
		// Fill L0 test data performance to the main memory.
		// L0 must be trained and evaluated values must be written to the main memory.
		// This refresh L2 input source.
		case INS_CUSTOMLOGIC:
			FillCustomLogicBits();
			ins++;
			break;
			
			
		// Fill calendar events to the main memory.
		// These events might be public speeches and national publications, which tends to
		// override technical rules with large volatility. They should be avoided usually.
		case INS_CALENDARLOGIC:
			FillCalendarBits();
			ins++;
			break;
			
			
		// Start logic training if it's time to.
		case INS_REALIZE_LOGICTRAINING:
			RealizeLogicTraining();
			ins++;
			break;
			
			
		// Evaluate logic and write it to the main memory.
		// Requires training to be finished.
		case INS_LOGICBITS:
			FillLogicBits();
			
			// Don't continue to INS_REFRESH_REAL before all levels have been written.
			if (main_mem[MEM_COUNTED_L2] == 0)
				ins = 1;
			else
				ins++;
			break;
			
			
		// Fill some statistics
		case INS_STATS:
			FillStatistics();
			ins++;
			break;
			
			
		// Handle real MT4 account.
		case INS_REFRESH_REAL:
			if (RefreshReal())
				ins = 0; // go to waiting when success
			else
				ins = 1;
			break;
			
		}
		
		
	}
	
	main_running = false;
	main_stopped = true;
}

void System::RealizeMainWorkQueue() {
	
	// Run only once
	if (main_reg[REG_WORKQUEUE_INITED]) {
		
		// Update data count
		main_mem[MEM_INDIBARS] = GetCountMain(main_tf);
		return;
	}
	
	
	// Debug assertions
	ASSERT(main_tf_ids.IsEmpty());
	ASSERT(main_sym_ids.IsEmpty());
	ASSERT(main_indi_ids.IsEmpty());
	
	
	// Add indicators
	FactoryDeclaration decl;
	decl.factory = Find<DataBridge>();							main_indi_ids.Add(decl);
	decl.factory = Find<MovingAverage>();						main_indi_ids.Add(decl);
	decl.factory = Find<MovingAverageConvergenceDivergence>();	main_indi_ids.Add(decl);
	decl.factory = Find<BollingerBands>();						main_indi_ids.Add(decl);
	decl.factory = Find<ParabolicSAR>();						main_indi_ids.Add(decl);
	decl.factory = Find<StandardDeviation>();					main_indi_ids.Add(decl);
	decl.factory = Find<AverageTrueRange>();					main_indi_ids.Add(decl);
	decl.factory = Find<BearsPower>();							main_indi_ids.Add(decl);
	decl.factory = Find<BullsPower>();							main_indi_ids.Add(decl);
	decl.factory = Find<CommodityChannelIndex>();				main_indi_ids.Add(decl);
	decl.factory = Find<DeMarker>();							main_indi_ids.Add(decl);
	decl.factory = Find<Momentum>();							main_indi_ids.Add(decl);
	decl.factory = Find<RelativeStrengthIndex>();				main_indi_ids.Add(decl);
	decl.factory = Find<RelativeVigorIndex>();					main_indi_ids.Add(decl);
	decl.factory = Find<StochasticOscillator>();				main_indi_ids.Add(decl);
	decl.factory = Find<AcceleratorOscillator>();				main_indi_ids.Add(decl);
	decl.factory = Find<AwesomeOscillator>();					main_indi_ids.Add(decl);
	decl.factory = Find<PeriodicalChange>();					main_indi_ids.Add(decl);
	decl.factory = Find<VolatilityAverage>();					main_indi_ids.Add(decl);
	decl.factory = Find<VolatilitySlots>();						main_indi_ids.Add(decl);
	decl.factory = Find<VolumeSlots>();							main_indi_ids.Add(decl);
	decl.factory = Find<ChannelOscillator>();					main_indi_ids.Add(decl);
	decl.factory = Find<ScissorChannelOscillator>();			main_indi_ids.Add(decl);
	decl.factory = Find<CommonForce>();							main_indi_ids.Add(decl);
	decl.factory = Find<CorrelationOscillator>();				main_indi_ids.Add(decl);
	main_factory_ids.Clear();
	for (int i = 0; i < main_indi_ids.GetCount(); i++)
		main_factory_ids.Add(main_indi_ids[i].factory);
	
	
	// Add timeframes
	#if SYS_M15
	if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(15));
	if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(60));
	if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(240));
	#elif SYS_H1
	if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(60));
	if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(240));
	if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(1440));
	#endif
	ASSERT(main_tf_ids.GetCount() == TF_COUNT);
	main_tf = main_tf_ids[main_tf_pos];
	
	
	// Add symbols
	for (int i = 0; i < GetCommonCount(); i++) {
		for (int j = 0; j < GetCommonSymbolCount(); j++)
			main_sym_ids.Add(GetCommonSymbolId(i, j));
		main_sym_ids.Add(GetCommonSymbolId(i));
	}
	
	
	// Get queue of cores
	main_work_queue.Clear();
	GetCoreQueue(main_work_queue, main_sym_ids, main_tf_ids, main_indi_ids);
	
	
	// Refresh all cores
	ProcessMainWorkQueue(true);
	if (!main_running) return;
	
	
	// Update data count
	int bars = GetCountMain(main_tf);
	main_mem[MEM_INDIBARS] = bars;
	
	
	// Get ordered core vector for easy locating.
	int ordered_cores_total = main_tf_ids.GetCount() * main_sym_ids.GetCount() * main_indi_ids.GetCount();
	ordered_cores.Clear();
	ordered_cores.SetCount(ordered_cores_total, NULL);
	
	for (int i = 0; i < main_work_queue.GetCount(); i++) {
		CoreItem& ci = *main_work_queue[i];
		ASSERT(&*ci.core != NULL);
		
		int sym_pos = main_sym_ids.Find(ci.sym);
		int tf_pos = main_tf_ids.Find(ci.tf);
		int factory_pos = main_factory_ids.Find(ci.factory);
		
		if (sym_pos == -1) Panic("Symbol not found");
		if (tf_pos == -1) Panic("Tf not found");
		if (factory_pos == -1) Panic("Factory not found");
		
		int core_pos = GetOrderedCorePos(sym_pos, tf_pos, factory_pos);
		ASSERT(ordered_cores[core_pos] == NULL);
		ordered_cores[core_pos] = &*ci.core;
	}
	
	for (int i = 0; i < ordered_cores.GetCount(); i++) {
		ASSERT(ordered_cores[i] != NULL);
	}
	
	
	// Find starting position of data
	main_begin.SetCount(GetCommonCount(), 0);
	for (int c = 0; c < GetCommonCount(); c++) {
		int common_id = GetCommonSymbolId(c);
		int& begin = main_begin[c];
		for (int i = 0; i < main_tf_ids.GetCount(); i++) {
			int tf = main_tf_ids[i];
			
			for (int j = 0; j < GetCommonSymbolCount(); j++) {
				int sym_id = GetCommonSymbolId(c, j);
				int pos = GetShiftTf(sym_id, tf, common_id, main_tf, 1);
				if (pos > begin) begin = pos;
			}
		}
	}
	
	
	main_reg[REG_WORKQUEUE_INITED] = true;
}

void System::ProcessMainWorkQueue(bool store_cache) {
	EnterProcessing();
	dword& queue_cursor = main_reg[REG_WORKQUEUE_CURSOR];
	for (queue_cursor = 0; queue_cursor < main_work_queue.GetCount() && main_running; queue_cursor++)
		Process(*main_work_queue[queue_cursor], store_cache);
	LeaveProcessing();
}

void System::ProcessEnds() {
	for (int i = 0; i < main_tf_ids.GetCount(); i++) {
		int tf = main_tf_ids[i];
		RefreshTimeTfVectors(tf);
		for (int j = 0; j < main_sym_ids.GetCount(); j++) {
			int sym = main_sym_ids[j];
			RefreshTimeSymVectors(sym, tf);
		}
	}
}

void System::FillIndicatorBits() {
	ASSERT(!main_tf_ids.IsEmpty());
	ASSERT(!main_sym_ids.IsEmpty());
	ASSERT(!main_indi_ids.IsEmpty());
	
	dword& cursor = main_mem[MEM_COUNTED_INDI];
	bool init = cursor == 0;
	
	cursor = Upp::max(0, (int)cursor - 8); // set previous BIT_REALSIGNAL
		
	VectorBool vec;
	vec.SetCount(ASSIST_COUNT);
	
	int bars = main_mem[MEM_INDIBARS];
	ASSERT(bars > 0);
	int64 main_data_count = GetMainDataPos(bars, 0, 0, 0);
	main_data.SetCount(main_data_count);
	
	#ifdef flagDEBUG
	cursor = bars;
	#else
	
	for (; cursor < bars && main_running; cursor++) {
	
		for (int i = 0; i < main_tf_ids.GetCount(); i++) {
			int tf = main_tf_ids[i];
			
			for (int j = 0; j < main_sym_ids.GetCount(); j++) {
				int sym = main_sym_ids[j];
				int core_cursor = GetShiftMainTf(main_tf, sym, tf, cursor);
				int core_bars = GetCountTf(sym, tf);
				
				vec.Zero();
				
				for (int k = 0; k < main_indi_ids.GetCount(); k++) {
					int core_pos = GetOrderedCorePos(j, i, k);
					Core& core = *ordered_cores[core_pos];
					core.Assist(core_cursor, vec);
					
					// First indi_id is DataBridge and open price data is used to set real signal bit
					if (k == 0 && core_cursor < core_bars - 1) {
						ConstBuffer& open_buf = core.GetBuffer(0);
						double next = open_buf.GetUnsafe(core_cursor + 1);
						double curr = open_buf.GetUnsafe(core_cursor);
						bool signal = next < curr;
						int64 pos = GetMainDataPos(cursor, j, i, BIT_REALSIGNAL);
						main_data.Set(pos, signal);
						pos = GetMainDataPos(cursor, j, i, BIT_WRITTEN_REAL);
						main_data.Set(pos, true);
					}
				}
				
				int64 main_pos = GetMainDataPos(cursor, j, i, BIT_L0BITS_BEGIN);
				for (int k = 0; k < ASSIST_COUNT; k++) {
					main_data.Set(main_pos++, vec.Get(k));
				}
			}
		}
	}
	#endif
	
	main_reg[REG_INDIBITS_INITED] = true;
	if (init) StoreAll();
}

int System::GetOrderedCorePos(int sym_pos, int tf_pos, int factory_pos) {
	return (sym_pos * main_tf_ids.GetCount() + tf_pos) * main_factory_ids.GetCount() + factory_pos;
}

void System::FillTrainableBits() {

	// Set trainable bits only once to keep fixed training dataset
	dword& is_set_already = main_mem[MEM_TRAINABLESET];
	if (is_set_already)
		return;
		
	dword bars = main_mem[MEM_INDIBARS];
	ASSERT(bars > 0);
	
	dword& train_bars		= main_mem[MEM_TRAINBARS];
	train_bars = bars;
	
	for (int i = 0; i < GetCommonCount(); i++) {
		dword& train_midstep	= main_mem[MEM_TRAINMIDSTEP + i];
		dword& train_begin		= main_mem[MEM_TRAINBEGIN   + i];
		
		train_begin = main_begin[i];
		train_midstep = train_begin + (train_bars - train_begin) * 2 / 3;
		ASSERT(train_begin < train_midstep && train_midstep < train_bars);
	}
	
	is_set_already = true;
}

void System::FillCustomLogicBits() {
	dword& cursor = main_mem[MEM_COUNTED_ENABLED];
	
	cursor = Upp::max(0, (int)cursor - 8); // set previous BIT_REALENABLED
		
	int bars = main_mem[MEM_COUNTED_L0];
	if (!bars) return;
	
	int first_common_id = GetCommonSymbolId(0);
	
	for (; cursor < bars && main_running; cursor++) {
		
		for (int i = 0; i < main_tf_ids.GetCount(); i++) {
			int tf = main_tf_ids[i];
			
			for (int j = 0; j < main_sym_ids.GetCount(); j++) {
				int sym = main_sym_ids[j];
				int core_cursor = GetShiftMainTf(main_tf, sym, tf, cursor);
				int core_bars = GetCountTf(sym, tf);
				int db_pos = GetOrderedCorePos(j, i, 0);
				Core& db = *ordered_cores[db_pos];
				ConstBuffer& open_buf = db.GetBuffer(0);
				double point = spread_points[sym];
				ASSERT(sym >= first_common_id || point > 0.0);
				
				if (core_cursor >= core_bars - 1)
					continue;
					
				for (int k = 0; k <= L2_INPUT; k++) {
					int pos = Upp::max(0, (int)cursor - L2_INPUT + k);
					bool real_sig	= main_data.Get(GetMainDataPos(pos, j, i, BIT_REALSIGNAL));
					bool l0_sig		= main_data.Get(GetMainDataPos(pos, j, i, BIT_L0_SIGNAL));
					bool enabled	= real_sig == l0_sig;
					if (enabled) {
						// Check spread
						int core_pos = GetShiftMainTf(main_tf, sym, tf, pos);
						double curr = open_buf.GetUnsafe(core_pos);
						double next = open_buf.GetUnsafe(core_pos + 1);
						
						if (!real_sig)
							enabled = next >= curr + point;
						else
							enabled = next <= curr - point;
					}
					
					if (k < L2_INPUT)
						main_data.Set(GetMainDataPos(cursor, j, i, BIT_L2BITS_BEGIN + k), enabled);
					else
						main_data.Set(GetMainDataPos(cursor, j, i, BIT_REALENABLED), enabled);
				}
			}
		}
	}
}

void System::FillCalendarBits() {
	Calendar& cal = GetCalendar();
	cal.Data();
	
	dword& cursor = main_mem[MEM_COUNTED_CALENDAR];
	
	cursor = Upp::max(0, (int)cursor - 8); // set trailing bits
	
	Time now = GetUtcTime();
	int bars = main_mem[MEM_INDIBARS];
	
	for (; cursor < cal.GetCount(); cursor++) {
		const CalEvent& ce = cal.GetEvent(cursor);
		if (ce.timestamp > now) break;
		
		if (ce.impact < 2) continue;
		
		Time t = ce.timestamp;
		#if SYS_M15
		t += (15 * 0.5) * 60;
		t -= (t.minute % 15) * 60 + t.second;
		int end_count = 1;
		#elif SYS_H1
		t.minute = 0;
		t.second = 0;
		int end_count = 0;
		#endif
		int current_main_pos = main_time[main_tf].Find(t);
		if (current_main_pos == -1) continue;
		
		int begin = Upp::max(0, current_main_pos - 1);
		int end = Upp::min(bars, current_main_pos + end_count);
		for(int c = begin; c < end; c++) {
			for(int i = 0; i < main_sym_ids.GetCount(); i++) {
				for(int j = 0; j < main_tf_ids.GetCount(); j++) {
					int pos = GetMainDataPos(c, i, j, BIT_SKIP_CALENDAREVENT);
					main_data.Set(pos, true);
				}
			}
		}
	}
}

void System::RealizeLogicTraining() {
	for (int l = 0; l < level_count; l++) {
		dword& is_trained = main_mem[MEM_TRAINED_L0 + l];
		
		if (l < 2 && main_mem[MEM_COUNTED_INDI] == 0) return;
		if (l == 2 && main_mem[MEM_COUNTED_ENABLED] == 0) return;
		
		if (!is_trained) {
			dword& is_training = main_reg[REG_LOGICTRAINING_L0_ISRUNNING + l];
			if (!is_training) {
				for (int i = 0; i < GetCommonCount(); i++) LearnLogic(l, i);
				is_training = true;
			}
			return;
		}
	}
}

void System::LearnLogic(int level, int common_pos) {
	main_lock.EnterWrite();
	MainJob& job		= main_jobs.Add();
	job.level			= level;
	job.common_pos		= common_pos;
	job.being_processed	= 0;
	main_lock.LeaveWrite();
}

void System::Worker(int id) {
	MainJob* rem_job = NULL;
	
	while (main_running && !Thread::IsShutdownThreads()) {
		bool processed_something = false;
		main_lock.EnterRead();
		for (int i = 0; i < main_jobs.GetCount(); i++) {
			MainJob& job = main_jobs[i];
			if (job.is_finished)
				continue;
			bool got_job = ++job.being_processed == 1;
			if (got_job) {
				job.is_finished = ProcessMainJob(job);
				processed_something = true;
			}
			job.being_processed--;
			if (job.is_finished) {
				rem_job = &job;
				break;
			}
		}
		main_lock.LeaveRead();
		
		if (rem_job) {
			main_lock.EnterWrite();
			for (int i = 0; i < main_jobs.GetCount(); i++) {
				MainJob& job = main_jobs[i];
				if (job.is_finished && job.being_processed == 0) {
					main_jobs.Remove(i);
					i--;
					continue;
				}
			}
			main_lock.LeaveWrite();
			rem_job = NULL;
		}
		
		if (!processed_something) Sleep(100);
	}
	
	workers_started--;
}

template <class T> int TrainLogic(System::MainJob& job, System& sys, T& logic) {

	// Random event probability (not used anyway)
	logic.dqn_trainer.SetEpsilon(0);
	
	// Future reward discount factor.
	// 0 means, that the loss or reward event is completely separate event from future events.
	logic.dqn_trainer.SetGamma(0);
	
	// Learning rate.
	logic.dqn_trainer.SetAlpha(0.005);
	
	const dword train_bars		= sys.main_mem[System::MEM_TRAINBARS];
	const dword train_midstep	= sys.main_mem[System::MEM_TRAINMIDSTEP + job.common_pos];
	const dword train_begin		= sys.main_mem[System::MEM_TRAINBEGIN + job.common_pos];
	
	int data_count, data_begin;
	if      (job.level == 0)		{data_count = train_midstep - train_begin;	data_begin = train_begin;}
	else if (job.level == 1)		{data_count = train_bars - train_begin;		data_begin = train_begin;}
	else if (job.level == 2)		{data_count = train_bars - train_midstep;	data_begin = train_midstep;}
	else Panic("Invalid level");
	ASSERT(data_count > 0);
	
	double correct[logic.dqn_trainer.OUTPUT_SIZE];
	typename T::DQN::MatType tmp_before_state, tmp_after_state;
	
	double av_tderror = 0.0;
	for (int i = 0; i < 50; i++) {
		int count = data_count - 1;
		if (count < 1) break;
		int pos = data_begin + Random(count);
		if (pos < 0 || pos >= train_bars) continue;
		sys.LoadInput(job.level, job.common_pos, pos, tmp_before_state);
		sys.LoadInput(job.level, job.common_pos, pos + 1, tmp_after_state);
		sys.LoadOutput(job.level, job.common_pos, pos, correct, logic.dqn_trainer.OUTPUT_SIZE);
		av_tderror = logic.dqn_trainer.Learn(tmp_before_state, correct, tmp_after_state);
		
		logic.dqn_round++;
	}
	
	int exp_pts_count = logic.dqn_round / 5000 + 1;
	if (job.training_pts.GetCount() < exp_pts_count)
		job.training_pts.Add(av_tderror);
		
	job.actual = logic.dqn_round;
	job.total = logic.dqn_max_rounds;
	
	return logic.dqn_round >= logic.dqn_max_rounds;
}

int System::ProcessMainJob(MainJob& job) {
	int ret = 0;
	if      (job.level == 0)		ret = TrainLogic(job, *this, logic0[job.common_pos]);
	else if (job.level == 1)		ret = TrainLogic(job, *this, logic1[job.common_pos]);
	else if (job.level == 2)		ret = TrainLogic(job, *this, logic2[job.common_pos]);
	else Panic("Invalid level");
	return ret;
}

template <class T> void FillLogicBits(int level, int common_pos, System& sys, T& logic) {
	dword bars					= sys.main_mem[System::MEM_INDIBARS];
	dword cursor				= sys.main_mem[System::MEM_COUNTED_L0 + level]; // don't ref
	const dword output_size		= logic.dqn_trainer.OUTPUT_SIZE;
	ASSERT(output_size > 0);
	
	cursor = Upp::max(0, (int)cursor - 8); // set previous BIT_REALENABLED
		
	double evaluated[output_size];
	typename T::DQN::MatType tmp_before_state;
	
	for (; cursor < bars; cursor++) {
		sys.LoadInput(level, common_pos, cursor, tmp_before_state);
		
		logic.dqn_trainer.Evaluate(tmp_before_state, evaluated, output_size);
		
		sys.StoreOutput(level, common_pos, cursor, evaluated, output_size);
		
		#ifdef flagDEBUG
		cursor = bars;
		break;
		#endif
	}
}

void System::FillLogicBits() {
	for (int l = 0; l < level_count; l++) {
		bool is_trained				= main_mem[MEM_TRAINED_L0 + l];
		if (!is_trained) break;
		
		dword bars					= main_mem[System::MEM_INDIBARS];
		dword& cursor				= main_mem[System::MEM_COUNTED_L0 + l];
		
		for (int common_pos = 0; common_pos < GetCommonCount(); common_pos++) {
			if      (l == 0)		::Overlook::FillLogicBits(l, common_pos, *this, logic0[common_pos]);
			else if (l == 1)		::Overlook::FillLogicBits(l, common_pos, *this, logic1[common_pos]);
			else if (l == 2)		::Overlook::FillLogicBits(l, common_pos, *this, logic2[common_pos]);
			else Panic("Invalid level");
		}
		
		cursor = bars;
	}
}

void System::FillStatistics() {
	if (main_mem[System::MEM_COUNTED_L2] == 0) return;
	if (main_reg[REG_DD_L0TEST] != 0) return;
	
	int midstep = 0, begin = 0;
	for(int i = 0; i < GetCommonCount(); i++)
		midstep = Upp::max(midstep, (int)main_mem[MEM_TRAINMIDSTEP + i]);
	for(int i = 0; i < GetCommonCount(); i++)
		begin = Upp::max(begin, (int)main_mem[MEM_TRAINBEGIN + i]);
	
	int count_mul = main_sym_ids.GetCount();
	int bars = main_mem[MEM_TRAINBARS];
	int count = (bars - midstep) * count_mul;
	
	int l0_correct = 0;
	int l1_correct = 0;
	int l2_correct = 0;
	int l0l2_correct = 0;
	int l1l2_correct = 0;
	int l0ena_correct = 0, l0ena_count = 0;
	int l1ena_correct = 0, l1ena_count = 0;
	
	for(int i = midstep; i < bars; i++) {
		for (int tf_pos = 0; tf_pos < main_tf_ids.GetCount(); tf_pos++) {
			for (int sym_pos = 0; sym_pos < main_sym_ids.GetCount(); sym_pos++) {
				int64 pos;
				
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_REALSIGNAL);
				bool real_sig = main_data.Get(pos);
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_L0_SIGNAL);
				bool l0_sig = main_data.Get(pos);
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_L1_SIGNAL);
				bool l1_sig = main_data.Get(pos);
				
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_REALENABLED);
				bool real_ena = main_data.Get(pos);
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_L2_ENABLED);
				bool l2_ena = main_data.Get(pos);
				
				
				if (l0_sig == real_sig)		l0_correct++;
				if (l1_sig == real_sig)		l1_correct++;
				if (l2_ena == real_ena)		l2_correct++;
				if (l2_ena == real_ena && (!l2_ena || l0_sig == real_sig))	l0l2_correct++;
				if (l2_ena == real_ena && (!l2_ena || l1_sig == real_sig))	l1l2_correct++;
				
				if (l2_ena) {
					if (l0_sig == real_sig)	l0ena_correct++;
					l0ena_count++;
					if (l1_sig == real_sig)	l1ena_correct++;
					l1ena_count++;
				}
			}
		}
	}
	if (!l0ena_count) l0ena_count++;
	if (!l1ena_count) l1ena_count++;
	
	int train_count = (midstep * begin) * count_mul;
	int train_l0_correct = 0;
	for(int i = begin; i < midstep; i++) {
		for (int tf_pos = 0; tf_pos < main_tf_ids.GetCount(); tf_pos++) {
			for (int sym_pos = 0; sym_pos < main_sym_ids.GetCount(); sym_pos++) {
				int64 pos;
				
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_REALSIGNAL);
				bool real_sig = main_data.Get(pos);
				pos = GetMainDataPos(i, sym_pos, tf_pos, BIT_L0_SIGNAL);
				bool l0_sig = main_data.Get(pos);
				
				if (l0_sig == real_sig)		train_l0_correct++;
			}
		}
	}
	
	
	main_reg[REG_SIG_L0TOREAL]		= 1000 * l0_correct / count;
	main_reg[REG_SIG_L1TOREAL]		= 1000 * l1_correct / count;
	main_reg[REG_SIG_L2TOREAL]		= 1000 * l2_correct / count;
	main_reg[REG_SIG_L0TOL1]		= 1000 * main_reg[REG_SIG_L0TOREAL] / main_reg[REG_SIG_L1TOREAL];
	main_reg[REG_ENA_L0L2TOREAL]	= 1000 * l0l2_correct / count;
	main_reg[REG_ENA_L1L2TOREAL]	= 1000 * l1l2_correct / count;
	main_reg[REG_SIG_L0L2TOL1L2]	= 1000 * main_reg[REG_ENA_L0L2TOREAL] / main_reg[REG_ENA_L1L2TOREAL];
	main_reg[REG_DD_L0TRAIN]		= 1000 * (train_count - train_l0_correct) / train_count;
	main_reg[REG_DD_L0TEST]			= 1000 * (count - l0_correct) / count;
	main_reg[REG_L0ENA]				= 1000 * l0ena_correct / l0ena_count;
	main_reg[REG_L1ENA]				= 1000 * l1ena_correct / l1ena_count;
	
}

bool System::RefreshReal() {
	Time now = GetUtcTime();
	int wday				= DayOfWeek(now);
	Time after_3hours		= now + 3 * 60 * 60;
	int wday_after_3hours	= DayOfWeek(after_3hours);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return true;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_3hours == 6) {
		WhenInfo("Closing all orders before market break");
		
		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}
		
		mt.SignalOrders(true);
		return true;
	}
	
	int64 step = periods[main_tf] * 60;
	now -= now.Get() % step;
	int current_main_pos = main_time[main_tf].Find(now);
	
	if (current_main_pos == -1) {
		LOG("error: current main pos not found");
		return false;
	}
	
	if (current_main_pos >= main_mem[MEM_INDIBARS])
		return false;
		
	
	WhenInfo("Updating MetaTrader");
	WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		mt.RefreshLimits();
		int open_count = 0;
		Vector<int> signals;
		for (int i = 0; i < GetCommonCount(); i++) {
			for (int j = 0; j < GetCommonSymbolCount(); j++) {
				int sym_id = GetCommonSymbolId(i, j);
				int sym_pos = main_sym_ids.Find(sym_id);
				
				// Do some quality checks
				if (0) {
					int64 pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_WRITTEN_REAL);
					int written_real = main_data.Get(pos);
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_WRITTEN_L0);
					int not_written_L0 = !main_data.Get(pos);
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_WRITTEN_L1);
					int not_written_L1 = !main_data.Get(pos);
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_WRITTEN_L2);
					int not_written_L2 = !main_data.Get(pos);
					int e = (written_real << 3) | (not_written_L0 << 2) | (not_written_L1 << 1) | (not_written_L2 << 0);
					if (e)
						throw UserExc("Real account function quality check failed: error code " + IntStr(e) + " sym=" + IntStr(sym_id));
				}
				
				int64 signal_pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_L1_SIGNAL);
				int64 enabled_pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_L2_ENABLED);
				bool signal_bit = main_data.Get(signal_pos);
				bool enabled_bit = main_data.Get(enabled_pos);
				int sig = enabled_bit ? (signal_bit ? -1 : + 1) : 0;
				int prev_sig = mt.GetSignal(sym_id);
				
				// Avoid unpredictable calendar events...
				{
					int pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_SKIP_CALENDAREVENT);
					bool skip = main_data.Get(pos);
					if (skip) sig = 0;
				}
					
				signals.Add(sig);
				if (sig == prev_sig && sig != 0)
					open_count++;
			}
		}
		int sig_pos = 0;
		for (int i = 0; i < GetCommonCount(); i++) {
			for (int j = 0; j < GetCommonSymbolCount(); j++) {
				int sym_id = GetCommonSymbolId(i, j);
				int sig = signals[sig_pos++];
				int prev_sig = mt.GetSignal(sym_id);
				
				if (sig == prev_sig && sig != 0)
					mt.SetSignalFreeze(sym_id, true);
				else {
					if ((!prev_sig && sig) || (prev_sig && sig != prev_sig)) {
						if (open_count >= MAX_SYMOPEN)
							sig = 0;
						else
							open_count++;
					}
					
					mt.SetSignal(sym_id, sig);
					mt.SetSignalFreeze(sym_id, false);
				}
				LOG("Real symbol " << sym_id << " signal " << sig);
			}
		}
		
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale(MAX_SYMOPEN);
		mt.SignalOrders(true);
	}
	catch (UserExc e) {
		LOG(e);
		return false;
	}
	catch (...) {
		return false;
	}
	
	
	WhenRealtimeUpdate();
	WhenPopTask();
	
	return true;
}

int System::GetCommonSymbolId(int common_pos, int symbol_pos) const {
	ASSERT(common_pos >= 0 && common_pos < COMMON_COUNT);
	ASSERT(symbol_pos >= 0 && symbol_pos < SYM_COUNT);
	return sym_priority[common_pos * SYM_COUNT + symbol_pos];
}

int System::FindCommonSymbolId(int sym_id) const {
	return common_symbol_id[sym_id];
}

int System::FindCommonSymbolPos(int sym_id) const {
	return common_symbol_pos[sym_id];
}

int System::FindCommonSymbolPos(int common_id, int sym_id) const {
	return common_symbol_group_pos[common_id][sym_id];
}

int64 System::GetMainDataPos(int64 cursor, int64 sym_pos, int64 tf_pos, int64 bit_pos) const {
	ASSERT(cursor >= 0 && cursor <= main_mem[MEM_INDIBARS]);
	ASSERT(sym_pos >= 0 && sym_pos < main_sym_ids.GetCount());
	ASSERT(tf_pos >= 0 && tf_pos < main_tf_ids.GetCount());
	ASSERT(bit_pos >= 0 && bit_pos < BIT_COUNT);
	return (((cursor * main_sym_ids.GetCount() + sym_pos) * main_tf_ids.GetCount() + tf_pos) * BIT_COUNT + bit_pos);
}

void System::LoadInput(int level, int common_pos, int cursor, double* buf, int bufsize) {
	int buf_pos = 0;
	
	// Time bits
	#if SYS_HAVETIMEIN
	for (int i = 0; i < 5 + SYS_HOURBITS + SYS_MINBITS; i++)
		buf[buf_pos + i] = 1.0;
		
	Time t = GetTimeMain(main_tf, cursor);
	
	int wday = Upp::max(0, Upp::min(5, DayOfWeek(t) - 1));
	buf[buf_pos + wday] = 0.0;
	buf_pos += 5;
	
	buf[buf_pos + t.hour] = 0.0;
	buf_pos += 24;
	
	#if SYS_M15
	buf[buf_pos + t.minute / 15] = 0.0;
	buf_pos += 4;
	#endif
	#endif
	
	int bit = level < 2 ? BIT_L0BITS_BEGIN : BIT_L2BITS_BEGIN;
	int bit_count = level < 2 ? ASSIST_COUNT : L2_INPUT;
	
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			int64 pos = GetMainDataPos(cursor, sym_pos, j, bit);
			for (int k = 0; k < bit_count; k++) {
				bool b = main_data.Get(pos++);
				buf[buf_pos++] =  b ? 0.0 : 1.0;
			}
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == (level < 2 ? LogicLearner0::INPUT_SIZE : LogicLearner2::INPUT_SIZE));
}

void System::LoadOutput(int level, int common_pos, int cursor, double* buf, int bufsize) {
	int bit = level < 2 ? BIT_REALSIGNAL : BIT_REALENABLED;
	int buf_pos = 0;
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			int64 pos = GetMainDataPos(cursor, sym_pos, j, bit);
			bool action = main_data.Get(pos);
			buf[buf_pos++] =  action ? 0.0 : 1.0;
			buf[buf_pos++] = !action ? 0.0 : 1.0;
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == (level < 2 ? LogicLearner0::OUTPUT_SIZE : LogicLearner2::OUTPUT_SIZE));
}

void System::StoreOutput(int level, int common_pos, int cursor, double* buf, int bufsize) {
	int bit, chk_bit;
	switch (level) {
		case 0: bit = BIT_L0_SIGNAL;	chk_bit = BIT_WRITTEN_L0; break;
		case 1: bit = BIT_L1_SIGNAL;	chk_bit = BIT_WRITTEN_L1; break;
		case 2: bit = BIT_L2_ENABLED;	chk_bit = BIT_WRITTEN_L2; break;
		default: Panic("Invalid level");
	}
	int buf_pos = 0;
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			int64 pos = GetMainDataPos(cursor, sym_pos, j, bit);
			double true_prob  = 1.0 - buf[buf_pos++];
			double false_prob = 1.0 - buf[buf_pos++];
			bool action = true_prob > false_prob;
			main_data.Set(pos, action);
			
			pos = GetMainDataPos(cursor, sym_pos, j, chk_bit);
			main_data.Set(pos, true);
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == (level < 2 ? LogicLearner0::OUTPUT_SIZE : LogicLearner2::OUTPUT_SIZE));
}

String System::GetRegisterKey(int i) const {
	switch (i) {
	case REG_INS: return "Instruction";
	case REG_WORKQUEUE_CURSOR: return "Work queue cursor";
	case REG_WORKQUEUE_INITED: return "Is workqueue initialised";
	case REG_INDIBITS_INITED: return "Is indicator bits initialised";
	case REG_LOGICTRAINING_L0_ISRUNNING: return "Is level 0 training running";
	case REG_LOGICTRAINING_L1_ISRUNNING: return "Is level 1 training running";
	case REG_LOGICTRAINING_L2_ISRUNNING: return "Is level 2 training running";
	
	case REG_SIG_L0TOREAL:		return "L0/real";
	case REG_SIG_L1TOREAL:		return "L1/real";
	case REG_SIG_L2TOREAL:		return "L2/real";
	case REG_SIG_L0TOL1:		return "L0/L1";
	case REG_ENA_L0L2TOREAL:	return "L0L2/real";
	case REG_ENA_L1L2TOREAL:	return "L1L2/real";
	case REG_SIG_L0L2TOL1L2:	return "L0L2/L1L2";
	case REG_DD_L0TRAIN:		return "L0 DD Train";
	case REG_DD_L0TEST:			return "L0 DD Test";
	case REG_L0ENA:				return "L0 enabled / real";
	case REG_L1ENA:				return "L1 enabled / real";
		
	default: return IntStr(i);
	}
}

String System::GetRegisterValue(int i, int j) const {
	switch (i) {
	case REG_INS:
		switch (j) {
		case INS_WAIT_NEXTSTEP:				return "Waiting next timestep";
		case INS_REFRESHINDI:				return "Indicator refresh";
		case INS_INDIBITS:					return "Indicator bits in main data";
		case INS_TRAINABLE:					return "Training values";
		case INS_CUSTOMLOGIC:				return "Custom logic values";
		case INS_CALENDARLOGIC:				return "Calendar logic values";
		case INS_REALIZE_LOGICTRAINING:		return "Realize logic training";
		case INS_WAIT_LOGICTRAINING:		return "Wait logic training to finish";
		case INS_LOGICBITS:					return "Logic bits in main data";
		case INS_REFRESH_REAL:				return "Real account";
		default:							return "Unknown";
		};
	
	case REG_WORKQUEUE_CURSOR:
		return IntStr(j) + "/" + IntStr(main_work_queue.GetCount());
	
	case REG_WORKQUEUE_INITED:
	case REG_INDIBITS_INITED:
	case REG_LOGICTRAINING_L0_ISRUNNING:
	case REG_LOGICTRAINING_L1_ISRUNNING:
	case REG_LOGICTRAINING_L2_ISRUNNING:
		return j ? "Yes" : "No";
	
	case REG_SIG_L0TOREAL:
	case REG_SIG_L1TOREAL:
	case REG_SIG_L2TOREAL:
	case REG_SIG_L0TOL1:
	case REG_ENA_L0L2TOREAL:
	case REG_ENA_L1L2TOREAL:
	case REG_SIG_L0L2TOL1L2:
	case REG_DD_L0TRAIN:
	case REG_DD_L0TEST:
	case REG_L0ENA:
	case REG_L1ENA:
		return DblStr(j * 0.1) + "%";
	
	default:
		return IntStr(j);
	}
}

String System::GetMemoryKey(int i) const {
	if (i >= MEM_TRAINMIDSTEP && i < MEM_TRAINMIDSTEP + COMMON_COUNT)	return "Midstep (common " + IntStr(i - MEM_TRAINMIDSTEP) + ")";
	if (i >= MEM_TRAINBEGIN   && i < MEM_TRAINBEGIN   + COMMON_COUNT)	return "Beginning of training (common " + IntStr(i - MEM_TRAINBEGIN) + ")";
	
	switch (i) {
	case MEM_TRAINABLESET:		return "Is trainable bits set";
	case MEM_INDIBARS:			return "Bars (for all)";
	case MEM_COUNTED_INDI:		return "Counted indicator data";
	case MEM_COUNTED_ENABLED:	return "Counted enabled bits";
	case MEM_COUNTED_CALENDAR:	return "Counted calendar bits";
	case MEM_TRAINBARS:			return "Bars (for training)";
	case MEM_COUNTED_L0:		return "Counted for level 0";
	case MEM_COUNTED_L1:		return "Counted for level 1";
	case MEM_COUNTED_L2:		return "Counted for level 2";
	case MEM_TRAINED_L0:		return "Is level 0 trained";
	case MEM_TRAINED_L1:		return "Is level 1 trained";
	case MEM_TRAINED_L2:		return "Is level 2 trained";
	
	default: return IntStr(i);
	}
}

String System::GetMemoryValue(int i, int j) const {
	switch (i) {
	case MEM_TRAINABLESET:
	case MEM_TRAINED_L0:
	case MEM_TRAINED_L1:
	case MEM_TRAINED_L2:
		return j ? "Yes" : "No";
	case MEM_INDIBARS:
	case MEM_COUNTED_INDI:
	case MEM_TRAINBARS:
	case MEM_TRAINMIDSTEP:
	case MEM_TRAINBEGIN:
	case MEM_COUNTED_L0:
	case MEM_COUNTED_L1:
	case MEM_COUNTED_L2:
	default:
		return IntStr(j);
	}
}


/*

void RunSimBroker() {
	System& sys = GetSystem();
	DataBridge* db = dynamic_cast<DataBridge*>(GetInputCore(0, GetSymbol(), GetTf()));

	Buffer& open_buf	= db->GetBuffer(0);
	Buffer& low_buf		= db->GetBuffer(1);
	Buffer& high_buf	= db->GetBuffer(2);
	Buffer& volume_buf	= db->GetBuffer(3);

	int sym = GetSymbol();
	int tf = GetTf();
	int bars = GetBars();
	db->ForceCount(bars);


	sb.Brokerage::operator=(GetMetaTrader());
	sb.SetInitialBalance(10000);
	sb.Init();
	sb.SetFreeMarginLevel(FMLEVEL);

	for(int i = 0; i < SYM_COUNT; i++) {
		check_sum1[i] = 0;
		check_sum2[i] = 0;
	}

	for(int i = main_begin; i < bars; i++) {
		DQN::DQVectorType& current = data[i];

		for(int j = 0; j < SYM_COUNT; j++) {
			int pos = sys.GetShiftTf(GetSymbol(), tf_ids[0], sys.GetCommonSymbol(j), tf_ids[0], i);
			ConstBuffer& open_buf = *this->open_buf[j];
			double curr = open_buf.GetUnsafe(pos);
			int symbol = sys.GetCommonSymbol(j);
			sb.SetPrice(symbol, curr);
		}
		sb.RefreshOrders();

		Time time = sys.GetTimeTf(sym, tf, i);
		int t = time.hour * 100 + time.minute;
		int open_count = 0;

		for(int j = 0; j < SYM_COUNT; j++) {
			int sym			= sys.GetCommonSymbol(j);
			int prev_sig	= sb.GetSignal(sym);
			bool prev_signal  = prev_sig == +1 ? false : true;
			bool prev_enabled = prev_sig !=  0;

			int sig = 0;

			if (is_enabled) {
				sig = signal ? -1 : +1;
				open_count++;
			}
			else if (is_priority)
				open_count++;


			if (sig == sb.GetSignal(sym) && sig != 0)
				sb.SetSignalFreeze(sym, true);
			else {
				sb.SetSignal(sym, sig);
				sb.SetSignalFreeze(sym, false);
			}
		}

		sb.SetFreeMarginScale(open_count ? open_count : 1);

		sb.SignalOrders(false);


		double value = sb.AccountEquity();
		if (value < 0.0) {
			sb.ZeroEquity();
			value = 0.0;
		}
		open_buf.Set(i, value);
		low_buf.Set(i, value);
		high_buf.Set(i, value);
	}
}
*/

}
