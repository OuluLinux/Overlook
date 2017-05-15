#ifndef _Overlook_EventCtrl_h_
#define _Overlook_EventCtrl_h_

namespace Overlook {


class EventCtrl : public CustomCtrl {
	ArrayCtrl ctrl;
	
public:
	EventCtrl();
	
	
	void RefreshData();
	
};

}

#endif
