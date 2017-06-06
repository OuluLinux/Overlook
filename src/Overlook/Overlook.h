#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_

#include "Common.h"
#include "QueryTable.h"

#include "Core.h"
#include "CustomCtrl.h"
#include "System.h"
#include "SimBroker.h"

#include "DataBridge.h"
#include "Indicators.h"
#include "Template.h"
#include "Forecasters.h"

#include "GraphCtrl.h"
#include "GraphGroupCtrl.h"

#include "BrokerCtrl.h"

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_header.h>

namespace Overlook {

class Overlook : public TopWindow {
	
protected:
	friend class PrioritizerCtrl;
	
	Splitter droplist_split;
	DropList ctrllist, symlist, tflist;
	Button config;
	VectorMap<String, Ctrl*> ctrls;
	CustomCtrl* prev_view;
	Core* prev_core;
	TimeCallback tc;
	
	System sys;
	
	void SetView();
	void Configure();
public:
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	
	void Init();
	void Deinit();
	void Refresher();
	void PostRefresher() {tc.Kill(); PostCallback(THISBACK(Refresher));}
	
};



}

#endif
