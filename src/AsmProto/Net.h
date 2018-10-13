#ifndef _AsmProto_Net_h_
#define _AsmProto_Net_h_

enum {RAN, TRAINED, TRAINING_FINISHED, ALL_LAYERS_TRAINED};

class Net {
	Array<LayerBase> layers;
	Array<Net> sub;
	int id = 0;
	int training_iter = 0;
	
public:
	typedef Net CLASSNAME;
	Net();
	
	
	
	void Parse(const Conf& c);
	void Clear();
	Net& SetId(int i) {id = i; return *this;}
	String ToString(int indent=0);
	void Backward(IOData& d);
	int Forward(bool learn);
	int Forward(bool learn, int& iter, int train_iter);
	
	int GetId() const {return id;}
	
	Net& AddSubNet() {return sub.Add();}
	//LayerBase& AddLayer() {return layers.Add();}
	LayerBase&	AddVolatilityLayer();
	LayerBase&	AddPolarityLayer();
	LayerBase&	AddCorrelationLayer(int depth, double ratio);
	LayerBase&	AddMuxer(int depth);
};
#endif
