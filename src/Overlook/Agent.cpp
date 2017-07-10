#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent(System& sys) : sys(&sys) {
	not_stopped = 0;
	running = false;
	input_width = 0;
	input_height = 0;
	input_depth = 0;
	test_interval = 1000;
	training_limit = 1000;
	session_cur = 0;
	max_sequences = 5;
	buf_count = 0;
	
	thrd_count = 2;//CPU_Cores();
	max_tmp_sequences = 8;
	sequence_count = 0;
	
	symset_hash = 0;
	prev_symset_hash = 0;
	seq_cur = 0;
	
	// what epsilon value do we bottom out on? 0.0 => purely deterministic policy at end
	epsilon_min = 0.005;
	

	// number of epochs we will learn for
	learning_epochs_total = 100;
	
	// how many epochs of the above to perform only random actions (in the beginning)?
	learning_epochs_burnin = 5;
	
}

Agent::~Agent() {
	Stop();
	StoreThis();
}

void Agent::LoadThis() {
	LoadFromFile(*this,	ConfigFile("trainer.bin"));
}

void Agent::StoreThis() {
	String backup_dir = ConfigFile("backups");
	RealizeDirectory(backup_dir);
	Time t = GetSysTime();
	String backup_file = Format("%d_%d_%d_%d_%d.bin", t.year, t.month, t.day, t.hour, t.minute);
	StoreToFile(*this,	AppendFileName(backup_dir, backup_file));
	StoreToFile(*this,	ConfigFile("trainer.bin"));
}

void Agent::Init() {
	int tf, indi;
	
	LoadThis();
	
	
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
	
	tf_limit = tf_ids.GetCount() - 1; // skip M30
	
	
	tf_muls.SetCount(tf_ids.GetCount(), 1.0);
	tf_periods.SetCount(tf_ids.GetCount(), 1);
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int period = sys->GetPeriod(tf_ids[i]) / sys->GetPeriod(tf_ids.Top());
		tf_muls[i] = 1.0 / period;
		tf_periods[i] = period;
	}
	
	
	MetaTrader& mt = GetMetaTrader();
	String acc_cur = mt.AccountCurrency();
	
	
	int sym_count = mt.GetSymbolCount();
	CombineHash ch;
	for(int i = 0; i < sym_count; i++) {
		const Symbol& sym = mt.GetSymbol(i);
		if (sym.tradeallowed)
			sym_ids.Add(i);
		ch << 1 << sym.name.GetHashValue();
	}
	symset_hash = ch;
	ASSERT_(prev_symset_hash == 0 || prev_symset_hash == symset_hash, "Symbol-set has changed. Remove previous saves.");
	
	
	indi = sys->Find<OpenValueChange>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	indi = sys->Find<PeriodicalChange>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
	/*indi = sys->Find<StochasticOscillator>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);*/
	
	
}

void Agent::InitThreads() {
	ASSERT(buf_count != 0);
	input_width  = sym_ids.GetCount();
	input_height = tf_ids.GetCount();
	input_depth  = buf_count + 1;
	output_width = (sym_ids.GetCount() + 1) * ACTIONCOUNT;
	int volume = input_width * input_height * input_depth;
	
	if (ses.GetInput() == NULL) {
		String params =
			"[\n"
			"\t{\"type\":\"input\""
				", \"input_width\":"  + IntStr(input_width)  +
				", \"input_height\":" + IntStr(input_height) +
				", \"input_depth\":"  + IntStr(input_depth)  +
				"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"fc\", \"neuron_count\":20, \"activation\": \"relu\"},\n"
			"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(output_width) + "},\n"
			"\t{\"type\":\"sgd\", \"learning_rate\":0.01, \"momentum\":0.9, \"batch_size\":5, \"l2_decay\":0.0}\n"
			"]\n";
		
		ses.MakeLayers(params);
	}
	
	ro_ses.CopyFrom(ses);
	
	thrds.SetCount(thrd_count);
	for(int i = 0; i < thrds.GetCount(); i++) {
		SequencerThread& t = thrds[i];
		
		t.average.sym_count = sym_ids.GetCount();
		t.average.tf_periods <<= tf_periods;
		t.ses.CopyFrom(ses);
		
		SimBroker& sb = t.broker;
		sb.Brokerage::operator=((Brokerage&)GetMetaTrader());
		sb.InitLightweight();
	}
	
	GenerateSnapshots();
	
	thrd_equities.SetCount(thrd_count);
	for(int i = 0; i < thrd_equities.GetCount(); i++)
		thrd_equities[i].SetCount(snaps.GetCount(), 0);
	
	
	ResetSnapshot(latest_snap);
	Seek(latest_snap, sys->GetCountTf(tf_ids.Top())-1);
	latest_broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	latest_broker.InitLightweight();
}

