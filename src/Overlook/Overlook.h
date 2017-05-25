#ifndef _Overlook_Overlook_h_
#define _Overlook_Overlook_h_

#include "Common.h"
#include "Factory.h"
#include "Core.h"
#include "CustomCtrl.h"
#include "BaseSystem.h"
#include "Prioritizer.h"
#include "SimBroker.h"
#include "EventManager.h"

#include "DataBridge.h"
#include "CostUtils.h"
#include "Indicators.h"
#include "Features.h"
#include "Periodicals.h"
#include "QueryTable.h"
#include "Neural.h"

#include "GraphCtrl.h"
#include "GraphGroupCtrl.h"
#include "ForecasterCtrl.h"
#include "ForecasterCombCtrl.h"
#include "AgentCtrl.h"
#include "AgentCombCtrl.h"

#include "RecurrentCtrl.h"
#include "NarxCtrl.h"
#include "BrokerCtrl.h"
#include "EventCtrl.h"
#include "PrioritizerCtrl.h"

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
	
	Prioritizer user, optimizer;
	
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
