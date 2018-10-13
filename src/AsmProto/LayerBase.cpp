#include "AsmProto.h"

#define COPYVECTOR(src, dst) {dst.SetCount(src.GetCount()); for(int i=0; i < dst.GetCount();i++) dst[i] = src[i];}

LayerBase::LayerBase() {
	
}

String LayerBase::ToString(int indent) {
	String s;
	s.Cat('\t', indent);
	s.Cat("layer " + GetTypeString() + "\n");
	return s;
}

String LayerBase::GetTypeString() {
	switch (type) {
		case TYPE_VOLAT: return "volat";
		case TYPE_POLAR: return "polar";
		case TYPE_CORR: return "correlation";
		case TYPE_MUX: return "data muxer";
	}
	return "";
}

void LayerBase::Backward(IOData& data) {
	switch (type) {
		case TYPE_VOLAT: BackwardVolatility(data); break;
		case TYPE_POLAR: BackwardPolarity(data); break;
		case TYPE_CORR: BackwardCorrelation(data); break;
		case TYPE_MUX: BackwardMux(data); break;
	}
}

int LayerBase::Forward(bool learn) {
	switch (type) {
		case TYPE_VOLAT: return ForwardVolatility(learn); break;
		case TYPE_POLAR: return ForwardPolarity(learn); break;
		case TYPE_CORR: return ForwardCorrelation(learn); break;
		case TYPE_MUX: return ForwardMux(learn); break;
	}
	return -1;
}

void LayerBase::BackwardVolatility(IOData& data) {
	
	COPYVECTOR(data.in, dqn_data.in);
	COPYVECTOR(data.out, dqn_data.out);
	
}

void LayerBase::BackwardPolarity(IOData& data) {
	
	bwd_data.SetCount(data);
	dqn_data.SetCount(data);
	
	for (int v = 0; v < 2; v++) {
		for(int i = 0; i < data[v].GetCount(); i++) {
			double d = data[v][i];
			bwd_data[v][i] = fabs(d);
			dqn_data[v][i] = d >= 0 ? +1 : -1;
		}
	}
	
}

void LayerBase::BackwardCorrelation(IOData& data) {
	int in_size = data.in.GetCount() / depth;
	int out_size = data.out.GetCount() / depth;
	if (data.in.GetCount() % depth != 0)
		throw Exc("Invalid input");
	if (data.out.GetCount() % depth != 0)
		throw Exc("Invalid output");
	
	bwd_data.SetCount((in_size - 1) * depth, out_size * depth);
	tmp.SetCount(depth, depth);
	
	double a = ratio;
	double b = 1.0 - a;
	
	for(int i = 1; i < in_size; i++) {
		double sum = 0.0;
		for(int j = 0; j < depth; j++) {
			int k0 = j * in_size + i;
			int k1 = k0 - 1;
			double d0 = data.in[k0];
			double d1 = data.in[k1];
			double diff = d0 - d1;
			sum += diff;
			tmp.in[j] = diff;
		}
		sum /= depth;
		for(int j = 0; j < depth; j++) {
			double diff = tmp.in[j];
			double val = (diff - a * sum * 1.0) / b;
			bwd_data.in[j * (in_size-1) + (i-1)] = val;
		}
	}
	
	for(int i = 0; i < out_size; i++) {
		double sum = 0.0;
		for(int j = 0; j < depth; j++) {
			int k0 = j * out_size + i;
			int k1 = k0 - 1;
			double d0 = data.out[k0];
			double d1 = i == 0 ? data.in[j * in_size + in_size-1] : data.in[k1];
			double diff = d0 - d1;
			sum += diff;
			tmp.out[j] = diff;
		}
		sum /= depth;
		for(int j = 0; j < depth; j++) {
			double diff = tmp.out[j];
			double val = (diff - a * sum * 1.0) / b;
			bwd_data.out[j * (out_size-1) + i] = val;
		}
	}
}

void LayerBase::BackwardMux(IOData& data) {
	int in_size = data.in.GetCount() / depth;
	int out_size = data.out.GetCount() / depth;
	if (data.in.GetCount() % depth != 0)
		throw Exc("Invalid input");
	if (data.out.GetCount() % depth != 0)
		throw Exc("Invalid output");
	
	bwd_data.SetCount(in_size, out_size);
	
	int it = net->GetId() * in_size;
	for(int i = 0; i < in_size; i++, it++)
		bwd_data.in[i] = data.in[it];
	
	it = net->GetId() * out_size;
	for(int i = 0; i < out_size; i++, it++)
		bwd_data.out[i] = data.out[it];
	
}

