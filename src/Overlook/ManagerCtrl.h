#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>


class GroupOverview : public WithGroupOverview<ParentCtrl> {
	
protected:
	AgentGroup* group;
	String label;
	
	void Progress(int actual, int total, String label);
	void SubProgress(int actual, int total);
	
public:
	typedef GroupOverview CLASSNAME;
	GroupOverview();
	
	void SetGroup(AgentGroup& group);
	
	void Data();
	void PostProgress(int actual, int total, String label) {PostCallback(THISBACK3(Progress, actual, total, label));}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	
};

class GroupTabCtrl : public TabCtrl {
	AgentGroup*				group;
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
	void SetEnabled();
	void SetFreeMargin();
	
};

class AgentTabCtrl : public TabCtrl {
	Agent* agent;
	WithAgentOverview<ParentCtrl>	overview;
	AgentTraining					agent_view;
	
	
	
public:
	typedef AgentTabCtrl CLASSNAME;
	AgentTabCtrl();
	
	void Data();
	void SetAgent(Agent& agent);
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, glist;
	ParentCtrl						ctrl;
	ParentCtrl						mainview;
	WithNewAgentGroup<ParentCtrl>	newview;
	Button							add_new;
	Array<Option>					new_opts;
	
	GroupTabCtrl					group_tabs;
	AgentTabCtrl					agent_tabs;
	
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
