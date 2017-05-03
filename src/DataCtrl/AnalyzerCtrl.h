#ifndef _DataCtrl_AnalyzerCtrl_h_
#define _DataCtrl_AnalyzerCtrl_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class AnalyzerCtrl : public MetaNodeCtrl {
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	~AnalyzerCtrl();
	
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "analyzerctrl";}
	static String GetKeyStatic()  {return "analyzerctrl";}
	
};

}

#endif