void Agent::Start() {
	Stop();
	
	running = true;
	for(int i = 0; i < thrds.GetCount(); i++) {
		not_stopped++;
		Thread::Start(THISBACK1(SequencerHandler, i));
	}
	not_stopped++;
	Thread::Start(THISBACK(TrainerHandler));
}

void Agent::Stop() {
	running = false;
	while (not_stopped) Sleep(100);
}

void Agent::TrainerHandler() {
	int forward_time, backward_time;
	Vector<VolumePtr> vec;
	
	int step_num = 0;
	int predict_interval = 10;
	bool test_predict = true;
	
	TrainerBase& trainer = *ses.GetTrainer();
	ConvNet::Net& net = ses.GetNetwork();
	Vector<int> train_pos;
	epoch_actual = 0;
	epoch_total = snaps.GetCount();
	vec.SetCount(1);
	
	bool forced_next_sequence = false;
	
	while (running) {
		
		// Wait until threads have provided sequences
		if (sequences.IsEmpty()) {
			Sleep(100);
			continue;
		}
		
		// Reshuffle training data after epoch
		if (forced_next_sequence || epoch_actual >= sequences[seq_cur].exps.GetCount()) {
			forced_next_sequence = false;
			
			// Return trained network for sequencer
			sequencer_lock.Enter();
			ro_ses.CopyFrom(ses);
			sequencer_lock.Leave();
			
			
			// Set next sequence
			seq_cur++;
			if (seq_cur >= sequences.GetCount())
				seq_cur = 0;
			
			
			// Reset position in sequence
			epoch_actual = 0;
			epochs++;
		}
		
		trainer_lock.Enter();
		Sequence& seq = sequences[seq_cur];
		trainer_lock.Leave();
		
		Experience& exp = seq.exps[epoch_actual];
		Volume& x = exp.input;
		
		
		TimeStop ts;
		
		
		// Train network
		trainer.Train(x, ACTIONCOUNT, exp.action, exp.reward);
		
		
		// Collect some stats
		backward_time = ts.Elapsed();
		double loss = trainer.GetLoss();
		double loss_l1d = trainer.GetL1DecayLoss();
		double loss_l2d = trainer.GetL2DecayLoss();
		
		
		//reward_window.Add(av_perf_value);
		//loss_window.Add(loss);
		//st.l1_loss_window.Add(loss_l1d);
		//st.l2_loss_window.Add(loss_l2d);
		
		
		//if ((step_num % step_cb_interal) == 0)
		//	WhenStepInterval(step_num);
		step_num++;
		epoch_actual++;
	}
	
	not_stopped--;
}

