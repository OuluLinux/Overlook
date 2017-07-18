#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class SnapshotDraw : public Ctrl {
	AgentGroup* group;
	int snap_id;
	
public:
	SnapshotDraw();
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
	void SetGroup(AgentGroup& group) {this->group = &group;}
};

class EquityGraph : public Ctrl {
	
protected:
	TraineeBase* trainee;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef EquityGraph CLASSNAME;
	EquityGraph();
	
	virtual void Paint(Draw& w);
	
	void SetTrainee(TraineeBase& trainee) {this->trainee = &trainee; clr = RainbowColor(Randomf());}
	
};

class ResultGraph : public Ctrl {
	TraineeBase* trainee;
	Vector<Point> polyline;
	
public:
	typedef ResultGraph CLASSNAME;
	ResultGraph();
	
	virtual void Paint(Draw& w);
	
	void SetTrainee(TraineeBase& trainee) {this->trainee = &trainee;}
	
};

class TrainingCtrl : public ParentCtrl {
	
protected:
	friend class SnapshotDraw;
	TraineeBase* trainee;
	Splitter hsplit;
	
	Label epoch;
	SnapshotDraw draw;
	ResultGraph reward;
	EquityGraph stats;
	ConvNet::HeatmapTimeView timescroll;
	
	BrokerCtrl broker;
	
	Splitter bsplit, vsplit;
	ConvNet::TrainingGraph graph;
	
public:
	typedef TrainingCtrl CLASSNAME;
	TrainingCtrl();
	
	void Data();
	void ApplySettings();
	void SetTrainee(TraineeBase& trainee);
	
};

class SnapshotCtrl : public ParentCtrl {
	AgentGroup* group;
	Splitter hsplit;
	ArrayCtrl list;
	SnapshotDraw draw;
	
public:
	typedef SnapshotCtrl CLASSNAME;
	SnapshotCtrl();
	
	void SetGroup(AgentGroup& group) {this->group = &group; draw.SetGroup(group);}
	void Data();
};

}

#endif
