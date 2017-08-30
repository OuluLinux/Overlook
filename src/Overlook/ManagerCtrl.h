#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>


class ManagerLoader : public WithLoader<TopWindow> {
	
public:
	typedef ManagerLoader CLASSNAME;
	ManagerLoader() {CloseBoxRejects(); CtrlLayout(*this); Title("Loading agent group");}
	
	void Progress(String status, int actual, int total) {this->status.SetLabel(status); prog.Set(actual, total); if (actual >= total) {Close(); Close();}}
	void PostProgress(int actual, int total, String status) {PostCallback(THISBACK3(Progress, status, actual, total));}
	
	void SubProgress(int actual, int total) {subprog.Set(actual, total);}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	
};

inline ManagerLoader& GetManagerLoader() {return Single<ManagerLoader>();}

class GroupOverview : public WithGroupOverview<ParentCtrl> {
	
protected:
	String label;
	
	void Progress(int actual, int total, String label);
	void SubProgress(int actual, int total);
	
public:
	typedef GroupOverview CLASSNAME;
	GroupOverview();
	
	void Data();
	
};

class GroupTabCtrl : public TabCtrl {
	GroupOverview			overview;
	SnapshotCtrl			snapctrl;
	Array<EditDoubleSpin>	tflimitedit;
	
public:
	typedef GroupTabCtrl CLASSNAME;
	GroupTabCtrl();
	
	void Data();
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

class JoinerTabCtrl : public TabCtrl {
	Joiner* joiner;
	WithJoinerOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
public:
	typedef JoinerTabCtrl CLASSNAME;
	JoinerTabCtrl();
	
	void Data();
	void SetJoiner(Joiner& joiner);
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, glist;
	ParentCtrl						mainview;
	Button							add_new;
	Array<Option>					new_opts;
	
	GroupTabCtrl					group_tabs;
	AgentTabCtrl					agent_tabs;
	JoinerTabCtrl					joiner_tabs;
	DataCtrl						datactrl;
	
	int view;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl();
	
	void Data();
	void SetView();
	
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
