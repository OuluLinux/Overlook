#ifndef _DataCtrl_EventCtrl_h_
#define _DataCtrl_EventCtrl_h_


namespace DataCtrl {
using namespace DataCore;

class EventCtrl : public MetaNodeCtrl {
	ArrayCtrl ctrl;
	MetaVar data;
	
public:
	EventCtrl();
	
	virtual String GetKey() const {return "events";}
	static String GetKeyStatic()  {return "events";}
	
	void SetEventManager(MetaVar src);
	
	void RefreshData();
	
};

}

#endif
