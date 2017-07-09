#ifndef _Overlook_ExposureTester_h_
#define _Overlook_ExposureTester_h_

#include "BrokerCtrl.h"

namespace Overlook {

class ExposureTester : public ParentCtrl {
	Splitter hsplit;
	ParentCtrl expctrl;
	Button next, reset;
	ArrayCtrl siglist;
	MainBrokerCtrl brokerctrl;
	SimBroker broker;
	Array<EditIntSpin> edits;
	
public:
	typedef ExposureTester CLASSNAME;
	ExposureTester();
	
	void Init();
	void Data();
	void Signal();
	void Next();
	void Reset();
	
};

}

#endif
