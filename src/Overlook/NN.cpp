#include "Overlook.h"

namespace Overlook {

void NetNN::Init() {
	System& sys = GetSystem();
	sym_count = sys.GetNetCount();

	int tf = GetTf();

	for(int i = 0; i < sym_count; i++) {
		cl_net.AddSymbol("Net" + IntStr(i));
	}
	cl_net.AddTf(tf);
	cl_net.AddIndi(0);
	cl_net.Init();
	cl_net.Refresh();


	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void NetNN::InitNN(Data& d) {
	d.ses.MakeLayers(
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":" + IntStr(sym_count) + ", \"input_height\":" + IntStr(input_length) + ", \"input_depth\":1},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(sym_count) + "},\n"
		"\t{\"type\":\"adadelta\", \"learning_rate\":1, \"batch_size\":50, \"l1_decay\":0.001, \"l2_decay\":0.001}\n"
		"]\n"
	);
	d.ses.Reset();
}

void NetNN::Sample(Data& d) {
	int data_count = cl_net.GetBuffer(0, 0, 0).GetCount();
	for(int i = 0; i < cl_net.GetSymbolCount(); i++)
		data_count = min(cl_net.GetBuffer(i, 0, 0).GetCount(), data_count);
	
	if (!d.is_realtime) {
		data_count *= 0.5;
		buf_begin = data_count;
	}

	cl_net.Refresh();

	if (d.ses.GetStepCount() >= NNCore::MAX_TRAIN_STEPS) {
		d.ses.Data().ClearData();
		return;
	}
	
	ConvNet::SessionData& data = d.ses.Data();
	data.BeginDataResult(sym_count, data_count-input_length-2, sym_count * input_length);

	for(int i = input_length+1, row = 0; i < data_count-1; i++, row++) {

		int col = 0;
		for(int j = 0; j < sym_count; j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);

			double next = buf.Get(i);
			double next2 = buf.Get(i+1);
			for(int k = 0; k < input_length; k++) {
				double cur = buf.Get(i-k-1);
				double ch = next / cur - 1.0;
				ch *= 100;
				if (!IsFin(ch)) ch = 0;
				data.SetData(row, col++, ch);
				next = cur;
			}
			double ch = next2 / next - 1.0;
			ch *= 100;
			data.SetResult(row, j, ch);
		}
	}
	data.EndData();

}

void NetNN::Start(Data& d, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	System::NetSetting& n = sys.GetNet(0);

	output.SetCount(sym_count);
	for(int i = 0; i < sym_count; i++)
		output[i] = 0;


	if (pos >= cl_net.GetBuffer(0, 0, 0).GetCount())
		cl_net.Refresh();
	if (pos >= cl_net.GetBuffer(0, 0, 0).GetCount())
		return;

	ConvNet::Volume vol;
	vol.Init(sym_count, input_length, 1, 0);
	int col = 0;
	for(int i = 0; i < sym_count; i++) {
		ConstBuffer& buf = cl_net.GetBuffer(i, 0, 0);

		double next = buf.Get(pos);
		for(int k = 0; k < input_length; k++) {
			double cur = buf.Get(max(0, pos-k-1));
			double ch = next / cur - 1.0;
			ch *= 100;
			if (!IsFin(ch)) ch = 0;
			vol.Set(col++, ch);
			next = cur;
		}
	}

	ConvNet::Net& net = d.ses.GetNetwork();
	ConvNet::Volume& result = net.Forward(vol);

	for(int i = 0; i < sym_count; i++) {
		output[i] = result.Get(i);
	}
}


