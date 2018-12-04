#ifndef _Overlook_NNAutomation_h_
#define _Overlook_NNAutomation_h_


namespace Overlook {
using namespace libmt;


class NNAutomation : public Common {
	
protected:
	friend class NNAutomationCtrl;
	
	struct SymStat : Moveable<SymStat> {
		double open = 0;
		double lots = 0;
		int type = 0;
	};
	
	// Persistent
	ConvNet::Session ses;
	
	// Temporary
	Vector<SymStat> symstats;
	Vector<double> points;
	Vector<double> slist;
	CoreList cl_net, cl_sym;
	
	
	static const int sym_count = 10;
	static const int input_length = 10;
	static const int input_count = sym_count * input_length;
	static const int output_count = sym_count*2+1;
	
	
	void SetRealSymbolLots(int sym_, double lots);
	void LoadThis() {LoadFromFile(*this, ConfigFile("NNAutomation.bin"));}
	void StoreThis() {StoreToFile(*this, ConfigFile("NNAutomation.bin"));}
	
public:
	typedef NNAutomation CLASSNAME;
	NNAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
	void Serialize(Stream& s) {s % ses;}
};

inline NNAutomation& GetNNAutomation() {return GetSystem().GetCommon<NNAutomation>();}

class NNAutomationCtrl : public CommonCtrl {
	
	TabCtrl tabs;
	ConvNet::SessionConvLayers ses_view;
	ConvNet::TrainingGraph train_view;
	Upp::Label status;
	bool init = true;
	
public:
	typedef NNAutomationCtrl CLASSNAME;
	NNAutomationCtrl();
	
	virtual void Data();
};


}

#endif
