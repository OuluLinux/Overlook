#include "Overlook.h"

namespace Overlook {
using namespace Upp;

Trainer::Trainer(System& sys) : sys(&sys) {
	not_stopped = 0;
	running = false;
	input_width = 0;
	input_height = 0;
	input_depth = 0;
	
	thrd_count = CPU_Cores();
	
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
	if (!thrds.IsEmpty())
		return;
	
	input_width  = sym_ids.GetCount();
	input_height = tf_ids.GetCount();
	input_depth  = 1;
	output_width = input_width * (input_height + 1);
	int volume = input_width * input_height * input_depth;
	
	thrds.SetCount(thrd_count);
	
	for(int i = 0; i < thrds.GetCount(); i++) {
		SessionThread& t = thrds[i];
		
		String net =
			"[\n"
			"\t{\"type\":\"input\""
				", \"input_width\":"  + IntStr(input_width)  +
				", \"input_height\":" + IntStr(input_height) +
				", \"input_depth\":"  + IntStr(input_depth)  +
				"},\n" // 2 inputs: x, y
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
		
		
		t.ses.MakeLayers(net);
		
		SimBroker& sb = t.broker;
		sb.Brokerage::operator=((Brokerage&)GetMetaTrader());
		sb.InitLightweight();
	}
}

void Trainer::Start() {
	Stop();
	
	running = true;
	for(int i = 0; i < thrds.GetCount(); i++) {
		not_stopped++;
		Thread::Start(THISBACK1(Runner, i));
	}
}

void Trainer::Stop() {
	running = false;
	while (not_stopped) Sleep(100);
}

void Trainer::Runner(int i) {
	Vector<VolumePtr> vec;
	SessionData d;
	int forward_time, backward_time;
	
	int step_num = 0;
	int predict_interval = 10;
	bool test_predict = true;
	int perf_begin = tf_ids.GetCount() * sym_ids.GetCount();
	
	// Sanity checks
	ASSERT(!thrds.IsEmpty());
	SessionThread& st = thrds[i];
	TrainerBase& trainer = *st.ses.GetTrainer();
	ConvNet::Net& net = st.ses.GetNetwork();
	Volume& x = iter.volume_in;
	VolumeData<double>& y = iter.volume_out[i];
	
	while (running) {
		
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
		
		vec.SetCount(1);
		vec[0] = &x;
		const Volume& fwd = net.Forward(vec, true);
		
		
		// Experimental internal performance value
		// - value which gives succeeding limit between less & more predictable output values
		for(int i = 0; i < sym_ids.GetCount(); i++) {
			int tf = 0;
			for(; tf < tf_ids.GetCount(); tf++) {
				int pos = i + tf * sym_ids.GetCount();
				double predicted	= fwd.Get(pos);
				double real			= y.Get(pos);
				bool same_dir = (predicted * real) > 0.0;
				if (!same_dir) break;
			}
			double perf_value = (double)tf / (double)tf_ids.GetCount();
			y.Set(perf_begin + i, perf_value);
		}
		
		
		trainer.Backward(y);
		trainer.TrainImplem();
		
		backward_time = ts.Elapsed();
		
		double reward = trainer.GetReward();
		double loss = trainer.GetLoss();
		double loss_l1d = trainer.GetL1DecayLoss();
		double loss_l2d = trainer.GetL2DecayLoss();
		step_num++;
		
		
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
		
		st.reward_window.Add(reward);
		st.loss_window.Add(loss);
		st.l1_loss_window.Add(loss_l1d);
		st.l2_loss_window.Add(loss_l2d);
		
		
		//if ((step_num % step_cb_interal) == 0)
		//	WhenStepInterval(step_num);
		LOG(Format("thrd=%d step=%d", i, step_num));
		step_num++;
	}
	
	not_stopped--;
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

void Trainer::ResetIterator() {
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
	Seek(0);
}

bool Trainer::Seek(int shift) {
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

bool Trainer::SeekCur(int shift) {
	int new_shift = iter.pos.Top() + shift;
	if (new_shift < 0) return false;
	if (new_shift >= iter.bars) return false;
	return Seek(new_shift);
}

}