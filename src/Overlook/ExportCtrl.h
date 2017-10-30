#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class ExportCtrl;

class DataGraph : public Ctrl {
	
protected:
	ExportCtrl* dc;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef DataGraph CLASSNAME;
	DataGraph(ExportCtrl* dc);
	
	virtual void Paint(Draw& w);
	
};

class SignalGraph : public Ctrl {
	ExportCtrl* dc;
	
public:
	typedef SignalGraph CLASSNAME;
	SignalGraph(ExportCtrl* dc);
	
	virtual void Paint(Draw& w);
	
};

class ExportCtrl : public ParentCtrl {
	DataGraph graph;
	SignalGraph siggraph;
	Label data;
	SliderCtrl timeslider;
	Splitter hsplit;
	ArrayCtrl siglist, trade;
	Vector<int> poslist;
	int last_pos;
	
public:
	typedef ExportCtrl CLASSNAME;
	ExportCtrl();
	
	void Data();
	void GuiData();
	
	
	Vector<double> equity;
};

}

#endif
