#ifndef _Overlook_ExpertCtrl_h_
#define _Overlook_ExpertCtrl_h_

namespace Overlook {


class EvolutionGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class TestEquityGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class ExpertOptimizerCtrl : public ParentCtrl {
	EvolutionGraph graph;
	ArrayCtrl pop, unit;
	Splitter vsplit, hsplit;
	
public:
	typedef ExpertOptimizerCtrl CLASSNAME;
	ExpertOptimizerCtrl();
	
	void Data();
	
};

class ExpertGroupOptimizerCtrl : public ParentCtrl {
	
public:
	typedef ExpertGroupOptimizerCtrl CLASSNAME;
	ExpertGroupOptimizerCtrl();
	
	void Data();
	
};

class ExpertRealCtrl : public ParentCtrl {
	Label last_update;
	Button refresh_now;
	ArrayCtrl pop, unit;
	Splitter hsplit;
	ProgressIndicator prog;
	TestEquityGraph testequity;
	
public:
	typedef ExpertRealCtrl CLASSNAME;
	ExpertRealCtrl();
	
	void Data();
	void RefreshNow();
	
};


}

#endif
