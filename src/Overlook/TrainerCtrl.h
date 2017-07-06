#ifndef _Overlook_TrainerCtrl_h_
#define _Overlook_TrainerCtrl_h_


namespace Overlook {
using namespace Upp;

class TrainerThreadCtrl;

#define LAYOUTFILE <Overlook/TrainerCtrl.lay>
#include <CtrlCore/lay.h>

class TrainerDraw : public Ctrl {
	TrainerThreadCtrl* ctrl;
	
public:
	TrainerDraw(TrainerThreadCtrl& ctrl);
	
	virtual void Paint(Draw& w);
	
};

class StatsGraph : public Ctrl {
	
protected:
	friend class LayerView;
	Trainer* trainer;
	RealtimeStatistics* stats;
	Vector<Point> polyline;
	ConvNet::Window null_win;
	int mode;
	
	const ConvNet::Window& GetData(int thrd);
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph(int mode, Trainer& t);
	
	virtual void Paint(Draw& w);
};

class TrainerThreadCtrl : public ParentCtrl {
	
protected:
	friend class TrainerDraw;
	
	Trainer* trainer;
	SliderCtrl time_slider;
	Label time_lbl, epoch;
	ProgressIndicator prog;
	TrainerDraw draw;
	Splitter hsplit;
	ConvNet::SessionConvLayers conv;
	ConvNet::HeatmapTimeView timescroll;
	StatsGraph reward;
	SessionThread* prev_thrd;
	int thrd_id;
	
public:
	typedef TrainerThreadCtrl CLASSNAME;
	TrainerThreadCtrl(Trainer& trainer, int thrd_id);
	
	void Data();
	
};

class TrainerCtrl : public ParentCtrl {
protected:
	Trainer* trainer;
	Array<TrainerThreadCtrl> thrds;
	DropList thrdlist;
	bool init;
	
public:
	typedef TrainerCtrl CLASSNAME;
	TrainerCtrl(Trainer& trainer);
	
	void Data();
	void SetView();
	
};

class TrainerResult : public WithStatistics<ParentCtrl> {
	
protected:
	friend class StatsGraph;
	Trainer* trainer;
	ArrayCtrl seslist;
	Splitter hsplit, vsplit;
	StatsGraph graph, reward, loss, avdepth, sigtotal, sigbroker;
	
public:
	typedef TrainerResult CLASSNAME;
	TrainerResult(Trainer& trainer);
	
	void Data();
	
	
};

class RealtimeNetworkCtrl : public ParentCtrl {
	Trainer* trainer;
	RealtimeSession* rtses;
	
	Button refresh_signals;
	
public:
	typedef RealtimeNetworkCtrl CLASSNAME;
	RealtimeNetworkCtrl(Trainer& trainer, RealtimeSession& rtses);
	
	void Data();
	void RefreshSignals();
	
};

}

#endif
