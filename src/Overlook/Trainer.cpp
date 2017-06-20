#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Trainer::Trainer(System& sys) : sys(&sys) {
	stopped = true;
	running = false;
}

void Trainer::Init() {
	int tf, indi;
	
	
	tf = sys->FindPeriod(10080);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	tf = sys->FindPeriod(1440);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	tf = sys->FindPeriod(240);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	tf = sys->FindPeriod(30);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	
	
	MetaTrader& mt = GetMetaTrader();
	String acc_cur = mt.AccountCurrency();
	
	if (0) {
		int basket_begin = mt.GetSymbolCount() + mt.GetCurrencyCount();
		
		for(int i = mt.GetSymbolCount(), j = 0; i < basket_begin && j < 4; i++, j++) {
			sym_ids.Add(i);
		}
		
		for(int i = basket_begin, j = 0; i < sys->GetTotalSymbolCount() && j < 4; i++, j++) {
			sym_ids.Add(i);
		}
	} else {
		int basket_begin = mt.GetSymbolCount() + mt.GetCurrencyCount();
		for(int i = basket_begin, j = 0; i < sys->GetTotalSymbolCount() && j < 2; i++, j++) {
			sym_ids.Add(i);
		}
	}
	
	
	indi = sys->Find<MovingAverageConvergenceDivergence>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<StandardDeviation>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<CommodityChannelIndex>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<WilliamsPercentRange>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<CorrelationOscillator>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	
}

void Trainer::InitAgents() {
	int buf_count = value_buffers[0][0].GetCount();
	ASSERT(buf_count);
	String param_str =	"{\n"
						"\t\"update\":\"qlearn\",\n"
						"\t\"gamma\":0.9,\n"
						"\t\"epsilon\":0.2,\n"
						"\t\"alpha\":0.005,\n"
						"\t\"experience_add_every\":5,\n"
						"\t\"experience_size\":10000,\n"
						"\t\"learning_steps_per_iteration\":5,\n"
						"\t\"tderror_clamp\":1.0,\n"
						"\t\"num_hidden_units\":100,\n"
						"}\n";
	seqs.SetCount(4);
	agents.SetCount(tf_ids.GetCount() * indi_ids.GetCount());
	const int sensor_count = 1 + 1 + 5 + buf_count * 2 + (sym_ids.GetCount()-1) + 2 + tf_ids.GetCount();
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		for(int j = 0; j < indi_ids.GetCount(); j++) {
			int agent_id = j * tf_ids.GetCount() + i;
			SDQNAgent& agent = agents[agent_id];
			agent.Init(1, sensor_count, 3);
			agent.LoadInitJSON(param_str);
			agent.Reset();
		}
	}
}

void Trainer::Start() {
	stopped = false;
	running = true;
	Thread::Start(THISBACK(Runner));
}

void Trainer::Stop() {
	running = false;
	while (!stopped) Sleep(100);
}

