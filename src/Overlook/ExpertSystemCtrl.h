#ifndef _Overlook_ExpertSystemCtrl_h_
#define _Overlook_ExpertSystemCtrl_h_


namespace Overlook {
using namespace Upp;


struct BooleansDraw : public Ctrl {
	virtual void Paint(Draw& w);
	
	int cursor = 0;
	int tf = 0;
};

struct BitProcessManagerCtrl : public ParentCtrl {
	
	BooleansDraw bools;
	DropList tflist;
	SliderCtrl slider;
	
	
public:
	typedef BitProcessManagerCtrl CLASSNAME;
	BitProcessManagerCtrl();
	
	void Data();
	void SetTf();
};

class ExchangeSlotsCtrl : public ParentCtrl {
	
public:
	Splitter splitter;
	ArrayCtrl slotlist, openlist;
	
public:
	typedef ExchangeSlotsCtrl CLASSNAME;
	ExchangeSlotsCtrl();
	
	void Data();
	
};

class SlotSignalsCtrl : public ParentCtrl {
	
public:
	DropList symlist;
	Button find_current;
	Splitter splitter;
	ArrayCtrl slotlist, openlist;
	
public:
	typedef SlotSignalsCtrl CLASSNAME;
	SlotSignalsCtrl();
	
	void Data();
	void FindCurrent();
	
};

}

#endif