void NetNN::FillVector(Data& d) {
	System& sys = GetSystem();
	
	d.ses.Data().ClearData();
	
	cl_sym.Refresh();
	
	Vector<double> output, symlots;
	VectorMap<int, double> abslots;
	
	System::NetSetting& n = sys.GetNet(0);
	symlots.SetCount(n.symbols.GetCount());
	
	int begin = sym_signals.GetCount() / output_sym_count;
	int count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	sym_signals.SetCount(count * output_sym_count, 0);
	
	for(int i = begin; i < count; i++) {
		Start(d, i, output);
		
		for(int j = 0; j < symlots.GetCount(); j++)
			symlots[j] = 0;
		
		for(int j = 0; j < output.GetCount(); j++) {
			System::NetSetting& n = sys.GetNet(j);
	
			double pred = output[j];
			int sig = pred > 0 ? +1 : -1;
	
			for(int j = 0; j < n.symbols.GetCount(); j++) {
				symlots[j] += sig * n.symbols[j];
			}
		}
		
		abslots.Clear();
		for(int j = 0; j < symlots.GetCount(); j++) {
			abslots.Add(j, fabs(symlots[j]));
		}
		SortByValue(abslots, StdGreater<double>());
		
		for(int j = 0; j < output_sym_count; j++) {
			int sym = abslots.GetKey(j);
			
			if (symlots[sym] >= 0)
				sym = -sym - 1;
			
			sym_signals[i * output_sym_count + j] = sym;
		}
	}
	
	
	for(int i = d.counted; i < d.buf.GetCount(); i++) {
		double value;
		
		int pos = buf_begin + i;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = d.buf[i-1];
			Start(d, pos-1, output);
			
			for(int j = 0; j < symlots.GetCount(); j++)
				symlots[j] = 0;
			
			for(int j = 0; j < output.GetCount(); j++) {
				System::NetSetting& n = sys.GetNet(j);
		
				double pred = output[j];
				int sig = pred > 0 ? +1 : -1;
		
				for(int j = 0; j < n.symbols.GetCount(); j++) {
					symlots[j] += sig * n.symbols[j] * 0.01;
				}
			}
			
			double max_lot_sum = 1.0;
			double lot_sum = 0;
			for(int j = 0; j < symlots.GetCount(); j++)
				lot_sum += fabs(symlots[j]);
			double factor = max_lot_sum / lot_sum;
			for(int j = 0; j < symlots.GetCount(); j++)
				symlots[j] *= factor;
			
			value = prev_value;
			
			VectorMap<int, double> abslots;
			for(int j = 0; j < symlots.GetCount(); j++) {
				abslots.Add(j, fabs(symlots[j]));
			}
			SortByValue(abslots, StdGreater<double>());
			
			//for(int j = 0; j < n.symbols.GetCount(); j++) {
			for(int j = 0; j < 2; j++) {
				int sym = abslots.GetKey(j);
				
				ConstBuffer& sym_buf = cl_sym.GetBuffer(sym, 0, 0);
				if (pos >= sym_buf.GetCount())
					continue;
				
				double point = cl_sym.GetDataBridge(sym)->GetPoint();
				
				symlots[sym] *= -1;
				
				double cur = sym_buf.Get(pos);
				double prev = sym_buf.Get(pos-1);
				
				//if (symlots[sym] > 0) prev += point * CommonSpreads()[sym];
				//else cur += point * CommonSpreads()[sym];
				
				double mult = cur / prev - 1.0;
				
				
				mult *= symlots[sym] * 0.1;
				//ASSERT(IsFin(mult));
				if (IsFin(mult))
					value += mult;
			}
		}
		
		ASSERT(value >= 0);
		d.buf[i] = value;
	}
}



















void IntPerfNN::Init() {
	System& sys = GetSystem();
	sym_count = sys.GetNetCount();

	int tf = GetTf();

	for(int i = 0; i < sym_count; i++) {
		cl_net.AddSymbol("Net" + IntStr(i));
	}
	cl_net.AddTf(tf);
	cl_net.AddIndi(0);
	cl_net.Init();
	cl_net.Refresh();


	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void IntPerfNN::InitNN(Data& d) {
	d.ses.MakeLayers(
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":" + IntStr(sym_count) + ", \"input_height\":" + IntStr(input_length) + ", \"input_depth\":1},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(sym_count) + "},\n"
		"\t{\"type\":\"adadelta\", \"learning_rate\":1, \"batch_size\":50, \"l1_decay\":0.001, \"l2_decay\":0.001}\n"
		"]\n"
	);
	d.ses.Reset();
}

void IntPerfNN::Sample(Data& d) {
	NNCore& c = GetInputCore(0);
	const Vector<double>& buf = c.GetBuffer(false); // not realtime
	
	int data_begin = buf.GetCount() * 0.333;
	int data_end = d.is_realtime ? buf.GetCount() - 1 : buf.GetCount() * 0.666;
	int data_count = data_end - data_begin;
	
	if (!d.is_realtime) buf_begin = data_end;
	
	ConvNet::SessionData& data = d.ses.Data();
	data.BeginDataResult(sym_count, data_count, sym_count * input_length);
	
	Vector<double> output;
	
	for(int i = data_begin, row = 0; i < data_end; i++, row++) {
		
		c.Start(false, i, output);
		
		int col = 0;
		for(int j = 0; j < sym_count; j++) {
			ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);

			double next = buf.Get(i);
			double next2 = buf.Get(i+1);
			for(int k = 0; k < input_length; k++) {
				double cur = buf.Get(i-k-1);
				double ch = next / cur - 1.0;
				ch *= 100;
				if (!IsFin(ch)) ch = 0;
				data.SetData(row, col++, ch);
				next = cur;
			}
			double ch = next2 / next - 1.0;
			ch *= 100;
			
			ch = fabs(ch - output[j]);
			
			data.SetResult(row, j, ch);
		}
	}
	data.EndData();
}

void IntPerfNN::Start(Data& d, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	Vector<double> net_sigs;
	
	NNCore& c = GetInputCore(0);
	c.Start(d.is_realtime, pos, net_sigs);
	
	
	ConvNet::Volume vol;
	vol.Init(sym_count, input_length, 1, 0);
	
	int col = 0;
	for(int j = 0; j < sym_count; j++) {
		ConstBuffer& buf = cl_net.GetBuffer(j, 0, 0);
		int max_pos = buf.GetCount() - 1;
		
		double next = buf.Get(min(max_pos, pos));
		for(int k = 0; k < input_length; k++) {
			double cur = buf.Get(min(max_pos, max(0, pos-k-1)));
			double ch = next / cur - 1.0;
			ch *= 100;
			if (!IsFin(ch)) ch = 0;
			vol.Set(col++, ch);
			next = cur;
		}
	}
	
	
	ConvNet::Net& net = d.ses.GetNetwork();
	ConvNet::Volume& fwd = net.Forward(vol);
	
	VectorMap<int, double> list;
	for(int i = 0; i < fwd.GetCount(); i++) {
		list.Add(i, fwd.Get(i));
	}
	SortByValue(list, StdGreater<double>());
	
	System::NetSetting& n = sys.GetNet(0);
	output.SetCount(n.symbols.GetCount());
	for(double& d : output) d = 0.0;
	
	for(int i = 0; i < 3; i++) {
		int j = list.GetKey(i);
		System::NetSetting& n = sys.GetNet(j);
		double pred = -1 * net_sigs[j]; // use worst values negatively
		int sig = pred > 0 ? +1 : -1;
		
		for(int k = 0; k < n.symbol_ids.GetCount(); k++) {
			output[k] += sig * n.symbols[k] * 0.01;
		}
	}
	
}

