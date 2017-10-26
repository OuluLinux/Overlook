#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class SnapshotDraw : public Ctrl {
	int snap_id;
	
public:
	SnapshotDraw();
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
};
/*
class EquityGraph : public Ctrl {
	
protected:
	Agent* agent;
	AgentFuse* fuse;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	int type;
	
	const Vector<double>& GetEquityData();
	
public:
	typedef EquityGraph CLASSNAME;
	EquityGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent, int type) {this->agent = &agent; clr = Color(49, 28, 150); this->type = type;}
	void SetFuse(AgentFuse& fuse) {this->fuse = &fuse; type = PHASE_FUSE_TRAINING;}
};

class ResultGraph : public Ctrl {
	Agent* agent;
	AgentFuse* fuse;
	Vector<Point> polyline;
	int type = -1;
	
	const Vector<double>& GetEquityData();
	const Vector<double>& GetDrawdownData();
	
public:
	typedef ResultGraph CLASSNAME;
	ResultGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent, int type) {this->agent = &agent; this->type = type;}
	void SetFuse(AgentFuse& fuse) {this->fuse = &fuse; type = PHASE_FUSE_TRAINING;}
};

class RewardGraph : public Ctrl {
	Agent* agent = NULL;
	AgentFuse* fuse = NULL;
	int type = -1;
	Color clr;
	Vector<Point> polyline;
	
public:
	typedef RewardGraph CLASSNAME;
	RewardGraph();
	
	virtual void Paint(Draw& d);
	
	void SetAgent(Agent& agent, int type);
	void SetFuse(AgentFuse& fuse);
};

class TrainingCtrl : public ParentCtrl {
	
protected:
	friend class SnapshotDraw;
	Agent* agent;
	AgentFuse* fuse;
	Splitter hsplit;
	int type;
	
	Label epoch;
	SnapshotDraw draw;
	ResultGraph result;
	EquityGraph stats;
	RewardGraph reward;
	HeatmapTimeView timescroll;
	ArrayCtrl trade;
	
	
	Splitter bsplit, vsplit;
	
public:
	typedef TrainingCtrl CLASSNAME;
	TrainingCtrl();
	
	void Data();
	void ApplySettings();
	void SetAgent(Agent& agent, int type);
	void SetFuse(AgentFuse& fuse);
};
*/
/*
class SnapshotCtrl : public ParentCtrl {
	SliderCtrl slider;
	Splitter hsplit;
	ArrayCtrl symtf, symcl, sym, one;
	SnapshotDraw draw;
	
public:
	typedef SnapshotCtrl CLASSNAME;
	SnapshotCtrl();
	
	void Data();
};
*/

class TrainingCtrl : public ParentCtrl {
	
};



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
