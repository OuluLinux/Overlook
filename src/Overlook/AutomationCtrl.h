#ifndef _Overlook_AutomationCtrl_h_
#define _Overlook_AutomationCtrl_h_

namespace Overlook {
using namespace Upp;


class AutomationCtrl : public ParentCtrl {
	ArrayCtrl symbols;
	ParentCtrl symctrl_place;
	Splitter hsplit;
	Button toggle;
	
	TabCtrl tabs;
	
	ParentCtrl evolvectrl;
	Array<ProgressIndicator> evolveprog;
	Splitter evolvesplit;
	
public:
	typedef AutomationCtrl CLASSNAME;
	AutomationCtrl();
	~AutomationCtrl() {}
	
	void Data();
	void Start() {toggle.SetLabel("Stop"); toggle.WhenAction = THISBACK(Stop); GetAutomation().StartJobs();}
	void Stop() {toggle.SetLabel("Start"); toggle.WhenAction = THISBACK(Start); GetAutomation().StopJobs();}
};


}

#endif
