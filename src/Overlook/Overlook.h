#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_

#include "Common.h"
#include "QueryTable.h"

#include "System.h"
#include "Core.h"
#include "CustomCtrl.h"
#include "SimBroker.h"
#include "ExposureTester.h"

#include "DataBridge.h"
#include "Indicators.h"
#include "QtStats.h"

#include "Trainer.h"
#include "TrainerCtrl.h"

#include "GraphCtrl.h"
#include "GraphGroupCtrl.h"

#include "BrokerCtrl.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_header.h>

namespace Overlook {

class LoaderWindow : public TopWindow {
	
protected:
	Overlook* ol;
	Label lbl;
	String label;
	ProgressIndicator prog, sub, subsub;
	int ret_value;
	
	void Progress(int actual, int total, String label);
	void SubProgress(int actual, int total);
	void SubSubProgress(int actual, int total);
	void Close0() {Close();}
	
public:
	typedef LoaderWindow CLASSNAME;
	LoaderWindow(Overlook& ol);
	
	bool IsFail() {return ret_value;}
	void PostProgress(int actual, int total, String label) {PostCallback(THISBACK3(Progress, actual, total, label));}
	void PostSubProgress(int actual, int total) {PostCallback(THISBACK2(SubProgress, actual, total));}
	void PostSubSubProgress(int actual, int total) {PostCallback(THISBACK2(SubSubProgress, actual, total));}
	void PostClose() {PostCallback(THISBACK(Close0));}
};

class Overlook : public TopWindow {
	
protected:
	friend class PrioritizerCtrl;
	
	// Loader
	One<LoaderWindow>		loader;
	System					sys;
	
	// Main view
	TabCtrl					tabs;
	
	// Traditional view
	ParentCtrl				visins;
	Splitter				droplist_split;
	DropList				ctrllist, symlist, tflist;
	Button					config;
	VectorMap<String, Ctrl*> ctrls;
	CustomCtrl*				prev_view;
	Core*					prev_core;
	TimeCallback			tc;
	
	// Exposure tester
	ExposureTester			exposurectrl;
	
	// Trainer view
	Trainer					trainer;
	TrainerCtrl				trainerctrl;
	TrainerResult			resultctrl;
	
	// Realtime view
	BrokerCtrl				rt_ctrl;
	
	
	void SetView();
	void Configure();
	void Loader();
	void Data(bool periodic);
	
public:
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	
	void Init();
	void Load();
	void Start();
	void Deinit();
	void Refresher();
	void PostRefresher() {tc.Kill(); PostCallback(THISBACK(Refresher));}
	
};



}

#endif
	