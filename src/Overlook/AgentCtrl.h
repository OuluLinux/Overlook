#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class AgentDraw : public Ctrl {
	AgentGroup* group;
	int snap_id;
	
public:
	AgentDraw();
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
	void SetGroup(AgentGroup& group) {this->group = &group;}
};

class StatsGraph : public Ctrl {
	
protected:
	Agent* agent;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent) {this->agent = &agent; clr = RainbowColor(Randomf());}
	
};

class AgentThreadCtrl : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	
	Agent* agent;
	SliderCtrl time_slider;
	Splitter hsplit;
	AgentDraw draw;
	BrokerCtrl brokerctrl;
	int thrd_id;
	
public:
	typedef AgentThreadCtrl CLASSNAME;
	AgentThreadCtrl();
	
	void Data();
	
};

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

class TrainingGraph : public Ctrl {
	Agent* agent;
	Vector<Point> polyline;
	
public:
	typedef TrainingGraph CLASSNAME;
	TrainingGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent) {this->agent = &agent;}
	
};

class AgentTraining : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	Agent* agent;
	Splitter hsplit;
	bool init;
	
	Label epoch;
	AgentDraw draw;
	TrainingGraph reward;
	StatsGraph stats;
	ConvNet::HeatmapTimeView timescroll;
	
	BrokerCtrl broker;
	
	Splitter bsplit, vsplit;
	Option update_broker;
	ConvNet::TrainingGraph graph;
	
public:
	typedef AgentTraining CLASSNAME;
	AgentTraining();
	
	void Data();
	void ApplySettings();
	void SetAgent(Agent& agent);
};


class RealtimeNetworkCtrl : public ParentCtrl {
	Agent* agent;
	RealtimeSession* rtses;
	
	Button refresh_signals, killall_signals;
	Splitter hsplit;
	AgentDraw draw;
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
	AgentDraw draw;
	
public:
	typedef SnapshotCtrl CLASSNAME;
	SnapshotCtrl();
	
	void SetGroup(AgentGroup& group) {this->group = &group; draw.SetGroup(group);}
	void Data();
};

}

#endif
