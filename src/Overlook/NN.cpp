#include "Overlook.h"

namespace Overlook {

void NetNN::Init() {
	System& sys = GetSystem();
	
	int tf = GetTf();
	System::NetSetting& net = sys.GetNet(0);
	cl_sym.AddSymbol(sys.GetSymbol(GetSymbol()));
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
	for(int i = 0; i < 2; i++) {
		d[i].posv.SetCount(max_martingale, 0);
		d[i].negv.SetCount(max_martingale, 0);
	}
	
	RefreshTicks();
}

void NetNN::RefreshTicks() {
	double point = cl_sym.GetDataBridge(0)->GetPoint();
	
	ConstBuffer& open = cl_sym.GetBuffer(0, 0, 0);
	if (tick_pos.IsEmpty())
		tick_pos.Add(0);
	double last_tick = open.Get(tick_pos.Top());
	for(int i = tick_counted; i < open.GetCount(); i++) {
		double cur = open.Get(i);
		int pips = (cur - last_tick) / point;
		if (pips >= +pip_step || pips <= -pip_step) {
			tick_pos.Add(i);
			signals.Add(pips > 0 ? false : true);
			last_tick = cur;
		}
	}
	
	tick_counted = open.GetCount();
}

void NetNN::InitNN(ConvNet::Brain& brain) {
	brain.Init(input_count, ACT_COUNT, NULL, 100000, 3000);
	brain.Reset();
	brain.MakeLayers(
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":" + IntStr(input_count) + "},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(ACT_COUNT) + "},\n"
		"\t{\"type\":\"sgd\", \"learning_rate\":0.001, \"momentum\":0.0, \"batch_size\":64, \"l2_decay\":0.01}\n"
		"]\n"
	);
	brain.Session::Reset();
}

void NetNN::Iterate(ConvNet::Brain& brain, bool is_realtime, Vector<double>& out) {
	Data& d = this->d[is_realtime];
	
	
	// Forward
	d.sensors.SetCount(input_count, 0);
	
	for(int i = 0; i < input_length; i++) {
		int pos = max(0, d.iter_pos - 1 - i);
		bool sig = signals[pos];
		d.sensors[i] = sig * 1.0;
	}
	
	for(int i = 0; i < max_martingale; i++) {
		d.sensors[input_length + i] = d.posv[i];
		d.sensors[input_length + max_martingale + i] = d.negv[i];
	}
	
	d.sensors[d.sensors.GetCount() - 1] = d.collected_reward > 0 ? 1.0 : 0.0;
	
	int action = brain.Forward(d.sensors);
	
	
	
	// Backward
	double reward = 0;
	
	double point = cl_sym.GetDataBridge(0)->GetPoint();
	ConstBuffer& buf = cl_sym.GetBuffer(0, 0, 0);
	double close = buf.Get(tick_pos[d.iter_pos+1]);
	double open = buf.Get(tick_pos[d.iter_pos]);
	if (action < ACT_COLLECT) {
		if (action == ACT_DOUBLE_BUY)
			open += spread_pips * point;
		else
			close += spread_pips * point;
	}
	int pips = (close - open) / point;
	if (action == ACT_DOUBLE_SELL)
		pips *= -1;
	
	
	if (d.doublelen >= max_martingale)
		action = ACT_COLLECT;
	switch (action) {
		
	case ACT_DOUBLE_BUY:
	case ACT_DOUBLE_SELL:
		d.collected_reward += pips * d.multiplier;
		if (pips < 0) {
			d.negv[d.doublelen] = 1.0;
		} else {
			d.posv[d.doublelen] = 1.0;
		}
		d.multiplier *= 2;
		d.doublelen++;
		break;
		
	case ACT_WAIT:
		d.collected_reward += pips * d.multiplier;
		if (pips < 0) {
			d.negv[d.doublelen] = 1.0;
		} else {
			d.posv[d.doublelen] = 1.0;
		}
		break;
		
	case ACT_COLLECT:
		d.multiplier = 1;
		reward = d.collected_reward;
		d.equity += d.collected_reward;
		d.collected_reward = 0;
		d.doublelen = 0;
		for(int i = 0; i < d.posv.GetCount(); i++) {
			d.posv[i] = 0;
			d.negv[i] = 0;
		}
		break;
		
	}
	
	
	// pass to brain for learning
	if (!IsFin(reward)) reward = 0;
	reward *= 0.1;
	brain.Backward(reward);
	
	
	if (action < ACT_COLLECT) {
		int data_count = is_realtime ? tick_pos.GetCount() -1 : tick_pos.GetCount() * 0.5;
		out.SetCount(data_count, 0);
		out[d.iter_pos] = d.equity;
		d.iter_pos++;
		if (d.iter_pos >= data_count) {
			d.iter_pos = 0;
			d.doublelen = 0;
			d.collected_reward = 0;
			d.multiplier = 1;
			d.equity = 0;
			for(int i = 0; i < d.posv.GetCount(); i++) {
				d.posv[i] = 0;
				d.negv[i] = 0;
			}
		}
		
		if (is_realtime && d.iter_pos % 100 == 0)
			WhenValueAdd();
	}
}

void NetNN::Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	
	if (pos >= cl_net.GetBuffer(0, 0, 0).GetCount())
		cl_net.Refresh();
	
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
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume& result = net.Forward(vol);
	
	System::NetSetting& n = sys.GetNet(0);
	
	output.SetCount(n.symbols.GetCount());
	for(int i = 0; i < output.GetCount(); i++)
		output[i] = 0;
	
	for(int i = 0; i < sym_count; i++) {
		System::NetSetting& n = sys.GetNet(i);
		
		double pred = result.Get(i);
		int sig = pred > 0 ? +1 : -1;
		
		for(int j = 0; j < n.symbols.GetCount(); j++) {
			output[j] += sig * n.symbols[j] * 0.01;
		}
	}
}

void NetNN::FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted) {
	
}
















void CombineNN::Init() {
	is_trainable = false;
	
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

void CombineNN::InitNN(ConvNet::Brain& brain) {
	
}

void CombineNN::Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output) {
	
}

void CombineNN::FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted) {
	
}

}
