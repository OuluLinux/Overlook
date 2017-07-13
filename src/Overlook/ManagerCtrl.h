#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>


class GroupOverview : public ParentCtrl {
	
protected:
	AgentGroup* group;
	Label lbl;
	String label;
	ProgressIndicator prog, sub, subsub;
	int ret_value;
	
	void Progress(int actual, int total, String label);
	void SubProgress(int actual, int total);
	void SubSubProgress(int actual, int total);
	void Close0() {Close();}
	
public:
	typedef GroupOverview CLASSNAME;
	GroupOverview();
	
	void SetGroup(AgentGroup& group);
	
	void Data();
	bool IsFail() {return ret_value;}
	void PostProgress(int actual, int total, String label) {PostCallback(THISBACK3(Progress, actual, total, label));}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	void PostSubSubProgress(int actual, int total) {PostCallback(THISBACK2(SubSubProgress, actual, total));}
	void PostClose() {PostCallback(THISBACK(Close0));}
};

class GroupTabCtrl : public TabCtrl {
	GroupOverview			overview;
	SnapshotCtrl			snapctrl;
	AgentCtrl				agentctrl;
	AgentTraining			trainingctrl;
	RealtimeNetworkCtrl		rtnetctrl;
	
public:
	typedef GroupTabCtrl CLASSNAME;
	GroupTabCtrl();
	
	void Data();
	void SetGroup(AgentGroup& group);
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, glist;
	ParentCtrl						ctrl;
	ParentCtrl						mainview;
	WithNewAgentGroup<ParentCtrl>	newview;
	WithConfigure<ParentCtrl>		confview;
	MultiButton						buttons;
	Array<Option>					new_opts;
	
	GroupTabCtrl					group_tabs;
	AgentTraining					agent_view;
	
	System* sys;
	int view;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl(System& sys);
	
	void SelectAll();
	void SelectNone();
	void Data();
	void SetView(int i);
	void NewAgent();
};

}

#endif
