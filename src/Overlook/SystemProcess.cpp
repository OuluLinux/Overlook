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
	logic.SetCount(GetCommonCount() * COST_LEVEL_COUNT);
	workers_started = 0;
	for (int i = 0; i < REG_COUNT; i++) main_reg[i] = 0;
	int prev_step = -1;
	
	
	// Start workers
	int workers = Upp::min(GetCommonSymbolCount(), Upp::max(1, CPU_Cores() - 2));
	for (int i = 0; i < workers; i++) {
		Thread::Start(THISBACK1(Worker, i));
		workers_started++;
	}
	
	
	// For developing purposes
	////ClearCounters();
	////ClearL2();
	
	
	// Initial refresh
	mainloop_lock.Enter();
	DataBridgeCommon& common = GetDataBridgeCommon();
	common.InspectInit();
	common.DownloadAskBid();
	common.RefreshAskBidData(true);
	
	
	// Run main loop
	while (main_running && !Thread::IsShutdownThreads()) {
		
		Time t = GetMetaTrader().GetTime();
		#if SYS_M1
		int step = t.minute;
		#elif SYS_M15
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
				mainloop_lock.Leave();
				Sleep(100);
				mainloop_lock.Enter();
				break;
			}
			prev_step = step;
			ins++;
			break;
		
		
		// Wait until logic is trained by worker threads
		case INS_WAIT_LOGICTRAINING:
			for (int l = 0; l < COST_LEVEL_COUNT; l++) {
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
			common.DownloadAskBid();
			common.RefreshAskBidData(true);
			SetEnd(GetUtcTime());
			RealizeMainWorkQueue();
			ProcessMainWorkQueue(true);
			//ProcessEnds();
			ins++;
			break;
			
			
		// Write boolean values from indicators to the main memory.
		// This refresh L0 & L1 input sources.
		case INS_INPUTBITS:
			FillInputBits();
			ins++;
			break;
			
			
		// Refresh persistent variables for training. Only once at first time.
		case INS_TRAINABLE:
			FillTrainableBits();
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
			for(int i = 0; i < COST_LEVEL_COUNT; i++)
				FillLogicBits(i);
			
			// Don't continue to INS_REFRESH_REAL before all levels have been written.
			if (main_mem[MEM_COUNTED_L0 + COST_LEVEL_COUNT - 1] == 0)
				ins = 1;
			else
				ins++;
			break;
			
		
		// Fill active level
		case INS_ACTIVELEVEL:
			FillActive();
			ins++;
		
		
		// Fill some statistics
		case INS_STATS:
			FillStatistics();
			ins++;
			break;
			
			
		// Handle real MT4 account.
		case INS_REFRESH_REAL:
			if (RefreshReal()) {
				ins = 0; // go to waiting when success
				GetMetaTrader().Data();
				PostCallback(WhenRealRefresh);
			}
			else
				ins = 1;
			break;
			
		}
		
		
	}
	
	mainloop_lock.Leave();
	
	main_running = false;
	main_stopped = true;
}

void System::ClearCounters() {
	main_mem[MEM_INPUTBARS] = 0;
	main_mem[MEM_COUNTED_INPUT] = 0;
	for(int i = 0; i < COST_LEVEL_COUNT; i++)
		main_mem[MEM_COUNTED_L0 + i] = 0;
}

