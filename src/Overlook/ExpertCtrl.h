#ifndef _Overlook_ExpertCtrl_h_
#define _Overlook_ExpertCtrl_h_

namespace Overlook {


class SourceProcessingGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class SourceEquityGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class ExpertOptimizerCtrl : public ParentCtrl {
	SourceProcessingGraph graph;
	ArrayCtrl pop, unit;
	Splitter vsplit, hsplit;
	
public:
	typedef ExpertOptimizerCtrl CLASSNAME;
	ExpertOptimizerCtrl();
	
	void Data();
	
};



class OptimizationGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class OptimizationEquityGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class ExpertGroupOptimizerCtrl : public ParentCtrl {
	OptimizationGraph opt;
	OptimizationEquityGraph equity;
	Splitter vsplit;
	Label status;
	ProgressIndicator prog;
	
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
	SourceEquityGraph testequity;
	
public:
	typedef ExpertRealCtrl CLASSNAME;
	ExpertRealCtrl();
	
	void Data();
	void RefreshNow();
	
};


}

#endif
