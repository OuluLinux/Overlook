#ifndef _Overlook_ManagerCtrl_h_
#define _Overlook_ManagerCtrl_h_

namespace Overlook {

#define LAYOUTFILE <Overlook/ManagerCtrl.lay>
#include <CtrlCore/lay.h>

class ManagerLoader : public WithLoader<TopWindow> {
	
public:
	typedef ManagerLoader CLASSNAME;
	ManagerLoader() {Icon(OverlookImg::icon()); CloseBoxRejects(); CtrlLayout(*this); Title("Loading agent group");}
	
	void Progress(String status, int actual, int total) {this->status.SetLabel(status); prog.Set(actual, total); if (actual >= total) {Close(); Close();}}
	void PostProgress(int actual, int total, String status) {PostCallback(THISBACK3(Progress, status, actual, total));}
	
	void SubProgress(int actual, int total) {subprog.Set(actual, total);}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	
};

inline ManagerLoader& GetManagerLoader() {return Single<ManagerLoader>();}

class RealtimeCtrl : public ParentCtrl {
	typedef Tuple3<String, String, String> Msg;
	Vector<Msg>				messages;
	Mutex					lock;
	Splitter				hsplit;
	BrokerCtrl				brokerctrl;
	ArrayCtrl				journal;
	
public:
	typedef RealtimeCtrl CLASSNAME;
	RealtimeCtrl();
	~RealtimeCtrl();
	
	void Data();
	void Init();
	void PostData() {PostCallback(THISBACK(Data));}
	
	void AddMessage(String time, String level, String msg);
	void Info(String msg);
	void Error(String msg);
};

}

#endif
