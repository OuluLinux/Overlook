#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>


class ManagerLoader : public WithLoader<TopWindow> {
	
public:
	typedef ManagerLoader CLASSNAME;
	ManagerLoader() {CtrlLayout(*this); Title("Agent group loader");}
	
	void Progress(String status, int actual, int total) {this->status.SetLabel(status); prog.Set(actual, total);}
	void PostProgress(String status, int actual, int total) {PostCallback(THISBACK3(Progress, status, actual, total));}
	
};

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
	TrainingCtrl			trainingctrl;
	DataCtrl				datactrl;
	Array<EditDoubleSpin>	tflimitedit;
	
public:
	typedef GroupTabCtrl CLASSNAME;
	GroupTabCtrl();
	
	void Data();
	void SetGroup(AgentGroup& group);
};

class AgentTabCtrl : public TabCtrl {
	//Agent* agent;
	WithAgentOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
public:
	typedef AgentTabCtrl CLASSNAME;
	AgentTabCtrl();
	
	void Data();
	//void SetAgent(Agent& agent);
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, tfglist, glist;
	ParentCtrl						mainview;
	Button							add_new;
	Array<Option>					new_opts;
	
	GroupTabCtrl					group_tabs;
	AgentTabCtrl					agent_tabs;
	
	int view;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl();
	
	void Data();
	void SetView(int i);
	
};

class RealtimeCtrl : public ParentCtrl {
	Splitter				hsplit;
	BrokerCtrl				brokerctrl;
	ArrayCtrl				journal;
	
public:
	typedef RealtimeCtrl CLASSNAME;
	RealtimeCtrl();
	
	void Data();
	void Init();
	void PostData() {PostCallback(THISBACK(Data));}
	
	void AddMessage(String time, String level, String msg);
	void Info(String msg);
	void Error(String msg);
};

}

#endif
