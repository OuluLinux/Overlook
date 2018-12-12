#include "Overlook.h"

namespace Overlook {

void MainNN::Init() {
	System& sys = GetSystem();
	
	int tf = GetTf();
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++)
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	ASSERT(net.symbols.GetCount() == sym_count);
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
}

void MainNN::InitNN(ConvNet::Brain& brain) {
	int input_count = sym_count * input_length * 2;
	
	brain.Init(input_count, sym_count*2, NULL, 100000, 3000);
	brain.Reset();
	brain.MakeLayers(
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":" + IntStr(sym_count) + ", \"input_height\":" + IntStr(input_length) + ", \"input_depth\":2},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\": 50, \"activation\":\"relu\"},\n"
		"\t{\"type\":\"regression\", \"neuron_count\":" + IntStr(sym_count*2) + "},\n"
		"\t{\"type\":\"sgd\", \"learning_rate\":0.001, \"momentum\":0.0, \"batch_size\":64, \"l2_decay\":0.01}\n"
		"]\n"
	);
	brain.Session::Reset();
}

void MainNN::Iterate(ConvNet::Brain& brain, bool is_realtime, Vector<double>& out) {
	Data& d = this->d[is_realtime];
	
	
	// Forward
	if (d.vol.GetCount() == 0)
		d.vol.Init(sym_count, input_length, 2);
	
	for(int i = 0; i < sym_count; i++) {
		ConstBuffer& buf = cl_sym.GetBuffer(i, 0, 0);
		double next = buf.Get(d.iter_pos);
		for(int j = 0; j < input_length; j++) {
			double cur = buf.Get(max(0, d.iter_pos - (j+1) * 240));
			double ch = cur / next - 1;
			ch *= 100;
			d.vol.Set(i, j, 0, ch);
			next = cur;
		}
		
		double point = cl_sym.GetDataBridge(i)->GetPoint();
		int pos = d.iter_pos - 1;
		int input_pos = 0;
		next = buf.Get(d.iter_pos);
		while (pos >= 0 && input_pos < input_length) {
			double cur = buf.Get(pos);
			double pips = (next - cur) / point;
			if (pips >= +pip_step || pips <= -pip_step) {
				d.vol.Set(i, input_pos, 1, pips > 0 ? 0.0 : 1.0);
				input_pos++;
				next = cur;
			}
			pos--;
		}
	}
	
	
	int action = brain.Forward(d.vol.GetWeights());
	int sym = action % sym_count;
	int act = action / sym_count;
	
	// Backward
	double reward = 0;
	
	double point = cl_sym.GetDataBridge(sym)->GetPoint();
	ConstBuffer& buf = cl_sym.GetBuffer(sym, 0, 0);
	int data_count = is_realtime ? buf.GetCount() : buf.GetCount() * 0.5;
	out.SetCount(max(out.GetCount(), data_count), 0);
	double open = buf.Get(d.iter_pos);
	
	out[d.iter_pos] = d.equity;
	d.iter_pos++;
	int pips;
	while (d.iter_pos < data_count - 1) {
		double close = buf.Get(d.iter_pos);
		
		if (act == ACT_BUY)
			open += spread_pips * point;
		else
			close += spread_pips * point;
		pips = (close - open) / point;
		if (act != ACT_BUY)
			pips *= -1;
		
		if (pips >= +open_pip_step || pips <= -open_pip_step) {
			break;
		}
		
		out[d.iter_pos] = d.equity;
		d.iter_pos++;
	}
	
	reward = pips * 0.01;
	d.equity += pips;
	
	brain.Backward(reward);
	
	
	out[d.iter_pos] = d.equity;
	
	d.iter_pos++;
	if (d.iter_pos >= data_count) {
		d.iter_pos = 0;
		d.equity = 0;
	}
	
	if (is_realtime && d.iter_pos % 100 == 0)
		WhenValueAdd();
}

void MainNN::Start(ConvNet::Brain& brain, bool is_realtime, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	
	/*if (pos >= cl_net.GetBuffer(0, 0, 0).GetCount())
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
	}*/
}

void MainNN::FillVector(ConvNet::Brain& brain, bool is_realtime, Vector<double>& buf, int counted) {
	
}















}