void Trainer::Runner() {
	
	// Sanity checks
	ASSERT(!seqs.IsEmpty());
	ASSERT(!agents.IsEmpty());
	
	
	// Construct variables
	Vector<double> slist;
	Vector<Vector<int> > prev_actions;
	Vector<int> rand_signals;
	CoWork co;
	int sequence_size = 24;
	int subtimesteps = 4;
	int buf_count = value_buffers[0][0].GetCount();
	ASSERT(buf_count);
	rand_signals.SetCount(sym_ids.GetCount());
	prev_actions.SetCount(sym_ids.GetCount());
	for(int i = 0; i < prev_actions.GetCount(); i++) prev_actions[i].SetCount(tf_ids.GetCount(), 0);
	
	
	// Set sequence-count in agents
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].SetSequenceCount(seqs.GetCount());
	
	
	while (running) {
		
		// Train different timeframes
		for (int tf_iter = 0; tf_iter < tf_ids.GetCount() && running; tf_iter++) {
			Iterator& iter = iters[tf_iter];
			int begin = iter.pos.Top();
			int best_seq = -1;
			double best_seq_max = -DBL_MAX;
			bool reset_pos = false;
			
			// Randomize initial signals
			for(int i = 0; i < rand_signals.GetCount(); i++)
				rand_signals[i] = -5 + Random(11);
			
			for (int seq = 0; seq < seqs.GetCount() && running; seq++) {
				
				// Reset simbroker
				SimBroker& sb = seqs[seq];
				sb.Clear();
				
				// Set initial random signal to every sequence to avoid overfitting 0-signal beginning
				for(int i = 0; i < rand_signals.GetCount(); i++)
					sb.SetSignal(sym_ids[i], rand_signals[i]);
				
				// Seek begin of the sequence time
				Seek(tf_iter, begin);
				
				// Iterate sequence
				for(int i = 0; i < agents.GetCount(); i++)
					agents[i].BeginSequence(seq);
				
				// Iterate timesteps
				for(int i = 0; i < sequence_size && running; i++) {
					
					// Iterate steps in one timestep
					for (int substep = 0; substep < subtimesteps; substep++) {
						
						for(int tf = 0; tf <= tf_iter; tf++) {
							for(int sym = 0; sym < sym_ids.GetCount(); sym++) {
								int sym_id = sym_ids[sym];
								int agent_id = sym * tf_ids.GetCount() + tf;
								SDQNAgent& agent = agents[agent_id];
								
								const int sensor_count = 1 + 1 + 5 + buf_count * 2 + (sym_ids.GetCount()-1) + 2 + tf_ids.GetCount();
								slist.SetCount(sensor_count);
								int j = 0;
								
								
								// Timeframe depth
								slist[j++] = tf_iter;
								
								
								// Time
								slist[j++] = substep;
								for(int i = 0; i < iter.time_values.GetCount(); i++)
									slist[j++] = iter.time_values[i];
								ASSERT(!slist.IsEmpty());
								
								
								// Technical indicators
								const Vector<DoublePair>& values = iter.value[tf][sym];
								for(int i = 0; i < values.GetCount(); i++) {
									const DoublePair& v = values[i];
									slist[j++] = v.a;
									slist[j++] = v.b != 0.0 ? (v.a / v.b - 1.0) : 0.0;
								}
								
								
								// Symbol signal
								int total_sig = 0;
								int sym_sig = 0;
								for(int s = 0; s < sym_ids.GetCount(); s++) {
									int sig = sb.GetSignal(s);
									if (s == sym)
										sym_sig = sig;
									else {
										slist[j++] = sig;
										total_sig += abs(sig);
									}
								}
								
								
								// Total agent volume
								double sym_frac_sig = total_sig != 0.0 ? (double)abs(sym_sig) / (double)total_sig : 0.0;
								slist[j++] = sym_frac_sig;
								slist[j++] = total_sig;
								
								
								// Longer tf action in same symbol
								for(int i = 0; i < tf_iter; i++)
									slist[j++] = prev_actions[sym][i];
								for(int i = tf_iter; i < tf_ids.GetCount(); i++)
									slist[j++] = 0.0;
								
								
								// Act according to sensor inputs
								ASSERT(j == sensor_count);
								int action = agent.Act(slist);
								prev_actions[sym][tf] = action;
								if (action == 1)
									sb.PutSignal(sym_id, +1);
								else if (action == 2)
									sb.PutSignal(sym_id, -1);
							}
						}
					}
					
					// Make final orders for timestep
					sb.Cycle();
					
					// Increase heatmap iterator timepos
					if (!SeekCur(tf_iter, +1)) {
						reset_pos = true;
						break;
					}
				}
				
				// Compare results
				LOG(Format("Compare: seq=%d, equity=%f", seq, sb.AccountEquity()));
				if (sb.AccountEquity() > sb.GetInitialBalance()) {
					double equity = sb.AccountEquity();
					if (equity >= best_seq_max) {
						best_seq_max = equity;
						best_seq = seq;
					}
				}
			}
			
			// Train best sequence with agents
			if (best_seq != -1 && running) {
				LOG(Format("Best sequence: seq=%d, equity=%f", best_seq, best_seq_max));
				for(int i = 0; i < agents.GetCount(); i++)
					co & THISBACK2(Learn, i, best_seq);
				co.Finish();
			}
			
			// Check for end of data
			if (reset_pos)
				Seek(tf_iter, 0);
		}
		
		Sleep(100);
	}
	
	stopped = true;
}

void Trainer::Learn(int agent_id, int best_seq) {
	SDQNAgent& agent = agents[agent_id];
	agent.Learn(best_seq, 0.0);
}

void Trainer::RefreshWorkQueue() {
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
}

void Trainer::ResetValueBuffers() {
	// Find value buffer ids
	VectorMap<int, int> bufout_ids;
	int buf_id = 0;
	for(int i = 0; i < indi_ids.GetCount(); i++) {
		bufout_ids.Add(indi_ids[i], buf_id);
		int indi = indi_ids[i];
		const FactoryRegister& reg = sys->GetRegs()[indi];
		buf_id += reg.out[0].visible;
	}
	int buf_count = buf_id;
	
	
	// Reserve memory for value buffer vector
	value_buffers.Clear();
	value_buffers.SetCount(sym_ids.GetCount());
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		int sym = sym_ids[i];
		Vector<Vector<ConstBuffer*> >& tf_buffers = value_buffers[i];
		tf_buffers.SetCount(tf_ids.GetCount());
		for(int j = 0; j < tf_buffers.GetCount(); j++)
			tf_buffers[j].SetCount(buf_count, NULL);
	}
	
	
	// Get value buffers
	int total_bufs = 0;
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.outputs[0];
		
		int sym = ci.sym;
		int tf  = ci.tf;
		int sym_id = sym_ids.Find(sym);
		int tf_id = tf_ids.Find(tf);
		if (sym_id == -1 || tf_id == -1) continue;
		
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
}

void Trainer::ProcessWorkQueue() {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		LOG(i << "/" << work_queue.GetCount());
		sys->WhenProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
}