void System::RealizeMainWorkQueue() {
	
	// Run only once
	if (main_reg[REG_WORKQUEUE_INITED]) {
		
		// Update data count
		main_mem[MEM_INPUTBARS] = GetCountMain(main_tf);
		return;
	}
	
	
	// Debug assertions
	ASSERT(main_tf_ids.IsEmpty());
	ASSERT(main_sym_ids.IsEmpty());
	ASSERT(main_indi_ids.IsEmpty());
	
	
	// Add indicators
	FactoryDeclaration decl;
	decl.factory = Find<DataBridge>();			main_indi_ids.Add(decl);
	main_factory_ids.Clear();
	for (int i = 0; i < main_indi_ids.GetCount(); i++)
		main_factory_ids.Add(main_indi_ids[i].factory);
	
	
	// Add timeframes
	#if SYS_M1
	if (main_tf_pos == 0) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(1)); // <---
		if (TF_COUNT > 1)	Panic("Invalid TF_COUNT");
	}
	else Panic("Invalid tf pos");
	#elif SYS_M15
	if (main_tf_pos == 0) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(15)); // <---
		if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(60));
		if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(240));
		if (TF_COUNT > 3)	Panic("Invalid TF_COUNT");
	}
	else if (main_tf_pos == 1) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(5));
		if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(15)); // <---
		if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(60));
		if (TF_COUNT > 3)	Panic("Invalid TF_COUNT");
	}
	else Panic("Invalid tf pos");
	#elif SYS_H1
	if (main_tf_pos == 0) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(60)); // <---
		if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(240));
		if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(1440));
		if (TF_COUNT > 3)	Panic("Invalid TF_COUNT");
	}
	else if (main_tf_pos == 1) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(15));
		if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(60)); // <---
		if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(240));
		if (TF_COUNT > 3)	Panic("Invalid TF_COUNT");
	}
	else if (main_tf_pos == 2) {
		if (TF_COUNT > 0)	main_tf_ids.Add(FindPeriod(5));
		if (TF_COUNT > 1)	main_tf_ids.Add(FindPeriod(15));
		if (TF_COUNT > 2)	main_tf_ids.Add(FindPeriod(60)); // <---
		if (TF_COUNT > 3)	Panic("Invalid TF_COUNT");
	}
	else Panic("Invalid tf pos");
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
	main_mem[MEM_INPUTBARS] = bars;
	
	
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

void System::FillInputBits() {
	ASSERT(!main_tf_ids.IsEmpty());
	ASSERT(!main_sym_ids.IsEmpty());
	ASSERT(!main_indi_ids.IsEmpty());
	
	int main_cursor = main_mem[MEM_COUNTED_INPUT];
	bool init = main_cursor == 0;
	
	main_cursor = Upp::max(0, main_cursor - FWD_COUNT); // set previous BIT_REALSIGNAL
	if (init) {
		main_cursor = INT_MAX;
		for(int i = 0; i < main_begin.GetCount(); i++)
			main_cursor = Upp::min(main_cursor, main_begin[i]);
	}
		
	
	int bars = main_mem[MEM_INPUTBARS];
	int train_bars = main_mem[MEM_TRAINBARS];
	ASSERT(bars > 0 && train_bars > 0);
	int64 main_data_count = GetMainDataPos(bars, 0, 0, 0);
	main_data.SetCount(main_data_count);
	
	Vector<bool> output_bits;
	
	for (int i = 0; i < main_tf_ids.GetCount(); i++) {
		int tf = main_tf_ids[i];
		
		for (int j = 0; j < main_sym_ids.GetCount(); j++) {
			int sym = main_sym_ids[j];
			
			int core_bars = GetCountTf(sym, tf);
			int core_train_bars = GetShiftMainTf(main_tf, sym, tf, train_bars - 1);
			
			for (int cost_level = 0; cost_level < COST_LEVEL_COUNT; cost_level++) {
				if (init) {
					output_bits.SetCount(core_bars);
					GetMinimalSignal(0, j, i, core_bars, output_bits.Begin(), core_bars, cost_level);
				}
				
				for (int cursor = main_cursor; cursor < bars && main_running; cursor++) {
					int core_cursor = GetShiftMainTf(main_tf, sym, tf, cursor);
					int begin = Upp::max(0, core_cursor - 100);
					
					// Input
					{
						int end = Upp::min(core_cursor + 1, core_bars);
						bool sigbuf[BWD_COUNT * BWD_COUNT];
						
						int64 main_pos = GetMainDataPos(cursor, j, i, LevelBwd(cost_level, 0));
						
						for (int l = 0; l < BWD_COUNT; l++)
							GetMinimalSignal(begin, j, i, end - l, &sigbuf[l * BWD_COUNT], BWD_COUNT, cost_level);
						
						for (int k = 0; k < BWD_COUNT; k++) {
							int av = BWD_COUNT - k;
							if ((av % 2) == 0) av--;
							int half_av = av / 2;
							int true_count = 0;
							for (int l = 0; l < av; l++) {
								//   k    01234
								// l0     10101		0  1  2  3  4
								// l1    01010		5  6  7  8  9
								// l2   10101		10 11 12 13 14
								// l3  01010		15 16 17 18 19
								// l4 10101			20 21 22 23 24
								// av     54321
								// k = 0, l = 0 --> 0
								// k = 0, l = 1 --> 6
								// k = 0, l = 2 --> 12
								// pos = l * (5 + 1) + k
								// av = 5 - k
								int pos = l * (BWD_COUNT + 1) + k;
								bool value = sigbuf[pos];
								if (value)
									true_count++;
							}
							bool sig = true_count > half_av;
							main_data.Set(main_pos++, sig);
						}
					}
					
					// Output
					if (init && core_cursor + FWD_COUNT <= core_train_bars) {
						int64 main_pos = GetMainDataPos(cursor, j, i, LevelFwd(cost_level, 0));
						for (int k = 0; k < FWD_COUNT; k++)
							main_data.Set(main_pos++, output_bits[core_cursor + k]);
						
						main_data.Set(GetMainDataPos(cursor, j, i, LevelWrittenFwd(cost_level)), true);
					}
				}
			}
		}
	}
	if (!main_running) return;
	
	main_mem[MEM_COUNTED_INPUT] = bars;
	main_reg[REG_INDIBITS_INITED] = true;
	if (init) StoreAll();
}

