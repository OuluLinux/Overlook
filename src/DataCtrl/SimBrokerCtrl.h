#ifndef _DataCtrl_SimBrokerCtrl_h_
#define _DataCtrl_SimBrokerCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include <GraphLib/GraphLib.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;


#define LAYOUTFILE <DataCtrl/SimBrokerCtrl.lay>
#include <CtrlCore/lay.h>

class SimBrokerCtrl : public WithMonaLayout<MetaNodeCtrl> {
	
public:
	typedef SimBrokerCtrl CLASSNAME;
	SimBrokerCtrl();
	
	void Refresher();
	void Reset();
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "simbrokerctrl";}
	static String GetKeyStatic()  {return "simbrokerctrl";}
	
};

}

#endif
