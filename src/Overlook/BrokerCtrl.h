#ifndef _Overlook_BrokerCtrl_h_
#define _Overlook_BrokerCtrl_h_

namespace Overlook {

using namespace ConvNet;

#define LAYOUTFILE <Overlook/BrokerCtrl.lay>
#include <CtrlCore/lay.h>

class BrokerCtrl : public WithBrokerLayout<CustomCtrl> {
	Brokerage* broker;
	
	ArrayCtrl trade, history, exposure;
	ArrayMap<int, Order> orders;
	Vector<int> open_tickets;
	
public:
	typedef BrokerCtrl CLASSNAME;
	BrokerCtrl();
	
	void Refresher();
	void Reset();
	void Close();
	void CloseAll();
	void PriceCursor();
	
	void Init();
	void Data();
	
	void SetBroker(Brokerage& broker) {this->broker = &broker;}
	void OpenOrder(int type);
	void ReadOnly();
};

}

#endif
