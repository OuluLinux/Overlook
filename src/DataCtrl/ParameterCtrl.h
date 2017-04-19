#ifndef _DataCtrl_ParameterCtrl_h_
#define _DataCtrl_ParameterCtrl_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class ParameterCtrl : public MetaNodeCtrl {
	
public:
	typedef ParameterCtrl CLASSNAME;
	ParameterCtrl();
	~ParameterCtrl();
	
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "params";}
	static String GetKeyStatic()  {return "params";}
	
};

}


#endif