void System::GetMinimalSignal(int begin, int sym_pos, int tf_pos, int end, bool* sigbuf, int sigbuf_size, int cost_level) {
	int core_pos = GetOrderedCorePos(sym_pos, tf_pos, 0);
	ConstBuffer& open_buf = ordered_cores[core_pos]->GetBuffer(0);
	double spread_point		= spread_points[main_sym_ids[sym_pos]];
	double slippage			= spread_point * cost_level;
	double cost				= spread_point + slippage;
	
	int write_begin = end - sigbuf_size;
	
	for(int i = begin; i < end; i++) {
		double open = open_buf.GetUnsafe(i);
		double close = open;
		int j = i + 1;
		bool can_break = false;
		bool break_label;
		double prev = open;
		for(; j < end; j++) {
			close = open_buf.GetUnsafe(j);
			if (!can_break) {
				double abs_diff = fabs(close - open);
				if (abs_diff >= cost) {
					break_label = close < open;
					can_break = true;
				}
			} else {
				bool change_label = close < prev;
				if (change_label != break_label) {
					j--;
					break;
				}
			}
			prev = close;
		}
		
		bool label = close < open;
		
		for(int k = i; k < j; k++) {
			int buf_pos = k - write_begin;
			if (buf_pos >= 0)
				sigbuf[buf_pos] = label;
		}
		
		i = j - 1;
	}
}

int System::GetOrderedCorePos(int sym_pos, int tf_pos, int factory_pos) {
	return (sym_pos * main_tf_ids.GetCount() + tf_pos) * main_factory_ids.GetCount() + factory_pos;
}

void System::FillTrainableBits() {

	// Set trainable bits only once to keep fixed training dataset
	dword& is_set_already = main_mem[MEM_TRAINABLESET];
	if (is_set_already)
		return;
		
	dword bars = main_mem[MEM_INPUTBARS];
	ASSERT(bars > 0);
	
	dword& train_bars = main_mem[MEM_TRAINBARS];
	train_bars = bars - FWD_COUNT - 1;
	
	for (int i = 0; i < GetCommonCount(); i++)
		main_mem[MEM_TRAINBEGIN + i] = main_begin[i];
	
	is_set_already = true;
}