void IntPerfNN::FillVector(Data& d) {
	System& sys = GetSystem();
	
	cl_sym.Refresh();
	cl_net.Refresh();
	
	Vector<double> output;
	
	for(int i = d.counted; i < d.buf.GetCount(); i++) {
		double value;
		
		int pos = buf_begin + i;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = d.buf[i-1];
			Start(d, pos-1, output);
			
			double max_lot_sum = 1.0;
			double lot_sum = 0;
			for(int j = 0; j < output.GetCount(); j++)
				lot_sum += fabs(output[j]);
			double factor = max_lot_sum / lot_sum;
			for(int j = 0; j < output.GetCount(); j++)
				output[j] *= factor;
			
			value = prev_value;
			
			for(int j = 0; j < output.GetCount(); j++) {
				ConstBuffer& sym_buf = cl_sym.GetBuffer(j, 0, 0);
				if (pos >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(pos);
				double prev = sym_buf.Get(pos-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				ASSERT(IsFin(mult));
				value += mult;
			}
		}
		
		ASSERT(value >= 0);
		d.buf[i] = value;
	}
	
}



















void MultiTfNetNN::Init() {
	System& sys = GetSystem();
	
	int tf = GetTf();
	
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
}

void MultiTfNetNN::InitNN(Data& d) {
	int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();

	if (!d.is_realtime) {
		data_count *= 0.666;
		buf_begin = data_count;
	}
}

void MultiTfNetNN::Sample(Data& d) {
	
}

int WeekBegin(int wday) {
	wday -= 1;
	if (wday == -1) wday += 7;
	return wday;
}

void MultiTfNetNN::Start(Data& d, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	System::NetSetting& net = sys.GetNet(0);
	
	
	output.SetCount(net.symbols.GetCount());
	for(int i = 0; i < output.GetCount(); i++)
		output[i] = 0;
	
	Time t = Time(1970,1,1) + cl_sym.GetBuffer(0, 0, 4).Get(pos);
	
	for(int i = 0; i < 3; i++) {
		int tf = GetTf() + i;
		const Index<Time>& idx = dbc.GetTimeIndex(tf);
		
		Time tf_time = t;
		switch (tf) {
			case 2: t.minute -= t.minute % 15; break;
			case 3: t.minute -= t.minute % 30; break;
			case 4: t.minute = 0; break;
			case 5: tf_time.hour -= tf_time.hour % 4; t.minute = 0; break;
			case 6: tf_time.hour = 0; t.minute = 0; break;
			case 7: tf_time.hour = 0; t.minute = 0;
				for(int j = 0; j < 7; j++) {
					if (idx.Find(tf_time) != -1)
						break;
					tf_time -= 24*60*60;
				}
				break;
			default: Panic("Invalid tf");
		}
		
		int tf_pos = idx.Find(tf_time);
		if (tf_pos == -1)
			continue;
		
		NNCore& c = GetInputCore(i);
		
		c.Start(d.is_realtime, tf_pos, tmp);
		
		for(int j = 0; j < output.GetCount(); j++)
			output[j] += tmp[j];
	}
}

void MultiTfNetNN::FillVector(Data& d) {
	System& sys = GetSystem();
	
	cl_sym.Refresh();
	
	Vector<double> output;
	
	for(int i = d.counted; i < d.buf.GetCount(); i++) {
		double value;
		
		int pos = buf_begin + i;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = d.buf[i-1];
			Start(d, pos-1, output);
			
			double max_lot_sum = 1.0;
			double lot_sum = 0;
			for(int j = 0; j < output.GetCount(); j++)
				lot_sum += fabs(output[j]);
			double factor = max_lot_sum / lot_sum;
			for(int j = 0; j < output.GetCount(); j++)
				output[j] *= factor;
			
			value = prev_value;
			
			for(int j = 0; j < output.GetCount(); j++) {
				ConstBuffer& sym_buf = cl_sym.GetBuffer(j, 0, 0);
				if (pos >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(pos);
				double prev = sym_buf.Get(pos-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				//ASSERT(IsFin(mult));
				if (IsFin(mult))
					value += mult;
			}
		}
		
		ASSERT(value >= 0);
		d.buf[i] = value;
	}
}

























void MartNN::Init() {
	is_optimized = true;
	
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void MartNN::InitNN(Data& d) {
	
	d.opt.Min().SetCount(OPT_COUNT, 0.0);
	d.opt.Max().SetCount(OPT_COUNT, 1.0);
	d.opt.Init(OPT_COUNT, 100);
}

void MartNN::Optimize(Data& d) {
	NNCore& c = GetInputCore(0);
	const Vector<double>& buf = c.GetBuffer(false); // not realtime
	
	int total = cl_sym.GetBuffer(0, 0, 0).GetCount();
	int data_begin = total * 0.5;
	int data_end = d.is_realtime ? total - 1 : total * (0.5 + 0.5*0.5);
	int data_count = data_end - data_begin;
	
	if (!d.is_realtime) buf_begin = data_end;
	
	MartData& m = this->data[d.is_realtime];
	
	double best_result = -DBL_MAX;
	int best_begin, best_end;

	if (!d.opt.IsEnd()) {
		
		
		d.buf.SetCount(0);
		while (!d.opt.IsEnd()) {
			d.opt.Start();
			
			LoadTrial(d.opt.GetTrialSolution(), m);
			
			/*int test_length = 1440 * 5*2 + max(m.group_av, m.signal_length);
			if (test_length >= data_count) test_length = data_count - 1;
			int test_begin = data_begin + Random(data_count - test_length);
			int test_end = test_begin + test_length;*/
			int test_begin = data_begin;
			int test_end = data_end;
			
			
			if (m.signal_minsum > m.signal_length) {
				d.opt.Stop(-1000000);
			}
			else {
				IterateOnce(d, m, test_begin, test_end);
				
				if (m.history_orders_total == 0)
					d.opt.Stop(-1000000);
				else {
					d.opt.Stop(m.balance);
					d.buf.Add(m.balance);
				}
				
				if (m.balance > best_result) {
					best_result = m.balance;
					best_trial[d.is_realtime] <<= d.opt.GetTrialSolution();
					best_begin = test_begin;
					best_end = test_end;
				}
			}
		}
		
		/*const auto& best_sol = d.opt.GetBestSolution();
		for(int i = 0; i < best_trial.GetCount(); i++) {
			if (best_trial[i] != best_sol[i])
				Panic("DIFFERENT BEST SOLUTION");
		}*/
		//LoadTrial(d.opt.GetBestSolution(), m);
		LoadTrial(best_trial[d.is_realtime], m);
		data_begin = total * (0.5 + 0.5*0.5);
		data_end = total - 10;
		write_equity = true;
		d.buf.SetCount(0);
		//IterateOnce(d, m, data_begin, data_end);
		IterateOnce(d, m, best_begin, best_end);
		write_equity = false;
		ReleaseLog("MartNN " + IntStr(d.is_realtime) + " equity " + DblStr(m.balance));
		
		#define DUMPM(x) ReleaseLog(#x " " + IntStr(x));
		DUMPM(m.group_period);
		DUMPM(m.group_step);
		DUMPM(m.group_av);
		DUMPM(m.signal_length);
		DUMPM(m.signal_minsum);
		DUMPM(m.signal_minlength);
		DUMPM(m.signal_minfactor);
		DUMPM(m.signal_mingroupsize);
		DUMPM(m.signal_finalminlength);
		DUMPM(m.max_martingale);
		DUMPM(m.stoploss);
		DUMPM(m.takeprofit);
		DUMPM(m.trailstoploss);
		DUMPM(m.trailtakeprofit);
	}
}

void MartNN::LoadTrial(const Vector<double>& trial, MartData& m) {
	#define LIMIT(mi, ma, z) max(mi, min(ma, (int)(trial[z] * ma)))
	m.group_period = LIMIT(2, 240, OPT_GROUPPERIOD);
	m.group_step = LIMIT(0, 100, OPT_GROUPSTEP);
	m.group_av = LIMIT(2, 100, OPT_GROUPAV);
	
	m.signal_length = LIMIT(1, 240, OPT_SIGLEN);
	m.signal_minsum = LIMIT(0, 30, OPT_MINSUM);
	m.signal_minlength = LIMIT(1, 240, OPT_MINLEN);
	m.signal_minfactor = LIMIT(1, 100, OPT_MINFACT);
	m.signal_mingroupsize = LIMIT(0, 10, OPT_MINGROUP);
	m.signal_finalminlength = LIMIT(1, 240, OPT_FINMINLEN);
	
	m.max_martingale = LIMIT(0, 10, OPT_MAXMART);
	m.stoploss = LIMIT(5, 100, OPT_SL);
	m.takeprofit = LIMIT(5, 100, OPT_TP);
	m.trailstoploss = LIMIT(5, 100, OPT_TRAILSL);
	m.trailtakeprofit = LIMIT(5, 100, OPT_TRAILTP);
}

void MartNN::ResetPattern(MartData& m) {
	m.pattern.SetCount(0);
	for(int i = 0; i < 64; i++) {
		int a = Random(m.group_period);
		int b;
		do {
			b = Random(m.group_period);
		}
		while (a == b);
		m.pattern.Add(Point(a, b));
	}
}

void MartNN::ResetDistanceAverages(MartData& m) {
	int sym_count = cl_sym.GetSymbolCount();
	int count = 0;
	for(int j0 = 0; j0 < sym_count; j0++)
		for(int j1 = j0+1; j1 < sym_count; j1++)
			count++;
	m.distance_averages.SetCount(count);
	for(int i = 0; i < count; i++)
		m.distance_averages[i].SetPeriod(m.group_av);
}

void MartNN::ResetSignalAverages(MartData& m) {
	int sym_count = cl_sym.GetSymbolCount();
	m.signal_pos_averages.SetCount(sym_count);
	m.signal_neg_averages.SetCount(sym_count);
	m.pos_group_signal.SetCount(sym_count, 0);
	m.neg_group_signal.SetCount(sym_count, 0);
	m.pos_signal_since_activation.SetCount(sym_count, 0);
	m.neg_signal_since_activation.SetCount(sym_count, 0);
	m.pos_final_since_activation.SetCount(sym_count, 0);
	m.neg_final_since_activation.SetCount(sym_count, 0);
	m.signals.SetCount(sym_count, 0);
	
	for(int i = 0; i < sym_count; i++) {
		m.signal_pos_averages[i].SetPeriod(m.signal_length);
		m.signal_neg_averages[i].SetPeriod(m.signal_length);
		
		m.pos_signal_since_activation[i] = 1000000;
		m.neg_signal_since_activation[i] = 1000000;
		m.pos_final_since_activation[i] = 1000000;
		m.neg_final_since_activation[i] = 1000000;
		m.signals[i] = 0;
	}
}

void MartNN::IterateOnce(Data& d, MartData& m, int begin, int end) {
	m.orders.SetCount(0);
	m.pattern.SetCount(0);
	m.distances.SetCount(0);
	m.distance_averages.SetCount(0);
	m.signal_pos_averages.SetCount(0);
	m.signal_neg_averages.SetCount(0);
	m.descriptors.SetCount(0);
	m.symbol_group.SetCount(0);
	m.symbol_factor.SetCount(0);
	m.pos_signal_since_activation.SetCount(0);
	m.neg_signal_since_activation.SetCount(0);
	m.pos_final_since_activation.SetCount(0);
	m.neg_final_since_activation.SetCount(0);
	m.pos_group_signal.SetCount(0);
	m.neg_group_signal.SetCount(0);
	m.signals.SetCount(0);
	m.prev_signals.SetCount(0);
	m.group_size.SetCount(0);
	m.symbol_added.SetCount(0);
	m.symbol_has_orders.SetCount(0);
	
	m.equity = 10000;
	m.balance = 10000;
	m.mult = 1;
	m.loss_count = 0;
	m.history_orders_total = 0;
	
	ResetPattern(m);
	ResetDistanceAverages(m);
	ResetSignalAverages(m);
	
	for(int i = begin; i < end; i++) {
		m.pos = i;
		
		IterateGroups(m, i);
		
		IterateSignals(m, i);
		
		if (i < begin + m.group_av) continue;
		if (i < begin + m.signal_length) continue;
		
		IterateLimits(m, i);
		
		if (write_equity)
			d.buf.Add(m.equity);
		if (m.equity < 0.0)
			break;
	}
	
	while (m.orders.GetCount())
		CloseOrder(m, 0, m.orders[0].lots);
}

void MartNN::IterateGroups(MartData& m, int i) {
	int sym_count = cl_sym.GetSymbolCount();
	m.descriptors.SetCount(sym_count);
	m.symbol_added.SetCount(sym_count);
	m.symbol_group.SetCount(sym_count);
	m.group_size.SetCount(sym_count);
	m.symbol_factor.SetCount(sym_count);
	
	for(int j = 0; j < cl_sym.GetSymbolCount(); j++) {
		ConstBuffer& buf = cl_sym.GetBuffer(j, 0, 0);
		uint64 desc = 0;
		for(int k = 0; k < 64; k++) {
			const Point& p = m.pattern[k];
			double a = buf.Get(max(0, i - p.x));
			double b = buf.Get(max(0, i - p.y));
			if (a < b)
				desc |= 1 << k;
		}
		m.descriptors[j] = desc;
	}
	
	m.distances.SetCount(0);
	for(int j0 = 0; j0 < sym_count; j0++) {
		for(int j1 = j0+1; j1 < sym_count; j1++) {
			uint64 a = m.descriptors[j0];
			uint64 b = m.descriptors[j1];
			int dist = PopCount64(a ^ b);
			PatternDistance& d = m.distances.Add();
			d.j0 = j0;
			d.j1 = j1;
			d.dist = dist * 100 / 64;
			d.absdist = d.dist >= 50 ? 100 - (d.dist - 50) * 2 : d.dist * 2;
		}
	}
	ASSERT(m.distances.GetCount() == m.distance_averages.GetCount());
	for(int i = 0; i < m.distances.GetCount(); i++) {
		OnlineAverageWindow1& av = m.distance_averages[i];
		PatternDistance& d = m.distances[i];
		av.Add(d.absdist);
		d.absdist = av.GetMean();
	}
	Sort(m.distances, PatternDistance());
	
	
	for(int i = 0; i < sym_count; i++) {
		m.symbol_added[i] = false;
		m.group_size[i] = 0;
	}
	int group_count = 0;
	for(int j = 0; j < m.distances.GetCount(); j++) {
		PatternDistance& d = m.distances[j];
		
		if (d.absdist > m.group_step) break;
		
		if (m.symbol_added[d.j0] && m.symbol_added[d.j1])
			continue;
		
		bool added = false;
		for(int k = 0; k < group_count; k++) {
			for(int sym = 0; sym < sym_count; sym++) {
				if (m.symbol_group[sym] != k)
					continue;
				if (d.j0 == sym && m.symbol_added[d.j1] == false) {
					m.group_size[k]++;
					m.symbol_group[d.j1] = k;
					m.symbol_added[d.j1] = true;
					added = true;
					int j0_factor = m.symbol_factor[d.j0];
					int j1_factor = j0_factor * (d.dist < 50 ? +1 : -1);
					m.symbol_factor[d.j1] = j1_factor;
					break;
				}
				else if (d.j1 == sym && m.symbol_added[d.j0] == false) {
					m.group_size[k]++;
					m.symbol_group[d.j0] = k;
					m.symbol_added[d.j0] = true;
					added = true;
					int j1_factor = m.symbol_factor[d.j1];
					int j0_factor = j1_factor * (d.dist < 50 ? +1 : -1);
					m.symbol_factor[d.j0] = j0_factor;
					break;
				}
			}
		}
		
		if (!added) {
			int group_id = group_count++;
			m.group_size[group_id] += 2;
			m.symbol_group[d.j0] = group_id;
			m.symbol_group[d.j1] = group_id;
			m.symbol_added[d.j0] = true;
			m.symbol_added[d.j1] = true;
			m.symbol_factor[d.j0] = +1;
			m.symbol_factor[d.j1] = d.dist < 50 ? +1 : -1;
		}
	}
	
	
	for(int j = 0; j < sym_count; j++) {
		if (m.symbol_added[j]) continue;
		
		int group_id = group_count++;
		m.group_size[group_id]++;
		m.symbol_group[j] = group_id;
		m.symbol_factor[j] = +1;
	}
}

void MartNN::IterateSignals(MartData& m, int i) {
	NNCore& c = GetInputCore(0);
	NetNN& nn = dynamic_cast<NetNN&>(c);
	int sym_count = cl_sym.GetSymbolCount();
	
	
	int nn_readpos = NetNN::output_sym_count * i;
	
	m.symbol_added.SetCount(sym_count);
	for(bool& b : m.symbol_added) b = false;
	
	
	for(int k = 0; k < NetNN::output_sym_count; k++) {
		int sym = nn.sym_signals[nn_readpos + k];
		if (sym < 0) {
			sym = -sym - 1;
			m.signal_pos_averages[sym].Add(0);
			m.signal_neg_averages[sym].Add(1);
		} else {
			m.signal_pos_averages[sym].Add(1);
			m.signal_neg_averages[sym].Add(0);
		}
		m.symbol_added[sym] = true;
	}
	
	for(int i = 0; i < sym_count; i++) {
		if (!m.symbol_added[i]) {
			m.signal_pos_averages[i].Add(0);
			m.signal_neg_averages[i].Add(0);
		}
	}
	
	
	for(int i = 0; i < sym_count; i++) {
		int pos_sum = m.signal_pos_averages[i].GetSum();
		int neg_sum = m.signal_neg_averages[i].GetSum();
		
		if (pos_sum >= m.signal_minsum && pos_sum > neg_sum) {
			m.pos_signal_since_activation[i] = 0;
		}
		if (neg_sum >= m.signal_minsum && neg_sum > pos_sum) {
			m.neg_signal_since_activation[i] = 0;
		}
	}
	
	for(int i = 0; i < sym_count; i++) {
		m.pos_group_signal[i] = 0;
		m.neg_group_signal[i] = 0;
	}
	for(int i = 0; i < sym_count; i++) {
		if (m.pos_signal_since_activation[i] < m.signal_minlength) {
			int group = m.symbol_group[i];
			int factor = m.symbol_factor[i];
			if (factor > 0) {
				m.pos_group_signal[group]++;
			} else {
				m.neg_group_signal[group]++;
			}
		}
		if (m.neg_signal_since_activation[i] < m.signal_minlength) {
			int group = m.symbol_group[i];
			int factor = m.symbol_factor[i];
			if (factor > 0) {
				m.neg_group_signal[group]++;
			} else {
				m.pos_group_signal[group]++;
			}
		}
	}
	
	
	for(int i = 0; i < sym_count; i++) {
		m.signals[i] = 0;
	}
	
	// Loop groups (sym_count == max_group_count)
	for(int i = 0; i < sym_count; i++) {
		int size = m.group_size[i];
		if (!size)
			break;
		if (size < m.signal_mingroupsize)
			continue;
		int pos = m.pos_group_signal[i];
		int neg = m.neg_group_signal[i];
		int sum = pos + neg;
		if (!sum) continue;
		int pos_factor = pos * 100 / sum;
		int neg_factor = neg * 100 / sum;
		
		if (pos_factor >= m.signal_minfactor && pos_factor > neg_factor) {
			int cheapest_sym = -1;
			for(int j = 0; j < sym_count; j++) {
				if (m.symbol_group[j] == i) {
					cheapest_sym = j;
					break;
				}
			}
			int factor = m.symbol_factor[cheapest_sym];
			if (factor > 0)
				m.pos_final_since_activation[cheapest_sym] = 0;
			else
				m.neg_final_since_activation[cheapest_sym] = 0;
		}
		
		if (neg_factor >= m.signal_minfactor && neg_factor > pos_factor) {
			int cheapest_sym = -1;
			for(int j = 0; j < sym_count; j++) {
				if (m.symbol_group[j] == i) {
					cheapest_sym = j;
					break;
				}
			}
			int factor = m.symbol_factor[cheapest_sym];
			if (factor > 0)
				m.neg_final_since_activation[cheapest_sym] = 0;
			else
				m.pos_final_since_activation[cheapest_sym] = 0;
		}
	}
	
	
	for(int i = 0; i < sym_count; i++) {
		if (m.pos_final_since_activation[i] < m.signal_finalminlength)
			m.signals[i]++;
		if (m.neg_final_since_activation[i] < m.signal_finalminlength)
			m.signals[i]--;
		
		m.pos_signal_since_activation[i]++;
		m.neg_signal_since_activation[i]++;
		m.pos_final_since_activation[i]++;
		m.neg_final_since_activation[i]++;
	}
}

void MartNN::IterateLimits(MartData& m, int i) {
	int sym_count = cl_sym.GetSymbolCount();
	
	m.prev_signals.SetCount(sym_count, 0);
	m.symbol_has_orders.SetCount(sym_count, 0);
	
	for(bool& b : m.symbol_has_orders) b = false;
	for(int i = 0; i < m.orders.GetCount(); i++) {
		m.symbol_has_orders[m.orders[i].symbol] = true;
	}
	int open_sym_count = 0;
	for(int i = 0; i < sym_count; i++)
		if (m.symbol_has_orders[i])
			open_sym_count++;
	
	for(int i = 0; i < sym_count; i++) {
		int sig = m.signals[i];
		
		if (sig != m.prev_signals[i]) {
			double max_lots_sum = m.balance * 0.001;
			double max_sym_lots = max_lots_sum / max_open_symbols;
			double max_mart = pow(2, m.max_martingale);
			double mart = pow(2, m.loss_count);
			double factor = min(1.0, mart / max_mart);
			double lots = max_sym_lots * factor;
			if (sig < 0) lots *= -1;
			if (open_sym_count >= max_open_symbols)
				lots = 0;
			SetSymbolLots(m, i, lots);
			if (lots != 0.0 && m.symbol_has_orders[i] == false)
				open_sym_count++;
		}
		
		m.prev_signals[i] = sig;
	}
	
	// Refresh equity, check stops & trailing stops
	m.equity = m.balance;
	for(int i = 0; i < m.orders.GetCount(); i++) {
		Order& o = m.orders[i];
		double point = cl_sym.GetDataBridge(o.symbol)->GetPoint();
		if (o.type == OP_BUY) {
			double bid = cl_sym.GetBuffer(o.symbol, 0, 0).Get(m.pos);
			double profit = (bid / o.open - 1.0) * 100000 * o.lots;
			m.equity += profit;
			
			// Check stops
			if (bid > o.highest_close) o.highest_close = bid;
			if (bid < o.lowest_close)  o.lowest_close  = bid;
			double stoploss = o.open - m.stoploss * point;
			double takeprofit = o.open + m.takeprofit * point;
			double trailing_sl = o.highest_close - m.trailstoploss * point;
			double trailing_tp = o.lowest_close + m.trailtakeprofit * point;
			if (bid <= stoploss    || bid >= takeprofit ||
				bid <= trailing_sl || bid >= trailing_tp) {
				CloseOrder(m, i, o.lots);
				i--;
			}
		}
		else if (o.type == OP_SELL) {
			// ask = bid + spread * point
			double ask = cl_sym.GetBuffer(o.symbol, 0, 0).Get(m.pos) + CommonSpreads()[o.symbol] * point;
			double profit = (ask / o.open - 1.0) * 100000 * o.lots;
			profit *= -1;
			m.equity += profit;
			
			// Check stops
			if (ask > o.highest_close) o.highest_close = ask;
			if (ask < o.lowest_close)  o.lowest_close  = ask;
			double stoploss = o.open + m.stoploss * point;
			double takeprofit = o.open - m.takeprofit * point;
			double trailing_sl = o.lowest_close + m.trailstoploss * point;
			double trailing_tp = o.highest_close - m.trailtakeprofit * point;
			if (ask >= stoploss    || ask <= takeprofit ||
				ask >= trailing_sl || ask <= trailing_tp) {
				CloseOrder(m, i, o.lots);
				i--;
			}
		}
	}
	
}

void MartNN::SetSymbolLots(MartData& m, int sym, double lots) {
	double buy_lots, sell_lots;
	if (lots > 0) {
		buy_lots = lots;
		sell_lots = 0;
	} else {
		sell_lots = -lots;
		buy_lots = 0;
	}
		
	double cur_buy_lots = 0;
	double cur_sell_lots = 0;
	
	for(int i = 0; i < m.orders.GetCount(); i++) {
		Order& o = m.orders[i];
		if (o.symbol != sym) continue;
		if (o.type == OP_BUY) {
			cur_buy_lots += o.lots;
		}
		else if (o.type == OP_SELL) {
			cur_sell_lots += o.lots;
		}
	}
	
	double close_buy_lots = 0, close_sell_lots = 0;
	double open_buy_lots = 0, open_sell_lots = 0;
	
	if (buy_lots == 0 && sell_lots == 0) {
		close_buy_lots = cur_buy_lots;
		close_sell_lots = cur_sell_lots;
	}
	else if (buy_lots > 0) {
		close_sell_lots = cur_sell_lots;
		close_buy_lots = max(0.0, cur_buy_lots - buy_lots);
		open_buy_lots = max(0.0, buy_lots - cur_buy_lots);
	}
	else if (sell_lots > 0) {
		close_buy_lots = cur_buy_lots;
		close_sell_lots = max(0.0, cur_sell_lots - sell_lots);
		open_sell_lots = max(0.0, sell_lots - cur_sell_lots);
	}
	
	if (close_buy_lots > 0.0 || close_sell_lots > 0.0) {
		for(int i = m.orders.GetCount()-1; i >= 0; i--) {
			Order& o = m.orders[i];
			if (o.symbol != sym) continue;
			
			if (o.type == OP_BUY && close_buy_lots > 0.0) {
				if (o.lots <= close_buy_lots) {
					close_buy_lots -= o.lots;
					CloseOrder(m, i, o.lots);
				}
				else {
					CloseOrder(m, i, close_buy_lots);
					close_buy_lots = 0;
				}
			}
			else if (o.type == OP_SELL && close_sell_lots > 0.0) {
				if (o.lots <= close_sell_lots) {
					close_sell_lots -= o.lots;
					CloseOrder(m, i, o.lots);
				}
				else {
					CloseOrder(m, i, close_sell_lots);
					close_sell_lots = 0;
				}
			}
		}
	}
	
	if (open_buy_lots > 0)
		OpenOrder(m, sym, OP_BUY, open_buy_lots);
	else if (open_sell_lots > 0)
		OpenOrder(m, sym, OP_SELL, open_sell_lots);
	
}

void MartNN::CloseOrder(MartData& m, int i, double lots) {
	Order& o = m.orders[i];
	if (o.type == OP_BUY || o.type == OP_SELL) {
		double close = cl_sym.GetBuffer(o.symbol, 0, 0).Get(m.pos);
		if (o.type == OP_SELL)
			close += CommonSpreads()[o.symbol] * cl_sym.GetDataBridge(o.symbol)->GetPoint(); // spread
		double profit = (close / o.open - 1.0) * 100000 * lots;
		if (o.type == OP_SELL)
			profit *= -1;
		if (profit < 0)
			m.loss_count++;
		else
			m.loss_count = 0;
		m.balance += profit;
		o.lots -= lots;
		ASSERT(o.lots >= 0);
		if (o.lots <= 0)
			m.orders.Remove(i);
	}
	else
		m.orders.Remove(i);
}

void MartNN::OpenOrder(MartData& m, int sym, int type, double lots) {
	Order& o = m.orders.Add();
	o.symbol = sym;
	o.type = type;
	o.lots = lots;
	ASSERT(lots > 0.0);
	o.open = cl_sym.GetBuffer(o.symbol, 0, 0).Get(m.pos);
	double close = o.open;
	double spread = CommonSpreads()[o.symbol] * cl_sym.GetDataBridge(o.symbol)->GetPoint();
	if (type == OP_BUY)
		o.open += spread;
	else
		close  += spread; // spread
	o.highest_close = close;
	o.lowest_close = close;
	m.history_orders_total++;
}

void MartNN::Start(Data& d, int pos, Vector<double>& output) {
	/*NNCore& c = GetInputCore(0);
	c.Start(d.is_realtime, pos, output);
	
	const Vector<double>& buf = c.GetBuffer(d.is_realtime);
	
	double cur = buf[pos];
	double prev = buf[max(0, pos-1)];
	if (cur >= prev)
		mult *= 2;
	else
		mult = 1;
	
	double max_lot_sum = 1.0;
	double lot_sum = 0;
	for(int j = 0; j < output.GetCount(); j++)
		lot_sum += fabs(output[j]);
	double factor = max_lot_sum / lot_sum;
	for(int j = 0; j < output.GetCount(); j++)
		output[j] *= factor;
	

	double max_factor = pow(2, max_martingale);
	factor = (double)mult / max_factor;
	if (factor > 1)
		factor = 1;
	
	for(int i = 0; i < output.GetCount(); i++)
		output[i] *= factor;
	*/
}

void MartNN::FillVector(Data& d) {
	/*System& sys = GetSystem();
	
	cl_sym.Refresh();
	
	Vector<double> output;
	
	
	for(int i = d.counted; i < d.buf.GetCount(); i++) {
		double value;
		
		int pos = buf_begin + i;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = d.buf[i-1];
			Start(d, pos-1, output);
			
			value = prev_value;
			
			for(int j = 0; j < output.GetCount(); j++) {
				ConstBuffer& sym_buf = cl_sym.GetBuffer(j, 0, 0);
				if (pos >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(pos);
				double prev = sym_buf.Get(pos-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				//ASSERT(IsFin(mult));
				if (IsFin(mult))
					value += mult;
			}
		}
		
		ASSERT(value >= 0);
		d.buf[i] = value;
	}*/
}


























void StatsNN::Init() {
	is_optimized = true;
	
	System& sys = GetSystem();
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
}

void StatsNN::InitNN(Data& data) {
	
}

void StatsNN::Optimize(Data& data) {
	/*NNCore& c = GetInputCore(0);
	NetNN& nn = dynamic_cast<NetNN&>(c);
	int sym_count = cl_sym.GetSymbolCount();
	
	int count = nn.sym_signals.GetCount() / NetNN::output_sym_count;
	
	for(int i = 0; i < count; i++) {
		int nn_readpos = NetNN::output_sym_count * i;
		
		
		for(int k = 0; k < NetNN::output_sym_count; k++) {
			int sym = nn.sym_signals[nn_readpos + k];
			if (sym < 0) {
				sym = -sym - 1;
				
			} else {
				
			}
		}
	}*/
}

void StatsNN::Start(Data& data, int pos, Vector<double>& output) {
	
}

void StatsNN::FillVector(Data& data) {
	
}



}
