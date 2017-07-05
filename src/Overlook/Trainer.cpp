#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Trainer::Trainer(System& sys) : sys(&sys) {
	not_stopped = 0;
	running = false;
	input_width = 0;
	input_height = 0;
	input_depth = 0;
	test_interval = 1000;
	training_limit = 1000;
	session_cur = 0;
	max_sessions = 100;
	
	thrd_count = 2;//CPU_Cores();
	session_count = 0;
	evol_cont_probability = 0.9;
	evol_scale = 0.7;
	
	// From MagicNet
	// -------------
	
	// optional inputs
	train_ratio = 0.7;
	num_folds = 10;
	num_candidates = 50; // we evaluate several in parallel
	
	// how many epochs of data to train every network? for every fold?
	// higher values mean higher accuracy in final results, but more expensive
	num_epochs = 50;
	
	// number of best models to average during prediction. Usually higher = better
	ensemble_size = 10;
	
	// candidate parameters
	batch_size_min = 10;
	batch_size_max = 300;
	l2_decay_min = -4;
	l2_decay_max = 2;
	learning_rate_min = -4;
	learning_rate_max = 0;
	momentum_min = 0.9;
	momentum_max = 0.9;
	neurons_min = 5;
	neurons_max = 30;
	
}

Trainer::~Trainer() {
	Stop();
	StoreThis();
}

void Trainer::LoadThis() {
	LoadFromFile(*this,	ConfigFile("trainer.bin"));
}

void Trainer::StoreThis() {
	StoreToFile(*this,	ConfigFile("trainer.bin"));
}

void Trainer::Init() {
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
	
	
	MetaTrader& mt = GetMetaTrader();
	String acc_cur = mt.AccountCurrency();
	
	int sym_count = mt.GetSymbolCount();
	for(int i = 0; i < sym_count; i++) {
		const Symbol& sym = mt.GetSymbol(i);
		if (sym.tradeallowed)
			sym_ids.Add(i);
	}
	
	indi = sys->Find<ValueChange>();
	ASSERT(indi != -1);
	indi_ids.Add(indi);
}

void Trainer::InitThreads() {
	input_width  = sym_ids.GetCount();
	input_height = tf_ids.GetCount();
	input_depth  = 1;
	output_width = input_width * (input_height + 1);
	int volume = input_width * input_height * input_depth;
	
	thrds.SetCount(thrd_count);
	iters.SetCount(thrd_count);
	thrd_priorities.SetCount(thrd_count);
	thrd_performances.SetCount(thrd_count);
}

void Trainer::Start() {
	Stop();
	
	running = true;
	for(int i = 0; i < thrds.GetCount(); i++) {
		not_stopped++;
		Thread::Start(THISBACK1(ThreadHandler, i));
	}
}

void Trainer::Stop() {
	running = false;
	while (not_stopped) Sleep(100);
}

void Trainer::ShuffleTraining(Vector<int>& shuffled_pos) {
	// Get vector of random positions of train_pos vector
	int count = train_pos.GetCount();
	shuffled_pos.SetCount(count);
	for(int i = 0; i < count; i++)
		shuffled_pos[i] = i;
	for(int i = 0; i < count; i++)
		Swap(shuffled_pos[i], shuffled_pos[Random(count)]);
	
	// Change position in train_pos to time-position
	for(int i = 0; i < count; i++)
		shuffled_pos[i] = train_pos[shuffled_pos[i]];
}

inline void ClearValueChangers(Vector<ValueChanger*>& v) {
	for(int i = 0; i < v.GetCount(); i++)
		delete v[i];
	v.Clear();
}