void System::FillCalendarBits() {
	Calendar& cal = GetCalendar();
	cal.Data();
	
	dword& cursor = main_mem[MEM_COUNTED_CALENDAR];
	
	cursor = Upp::max(0, (int)cursor - 8); // set trailing bits
	
	Time now = GetUtcTime();
	int bars = main_mem[MEM_INPUTBARS];
	
	for (; cursor < cal.GetCount(); cursor++) {
		const CalEvent& ce = cal.GetEvent(cursor);
		if (ce.timestamp > now) break;
		
		if (ce.impact < 2) continue;
		
		Time t = ce.timestamp;
		#if SYS_M1
		t.second = 0;
		int begin_count = 15, end_count = 15;
		#elif SYS_M15
		t += (15 * 0.5) * 60;
		t -= (t.minute % 15) * 60 + t.second;
		int begin_count = 1, end_count = 1;
		#elif SYS_H1
		t.minute = 0;
		t.second = 0;
		int begin_count = 1, end_count = 0;
		#endif
		int current_main_pos = main_time[main_tf].Find(t);
		if (current_main_pos == -1) continue;
		
		int begin = Upp::max(0, current_main_pos - begin_count);
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
	if (main_mem[MEM_COUNTED_INPUT] == 0) return;
	
	for (int l = 0; l < COST_LEVEL_COUNT; l++) {
		dword& is_trained = main_mem[MEM_TRAINED_L0 + l];
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
	const dword train_begin		= sys.main_mem[System::MEM_TRAINBEGIN + job.common_pos];
	
	int data_count = train_bars - train_begin;
	int data_begin = train_begin;
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
	return TrainLogic(job, *this, logic[job.level * GetCommonCount() + job.common_pos]);
}

template <class T> void FillLogicBits(int level, int common_pos, System& sys, T& logic) {
	dword bars					= sys.main_mem[System::MEM_INPUTBARS];
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
	}
}

void System::FillLogicBits(int level) {
	bool is_trained				= main_mem[MEM_TRAINED_L0 + level];
	if (!is_trained) return;
	
	dword bars					= main_mem[System::MEM_INPUTBARS];
	dword& cursor				= main_mem[System::MEM_COUNTED_L0 + level];
	
	for (int common_pos = 0; common_pos < GetCommonCount(); common_pos++) {
		::Overlook::FillLogicBits(level, common_pos, *this, logic[level * GetCommonCount() + common_pos]);
	}
	
	cursor = bars;
}

void System::FillStatistics() {
	if (main_mem[System::MEM_COUNTED_L0 + COST_LEVEL_COUNT - 1] == 0) return;
	if (main_reg[REG_SIG_L0TOREAL] != 0) return;
	
	int begin = 0;
	for(int i = 0; i < GetCommonCount(); i++)
		begin = Upp::max(begin, (int)main_mem[MEM_TRAINBEGIN + i]);
	
	int count_mul = main_sym_ids.GetCount() * main_tf_ids.GetCount();
	int bars = main_mem[MEM_TRAINBARS];
	int count = (bars - begin) * count_mul;
	
	for (int cost_level = 0; cost_level < COST_LEVEL_COUNT; cost_level++) {
		int correct = 0;
		
		for(int i = begin; i < bars; i++) {
			for (int tf_pos = 0; tf_pos < main_tf_ids.GetCount(); tf_pos++) {
				for (int sym_pos = 0; sym_pos < main_sym_ids.GetCount(); sym_pos++) {
					int64 pos;
					
					pos = GetMainDataPos(i, sym_pos, tf_pos, LevelFwd(cost_level, 0));
					bool real_sig = main_data.Get(pos);
					pos = GetMainDataPos(i, sym_pos, tf_pos, LevelSignal(cost_level));
					bool sig = main_data.Get(pos);
					
					if (sig == real_sig)		correct++;
				}
			}
		}
		
		main_reg[REG_SIG_L0TOREAL + cost_level]		= 1000 * correct / count;
	}
}

bool System::RefreshReal() {
	Time now = GetUtcTime();
	int wday				= DayOfWeek(now);
	Time after_3hours		= now + 3 * 60 * 60;
	int wday_after_3hours	= DayOfWeek(after_3hours);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	if (periods[main_tf] < 60) {
		now.minute -= now.minute % periods[main_tf];
	}
	else if (periods[main_tf] < 1440) {
		now.minute = 0;
		now.hour -= periods[main_tf] / 60;
	}
	else {
		now.minute = 0;
		now.hour = 0;
	}
	
	
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
	
	int current_main_pos = main_time[main_tf].Find(now);
	int last_pos = main_time[main_tf].GetCount() - 1;
	if (current_main_pos == -1) {
		LOG("error: current main pos not found");
		return false;
	}
	else if (current_main_pos != last_pos)
		Panic(
			"Invalid current pos: " + IntStr(current_main_pos) + " != " + IntStr(last_pos) +
			" (" + Format("%", main_time[main_tf].GetKey(current_main_pos)) + " != " + Format("%", main_time[main_tf].GetKey(last_pos)) + ")");
	
	if (current_main_pos >= main_mem[MEM_INPUTBARS])
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
				
				bool enabled_bit = false;
				int enabled_costlevel = -1;
				for(int k = 0; k < COST_LEVEL_COUNT; k++) {
					if (main_data.Get(GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, LevelActive(k)))) {
						enabled_bit = true;
						enabled_costlevel = k;
						break;
					}
				}
				
				int sig = 0;
				int prev_sig = mt.GetSignal(sym_id);
				if (enabled_bit) {
					bool signal_bit = main_data.Get(GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, LevelSignal(enabled_costlevel)));
					sig = signal_bit ? -1 : +1;
				}
				
				// Avoid unpredictable calendar events...
				if (sig && main_data.Get(GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, BIT_SKIP_CALENDAREVENT)))
						sig = 0;
					
				signals.Add(sig);
				if (sig == prev_sig && sig != 0)
					open_count++;
				
				// Do some quality checks
				if (enabled_costlevel != -1) {
					int64
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, LevelWrittenFwd(enabled_costlevel));
					int written_fwd = main_data.Get(pos);
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, LevelWrittenDqn(enabled_costlevel));
					int not_written_dqn = !main_data.Get(pos);
					pos = GetMainDataPos(current_main_pos, sym_pos, main_tf_pos, LevelActive(enabled_costlevel));
					int not_written_act = !main_data.Get(pos);
					int e = (written_fwd << 2) | (not_written_dqn << 1) | (not_written_act << 0);
					if (e)
						Panic("Real account function quality check failed: error code " + IntStr(e) + " sym=" + IntStr(sym_id));
				}
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
	ASSERT(cursor >= 0 && cursor <= main_mem[MEM_INPUTBARS]);
	ASSERT(sym_pos >= 0 && sym_pos < main_sym_ids.GetCount());
	ASSERT(tf_pos >= 0 && tf_pos < main_tf_ids.GetCount());
	ASSERT(bit_pos >= 0 && bit_pos < BIT_COUNT);
	return (((cursor * main_sym_ids.GetCount() + sym_pos) * main_tf_ids.GetCount() + tf_pos) * BIT_COUNT + bit_pos);
}

