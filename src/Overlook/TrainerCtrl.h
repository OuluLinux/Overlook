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
	int thrd_id;
	bool init;
	
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

class TrainerStatistics;

class StatsGraph : public Ctrl {
	
protected:
	friend class LayerView;
	Trainer* trainer;
	TrainerStatistics* stats;
	Vector<Point> polyline;
	int mode;
	
	const ConvNet::Window& GetData(int thrd);
	
public:
	typedef StatsGraph CLASSNAME;
	StatsGraph(int mode, Trainer& t, TrainerStatistics* s);
	
	virtual void Paint(Draw& w);
};

class TrainerStatistics : public WithStatistics<ParentCtrl> {
	
protected:
	friend class StatsGraph;
	Trainer* trainer;
	Splitter hsplit, vsplit0, vsplit1;
	StatsGraph profit, sigprofit, loss, reward, thrdprio, thrdperf;
	int mode;
	bool init;
	
public:
	typedef TrainerStatistics CLASSNAME;
	TrainerStatistics(Trainer& trainer);
	
	void Data();
	
	
};

}

#endif
