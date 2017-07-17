#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class RealtimeSession;

class SnapshotDraw : public Ctrl {
	AgentGroup* group;
	int snap_id;
	
public:
	SnapshotDraw();
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
	void SetGroup(AgentGroup& group) {this->group = &group;}
};

class StatsGraph : public Ctrl {
	
protected:
	TraineeBase* trainee;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph();
	
	virtual void Paint(Draw& w);
	
	void SetTrainee(TraineeBase& trainee) {this->trainee = &trainee; clr = RainbowColor(Randomf());}
	
};

class AgentThreadCtrl : public ParentCtrl {
	
protected:
	friend class SnapshotDraw;
	
	Agent* agent;
	SliderCtrl time_slider;
	Splitter hsplit;
	SnapshotDraw draw;
	BrokerCtrl brokerctrl;
	int thrd_id;
	
public:
	typedef AgentThreadCtrl CLASSNAME;
	AgentThreadCtrl();
	
	void Data();
	
};
/*
class AgentCtrl : public ParentCtrl {
protected:
	Agent* agent;
	Array<AgentThreadCtrl> thrds;
	StatsGraph reward;
	Option update_brokerctrl;
	bool init;
	
public:
	typedef AgentCtrl CLASSNAME;
	AgentCtrl();
	
	void Data();
	
};
*/
class TrainingGraph : public Ctrl {
	TraineeBase* trainee;
	Vector<Point> polyline;
	
public:
	typedef TrainingGraph CLASSNAME;
	TrainingGraph();
	
	virtual void Paint(Draw& w);
	
	void SetTrainee(TraineeBase& trainee) {this->trainee = &trainee;}
	
};

class TrainingCtrl : public ParentCtrl {
	
protected:
	friend class SnapshotDraw;
	TraineeBase* trainee;
	Splitter hsplit;
	bool init;
	
	Label epoch;
	SnapshotDraw draw;
	TrainingGraph reward;
	StatsGraph stats;
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


class RealtimeNetworkCtrl : public ParentCtrl {
	Agent* agent;
	RealtimeSession* rtses;
	
	Button refresh_signals, killall_signals;
	Splitter hsplit;
	SnapshotDraw draw;
	BrokerCtrl brokerctrl;
	Label tfcmplbl;
	
public:
	typedef RealtimeNetworkCtrl CLASSNAME;
	RealtimeNetworkCtrl();
	
	void Data();
	void RefreshSignals();
	void KillSignals();
	
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
