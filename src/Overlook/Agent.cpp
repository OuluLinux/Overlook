#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Agent::Agent() {
	proxy_sym = -1;
	sym = -1;
	tf = -1;
	group_id = -1;
	
	sys = NULL;
	group = NULL;
	databridge_core = NULL;
	databridge_proxy_core = NULL;
	
	not_stopped = 0;
	running = false;
	paused = false;
	input_width = 0;
	input_height = 0;
	test_interval = 1000;
	training_limit = 1000;
	session_cur = 0;
	max_sequences = 5;
	buf_count = 0;
	group_count = 0;
	prefer_high = true;
	train_single = false;
	iter = 0;
	rand_epsilon = 1.0;
	sig_freeze = true;
	smooth_reward = 0.0;
	
	thrd_count = 2;//CPU_Cores();
	max_tmp_sequences = 8;
	sequence_count = 0;
	
	epochs = 0;
	seq_cur = 0;
	
	global_free_margin_level = 0.97;
	
	// what epsilon value do we bottom out on? 0.0 => purely deterministic policy at end
	epsilon_min = 0.005;
	

	// number of epochs we will learn for
	learning_epochs_total = 1000;
	
	// how many epochs of the above to perform only random actions (in the beginning)?
	learning_epochs_burnin = 100;
	
}

Agent::~Agent() {
	Stop();
}

void Agent::Init() {
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
	tf = sys->FindPeriod(5);
	ASSERT(tf != -1);
	tf_ids.Add(tf);
	
	
	tf_muls.SetCount(tf_ids.GetCount(), 1.0);
	tf_periods.SetCount(tf_ids.GetCount(), 1);
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		int period = sys->GetPeriod(tf_ids[i]) / sys->GetPeriod(tf_ids.Top());
		tf_muls[i] = 1.0 / period;
		tf_periods[i] = period;
	}
	
	
	
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
	ASSERT(group_count != 0);
	
	input_width  = 1;
	input_height = (buf_count + group_count) * 2 * tf_ids.GetCount();
	
	dqn.Init(input_width, input_height, ACTIONCOUNT);
	dqn.Reset();
	
	ASSERT(!param_str.IsEmpty());
	dqn.LoadInitJSON(param_str);
		
	signal_average.tf_periods <<= tf_periods;
	reward_average.tf_periods <<= tf_periods;
	
	broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	broker.InitLightweight();
	
	GenerateSnapshots();
	
	thrd_equity.SetCount(snaps.GetCount(), 0);
	
	ResetSnapshot(latest_snap);
	Seek(latest_snap, sys->GetCountTf(tf_ids.Top())-1);
	latest_broker.Brokerage::operator=((Brokerage&)GetMetaTrader());
	latest_broker.InitLightweight();
}

void Agent::Start() {
	Stop();
	running = true;
	not_stopped++;
	Thread::Start(THISBACK(Main));
}

void Agent::Stop() {
	running = false;
	while (not_stopped) Sleep(100);
}

void Agent::Main() {
	epoch_actual = 0;
	epoch_total = snaps.GetCount();
	
	while (running) {
		
		// Wait until threads have provided sequences
		if (paused) {
			Sleep(100);
			continue;
		}
		
		// Do some action
		Action();
		
		// Store sequences periodically
		/*if (last_store.Elapsed() > 60 * 60 * 1000) {
			StoreThis();
			last_store.Reset();
		}*/
	}
	
	not_stopped--;
}

