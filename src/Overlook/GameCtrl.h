#ifndef _Overlook_GameCtrl_h_
#define _Overlook_GameCtrl_h_

#define LAYOUTFILE <Overlook/GameCtrl.lay>
#include <CtrlCore/lay.h>


namespace Overlook {

class GameMatchCtrl : public Ctrl {
	Vector<double> tmp;
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);
	
	int sym = 0;
	
};

class GameOrderCtrl : public Ctrl {
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);

	int sym = 0;
	
};

class GameCtrl : public ParentCtrl {
	Splitter split;
	ArrayCtrl opplist;
	WithGameView<ParentCtrl> view;
	GameMatchCtrl match;
	GameOrderCtrl order;
	
public:
	typedef GameCtrl CLASSNAME;
	GameCtrl();
	
	void Data();
	void SetView();
	void Start();
	void Stop();
	void GetValues();
	void SetValues();
};




class GameBrokerCtrl : public Ctrl {
	Vector<Point> polyline;
	
public:
	virtual void Paint(Draw& w);
	
};

class PerformanceCtrl : public ParentCtrl {
	GameBrokerCtrl broker;
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