void Trainer::ResetIterators() {
	ASSERT(!value_buffers.IsEmpty());
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	
	DoublePair zero_pair(0.0,0.0);
	
	iters.SetCount(tf_ids.GetCount());
	for(int i = 0; i < iters.GetCount(); i++) {
		int main_tf = tf_ids[i];
		
		Iterator& iter = iters[i];
		iter.bars = sys->GetCountTf(main_tf);
		iter.value_count = buf_count;
		
		// Add periods and their multipliers to next longer timeframes
		int tf_count = i + 1;
		iter.pos.SetCount(tf_count, 0);
		int prev_period = 0;
		for(int j = 0; j < tf_count; j++) {
			int tf = tf_ids[j];
			int period = sys->GetPeriod(tf);
			iter.tfs.Add(tf);
			iter.periods.Add(period);
			if (prev_period == 0) iter.period_in_slower.Add(0);
			else                  iter.period_in_slower.Add(prev_period / period);
			prev_period = period;
		}
		
		// Reserve memory for values
		iter.value.SetCount(tf_count);
		iter.min_value.SetCount(tf_count);
		iter.max_value.SetCount(tf_count);
		for(int j = 0; j < iter.value.GetCount(); j++) {
			Vector<Vector<DoublePair> >& sym_values		= iter.value[j];
			Vector<double>& min_sym_values			= iter.min_value[j];
			Vector<double>& max_sym_values			= iter.max_value[j];
			sym_values.SetCount(sym_count);
			min_sym_values.SetCount(sym_count, 0.0);
			max_sym_values.SetCount(sym_count, 0.0);
			for(int k = 0; k < sym_values.GetCount(); k++) {
				Vector<DoublePair>& values = sym_values[k];
				values.SetCount(buf_count, zero_pair);
			}
		}
		
		// Get time range between shortest timeframe and now
		iter.begin = sys->GetBegin(main_tf);
		iter.begin_ts = sys->GetBeginTS(main_tf);
		
		// Seek to beginning
		Seek(i, 0);
	}
}

bool Trainer::Seek(int tf_iter, int shift) {
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	int main_tf = tf_ids[tf_iter];
	Iterator& iter = iters[tf_iter];
	if (shift >= iter.bars) {
		iter.bars = sys->GetCountTf(main_tf);
		if (shift >= iter.bars)
			return false;
	}
	else if (shift < 0)
		return false;
	
	
	// Get some time values in binary format (starts from 0)
	Time t = sys->GetTimeTf(main_tf, shift);
	int month = t.month-1;
	int day = t.day-1;
	int hour = t.hour;
	int minute = t.minute;
	int dow = DayOfWeek(t);
	iter.time_values.SetCount(5);
	iter.time_values[0] = month;
	iter.time_values[1] = day;
	iter.time_values[2] = dow;
	iter.time_values[3] = hour;
	iter.time_values[4] = minute;
	
	
	// Find that time-position in longer timeframes
	iter.pos[tf_iter] = shift;
	for(int i = 0; i < tf_iter; i++) {
		int tf = iter.tfs[i];
		int slow_shift = sys->GetShiftTf(main_tf, tf, shift);
		ASSERT(slow_shift >= 0);
		iter.pos[i] = slow_shift;
	}
	
	// Refresh values
	for(int i = 0; i < iter.value.GetCount(); i++) {
		int pos = iter.pos[i];
		Vector<Vector<DoublePair> >& sym_values		= iter.value[i];
		Vector<double>& min_sym_values			= iter.min_value[i];
		Vector<double>& max_sym_values			= iter.max_value[i];
		sym_values.SetCount(sym_count);
		min_sym_values.SetCount(buf_count, 0.0);
		max_sym_values.SetCount(buf_count, 0.0);
		for(int j = 0; j < buf_count; j++) {
			min_sym_values[j] = +DBL_MAX;
			max_sym_values[j] = -DBL_MAX;
		}
		for(int j = 0; j < sym_values.GetCount(); j++) {
			Vector<DoublePair>& values = sym_values[j];
			Vector<ConstBuffer*>& indi_buffers = value_buffers[j][i];
			for(int k = 0; k < values.GetCount(); k++) {
				ConstBuffer& src = *indi_buffers[k];
				DoublePair& dst = values[k];
				dst.a = src.GetUnsafe(pos);
				if (pos) dst.b = src.GetUnsafe(pos-1);
				else dst.b = dst.a;
				double& min = min_sym_values[k];
				double& max = max_sym_values[k];
				if (dst.a < min) min = dst.a;
				if (dst.a > max) max = dst.a;
			}
		}
	}
	
	return true;
}

bool Trainer::SeekCur(int tf_iter, int shift) {
	Iterator& iter = iters[tf_iter];
	int new_shift = iter.pos.Top() + shift;
	if (new_shift < 0) return false;
	if (new_shift >= iter.bars) return false;
	return Seek(tf_iter, new_shift);
}

}
