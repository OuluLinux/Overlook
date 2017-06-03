#if 0

#ifndef _Overlook_PrioritizerCtrl_h_
#define _Overlook_PrioritizerCtrl_h_

namespace Overlook {

class PrioritizerCtrl : public CustomCtrl {
	TabCtrl tabs;
	ParentCtrl current, result;
	Splitter split;
	ArrayCtrl pipeline_queue, process_queue, job_queue, resultlist;
	
public:
	typedef PrioritizerCtrl CLASSNAME;
	PrioritizerCtrl();
	
	void RefreshData();
	
	
};

}

#endif
#endif
