#ifndef _DataCtrl_DataBridgeCtrl_h_
#define _DataCtrl_DataBridgeCtrl_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

#define LAYOUTFILE <DataCtrl/DataBridgeCtrl.lay>
#include <CtrlCore/lay.h>

class DataBridgeCtrl : public WithDataBridgeCtrlLayout<MetaNodeCtrl> {
	
protected:
	
	
public:
	typedef DataBridgeCtrl CLASSNAME;
	DataBridgeCtrl();
	~DataBridgeCtrl();
	
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "dbctrl";}
	static String GetKeyStatic()  {return "dbctrl";}
	
};

}

#endif