void System::FillActive() {
	dword& cursor = main_mem[MEM_COUNTED_ACTIVE];
	
	const int refresh_step = 15;
	
	if (!cursor) {
		cursor = main_mem[MEM_TRAINBARS];
		cursor -= cursor % refresh_step;
	}
	
	int bars = main_mem[MEM_INPUTBARS];
	
	for(; cursor < bars; cursor++) {
		
		// Copy previous
		if ((cursor % refresh_step) != 0) {
			for(int i = 0; i < main_sym_ids.GetCount(); i++) {
				for(int j = 0; j < main_tf_ids.GetCount(); j++) {
					for(int k = 0; k < COST_LEVEL_COUNT; k++) {
						int bit = LevelActive(j);
						main_data.Set(GetMainDataPos(cursor,   i, j, bit),
						main_data.Get(GetMainDataPos(cursor-1, i, j, bit)));
					}
				}
			}
		} else {
			int begin = cursor - 1 * 60 * 60;
			
			for(int i = 0; i < main_sym_ids.GetCount(); i++) {
				for(int j = 0; j < main_tf_ids.GetCount(); j++) {
					int sym_id = main_sym_ids[i];
					
					double top_result = -DBL_MAX;
					int top_cost_level = -1;
	
					for(int k = 0; k < COST_LEVEL_COUNT; k++) {
						double result = RunTest(sym_id, i, begin, cursor+1, k);
						
						if (result > top_result){
							top_result = result;
							top_cost_level = k;
						}
					}
					
					if (top_result < 0)
						top_cost_level = -1;
					
					for(int k = 0; k < COST_LEVEL_COUNT; k++) {
						int bit = LevelActive(k);
						main_data.Set(GetMainDataPos(cursor, i, j, bit), k == top_cost_level);
					}
				}
			}
		}
	}
}

