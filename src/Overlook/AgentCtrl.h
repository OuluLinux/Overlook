#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class AgentDraw : public Ctrl {
	Agent* agent;
	
public:
	AgentDraw(Agent& agent);
	
	virtual void Paint(Draw& w);
	
};

class StatsGraph : public Ctrl {
	
protected:
	friend class LayerView;
	Agent* agent;
	RealtimeStatistics* stats;
	Vector<Point> polyline;
	ConvNet::Window null_win;
	int mode;
	
	const ConvNet::Window& GetData(int thrd);
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph(int mode, Agent& t);
	
	virtual void Paint(Draw& w);
};

class AgentThreadCtrl : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	
	Agent* agent;
	SliderCtrl time_slider;
	Splitter hsplit;
	AgentDraw draw;
	BrokerCtrl brokerctrl;
	SequencerThread* prev_thrd;
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
	bool init;
	
public:
	typedef AgentCtrl CLASSNAME;
	AgentCtrl(Agent& agent);
	
	void Data();
	void SetView();
	
};

class AgentTraining : public ParentCtrl {
	
protected:
	friend class AgentDraw;
	friend class StatsGraph;
	Agent* agent;
	ArrayCtrl seslist;
	Splitter hsplit;
	
	Label epoch;
	ProgressIndicator prog;
	AgentDraw draw;
	ConvNet::SessionConvLayers conv;
	ConvNet::HeatmapTimeView timescroll;
	
	
public:
	typedef AgentTraining CLASSNAME;
	AgentTraining(Agent& agent);
	
	void Data();
	
	
};


class TfCompDraw : public Ctrl {
	Agent* agent;
	
public:
	TfCompDraw(Agent& agent);
	
	virtual void Paint(Draw& w);
	
};


class RealtimeNetworkCtrl : public ParentCtrl {
	Agent* agent;
	RealtimeSession* rtses;
	
	Button refresh_signals, killall_signals;
	Splitter hsplit;
	AgentDraw draw;
	BrokerCtrl brokerctrl;
	TfCompDraw tfcmp;
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
