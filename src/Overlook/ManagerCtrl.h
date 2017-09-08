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

class SystemOverview : public WithSystemOverview<ParentCtrl> {
	
protected:
	String label, phase_str;
	TimeStop ts;
	int sec = 0, min = 0, hour = 0;
	double prev_av_iters = 0;
	
	void Progress(int actual, int total, String label);
	void SubProgress(int actual, int total);
	
public:
	typedef SystemOverview CLASSNAME;
	SystemOverview();
	
	void Data();
	
};

class SystemTabCtrl : public TabCtrl {
	SystemOverview			overview;
	SnapshotCtrl			snapctrl;
	Array<EditDoubleSpin>	tflimitedit;
	
public:
	typedef SystemTabCtrl CLASSNAME;
	SystemTabCtrl();
	
	void Data();
};

class SignalTabCtrl : public TabCtrl {
	Agent* agent;
	WithSignalOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
public:
	typedef SignalTabCtrl CLASSNAME;
	SignalTabCtrl();
	
	void Data();
	void SetAgent(Agent& agent);
	
};

class AmpTabCtrl : public TabCtrl {
	Agent* agent;
	WithAmpOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
public:
	typedef AmpTabCtrl CLASSNAME;
	AmpTabCtrl();
	
	void Data();
	void SetAgent(Agent& agent);
	
};

class FuseTabCtrl : public TabCtrl {
	Agent* agent;
	WithFuseOverview<ParentCtrl>	overview;
	TrainingCtrl					trainingctrl;
	
public:
	typedef FuseTabCtrl CLASSNAME;
	FuseTabCtrl();
	
	void Data();
	void SetAgent(Agent& agent);
	
};

class ManagerCtrl : public ParentCtrl {
	Splitter						hsplit, listsplit;
	ArrayCtrl						alist, glist;
	ParentCtrl						mainview;
	Button							add_new;
	Array<Option>					new_opts;
	
	SystemTabCtrl					system_tabs;
	SignalTabCtrl					signal_tabs;
	AmpTabCtrl						amp_tabs;
	FuseTabCtrl						fuse_tabs;
	ExportCtrl						export_ctrl;
	
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
