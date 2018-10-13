#include "AsmProto.h"

Net::Net() {
	
}

void Net::Clear() {
	
}

String Net::ToString(int indent) {
	String s;
	s.Cat('\t', indent);
	s << "net " + IntStr(sub.GetCount()) + " " + IntStr(layers.GetCount()) + "\n";
	for(int i = 0; i < sub.GetCount(); i++) {
		s << sub[i].ToString(indent + 1);
	}
	for(int i = 0; i < layers.GetCount(); i++) {
		s << layers[i].ToString(indent + 1);
	}
	return s;
}

void Net::Parse(const Conf& conf) {
	if (conf.type == CONF_LINE) {
		for(int i = 0; i < conf.sub.GetCount(); i++) {
			const Conf& c = conf.sub[i];
			if (c.type == CONF_GROUP) {
				if (i > sub.GetCount())
					throw Exc("Sub-nets must be first to be parsed");
				for(int i = 0; i < c.arg_i; i++) {
					sub.Add().SetId(i).Parse(c.sub[0]);
				}
			}
			else if (c.type == CONF_ITEM) {
				Parse(c);
			}
			else throw Exc("Unexpected type " + IntStr(c.type));
		}
	}
	else if (conf.type == CONF_ITEM) {
		if (conf.arg_s == "volat") {
			AddVolatilityLayer();
		}
		else if (conf.arg_s == "polar") {
			AddPolarityLayer();
		}
		else if (conf.arg_s == "correlation") {
			if (conf.args.Find("depth") == -1) throw Exc("No depth argument for correlation layer");
			Value depth = conf.args.Get("depth");
			Value ratio = conf.args.Get("ratio");
			AddCorrelationLayer(depth, ratio);
		}
		else if (conf.arg_s == "mux") {
			if (conf.args.Find("depth") == -1) throw Exc("No depth argument for mux layer");
			Value depth = conf.args.Get("depth");
			AddMuxer(depth);
		}
		else throw Exc("Unexpected layer type " + conf.arg_s);
	}
	else if (conf.type == CONF_GROUP) {
		throw Exc("Throw unexpected group");
	}
	else throw Exc("Unexpected root type " + IntStr(conf.type));
}

LayerBase& Net::AddVolatilityLayer() {
	LayerBase& lb = layers.Add();
	lb.net = this;
	lb.type = TYPE_VOLAT;
	
	return lb;
}

LayerBase& Net::AddPolarityLayer() {
	LayerBase& lb = layers.Add();
	lb.net = this;
	lb.type = TYPE_POLAR;
	
	return lb;
}

LayerBase& Net::AddCorrelationLayer(int depth, double ratio) {
	LayerBase& lb = layers.Add();
	lb.net = this;
	lb.type = TYPE_CORR;
	lb.depth = depth;
	lb.ratio = ratio;
	return lb;
}

LayerBase& Net::AddMuxer(int depth) {
	LayerBase& lb = layers.Add();
	lb.net = this;
	lb.type = TYPE_MUX;
	lb.depth = depth;
	return lb;
}

void Net::Backward(IOData& data) {
	IOData* d = &data;
	for(int i = layers.GetCount()-1; i >= 0; i--) {
		LayerBase& lb = layers[i];
		lb.fwd_data = d;
		lb.Backward(*d);
		d = &lb.bwd_data;
	}
	for(int j = sub.GetCount()-1; j >= 0; j--) {
		sub[j].Backward(*d);
	}
}

int Net::Forward(bool learn) {
	int iter = 0;
	int r = Forward(learn, iter, training_iter);
	if (r == TRAINING_FINISHED) {
		training_iter++;
	}
	return r;
}

int Net::Forward(bool learn, int& iter, int train_iter) {
	for(int i = 0; i < sub.GetCount(); i++) {
		int r = sub[i].Forward(learn, iter, train_iter);
		if (learn && iter >= train_iter && (r == TRAINED || r == TRAINING_FINISHED))
			return r;
	}
	for(int i = 0; i < layers.GetCount(); i++) {
		int r = layers[i].Forward(learn && iter == train_iter);
		if (learn && iter >= train_iter)
			return r;
		iter++;
	}
	return RAN;
}