int  LayerBase::ForwardVolatility(bool learn) {
	IOData& fwd_data = *this->fwd_data;
	
	int in_win_size = fwd_data.in.GetCount();
	int out_win_size = fwd_data.out.GetCount();
	if (out_win_size != 1) throw Exc("Unexpected output size");
	
	if (learn) {
		
		if (iter == 0) {
			String t =
				"[\n"
				"\t{\"type\":\"input\", \"input_width\":1, \"input_height\":1, \"input_depth\":" + IntStr(in_win_size) + "},\n"
				"\t{\"type\":\"fc\", \"neuron_count\":50},\n"
				"\t{\"type\":\"fc\", \"neuron_count\":50},\n"
				"\t{\"type\":\"regression\", \"neuron_count\":1},\n"
				"\t{\"type\":\"adadelta\", \"learning_rate\":0.01, \"momentum\":0, \"batch_size\":1, \"l2_decay\":0.001}\n"
				"]\n";
			LOG(t);
			ses.MakeLayers(t);
		}
		
		for(int i = 0; i < fwd_data.in.GetCount(); i++)
			fwd_data.in[i] *= 10000;
		fwd_data.out[0] *= 10000;
		
		ConvNet::Volume x;
		x.Init(1, 1, in_win_size, fwd_data.in);
		
		ses.GetTrainer().Train(x, fwd_data.out);
		
		ConvNet::Volume& y = ses.GetNetwork().Forward(x);
		
		double d = y.Get(0);
		d *= 10000;
		//LOG("train " << d);
		
		iter++;
		
		if (iter >= DQN_ITERS)
			return TRAINING_FINISHED;
		
		return TRAINED;
	}
	else {
		for(int i = 0; i < fwd_data.in.GetCount(); i++)
			fwd_data.in[i] *= 10000;
		
		ConvNet::Volume x;
		x.Init(1, 1, in_win_size, fwd_data.in);
		
		ConvNet::Volume& y = ses.GetNetwork().Forward(x);
		
		double d = y.Get(0);
		d /= 10000;
		fwd_data.out[0] = d;
		//LOG("eval " << d);
		
		return RAN;
	}
}

int  LayerBase::ForwardPolarity(bool learn) {
	IOData& fwd_data = *this->fwd_data;
	
	int in_win_size = dqn_data.in.GetCount();
	int out_win_size = dqn_data.out.GetCount();
	if (out_win_size != 1) throw Exc("Unexpected output size");
	
	if (learn) {
		
		if (iter == 0) {
			dqn.Init(1, in_win_size, 2);
			dqn.Reset();
			dqn.SetGamma(0);
		}
		
		int a = dqn.Act(dqn_data.in);
		int correct = dqn_data.out[0] < 0.0;
		dqn.Learn(a == correct ? +1.0 : -1.0);
		iter++;
		
		if (iter >= DQN_ITERS)
			return TRAINING_FINISHED;
		
		return TRAINED;
	}
	else {
		int a = dqn.Act(dqn_data.in);
		
		double volat = bwd_data.out[0];
		double value = volat * (a > 0 ? -1 : +1);
		
		fwd_data.out[0] = value;
		
		return RAN;
	}
}

int  LayerBase::ForwardCorrelation(bool learn) {
	
	if (learn)
		return TRAINING_FINISHED;
	return RAN;
}

int  LayerBase::ForwardMux(bool learn) {
	IOData& fwd_data = *this->fwd_data;
	
	int in_size = fwd_data.in.GetCount() / depth;
	int out_size = fwd_data.out.GetCount() / depth;
	if (fwd_data.in.GetCount() % depth != 0)
		throw Exc("Invalid input");
	if (fwd_data.out.GetCount() % depth != 0)
		throw Exc("Invalid output");
	
	for(int v = 0; v < 2; v++) {
		int win_size = (v == 0 ? in_size : out_size);
		int it = net->GetId() * win_size;
		for(int i = 0; i < win_size; i++, it++) {
			double d = bwd_data[v][i];
			double orig = fwd_data[v][it];
			if (v == 1) {
				LOG(d << " vs " << orig);
				//Cout() << d << " vs " << orig << "\n";
				/*bool a = d < 0;
				bool b = orig < 0;
				if (a == b) {LOG("SIGN CORRECT");}*/
			}
			fwd_data[v][it] = d;
		}
	}
	
	if (learn)
		return TRAINING_FINISHED;
	return RAN;
}


