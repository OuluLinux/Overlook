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

void NetNN::InitNN(ConvNet::Session& ses) {
	ses.MakeLayers(
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
	ses.Reset();
}

void NetNN::Sample(ConvNet::Session& ses, bool is_realtime) {
	
	int data_count = cl_sym.GetBuffer(0, 0, 0).GetCount();
	
	if (!is_realtime)
		data_count *= 0.5;
			
	cl_net.Refresh();
	
	ConvNet::SessionData& d = ses.Data();
	d.BeginDataResult(sym_count, data_count-input_length-2, sym_count * input_length);
	
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
				d.SetData(row, col++, ch);
				next = cur;
			}
			double ch = next2 / next - 1.0;
			ch *= 100;
			d.SetResult(row, j, ch);
		}
	}
	d.EndData();
	
}

void NetNN::Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output) {
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

void NetNN::FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted) {
	System& sys = GetSystem();
	
	cl_sym.Refresh();
	
	Vector<double> output;
	
	for(int i = counted; i < buf.GetCount(); i++) {
		double value;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = buf[i-1];
			Start(ses, is_realtime, i-1, output);
			
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
				if (i >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(i);
				double prev = sym_buf.Get(i-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				ASSERT(IsFin(mult));
				value += mult;
			}
		}
		
		ASSERT(value >= 0);
		buf[i] = value;
	}
}








void MultiTfNetNN::Init() {
	System& sys = GetSystem();
	
	int tf = GetTf();
	ASSERT(tf == 4);
	
	System::NetSetting& net = sys.GetNet(0);
	for(int i = 0; i < net.symbols.GetCount(); i++) {
		cl_sym.AddSymbol(net.symbols.GetKey(i));
	}
	cl_sym.AddTf(tf);
	cl_sym.AddIndi(0);
	cl_sym.Init();
	cl_sym.Refresh();
	
}

void MultiTfNetNN::InitNN(ConvNet::Session& ses) {
	
}

void MultiTfNetNN::Sample(ConvNet::Session& ses, bool is_realtime) {
	
}

void MultiTfNetNN::Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output) {
	System& sys = GetSystem();
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	
	System::NetSetting& net = sys.GetNet(0);
	
	
	output.SetCount(net.symbols.GetCount());
	for(int i = 0; i < output.GetCount(); i++)
		output[i] = 0;
	
	Time t = Time(1970,1,1) + cl_sym.GetBuffer(0, 0, 4).Get(pos);
	
	for(int i = 0; i < 3; i++) {
		int tf = 4 + i;
		const Index<Time>& idx = dbc.GetTimeIndex(tf);
		
		Time tf_time = t;
		switch (tf) {
			case 4: break;
			case 5: tf_time.hour -= tf_time.hour % 4; break;
			case 6: tf_time.hour = 0; break;
			default: Panic("Invalid tf");
		}
		
		int tf_pos = idx.Find(tf_time);
		if (tf_pos == -1) continue;
		
		NNCore& c = GetInputCore(i);
		
		c.Start(is_realtime, tf_pos, tmp);
		
		for(int j = 0; j < output.GetCount(); j++)
			output[j] += tmp[j];
	}
}

void MultiTfNetNN::FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted) {
	System& sys = GetSystem();
	
	cl_sym.Refresh();
	
	Vector<double> output;
	
	for(int i = counted; i < buf.GetCount(); i++) {
		double value;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = buf[i-1];
			Start(ses, is_realtime, i-1, output);
			
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
				if (i >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(i);
				double prev = sym_buf.Get(i-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				ASSERT(IsFin(mult));
				value += mult;
			}
		}
		
		ASSERT(value >= 0);
		buf[i] = value;
	}
}