void Trainer::EvolveSettings(SessionThread& st) {
	int c0, r1, r2;
	
	int max_session = Upp::min(sessions.GetCount() - thrds.GetCount(), max_sessions);
	c0 = 1 + session_count % (max_session - 1);
	do {r1 = Random(max_session);} while (r1 == c0);
	do {r2 = Random(max_session);} while (r2 == c0 || r2 == r1);
	
	Vector<ValueChanger*> best_solution, trial_solution, cand, src1, src2;
	sessions[0]		.settings.GetChangers(best_solution);
	sessions[c0]	.settings.GetChangers(cand);
	sessions[r1]	.settings.GetChangers(src1);
	sessions[r2]	.settings.GetChangers(src2);
	
	st.settings.GetChangers(trial_solution);
	int dimension = trial_solution.GetCount();
	ASSERT(best_solution.GetCount() == dimension);
	ASSERT(cand.GetCount() == dimension);
	ASSERT(src1.GetCount() == dimension);
	ASSERT(src2.GetCount() == dimension);
	
	// Randomize starting position
	int n = Random(dimension);
	
	for(int i = 0; i < dimension; i++ )
		trial_solution[i]->Set(cand[i]->Get());
	
	for (int i = 0; (Randomf() < evol_cont_probability) && (i < dimension); i++) {
		trial_solution[n]->Set(best_solution[n]->Get() + evol_scale * (src1[n]->Get() - src2[n]->Get()));
		n = (n + 1) % dimension;
	}
	
	ClearValueChangers(best_solution);
	ClearValueChangers(trial_solution);
	ClearValueChangers(cand);
	ClearValueChangers(src1);
	ClearValueChangers(src2);
}


void Trainer::RandomSettings(SessionThread& st) {
	int max_layers = 15;
	
	SessionSettings& set = st.settings;
	set.layers.Clear();
	set.neural_layers = 1 + Random(max_layers);
	
	// Randomize all layers for DE even if all layers are not activated
	set.layers.SetCount(max_layers);
	for (int q = 0; q < set.layers.GetCount(); q++) {
		SessionSettings::Layer& lay = set.layers[q];
		lay.neuron_count = neurons_min + Random(neurons_max - neurons_min);
		lay.act = Random(3); // tanh, maxout, relu
		lay.bias_pref = lay.act == 2 ? 0.1 : 0.0; // 0.1 for relu
		lay.has_dropout = Randomf() < 0.5;
		lay.dropout_prob = Randomf();
	}
	
	set.batch_size	= batch_size_min + Random(batch_size_max - batch_size_min); // batch size
	set.l2			= pow(10, l2_decay_min + Randomf() + (l2_decay_max - l2_decay_min)); // l2 weight decay
	set.lr			= pow(10, learning_rate_min + Randomf() * (learning_rate_max - learning_rate_min)); // learning rate
	set.mom			= momentum_min + Randomf() * (momentum_max - momentum_min); // momentum. Lets just use 0.9, works okay usually ;p
	
	double tp		= Randomf(); // trainer type
	if (tp < 1.0/3.0)
		set.trainer_type = 0;
	else if (tp < 2.0/3.0)
		set.trainer_type = 1;
	else
		set.trainer_type = 2;
}

