#ifndef _DataCtrl_BrokerCtrl_h_
#define _DataCtrl_BrokerCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include <GraphLib/GraphLib.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;


#define LAYOUTFILE <DataCtrl/BrokerCtrl.lay>
#include <CtrlCore/lay.h>

class BrokerCtrl : public WithBrokerLayout<MetaNodeCtrl> {
	SlotPtr broker;
	ArrayCtrl trade, history, exposure, journal;
	
	DataBridge* db;
	
public:
	typedef BrokerCtrl CLASSNAME;
	BrokerCtrl();
	
	void Refresher();
	void Reset();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "brokerctrl";}
	static String GetKeyStatic()  {return "brokerctrl";}
	
	
	void DummyRunner();
};

}

#endif
