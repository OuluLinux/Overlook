#ifndef _Overlook_SystemCtrl_h_
#define _Overlook_SystemCtrl_h_


namespace Overlook {
using namespace Upp;


struct BooleansDraw : public Ctrl {
	virtual void Paint(Draw& w);
	
	int cursor = 0;
	int tf = 0;
};

class SystemCtrl : public ParentCtrl {
	ArrayCtrl symbols;
	ParentCtrl symctrl_place;
	Splitter hsplit;
	Button toggle;
	
	TabCtrl tabs;
	ParentCtrl boolctrl;
	BooleansDraw bools;
	ArrayCtrl stats, strands;
	SliderCtrl slider;
	
public:
	typedef SystemCtrl CLASSNAME;
	SystemCtrl();
	~SystemCtrl() {}
	
	void Data();
	void Start() {toggle.SetLabel("Stop"); toggle.WhenAction = THISBACK(Stop); GetSystem().StartJobs();}
	void Stop() {toggle.SetLabel("Start"); toggle.WhenAction = THISBACK(Start); GetSystem().StopJobs();}
};


}


#endif
