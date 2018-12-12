#ifndef _Overlook_NNAutomation_h_
#define _Overlook_NNAutomation_h_


namespace Overlook {
using namespace libmt;


class NNAutomation : public Common {
	
protected:
	friend class NNAutomationCtrl;
	
	static const int sym_count = 10;
	static const int input_length = 10;
	static const int input_count = sym_count * input_length;
	static const int output_count = sym_count*2+1;
	
	
	// Temporary
	Vector<Ptr<NNCoreItem> > ci_queue;
	Vector<double> points;
	CoreList cl_net, cl_sym;
	
	
	
	
	void SetRealSymbolLots(int sym_, double lots);
	
public:
	typedef NNAutomation CLASSNAME;
	NNAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
};

inline NNAutomation& GetNNAutomation() {return GetSystem().GetCommon<NNAutomation>();}

class NNAutomationCtrl : public CommonCtrl {
	
	struct NNBufCtrl : public Ctrl {
		Vector<double>* buf;
		Vector<Point> cache;
		virtual void Paint(Draw& d) {
			d.DrawRect(GetSize(), White());
			DrawVectorPolyline(d, GetSize(), *buf, cache);
		}
	};
	
	Splitter hsplit;
	ArrayCtrl queuelist;
	ParentCtrl itemctrl;
	Array<TabCtrl> tabslist;
	Array<ConvNet::SessionConvLayers> ses_view;
	Array<ConvNet::TrainingGraph> train_view;
	Array<Upp::Label> status;
	Array<NNBufCtrl> draws;
	Vector<ConvNet::Session*> ses_list;
	TabCtrl* prev_tabs = NULL;
	bool init = true;
	bool is_initing = false;
	
public:
	typedef NNAutomationCtrl CLASSNAME;
	NNAutomationCtrl();
	
	virtual void Data();
	
	void SetItem();
};


}

#endif