void Trainer::SampleCandidate(int thrd_id) {
	lock.Enter();
	session_count++;
	thrds[thrd_id].Create();
	SessionThread& d = *thrds[thrd_id];
	SessionSettings& set = d.settings;
	Session& cand = d.ses;
	d.id = session_count-1;
	d.total_sigchange = 0.0;
	d.is_finished = false;
	lock.Leave();
	
	d.epochs = 0;
	d.loss_window.Init(2000);
	d.reward_window.Init(2000);
	SimBroker& sb = d.broker;
	sb.Brokerage::operator=((Brokerage&)GetMetaTrader());
	sb.InitLightweight();
	
	if (session_count < 10 || Randomf() < 0.05) {
		RandomSettings(d);
	}
	else {
		set.layers.SetCount(15);
		EvolveSettings(d);
		
		// Normalize values
		if (set.neural_layers < 1) set.neural_layers = 1;
		if (set.neural_layers > 15) set.neural_layers = 15;
		
		for (int q = 0; q < set.layers.GetCount(); q++) {
			SessionSettings::Layer& lay = set.layers[q];
			if (lay.neuron_count < neurons_min)	lay.neuron_count = neurons_min;
			if (lay.neuron_count > neurons_max)	lay.neuron_count = neurons_max;
			if (lay.act < 0) lay.act = 0;
			if (lay.act > 2) lay.act = 2;
			lay.bias_pref = lay.act == 2 ? 0.1 : 0.0; // 0.1 for relu
			if (lay.dropout_prob < 0.0) lay.dropout_prob = 0.0;
			if (lay.dropout_prob > 1.0) lay.dropout_prob = 1.0;
		}
		if (set.batch_size < batch_size_min)	set.batch_size = batch_size_min;
		if (set.batch_size > batch_size_max)	set.batch_size = batch_size_max;
		
		if (set.l2 < l2_decay_min)	set.l2 = l2_decay_min;
		if (set.l2 > l2_decay_max)	set.l2 = l2_decay_max;
		
		if (set.lr < learning_rate_min)	set.lr = learning_rate_min;
		if (set.lr > learning_rate_max)	set.lr = learning_rate_max;
		
		if (set.mom < momentum_min)	set.mom = momentum_min;
		if (set.mom > momentum_max)	set.mom = momentum_max;
		
		if (set.trainer_type < 0)	set.trainer_type = 0;
		if (set.trainer_type > 2)	set.trainer_type = 2;
	}
	
	// sample network topology and hyperparameters
	cand.AddInputLayer(input_width, input_height, input_depth);
	
	for (int q = 0; q < set.neural_layers; q++) {
		const SessionSettings::Layer& lay = set.layers[q];
		
		FullyConnLayer& fc = cand.AddFullyConnLayer(lay.neuron_count);
		fc.bias_pref = lay.bias_pref;
		
		if (lay.act == 0) {
			cand.AddTanhLayer();
		}
		else if (lay.act == 1) {
			cand.AddMaxoutLayer(2); // 2 is default
		}
		else if (lay.act == 2) {
			cand.AddReluLayer();
		}
		else Panic("What activation");
		
		if (lay.has_dropout) {
			cand.AddDropoutLayer(lay.dropout_prob);
		}
	}
	
	cand.AddFullyConnLayer(output_width);
	cand.AddRegressionLayer();
	
	
	// Add trainer
	TrainerBase* trainer = NULL;
	if (set.trainer_type == 0) {
		trainer = new AdadeltaTrainer(cand.GetNetwork());
	}
	else if (set.trainer_type == 1) {
		trainer = new AdagradTrainer(cand.GetNetwork());
	}
	else {
		trainer = new SgdTrainer(cand.GetNetwork());
	}
	trainer->SetBatchSize(set.batch_size);
	trainer->SetLearningRate(set.lr);
	trainer->SetL2Decay(set.l2);
	trainer->SetMomentum(set.mom);
	cand.AttachTrainer(trainer); // Session will own and delete the trainer
}