void IntPerfNN::Init() {
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

void IntPerfNN::InitNN(ConvNet::Session& ses) {
	ses.MakeLayers(
		"[\n"
		"\t{\"type\":\"input\", \"input_width\":" + IntStr(input_length) + ", \"input_height\":1, \"input_depth\":1},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"fc\", \"neuron_count\":50, \"activation\": \"tanh\"},\n"
		"\t{\"type\":\"softmax\", \"class_count\":3},\n"
		"\t{\"type\":\"adadelta\", \"batch_size\":20, \"l2_decay\":0.001}\n"
		"]\n"
	);
	ses.Reset();
}

void IntPerfNN::Sample(ConvNet::Session& ses, bool is_realtime) {
	NNCore& c = GetInputCore(0);
	const Vector<double>& buf = c.GetBuffer(false); // not realtime
	
	int data_begin = buf.GetCount() * 0.5;
	int data_end = is_realtime ? buf.GetCount() - 1 : buf.GetCount() * 0.75;
	int data_count = data_end - data_begin;
	
	ConvNet::SessionData& d = ses.Data();
	d.BeginData(3, data_count, input_length);
	
	SimpleDistribution sd;
	for(int i = data_begin; i < data_end; i++) {
		double next = buf[i];
		double next2 = buf[i+1];
		double ch = next2 / next - 1.0;
		if (IsFin(ch))
			sd.Add(ch);
	}
	sd.Finish();
	double center_low = sd.Limit(0.333);
	double center_high = sd.Limit(0.667);
	ASSERT(center_low < center_high);
	
	for(int i = data_begin, row = 0; i < data_end; i++, row++) {
		double next = buf[i];
		double next2 = buf[i+1];
		for(int k = 0; k < input_length; k++) {
			double cur = buf[i-k-1];
			double ch = next / cur - 1.0;
			ch *= 100000;
			if (!IsFin(ch)) ch = 0;
			d.SetData(row, k, ch);
			next = cur;
		}
		double ch = next2 / next - 1.0;
		int lbl;
		if (ch <= center_low)
			lbl = LBL_NEG;
		else if (ch <= center_high)
			lbl = LBL_MID;
		else
			lbl = LBL_POS;
		d.SetLabel(row, lbl);
	}
	d.EndData();
}

void IntPerfNN::Start(ConvNet::Session& ses, bool is_realtime, int pos, Vector<double>& output) {
	NNCore& c = GetInputCore(0);
	c.Start(is_realtime, pos, output);
	
	const Vector<double>& buf = c.GetBuffer(is_realtime);
	
	
	ConvNet::Volume vol;
	vol.Init(input_length, 1, 1, 0);
	
	double next = buf[pos];
	for(int i = 0; i < input_length; i++) {
		double cur = buf[max(0, pos-i-1)];
		double ch = next / cur - 1.0;
		ch *= 100;
		if (!IsFin(ch)) ch = 0;
		vol.Set(i, ch);
		next = cur;
	}
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume& fwd = net.Forward(vol);
	int lbl = fwd.GetMaxColumn();
	
	if (pos >= op_hist.GetCount())
		op_hist.SetCount(pos+1);
	
	bool prev_op = pos > 0 ? op_hist.Get(pos-1) : false;
	
	bool op;
	switch (lbl) {
		case LBL_NEG: op = OP_SELL; break;
		case LBL_MID: op = prev_op; break;
		case LBL_POS: op = OP_BUY; break;
	}
	
	op_hist.Set(pos, op);
	
	if (op != OP_BUY) {
		for(int i = 0; i < output.GetCount(); i++)
			output[i] *= -1.0;
	}
	
}

void IntPerfNN::FillVector(ConvNet::Session& ses, bool is_realtime, Vector<double>& buf, int counted) {
	System& sys = GetSystem();
	
	cl_sym.Refresh();
	
	Vector<double> output;
	
	for(int i = counted; i < buf.GetCount(); i++) {
		double value;
		
		if (i == 0)
			value = 1.0;
		else {
			double prev_value = buf[i-1];
			Start(ses, is_realtime, i-1, output);
			
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
				if (i >= sym_buf.GetCount())
					continue;
				double cur = sym_buf.Get(i);
				double prev = sym_buf.Get(i-1);
				double mult = cur / prev - 1.0;
				mult *= output[j] * 0.1;
				ASSERT(IsFin(mult));
				value += mult;
			}
		}
		
		ASSERT(value >= 0);
		buf[i] = value;
	}
}

}
