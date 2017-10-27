#ifndef _Overlook_ExpertCtrl_h_
#define _Overlook_ExpertCtrl_h_

namespace Overlook {


class EvolutionGraph : public Ctrl {
	Vector<Point> polyline;
	
public:
	
	virtual void Paint(Draw& w);
	
};

class ExpertSectorsCtrl : public ParentCtrl {
	EvolutionGraph graph;
	ArrayCtrl pop, unit;
	Splitter vsplit, hsplit;
	
public:
	typedef ExpertSectorsCtrl CLASSNAME;
	ExpertSectorsCtrl();
	
	void Data();
	
};

class ExpertOptimizerCtrl : public TabCtrl {
	ExpertSectorsCtrl sectors;
	
public:
	typedef ExpertOptimizerCtrl CLASSNAME;
	ExpertOptimizerCtrl();
	
	void Data();
	
};

}

#endif
