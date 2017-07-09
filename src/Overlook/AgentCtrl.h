#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class AgentDraw : public Ctrl {
	Agent* agent;
	int snap_id;
	
public:
	AgentDraw(Agent& agent);
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
	
};

class StatsGraph : public Ctrl {
	
protected:
	Agent* agent;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph(Agent& t);
	
	virtual void Paint(Draw& w);
};

class AgentThreadCtrl : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	
	Agent* agent;
	SliderCtrl time_slider;
	Splitter hsplit;
	AgentDraw draw;
	MainBrokerCtrl brokerctrl;
	int thrd_id;
	
public:
	typedef AgentThreadCtrl CLASSNAME;
	AgentThreadCtrl(Agent& agent, int thrd_id);
	
	void Data();
	
};

class AgentCtrl : public ParentCtrl {
protected:
	Agent* agent;
	Array<AgentThreadCtrl> thrds;
	StatsGraph reward;
	DropList thrdlist;
	Option update_brokerctrl;
	bool init;
	
public:
	typedef AgentCtrl CLASSNAME;
	AgentCtrl(Agent& agent);
	
	void Data();
	void SetView();
	
};

class TrainingGraph : public Ctrl {
	Agent* agent;
	Vector<Point> polyline;
	
public:
	typedef TrainingGraph CLASSNAME;
	TrainingGraph(Agent& t);
	
	virtual void Paint(Draw& w);
};

class AgentTraining : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	Agent* agent;
	ArrayCtrl seslist;
	Splitter hsplit;
	bool init;
	
	Label epoch;
	ProgressIndicator prog;
	AgentDraw draw;
	TrainingGraph reward;
	ConvNet::SessionConvLayers conv;
	ConvNet::HeatmapTimeView timescroll;
	
	
public:
	typedef AgentTraining CLASSNAME;
	AgentTraining(Agent& agent);
	
	void Data();
	
	
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
	RealtimeNetworkCtrl(Agent& agent, RealtimeSession& rtses);
	
	void Data();
	void RefreshSignals();
	void KillSignals();
	
};

}

#endif