void Agent::SequencerHandler(int thrd_id) {
	SequencerThread& st = thrds[thrd_id];
	
	Snapshot& snap = snaps[0];
	
	Vector<VolumePtr> vec;
	Volume& x = snap.volume_in;
	//VolumeDouble& y = snap.volume_out;
	
	vec.SetCount(1);
	
	
	while (running) {
		ConvNet::Net& net = st.ses.GetNetwork();
		
		
		// Process sample
		Runner(thrd_id);
		
		
		// lock sample queue processing
		sequencer_lock.Enter();
		
		
		// Put sample to queue
		tmp_sequences.Add(st.seq.Detach());
		
		
		// Process whole queue if it's full
		if (tmp_sequences.GetCount() >= max_tmp_sequences) {
			
			// Find best sequence from the queue
			Sort(tmp_sequences, SequenceSorter());
			
			
			// Lock trainer
			trainer_lock.Enter();
			
			
			// Copy latest trained network
			if (ro_ses.GetInput())
				st.ses.CopyFrom(ro_ses);
			else {
				LOG("Agent::SequencerHandler: error: ro_ses is empty");
			}
			
			
			// Compute epsilon for the epsilon-greedy policy
			if (epochs < learning_epochs_burnin)
				st.rand_epsilon = 1.0;
			else
				st.rand_epsilon = min(1.0, max(epsilon_min, 1.0 - (double)(epochs - learning_epochs_burnin)/(learning_epochs_total - learning_epochs_burnin)));
			
			
			// Add the best sequence to trainer list and sort the list
			Sequence& seq = tmp_sequences[0];
			if (seq.exps.IsEmpty()) {
				LOG("Agent::SequencerHandler: error: empty sequence");
			} else {
				Sequence& seq = sequences.Insert(0, tmp_sequences.Detach(0));
				seq.id = sequence_count++;
				seq_results.Add(seq.equity);
			}
			
			
			// Prefer high equity (might give too insensitive response)
			if (0) {
				Sort(sequences, SequenceSorter());
			}
			
			
			// Remove exceeding amount of sequences
			if (sequences.GetCount() > max_sequences) {
				sequences.Remove(max_sequences, sequences.GetCount() - max_sequences);
			}
			
			
			// Store sequences periodically
			if (last_store.Elapsed() > 60 * 60 * 1000) {
				StoreThis();
				last_store.Reset();
			}
			
	
			// Release trainer lock
			trainer_lock.Leave();
		}
 
		
		// Release lock
		sequencer_lock.Leave();
	}
	
	not_stopped--;
}

void Agent::Runner(int thrd_id) {
	SequencerThread& st = thrds[thrd_id];
	ConvNet::Net& net = st.ses.GetNetwork();
	st.seq.Create();
	Sequence& seq = *st.seq;
	
	if (net.GetLayers().IsEmpty()) {
		LOG("Agent::Runner thrd=" << thrd_id << " no layers in the network!");
		return;
	}
	
	TimeStop ts;
	Vector<VolumePtr> vec;
	vec.SetCount(1);
	
	seq.exps.SetCount(snaps.GetCount());
	
	
	Vector<double>& thrd_equity = thrd_equities[thrd_id];
	
	st.broker.Clear();
	st.broker.SetSignal(0,0);
	st.average.Reset(snaps.GetCount());
	
	Vector<double> sym_profits;
	sym_profits.SetCount(sym_ids.GetCount(), 0.0);
	
	for(int i = 0; i < snaps.GetCount(); i++) {
		st.snap_id = i;
		Snapshot& snap = snaps[i];
		Experience& exp = seq.exps[i];
		exp.input = snap.volume_in;
		vec[0] = &exp.input;
		
		
		// Modify input to contain some additional sensor values
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			for(int k = 0; k < tf_ids.GetCount(); k++) {
				double average = st.average.Get(j, k);
				exp.input.Set(j, k, buf_count, average);
			}
		}
		
		
		// Forward propagate neural network
		const Volume& fwd = net.Forward(vec, false);
		
		
		// Set broker signals based on network output
		ForwardSignals(net, st.broker, fwd, exp, st.rand_epsilon, true);
		
		
		// Set signals to signal average calculator
		for(int j = 0; j < sym_ids.GetCount(); j++)
			st.average.Set(j, st.broker.GetSignal(sym_ids[j]));
		st.average.SeekNext();
		
		
		// Refresh values
		SetAskBid(st.broker, train_pos[i]);
		st.broker.RefreshOrders();
		st.broker.CycleChanges();
		double equity = st.broker.AccountEquity();
		
		
		// Get experience values
		if (i) {
			Experience& prev_exp = seq.exps[i - 1];
			const Vector<double>& sym_changes = st.broker.GetSymbolCycleChanges();
			prev_exp.reward.SetCount(sym_ids.GetCount() + 1);
			double sum = 0;
			for(int i = 0; i < sym_ids.GetCount(); i++) {
				double d = sym_changes[sym_ids[i]] / equity;
				prev_exp.reward[i] = d;
				sum += d;
			}
			ASSERT(IsFin(sum));
			prev_exp.reward[sym_ids.GetCount()] = sum;
		}
		
		
		// Refresh odrers
		if (i < snaps.GetCount()-1)	st.broker.Cycle();
		else						st.broker.CloseAll();
		
		
		
		// Write some stats for plotter
		thrd_equity[i] = equity;
	}
	
	
	// Remove last experience, because reward can't be calculated for it
	seq.exps.Remove(seq.exps.GetCount()-1);
	
	
	seq.equity = st.broker.AccountEquity();
	seq.orders = st.broker.GetHistoryOrders().GetCount();
}