double System::RunTest(int sym_id, int sym_pos, int begin, int end, int cost_level) {
	ConstBuffer& open_buf = ordered_cores[GetOrderedCorePos(sym_pos, main_tf_pos, 0)]->GetBuffer(0);
	
	double change_total = 0.0;
	bool prev_signal = 0;
	double spread_point = spread_points[sym_id];
	
	double open;
	int open_len = -1;
	bool first_open = true;
	int sig_bit = LevelSignal(cost_level);
	
	for(int cursor = begin; cursor < end; cursor++) {
		bool used_signal = main_data.Get(GetMainDataPos(cursor, sym_pos, main_tf_pos, sig_bit));
		
		open_len++;
		
		bool do_close = false, do_open = false;
		do_open = prev_signal != used_signal || first_open;
		do_close = do_open && open_len > 0;
		first_open = false;
		if (do_close || do_open) {
			double curr = open_buf.GetUnsafe(GetShiftFromMain(sym_id, main_tf, cursor));
			ASSERT(curr > 0.0);
			if (do_close) {
				double change;
				if (!prev_signal)	change = curr / (open + spread_point) - 1.0;
				else				change = 1.0 - curr / (open - spread_point);
				change_total += change;
			}
			if (do_open) {
				open = curr;
				open_len = 0;
			}
		}
		
		prev_signal = used_signal;
	}
	
	if (open_len > 0) {
		double curr = open_buf.GetUnsafe(GetShiftFromMain(sym_id, main_tf, end - 1));
		ASSERT(curr > 0.0);
		if (!prev_signal)	change_total += curr / (open + spread_point) - 1.0;
		else				change_total += 1.0 - curr / (open - spread_point);
	}
	
	return change_total;
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
	
	int bit = LevelBwd(level, 0);
	int bit_count = BWD_COUNT;
	
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			int64 pos = GetMainDataPos(cursor, sym_pos, j, bit);
			for (int k = 0; k < bit_count; k++) {
				bool b = main_data.Get(pos++);
				buf[buf_pos++] = b ? 1.0 : 0.0;
			}
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == LogicLearner::INPUT_SIZE);
}

void System::LoadOutput(int level, int common_pos, int cursor, double* buf, int bufsize) {
	int bit = LevelFwd(level, 0);
	int bit_count = FWD_COUNT;
	
	int buf_pos = 0;
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			int64 pos = GetMainDataPos(cursor, sym_pos, j, bit);
			for(int k = 0; k < bit_count; k++) {
				bool b = main_data.Get(pos++);
				buf[buf_pos++] = b ? 1.0 : 0.0;
			}
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == LogicLearner::OUTPUT_SIZE);
}

void System::StoreOutput(int level, int common_pos, int cursor, double* buf, int bufsize) {
	const int lside_count = FWD_COUNT - 1;
	const int rside_count = FWD_COUNT;
	int lside_bit = LevelBwd(level, BWD_COUNT - lside_count);
	int sig_bit = LevelSignal(level);
	int chk_bit	= LevelWrittenDqn(level);
	int buf_pos = 0;
	for (int i = 0; i <= GetCommonSymbolCount(); i++) {
		int sym_pos = common_pos * (GetCommonSymbolCount() + 1) + i;
		for (int j = 0; j < main_tf_ids.GetCount(); j++) {
			
			// Get symmetric average from real previous and predicted next
			int true_count = 0;
			int64 pos = GetMainDataPos(cursor, sym_pos, j, lside_bit);
			for (int k = 0; k < lside_count; k++) {
				if (main_data.Get(pos++))
					true_count++;
			}
			for(int k = 0; k < rside_count; k++) {
				if (buf[buf_pos++] >= 0.5)
					true_count++;
			}
			bool sig = true_count > lside_count;
			
			pos = GetMainDataPos(cursor, sym_pos, j, sig_bit);
			main_data.Set(pos, sig);
			
			pos = GetMainDataPos(cursor, sym_pos, j, chk_bit);
			main_data.Set(pos, true);
		}
	}
	ASSERT(buf_pos == bufsize);
	ASSERT(buf_pos == rside_count * (GetCommonSymbolCount() + 1) * main_tf_ids.GetCount());
	ASSERT(buf_pos == LogicLearner::OUTPUT_SIZE);
}