void Trainer::ThreadHandler(int thrd_id) {
	Vector<VolumePtr> vec;
	Iterator& iter = iters[thrd_id];
	Volume& x = iter.volume_in;
	VolumeData<double>& y = iter.volume_out[thrd_id];
	int perf_begin = tf_ids.GetCount() * sym_ids.GetCount();
	vec.SetCount(1);
	
	while (running) {
		// Sample candidate
		SampleCandidate(thrd_id);
		SessionThread& st = *thrds[thrd_id];
		ConvNet::Net& net = st.ses.GetNetwork();
		
		
		// Process sample
		Runner(thrd_id);
		
		
		// Test net with separate test data
		double total_change = 0;
		for(int i = 0; i < test_pos.GetCount(); i++) {
			Seek(thrd_id, test_pos[i]);
			vec[0] = &x;
			const Volume& fwd = net.Forward(vec, true);
			double av_perf_value = 0.0;
			double av_change = 0.0;
			for(int j = 0; j < sym_ids.GetCount(); j++) {
				int tf = 0;
				double change = 0.0;
				double sig = 0.0;
				for(; tf < tf_ids.GetCount(); tf++) {
					int pos = j + tf * sym_ids.GetCount();
					double real = y.Get(pos);
					if (real == 0.0) break;
					double predicted = fwd.Get(pos);
					bool same_dir = (predicted * real) > 0.0;
					change = (same_dir ? +1.0 : -1.0) * fabs(real);
					sig = predicted >= 0 ? +1.0 : -1.0;
					if (!same_dir) break;
				}
				double perf_value = (double)tf / (double)tf_ids.GetCount();
				y.Set(perf_begin + j, perf_value);
				av_perf_value += perf_value;
				av_change += change;
				total_change += change;
			}
			av_perf_value /= sym_ids.GetCount();
			av_change /= sym_ids.GetCount();
			
			st.test_reward_window.Add(av_perf_value);
			st.test_window0.Add(av_change);
		}
		
		st.total_sigchange = total_change;
		st.is_finished = true;
		
		
		// Sort results
		struct SessionSorter {
			bool operator()(const SessionThread& a, const SessionThread& b) const {
				if (!a.is_finished) return false;
				return a.total_sigchange > b.total_sigchange;
			}
		};
		lock.Enter();
		sessions.Add(thrds[thrd_id].Detach());
		ses_sigchanges.Add(total_change);
		Sort(sessions, SessionSorter());
		if (sessions.GetCount() > max_sessions)
			sessions.Remove(max_sessions, sessions.GetCount() - max_sessions);
		if (last_store.Elapsed() > 5 * 60 * 1000) {
			StoreThis();
			last_store.Reset();
		}
		lock.Leave();
	}
	
	not_stopped--;
}

