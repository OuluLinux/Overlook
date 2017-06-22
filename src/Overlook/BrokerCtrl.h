#ifndef _Overlook_BrokerCtrl_h_
#define _Overlook_BrokerCtrl_h_

namespace Overlook {

using namespace ConvNet;

#define LAYOUTFILE <Overlook/BrokerCtrl.lay>
#include <CtrlCore/lay.h>

class BrokerCtrl : public WithBrokerLayout<CustomCtrl> {
	Brokerage* broker;
	ArrayCtrl trade, history, exposure, journal;
	ArrayMap<int, Order> orders;
	Vector<int> open_tickets;
	
public:
	typedef BrokerCtrl CLASSNAME;
	BrokerCtrl();
	
	void Refresher();
	void Reset();
	
	virtual void Init();
	virtual void RefreshData();
	
	void SetBroker(Brokerage& broker) {this->broker = &broker;}
	void DummyRunner();
};

}

#endif

