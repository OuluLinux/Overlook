#if 0

#ifndef _Overlook_ExposureTester_h_
#define _Overlook_ExposureTester_h_

namespace Overlook {

class ExposureTester : public ParentCtrl {
	Splitter hsplit;
	ParentCtrl expctrl;
	ArrayCtrl siglist;
	BrokerCtrl brokerctrl;
	SimBroker broker;
	Array<EditIntSpin> edits;
	
public:
	typedef ExposureTester CLASSNAME;
	ExposureTester();
	
	void Init();
	void Signal();
	
};

}

#endif
#endif
