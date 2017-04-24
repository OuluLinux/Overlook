#ifndef _DataCtrl_Notification_h_
#define _DataCtrl_Notification_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class NotificationCtrl;

class NotificationDraw : public Ctrl {
	/*NotificationCtrl* runner;
	Vector<Vector<double> > tmp;
	Vector<Point> pts;
	Vector<double> xsteps;
	One<ImageDraw> tmp_draw;
	SlotPtr src;
	int sym, tf;
	int week;
	int mode;*/
	
public:
	typedef NotificationDraw CLASSNAME;
	NotificationDraw();
	void Init(int mode, NotificationCtrl* r);
	void RefreshDraw();
	
	virtual void Paint(Draw& w);
	
};

class Notification : public MetaNodeCtrl {
	
protected:
	/*friend class NotificationDraw;
	Vector<SlotProcessAttributes> attrs;
	int last_total_duration, last_total, last_total_ready;
	bool running, stopped;
	
	void Run();*/
	
public:
	typedef Notification CLASSNAME;
	Notification();
	~Notification();
	
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "notification";}
	static String GetKeyStatic()  {return "notification";}
	
};

}


#endif
