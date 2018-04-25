#ifndef _Overlook_RealtimeCtrl_h_
#define _Overlook_RealtimeCtrl_h_

#define LAYOUTFILE <Overlook/RealtimeCtrl.lay>
#include <CtrlCore/lay.h>


namespace Overlook {

class RealtimeMatchCtrl : public Ctrl {
	Vector<double> tmp;
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);
	
	int sym = 0;
	
};

class RealtimeOrderCtrl : public Ctrl {
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);

	int sym = 0;
	
};

class RealtimeCtrl : public ParentCtrl {
	Splitter split;
	ArrayCtrl opplist;
	WithRealtimeView<ParentCtrl> view;
	RealtimeMatchCtrl match;
	RealtimeOrderCtrl order;
	
public:
	typedef RealtimeCtrl CLASSNAME;
	RealtimeCtrl();
	
	void Data();
	void SetView();
	void Start();
	void Stop();
	void GetValues();
	void SetValues();
};




class RealtimeBrokerCtrl : public Ctrl {
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);
	
};

class PerformanceCtrl : public ParentCtrl {
	RealtimeBrokerCtrl broker;
	WithPerformanceView<ParentCtrl> view;
	Splitter split;
	
public:
	typedef PerformanceCtrl CLASSNAME;
	PerformanceCtrl();
	
	void Data();
	void GetValues();
	void SetValues();
	void Optimize();
	
};

}

#endif