void Agent::ForwardSignals(ConvNet::Net& net, Brokerage& broker, const Volume& fwd, Experience& exp, double epsilon, bool sig_freeze) {
	
	// Prepare some common variables
	bool rand_all = epsilon >= 1.0;
	if (exp.action.IsEmpty())
		exp.action.SetCount(sym_ids.GetCount()+1, 0);
	
	
	// Set signals and signal freezing
	for(int j = 0; j < sym_ids.GetCount(); j++) {
		
		// create network input
		int action = -1;
		
		// if we have enough to actually do something reasonable
		if (!rand_all) {
			
			double rf = Randomf();
			if (rf < epsilon) {
				// choose a random action with epsilon probability
				action = GetRandomAction();
			} else {
				// otherwise use max column to make decision
				action = GetAction(fwd, j);
			}
		} else {
			// pathological case that happens first few iterations
			// before we accumulate window_size inputs
			action = GetRandomAction();
		}
		
		int signal;
		if      (action == ACT_NOACT)  signal =  0;
		else if (action == ACT_INCSIG) signal = +1;
		else if (action == ACT_DECSIG) signal = -1;
		else Panic("Invalid action");
		
		exp.action[j] = action;
		
		// Set signal to broker
		int sym = sym_ids[j];
		if (!sig_freeze) {
			// Don't use signal freezing. Might cause unreasonable costs.
			broker.SetSignal(sym, signal);
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
	
	
	// Get free-margin stats
	int action = GetAction(fwd, sym_ids.GetCount());
	exp.action[sym_ids.GetCount()] = action;
	if (action == ACT_NOACT) {
		
	}
	else {
		double free_margin_level = broker.GetFreeMarginLevel();
		double free_margin_min = broker.GetMinFreeMargin();
		double free_margin_max = broker.GetMaxFreeMargin();
		if (free_margin_min < 0.60) free_margin_min = 0.60;
		if (free_margin_max > 0.95) free_margin_max = 0.95;
		double free_margin_diff = free_margin_max - free_margin_min;
		double free_margin_step = free_margin_diff / 100.0;
		
		if (action == ACT_INCSIG)
			free_margin_level += free_margin_step;
		else
			free_margin_level -= free_margin_step;
		
		if (free_margin_level < free_margin_min) free_margin_level = free_margin_min;
		if (free_margin_level > free_margin_max) free_margin_level = free_margin_max;
		
		broker.SetFreeMargin(free_margin_level);
	}
	
}

void Agent::RefreshWorkQueue() {
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	// Get DataBridge work queue
	Index<int> db_tf_ids, db_indi_ids;
	db_tf_ids.Add(tf_ids.Top());
	db_indi_ids.Add(sys->Find<DataBridge>());
	sys->GetCoreQueue(db_queue, sym_ids, db_tf_ids, db_indi_ids);
}

void Agent::ResetValueBuffers() {
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
	
	
	// Sort DataBridge work queue for easy reading positions
	databridge_cores.Clear();
	databridge_cores.SetCount(sym_ids.GetCount(), NULL);
	int factory = sys->Find<DataBridge>();
	for(int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];
		if (ci.factory != factory) continue;
		int j = sym_ids.Find(ci.sym);
		if (j == -1) continue;
		databridge_cores[j] = &*ci.core;
	}
	
	
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
	data_begins.SetCount(tf_ids.GetCount(), 0);
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
	
	
	int main_tf = tf_ids.Top();
	int pos = data_begins.Top();
	int bars = sys->GetCountTf(main_tf);
	train_pos.Reserve(bars - pos);
	for(int i = pos; i < bars; i++) {
		Time t = sys->GetTimeTf(main_tf, i);
		int wday = DayOfWeek(t);
		if (wday == 0 || wday == 6)
			continue;
		train_pos.Add(i);
	}
}

int Agent::GetAction(const Volume& fwd, int sym) const {
	int pos = sym * ACTIONCOUNT;
	double max_col = 0;
	double max_val = fwd.Get(pos);
	for(int i = 1; i < ACTIONCOUNT; i++) {
		double val = fwd.Get(pos + i);
		
		// Skip action with invalid values
		if (!IsFin(val))
			return ACT_NOACT;
		
		
		if (val > max_val) {
			max_val = val;
			max_col = i;
		}
	}
	return max_col;
}

void Agent::GenerateSnapshots() {
	
	// Generate snapshots
	TimeStop ts;
	snaps.SetCount(train_pos.GetCount());
	for(int i = 0; i < train_pos.GetCount(); i++) {
		if (i % 87 == 0)
			sys->WhenProgress(i, train_pos.GetCount());
		Snapshot& snap = snaps[i];
		ResetSnapshot(snap);
		Seek(snap, train_pos[i]);
	}
	LOG("Generating snapshots took " << ts.ToString());
}

void Agent::ProcessWorkQueue() {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		DLOG(i << "/" << work_queue.GetCount());
		sys->WhenProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
}

void Agent::ProcessDataBridgeQueue() {
	for(int i = 0; i < db_queue.GetCount(); i++) {
		DLOG(i << "/" << db_queue.GetCount());
		sys->WhenProgress(i, db_queue.GetCount());
		sys->Process(*db_queue[i]);
	}
}

void Agent::ResetSnapshot(Snapshot& snap) {
	ASSERT(!value_buffers.IsEmpty());
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	
	//DoubleTrio zero_trio(0.0,0.0,0.0);
	
	int tf_snap = tf_ids.GetCount()-1;
	int main_tf = tf_ids[tf_snap];
	
	snap.bars = sys->GetCountTf(main_tf);
	//   snap.value_count = buf_count;
	
	
	// Add periods and their multipliers to next longer timeframes
	int tf_count = tf_ids.GetCount();
	snap.pos.SetCount(tf_count, 0);
	int prev_period = 0;
	for(int j = 0; j < tf_count; j++) {
		int tf = tf_ids[j];
		int period = sys->GetPeriod(tf);
		snap.tfs.Add(tf);
		snap.periods.Add(period);
		if (prev_period == 0) snap.period_in_slower.Add(0);
		else                  snap.period_in_slower.Add(prev_period / period);
		prev_period = period;
	}
	
	
	// Reserve memory for values
	//    snap.value.SetCount(tf_count);
	snap.min_value.SetCount(tf_count);
	snap.max_value.SetCount(tf_count);
	for(int j = 0; j < tf_count; j++) {
		//    Vector<Vector<DoubleTrio> >& sym_values		= snap.value[j];
		Vector<double>& min_sym_values			= snap.min_value[j];
		Vector<double>& max_sym_values			= snap.max_value[j];
		//   sym_values.SetCount(sym_count);
		min_sym_values.SetCount(sym_count, 0.0);
		max_sym_values.SetCount(sym_count, 0.0);
		/*for(int k = 0; k < sym_count; k++) {
			Vector<DoubleTrio>& values = sym_values[k];
			values.SetCount(buf_count, zero_trio);
		}*/
	}
	ASSERT(input_width && input_height && input_depth);
	snap.volume_in = Volume(input_width, input_height, input_depth, 0.0);
	//snap.volume_out = Volume(output_width, 1, 1, 0.0);
	
	
	// Get time range between shortest timeframe and now
	snap.begin = sys->GetBegin(main_tf);
	snap.begin_ts = sys->GetBeginTS(main_tf);
	
	
	// Seek to beginning
	Seek(snap, 0);
}

bool Agent::Seek(Snapshot& snap, int shift) {
	int tf_snap = tf_ids.GetCount() - 1;
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	int main_tf = tf_ids[tf_snap];
	if (shift >= snap.bars) {
		snap.bars = sys->GetCountTf(main_tf);
		if (shift >= snap.bars)
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
	snap.time_values.SetCount(5);
	snap.time_values[0] = month;
	snap.time_values[1] = day;
	snap.time_values[2] = dow;
	snap.time_values[3] = hour;
	snap.time_values[4] = minute;
	
	
	// Find that time-position in longer timeframes
	snap.pos[tf_snap] = shift;
	for(int i = 0; i < tf_snap; i++) {
		int tf = snap.tfs[i];
		int slow_shift = sys->GetShiftTf(main_tf, tf, shift);
		ASSERT(slow_shift >= 0);
		snap.pos[i] = slow_shift;
	}
	
	
	// Refresh values (tf / sym / value)
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int pos = snap.pos[i];
		int next_pos = pos+1;
		//   Vector<Vector<DoubleTrio> >& sym_values		= snap.value[i];
		Vector<double>& min_sym_values			= snap.min_value[i];
		Vector<double>& max_sym_values			= snap.max_value[i];
		//   sym_values.SetCount(sym_count);
		min_sym_values.SetCount(buf_count, 0.0);
		max_sym_values.SetCount(buf_count, 0.0);
		for(int j = 0; j < buf_count; j++) {
			min_sym_values[j] = +DBL_MAX;
			max_sym_values[j] = -DBL_MAX;
		}
		for(int j = 0; j < sym_ids.GetCount(); j++) {
			//   Vector<DoubleTrio>& values = sym_values[j];
			Vector<ConstBuffer*>& indi_buffers = value_buffers[j][i];
			//    ASSERT(values.GetCount() == buf_count);
			for(int k = 0; k < buf_count; k++) {
				ConstBuffer& src = *indi_buffers[k];
				//   DoubleTrio& dst = values[k];
				double d = src.GetUnsafe(pos);
				/*dst.a = src.GetUnsafe(pos);
				if (pos) dst.b = src.GetUnsafe(pos-1);
				else dst.b = dst.a;
				if (next_pos < src.GetCount()) dst.c = src.GetUnsafe(next_pos);
				else dst.c = 0;*/
				double& min = min_sym_values[k];
				double& max = max_sym_values[k];
				if (d < min) min = d;
				if (d > max) max = d;
				snap.volume_in.Set(j, i, k, d);
			}
			// Reset space for previous symbol signal
			snap.volume_in.Set(j, i, buf_count, 0);
		}
	}
	
	return true;
}

bool Agent::SeekCur(Snapshot& snap, int shift) {
	int new_shift = snap.pos.Top() + shift;
	if (new_shift < 0) return false;
	if (new_shift >= snap.bars) return false;
	return Seek(snap, new_shift);
}

void Agent::SetAskBid(SimBroker& sb, int pos) {
	for(int i = 0; i < sym_ids.GetCount(); i++) {
		Core& core = *databridge_cores[i];
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(sym_ids[i], open.Get(pos));
	}
	sb.SetTime(sys->GetTimeTf(tf_ids.Top(), pos));
}

void Agent::SetBrokerageSignals(Brokerage& broker, int pos) {
	
	// Fill input volume
	Seek(latest_snap, pos);
	
	
	// Acquire network lock
	ConvNet::Net& net = ses.GetNetwork();
	Vector<VolumePtr> vec;
	vec.SetCount(1);
	net.Enter();
	
	
	// Check that network is giving positive results with one SimBroker
	latest_broker.Clear();
	int begin = sys->GetShiftFromTimeTf(GetSysTime() - 7*24*60*60, tf_ids.Top());
	int begin_pos = train_pos.GetCount() - 1;
	for (;begin_pos >= 0; begin_pos--) {
		if (train_pos[begin_pos] < begin) {
			begin_pos++;
			break;
		}
	}
	if (begin_pos < 0) begin_pos = 0;
	
	Experience tmp_exp;
	
	for(int i = begin_pos; i < snaps.GetCount(); i++) {
		Snapshot& snap = snaps[i];
		
		// Forward network
		vec[0] = &snap.volume_in;
		Volume& fwd = net.Forward(vec, false);
		
		
		// Set signals and signal freezing
		ForwardSignals(net, latest_broker, fwd, tmp_exp, 0.0, true);
		
		
		// Refresh values
		SetAskBid(latest_broker, train_pos[i]);
		if (i < snaps.GetCount()-1)	latest_broker.Cycle();
		else						latest_broker.CloseAll();
	}
	
	// Reset signals if result is bad
	if (latest_broker.AccountEquity() <= latest_broker.GetInitialBalance()) {
		LOG("Agent::SetBrokerageSignals: simbroker result was bad, reseting signals");
		
		for(int i = 0; i < broker.GetSymbolCount(); i++) {
			broker.SetSignal(i, 0);
			broker.SetSignalFreeze(i, false);
		}
	}
	
	// Otherwise set real signal
	else {
		LOG("Agent::SetBrokerageSignals: simbroker result was good, using signals");
		
		// Forward network
		vec[0] = &latest_snap.volume_in;
		Volume& fwd = net.Forward(vec, false);
		
		
		// Set signals
		ForwardSignals(net, broker, fwd, tmp_exp, 0.0, true);
	}
	
	net.Leave();
}














RealtimeSession::RealtimeSession(Agent& agent) :
	agent(&agent)
{
	running = false;
	stopped = true;
}

void RealtimeSession::Init() {
	
	
	broker = &GetMetaTrader();
	
	
}

void RealtimeSession::Run() {
	System& sys = *agent->sys;
	Agent& agent = *this->agent;
	
	int prev_pos = sys.GetShiftFromTimeTf(GetSysTime(), agent.tf_ids.Top());
	
	while (running) {
		int cur_pos = sys.GetShiftFromTimeTf(GetSysTime(), agent.tf_ids.Top());
		if (prev_pos < cur_pos)
			PostEvent(EVENT_REFRESH);
		
		if (event_queue.IsEmpty()) {Sleep(500); continue;}
		
		lock.Enter();
		int event = event_queue[0];
		event_queue.Remove(0);
		lock.Leave();
		
		if (event == EVENT_REFRESH) {
			GetMetaTrader().Data();
			agent.ProcessDataBridgeQueue();
			agent.SetBrokerageSignals(*broker, cur_pos);
			broker->SignalOrders();
		}
		else if (event == EVENT_KILL) {
			GetMetaTrader().Data();
			for(int i = 0; i < broker->GetSymbolCount(); i++)
				broker->SetSignal(i, 0);
			broker->SignalOrders();
		}
		
		Sleep(100);
	}
	stopped = true;
}

void RealtimeSession::Stop() {
	running = false;
	while (!stopped) Sleep(100);
}

void RealtimeSession::PostEvent(int event) {
	lock.Enter();
	event_queue.Add(event);
	lock.Leave();
}

}
