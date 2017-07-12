#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>

class AgentTabCtrl : public TabCtrl {
	SnapshotCtrl			snapctrl;
	AgentCtrl				agentctrl;
	AgentTraining			trainingctrl;
	RealtimeNetworkCtrl		rtnetctrl;
	
public:
	typedef AgentTabCtrl CLASSNAME;
	AgentTabCtrl(Agent& agent, RealtimeSession& rtses);
	
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, glist;
	ParentCtrl						ctrl;
	ParentCtrl						mainview;
	WithNewAgentGroup<ParentCtrl>	newview;
	WithConfigure<ParentCtrl>		confview;
	Button							add_new, configure;
	
	Array<AgentTabCtrl>				agent_tabs;
	
	System* sys;
	int view;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl(System& sys);
	
	void Data();
	void SetView(int i);
	void NewAgent();
};

}

#endif