void Agent::Action() {
	
	if (!epoch_actual) {
		broker.Clear();
		broker.SetSignal(0,0);
		signal_average.Reset(snaps.GetCount());
		reward_average.Reset(snaps.GetCount());
		prev_reward = 0;
	}
	else {
		Backward(prev_reward);
		reward_average.Set(prev_reward);
		reward_average.SeekNext();
	}
	
	
	// Set broker signals based on DQN-agent action
	Snapshot& snap = snaps[epoch_actual];
	Forward(snap, broker);
	
	
	// Refresh values
	SetAskBid(broker, train_pos[epoch_actual]);
	broker.RefreshOrders();
	broker.CycleChanges();
	double equity = broker.AccountEquity();
	
	
	// Get experience values
	if (epoch_actual) {
		const Vector<double>& sym_changes = broker.GetSymbolCycleChanges();
		prev_reward = sym_changes[sym] / equity;
	}
	
	
	// Refresh odrers
	if (epoch_actual < snaps.GetCount()-1)
		broker.Cycle();
	else
		broker.CloseAll();
	
	
	// Write some stats for plotter
	thrd_equity[epoch_actual] = equity;
	
}

void Agent::Forward(Snapshot& snap, Brokerage& broker) {
	ASSERT(group_id != -1);
	
	// in forward pass the agent simply behaves in the environment
	int ag_begin = group->GetSignalPos(snap.shift - 1);
	int ag_end   = group->GetSignalPos(snap.shift);
	int ag_diff  = ag_end - ag_begin;
	
	
	// Copy values from data input (expect values to be in 0-1 range
	int j = 0;
	for(int i = 0; i < tf_ids.GetCount(); i++) {
		for(int k = 0; k < buf_count; k++) {
			double d = snap.values[i * buf_count + k];
			ASSERT(d >= 0.0 && d <= 1.0);
			input_array[j++] = d;
		}
	}
	
	
	// Copy previous signals of other agents in the group to input
	Vector<double>& group_values = group->latest_signals;
	for(int i = ag_begin; i < ag_end; i++) {
		double d = group_values[i];
		ASSERT(d >= 0.0 && d <= 1.0);
		if (d >= 0) {
			input_array[j++] = 1.0 - d;
			input_array[j++] = 1.0;
		} else {
			input_array[j++] = 1.0;
			input_array[j++] = 1.0 + d;
		}
	}
	ASSERT(j == input_width * input_height);


	// Get action from DQN-agent
    int action = dqn.Act(input_array);
    
    
    // Convert action to simple signal
    int signal;
	if      (action == ACT_NOACT)  signal =  0;
	else if (action == ACT_INCSIG) signal = +1;
	else if (action == ACT_DECSIG) signal = -1;
	else Panic("Invalid action");
	
	
	// Collect average of the value
	signal_average.Set(signal);
	signal_average.SeekNext();
	
	
	// Write latest average to the group values
	ag_begin = group->GetSignalPos(snap.shift, group_id);
	ag_end   = group->GetSignalPos(snap.shift, group_id + 1);
	ASSERT(ag_end - ag_begin == tf_ids.GetCount());
	for(int i = ag_begin, j = 0; i < ag_end; i++, j++) {
		group_values[i] = signal_average.Get(j);
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
	
	broker.SetFreeMargin(global_free_margin_level);
}

void Agent::Backward(double reward) {
	
	// pass to brain for learning
	dqn.Learn(reward);
	
	smooth_reward += reward;
	
	if (iter % 50 == 0) {
		smooth_reward /= 50;
		WhenRewardAverage(smooth_reward);
		smooth_reward = 0;
	}
	iter++;
}

void Agent::RefreshWorkQueue() {
	ASSERT(sym != -1);
	Index<int> sym_ids;
	sym_ids.Add(sym);
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
	
	// Get DataBridge work queue
	const Symbol& s = GetMetaTrader().GetSymbol(sym);
	proxy_sym = s.proxy_id;
	if (proxy_sym != -1)
		sym_ids.Add(proxy_sym);
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
	
	
	// Get DataBridge core pointer for easy reading
	databridge_core = NULL;
	databridge_proxy_core = NULL;
	int factory = sys->Find<DataBridge>();
	for(int i = 0; i < db_queue.GetCount(); i++) {
		CoreItem& ci = *db_queue[i];
		if (ci.factory != factory) continue;
		if (ci.sym == sym) {
			databridge_core = &*ci.core;
			continue;
		}
		if (ci.sym == proxy_sym) {
			databridge_proxy_core = &*ci.core;
			continue;
		}
	}
	ASSERT(databridge_core);
	
	
	// Reserve memory for value buffer vector
	value_buffers.Clear();
	value_buffers.SetCount(tf_ids.GetCount());
	for(int j = 0; j < value_buffers.GetCount(); j++)
		value_buffers[j].SetCount(buf_count, NULL);
	
	
	// Get value buffers
	int total_bufs = 0;
	data_begins.SetCount(tf_ids.GetCount(), 0);
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = *work_queue[i];
		ASSERT(!ci.core.IsEmpty());
		const Core& core = *ci.core;
		const Output& output = core.outputs[0];
		
		if (ci.sym != sym) continue;
		int tf_id = tf_ids.Find(tf);
		if (tf_id == -1) continue;
		
		DataBridge* db = dynamic_cast<DataBridge*>(&*ci.core);
		if (db) data_begins[tf_id] = Upp::max(data_begins[tf_id], db->GetDataBegin());
		
		Vector<ConstBuffer*>& indi_buffers = value_buffers[tf_id];
		
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
	int expected_total = tf_ids.GetCount() * buf_count;
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
		if (t.hour < 16 || (t.hour == 16 && t.minute < 30))
			continue;
		if (t.hour >= 23)
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

void Agent::RefreshSnapshots() {
	ProcessWorkQueue();
	
	int tf_snap = tf_ids.GetCount()-1;
	int main_tf = tf_ids[tf_snap];
	int pos = train_pos.Top()+1;
	int bars = sys->GetCountTf(main_tf);
	train_pos.Reserve(bars - pos);
	for(int i = pos; i < bars; i++) {
		Time t = sys->GetTimeTf(main_tf, i);
		int wday = DayOfWeek(t);
		if (wday == 0 || wday == 6)
			continue;
		if (t.hour < 16 || (t.hour == 16 && t.minute < 30))
			continue;
		if (t.hour >= 23)
			continue;
		
		LOG("Agent::RefreshSnapshots: Creating snapshot at " << Format("%", t));
		One<Snapshot> snap;
		snap.Create();
		
		ResetSnapshot(*snap);
		Seek(*snap, i);
		
		snaps.Add(snap.Detach());
		train_pos.Add(i);
	}
}

void Agent::ResetSnapshot(Snapshot& snap) {
	ASSERT(!value_buffers.IsEmpty());
	int buf_count = value_buffers[0].GetCount();
	
	int tf_snap = tf_ids.GetCount()-1;
	int main_tf = tf_ids[tf_snap];
	
	snap.bars = sys->GetCountTf(main_tf);
	
	
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
	snap.values.SetCount(input_width * input_height, 0.0);
	
	
	// Get time range between shortest timeframe and now
	snap.begin = sys->GetBegin(main_tf);
	snap.begin_ts = sys->GetBeginTS(main_tf);
	
	
	// Seek to beginning
	Seek(snap, 0);
}

bool Agent::Seek(Snapshot& snap, int shift) {
	int tf_snap = tf_ids.GetCount() - 1;
	int buf_count = value_buffers[0].GetCount();
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
	snap.time = t;
	snap.added = GetSysTime();
	snap.is_valid = true;
	snap.shift = shift;
	
	
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
		Vector<ConstBuffer*>& indi_buffers = value_buffers[i];
		for(int k = 0; k < buf_count; k++) {
			ConstBuffer& src = *indi_buffers[k];
			double d = src.GetUnsafe(pos);
			snap.values[i * buf_count + k] = d;
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
	if (databridge_core) {
		Core& core = *databridge_core;
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(sym, open.Get(pos));
	}
	
	if (databridge_proxy_core) {
		Core& core = *databridge_proxy_core;
		ConstBuffer& open = core.GetBuffer(0);
		sb.SetPrice(proxy_sym, open.Get(pos));
	}
	
	sb.SetTime(sys->GetTimeTf(tf_ids.Top(), pos));
}

}