void Trainer::Runner(int thrd_id) {
	int forward_time, backward_time;
	Vector<VolumePtr> vec;
	
	int step_num = 0;
	int predict_interval = 10;
	bool test_predict = true;
	int perf_begin = tf_ids.GetCount() * sym_ids.GetCount();
	
	// Sanity checks
	ASSERT(!thrds.IsEmpty());
	Iterator& iter = iters[thrd_id];
	SessionThread& st = *thrds[thrd_id];
	
	TrainerBase& trainer = *st.ses.GetTrainer();
	ConvNet::Net& net = st.ses.GetNetwork();
	Volume& x = iter.volume_in;
	VolumeData<double>& y = iter.volume_out[thrd_id];
	
	Vector<int> train_pos;
	ShuffleTraining(train_pos);
	st.epoch_actual = 0;
	st.epoch_total = train_pos.GetCount();
	vec.SetCount(1);
	
	int last_top_step = 0;
	double last_top = 0;
	
	while (running) {
		
		// Reshuffle training data after epoch
		if (st.epoch_actual >= train_pos.GetCount()) {
			ShuffleTraining(train_pos);
			st.epoch_actual = 0;
			st.epochs++;
		}
		Seek(thrd_id, train_pos[st.epoch_actual]);
		
		// use x to build our estimate of validation error
		if (test_predict && (step_num % predict_interval) == 0) {
			TimeStop ts;
			Volume& v = net.Forward(x);
			forward_time = ts.Elapsed();
			
			// Mean squared error (skip yet unknown performance values)
			double mse = 0.0;
			for (int i = 0; i < perf_begin; i++) {
				double diff = y.Get(i) - v.Get(i);
				mse += diff * diff;
			}
			mse /= perf_begin;
			st.accuracy_window.Add(-mse);
		}
		
		TimeStop ts;
		
		
		vec[0] = &x;
		const Volume& fwd = net.Forward(vec, true);
		
		
		// Experimental internal performance value
		double av_perf_value = 0;
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			int tf = 0;
			for(; tf < tf_ids.GetCount(); tf++) {
				int pos = i + tf * sym_ids.GetCount();
				double real = y.Get(pos);
				if (real == 0.0) break;
				double predicted = fwd.Get(pos);
				bool same_dir = (predicted * real) > 0.0;
				if (!same_dir) break;
			}
			double perf_value = (double)tf / (double)tf_ids.GetCount();
			y.Set(perf_begin + i, perf_value);
			av_perf_value += perf_value;
		}
		av_perf_value /= sym_ids.GetCount();
		
		
		// Check for top value
		if (av_perf_value > last_top) {
			last_top = av_perf_value;
			last_top_step = step_num;
		}
		
		
		trainer.Backward(y);
		trainer.TrainImplem();
		
		backward_time = ts.Elapsed();
		
		double loss = trainer.GetLoss();
		double loss_l1d = trainer.GetL1DecayLoss();
		double loss_l2d = trainer.GetL2DecayLoss();
		
		
		// keep track of stats such as the average training error and loss
		// if last layer is softmax, then add prediction value to the average
		if (test_predict) {
			// Mean squared error
			Volume& v = net.GetOutput();
			double mse = 0.0;
			for (int i = 0; i < v.GetLength(); i++) {
				double diff = y.Get(i) - v.Get(i);
				mse += diff * diff;
			}
			mse /= v.GetLength();
			st.train_window.Add(-mse);
		}
		
		st.reward_window.Add(av_perf_value);
		st.loss_window.Add(loss);
		//st.l1_loss_window.Add(loss_l1d);
		//st.l2_loss_window.Add(loss_l2d);
		
		
		//if ((step_num % step_cb_interal) == 0)
		//	WhenStepInterval(step_num);
		step_num++;
		st.epoch_actual++;
		
		
		// Check for training limit
		if (step_num - last_top_step >= training_limit) {
			break;
		}
	}
}

void Trainer::RefreshWorkQueue() {
	sys->GetCoreQueue(work_queue, sym_ids, tf_ids, indi_ids);
}

void Trainer::ResetIterators() {
	for(int i = 0; i < iters.GetCount(); i++)
		ResetIterator(i);
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
	
	
	// Get unique data positions for training
	// - slower tf data-begin positions in fastest tf are required
	// - start from fastest tf data begin and seek to beginning with fastest available unique data step
	// - also, add all positions from fastest tf begin to end
	ASSERT(train_pos.IsEmpty());
	ASSERT(test_pos.IsEmpty());
	int main_tf = tf_ids.Top();
	Vector<int> fast_begins;
	for(int i = 0; i < tf_ids.GetCount()-1; i++)
		fast_begins.Add(sys->GetShiftTf(main_tf, tf_ids[i], 0));
	int pos = data_begins.Top();
	int bars = sys->GetCountTf(main_tf);
	train_pos.Reserve(bars * 0.6);
	for(int i = pos; i < bars; i++) train_pos.Add(i);
	pos--;
	while (pos >= 0) {
		int fastest = tf_ids.GetCount() - 2;
		for (; fastest >= 0; fastest--) {
			if (pos >= fast_begins[fastest])
				break;
		}
		if (fastest == -1)
			break;
		pos = sys->GetShiftTf(tf_ids[fastest], main_tf, sys->GetShiftTf(main_tf, tf_ids[fastest], pos));
		train_pos.Add(pos);
		pos--;
	}
	Sort(train_pos, StdLess<int>());
	
	// Move some training data to testing data
	int test_size = 0.1 * train_pos.GetCount();
	test_pos.SetCount(test_size);
	for(int i = 0; i < test_size; i++) {
		int j = Random(train_pos.GetCount());
		int pos = train_pos[j];
		train_pos.Remove(j);
		test_pos[i] = pos;
	}
	Sort(test_pos, StdLess<int>());
	
}

