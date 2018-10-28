#ifndef _Overlook_Automation_h_
#define _Overlook_Automation_h_


namespace Overlook {
using namespace libmt;


class Automation : public Common {
	Index<String> symbols;
	Time last_update;
	int mode = 0;
	
	enum {NO_PENDING, PENDING, WAITING};
	
public:
	typedef Automation CLASSNAME;
	Automation();
	
	virtual void Init();
	virtual void Start();
};


class AutomationCtrl : public CommonCtrl {
	
	
public:
	typedef AutomationCtrl CLASSNAME;
	AutomationCtrl();
	
	virtual void Data();
};



}

#endif