String System::GetRegisterKey(int i) const {
	switch (i) {
	case REG_INS: return "Instruction";
	case REG_WORKQUEUE_CURSOR: return "Work queue cursor";
	case REG_WORKQUEUE_INITED: return "Is workqueue initialised";
	case REG_INDIBITS_INITED: return "Is indicator bits initialised";
	case REG_LOGICTRAINING_L0_ISRUNNING: return "Is level 0 training running";
	
	case REG_SIG_L0TOREAL:		return "L0/real";
	case REG_SIG_L1TOREAL:		return "L1/real";
	case REG_SIG_L2TOREAL:		return "L2/real";
	case REG_SIG_L3TOREAL:		return "L3/real";
		
	default: return IntStr(i);
	}
}

String System::GetRegisterValue(int i, int j) const {
	switch (i) {
	case REG_INS:
		switch (j) {
		case INS_WAIT_NEXTSTEP:				return "Waiting next timestep";
		case INS_REFRESHINDI:				return "Indicator refresh";
		case INS_INPUTBITS:					return "Input bits in main data";
		case INS_TRAINABLE:					return "Training values";
		case INS_CALENDARLOGIC:				return "Calendar logic values";
		case INS_REALIZE_LOGICTRAINING:		return "Realize logic training";
		case INS_WAIT_LOGICTRAINING:		return "Wait logic training to finish";
		case INS_LOGICBITS:					return "Logic bits in main data";
		case INS_ACTIVELEVEL:				return "Active level";
		case INS_STATS:						return "Statistics";
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
	case REG_LOGICTRAINING_L3_ISRUNNING:
		return j ? "Yes" : "No";
	
	case REG_SIG_L0TOREAL:
	case REG_SIG_L1TOREAL:
	case REG_SIG_L2TOREAL:
	case REG_SIG_L3TOREAL:
		return DblStr(j * 0.1) + "%";
	
	default:
		return IntStr(j);
	}
}

String System::GetMemoryKey(int i) const {
	if (i >= MEM_TRAINBEGIN   && i < MEM_TRAINBEGIN   + COMMON_COUNT)	return "Beginning of training (common " + IntStr(i - MEM_TRAINBEGIN) + ")";
	
	switch (i) {
	case MEM_TRAINABLESET:		return "Is trainable bits set";
	case MEM_INPUTBARS:			return "Bars (for all)";
	case MEM_COUNTED_INPUT:		return "Counted indicator data";
	case MEM_COUNTED_CALENDAR:	return "Counted calendar bits";
	case MEM_TRAINBARS:			return "Bars (for training)";
	case MEM_COUNTED_L0:		return "Counted for level 0";
	case MEM_COUNTED_L1:		return "Counted for level 1";
	case MEM_COUNTED_L2:		return "Counted for level 2";
	case MEM_COUNTED_L3:		return "Counted for level 3";
	case MEM_TRAINED_L0:		return "Is level 0 trained";
	case MEM_TRAINED_L1:		return "Is level 1 trained";
	case MEM_TRAINED_L2:		return "Is level 2 trained";
	case MEM_TRAINED_L3:		return "Is level 3 trained";
	
	default: return IntStr(i);
	}
}

String System::GetMemoryValue(int i, int j) const {
	switch (i) {
	case MEM_TRAINABLESET:
	case MEM_TRAINED_L0:
	case MEM_TRAINED_L1:
	case MEM_TRAINED_L2:
	case MEM_TRAINED_L3:
		return j ? "Yes" : "No";
	case MEM_INPUTBARS:
	case MEM_COUNTED_INPUT:
	case MEM_TRAINBARS:
	case MEM_TRAINBEGIN:
	case MEM_COUNTED_L0:
	case MEM_COUNTED_L1:
	case MEM_COUNTED_L2:
	case MEM_COUNTED_L3:
	default:
		return IntStr(j);
	}
}

}
