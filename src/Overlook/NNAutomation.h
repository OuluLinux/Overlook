#ifndef _Overlook_NNAutomation_h_
#define _Overlook_NNAutomation_h_


namespace Overlook {
using namespace libmt;


class NNAutomation : public Common {
	
protected:
	friend class NNAutomationCtrl;
	
	static const int tf_count = 3;
	static const int tf_begin = 4;
	static const int sym_count = 10;
	static const int input_length = 10;
	static const int input_count = sym_count * input_length;
	static const int output_count = sym_count*2+1;
	
	
	// Persistent
	ConvNet::Session ses[tf_count];
	
	// Temporary
	Vector<Ptr<NNCoreItem> > ci_queue;
	Vector<double> points;
	CoreList cl_net[tf_count], cl_sym[tf_count];
	
	
	
	
	void SetRealSymbolLots(int sym_, double lots);
	void LoadThis() {LoadFromFile(*this, ConfigFile("NNAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("NNAutomation.bin"));}
	
public:
	typedef NNAutomation CLASSNAME;
	NNAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
	void Serialize(Stream& s) {for(int i = 0; i < tf_count; i++) s % ses[i];}
};

inline NNAutomation& GetNNAutomation() {return GetSystem().GetCommon<NNAutomation>();}

class NNAutomationCtrl : public CommonCtrl {
	
	TabCtrl tabs;
	Array<ConvNet::SessionConvLayers> ses_view;
	Array<ConvNet::TrainingGraph> train_view;
	Array<Upp::Label> status;
	bool init = true;
	
public:
	typedef NNAutomationCtrl CLASSNAME;
	NNAutomationCtrl();
	
	virtual void Data();
};


}

#endif
