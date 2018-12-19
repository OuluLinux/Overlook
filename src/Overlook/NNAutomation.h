#ifndef _Overlook_EventAutomation_h_
#define _Overlook_EventAutomation_h_


#if 0
namespace Overlook {
using namespace libmt;


class EventAutomation : public Common {
	
protected:
	friend class EventAutomationCtrl;
	
	static const int sym_count = 10;
	static const int input_length = 10;
	static const int input_count = sym_count * input_length;
	static const int output_count = sym_count*2+1;
	
	
	// Temporary
	Vector<Ptr<EventCoreItem> > ci_queue;
	Vector<double> points;
	CoreList cl_net, cl_sym;
	
	
	
	
	void SetRealSymbolLots(int sym_, double lots);
	
public:
	typedef EventAutomation CLASSNAME;
	EventAutomation();
	
	virtual void Init();
	virtual void Start();
	void Process();
	
};

inline EventAutomation& GetEventAutomation() {return GetSystem().GetCommon<EventAutomation>();}

class EventAutomationCtrl : public CommonCtrl {
	
	struct EventBufCtrl : public Ctrl {
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
	Array<EventBufCtrl> draws;
	Vector<ConvNet::Session*> ses_list;
	TabCtrl* prev_tabs = NULL;
	bool init = true;
	bool is_initing = false;
	
public:
	typedef EventAutomationCtrl CLASSNAME;
	EventAutomationCtrl();
	
	virtual void Data();
	
	void SetItem();
};


}

#endif
#endif
