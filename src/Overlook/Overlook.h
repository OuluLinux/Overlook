#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_

#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>

#include "Common.h"
#include "Optimizer.h"

#include "SimBroker.h"
#include "FixedSimBroker.h"
#include "ExpertSystem.h"
#include "System.h"
#include "Core.h"
#include "CustomCtrl.h"
#include "ExposureTester.h"

#include "DataBridge.h"
#include "Indicators.h"

#include "ManagerCtrl.h"

#include "GraphCtrl.h"
#include "GraphGroupCtrl.h"

#include "BrokerCtrl.h"
#include "ExportCtrl.h"

#include "ExpertCtrl.h"
#include "RandomForestCtrl.h"


namespace Overlook {


class Overlook : public TopWindow {
	
protected:
	friend class PrioritizerCtrl;
	
	// Main view
	TabCtrl						tabs;
	
	// Classic view
	ParentCtrl					visins;
	Splitter					droplist_split;
	DropList					ctrllist, symlist, tflist;
	Button						config;
	VectorMap<String, Ctrl*>	ctrls;
	CustomCtrl*					prev_view;
	Core*						prev_core;
	TimeCallback				tc;
	
	// Exposure tester
	ExposureTester				exposurectrl;
	
	// Expert System
	ExpertOptimizerCtrl			optimizer;
	ExpertGroupOptimizerCtrl	group_optimizer;
	ExpertRealCtrl				real;
	ExportCtrl					export_ctrl;
	
	
	RealtimeCtrl				rtctrl;
	StatusBar					status;
	
	Vector<String>				task_stack;
	
	void SetView();
	void Configure();
	void Data();
	
public:
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	
	void Init();
	void Refresher();
	void PostRefresher() {tc.Kill(); PostCallback(THISBACK(Refresher));}
	void PostPushTask(String task) {PostCallback(THISBACK1(PushTask, task));}
	void PostPopTask() {PostCallback(THISBACK(PopTask));}
	void PushTask(String task);
	void PopTask();
	void RefreshTaskStatus();
};



}

#endif
	