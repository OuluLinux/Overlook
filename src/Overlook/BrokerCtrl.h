#ifndef _Overlook_BrokerCtrl_h_
#define _Overlook_BrokerCtrl_h_

namespace Overlook {

using namespace ConvNet;


#define LAYOUTFILE <Overlook/BrokerCtrl.lay>
#include <CtrlCore/lay.h>

class BrokerCtrl : public WithBrokerLayout<CustomCtrl> {
	Core* broker;
	ArrayCtrl trade, history, exposure, journal;
	
	DataBridge* db;
	
public:
	typedef BrokerCtrl CLASSNAME;
	BrokerCtrl();
	
	void Refresher();
	void Reset();
	
	virtual void Init();
	
	
	void DummyRunner();
};

}

#endif