void Trainer::ProcessWorkQueue() {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		LOG(i << "/" << work_queue.GetCount());
		sys->WhenProgress(i, work_queue.GetCount());
		sys->Process(*work_queue[i]);
	}
}

void Trainer::ResetIterator(int thrd_id) {
	Iterator& iter = iters[thrd_id];
	ASSERT(!value_buffers.IsEmpty());
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	
	DoubleTrio zero_trio(0.0,0.0,0.0);
	
	int tf_iter = tf_ids.GetCount()-1;
	int main_tf = tf_ids[tf_iter];
	
	iter.bars = sys->GetCountTf(main_tf);
	iter.value_count = buf_count;
	
	
	// Add periods and their multipliers to next longer timeframes
	int tf_count = tf_ids.GetCount();
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
		Vector<Vector<DoubleTrio> >& sym_values		= iter.value[j];
		Vector<double>& min_sym_values			= iter.min_value[j];
		Vector<double>& max_sym_values			= iter.max_value[j];
		sym_values.SetCount(sym_count);
		min_sym_values.SetCount(sym_count, 0.0);
		max_sym_values.SetCount(sym_count, 0.0);
		for(int k = 0; k < sym_values.GetCount(); k++) {
			Vector<DoubleTrio>& values = sym_values[k];
			values.SetCount(buf_count, zero_trio);
		}
	}
	ASSERT(input_width && input_height && input_depth);
	iter.volume_in = Volume(input_width, input_height, input_depth, 0.0);
	iter.volume_out.SetCount(thrd_count);
	for(int i = 0; i < iter.volume_out.GetCount(); i++)
		iter.volume_out[i].SetCount(output_width, 0);
	
	
	// Get time range between shortest timeframe and now
	iter.begin = sys->GetBegin(main_tf);
	iter.begin_ts = sys->GetBeginTS(main_tf);
	
	
	// Seek to beginning
	Seek(thrd_id, 0);
}

bool Trainer::Seek(int thrd_id, int shift) {
	Iterator& iter = iters[thrd_id];
	int tf_iter = tf_ids.GetCount() - 1;
	int sym_count = sym_ids.GetCount();
	int buf_count = value_buffers[0][0].GetCount();
	int main_tf = tf_ids[tf_iter];
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
	
	
	// Refresh values (tf / sym / value)
	for(int i = 0; i < iter.value.GetCount(); i++) {
		int pos = iter.pos[i];
		int next_pos = pos+1;
		Vector<Vector<DoubleTrio> >& sym_values		= iter.value[i];
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
			Vector<DoubleTrio>& values = sym_values[j];
			Vector<ConstBuffer*>& indi_buffers = value_buffers[j][i];
			for(int k = 0; k < values.GetCount(); k++) {
				ConstBuffer& src = *indi_buffers[k];
				DoubleTrio& dst = values[k];
				dst.a = src.GetUnsafe(pos);
				if (pos) dst.b = src.GetUnsafe(pos-1);
				else dst.b = dst.a;
				if (next_pos < src.GetCount()) dst.c = src.GetUnsafe(next_pos);
				else dst.c = 0;
				double& min = min_sym_values[k];
				double& max = max_sym_values[k];
				if (dst.a < min) min = dst.a;
				if (dst.a > max) max = dst.a;
				for (int t = 0; t < iter.volume_out.GetCount(); t++)
					iter.volume_out[t].Set(j + i * sym_values.GetCount(), dst.c);
				iter.volume_in.Set(j, i, 0, dst.a);
			}
		}
	}
	
	return true;
}

bool Trainer::SeekCur(int thrd_id, int shift) {
	Iterator& iter = iters[thrd_id];
	int new_shift = iter.pos.Top() + shift;
	if (new_shift < 0) return false;
	if (new_shift >= iter.bars) return false;
	return Seek(thrd_id, new_shift);
}

}
