#ifndef _Overlook_ExpertSystemCtrl_h_
#define _Overlook_ExpertSystemCtrl_h_


namespace Overlook {
using namespace Upp;


class ExchangeSlotsCtrl : public ParentCtrl {
	
public:
	Splitter splitter;
	ArrayCtrl slotlist, openlist;
	Button find_current;
	
public:
	typedef ExchangeSlotsCtrl CLASSNAME;
	ExchangeSlotsCtrl();
	
	void Data();
	void FindCurrent();
	
};



}

#endif
