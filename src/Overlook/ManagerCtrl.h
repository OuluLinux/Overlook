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
	void SetEpsilon() {if (group) group->SetEpsilon(epsilon.GetData());}
	void SetMode();
	
	void Data();
	void PostProgress(int actual, int total, String label) {PostCallback(THISBACK3(Progress, actual, total, label));}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	
};

class GroupTabCtrl : public TabCtrl {
	AgentGroup*				group;
	GroupOverview			overview;
	SnapshotCtrl			snapctrl;
	TrainingCtrl			trainingctrl;
	
public:
	typedef GroupTabCtrl CLASSNAME;
	GroupTabCtrl();
	
	void Data();
	void SetGroup(AgentGroup& group);
	void SetEnabled();
	void ResetGroupOptimizer();
	
};

class AgentTabCtrl : public TabCtrl {
	Agent* agent;
	WithAgentOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
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
	void Select(int i);
	void Data();
	void SetView(int i);
	void NewAgent();
	void PostNewAgent() {Thread::Start(THISBACK(NewAgent));}
	void LastCursor() {glist.SetCursor(glist.GetCount()-1);}
};

class RealtimeCtrl : public ParentCtrl {
	Splitter				hsplit;
	BrokerCtrl				brokerctrl;
	ArrayCtrl				journal;
	System*					sys;
	
public:
	typedef RealtimeCtrl CLASSNAME;
	RealtimeCtrl(System& sys);
	
	void Data();
	void Init();
	void PostData() {PostCallback(THISBACK(Data));}
	
	void AddMessage(String time, String level, String msg);
	void Info(String msg);
	void Error(String msg);
};

}

#endif
