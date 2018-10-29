#ifndef _Forecaster_Forecaster_h
#define _Forecaster_Forecaster_h

#include <CtrlLib/CtrlLib.h>
#include <ConvNet/ConvNet.h>
using namespace Upp;
using namespace ConvNet;


#include "CompatAMP.h"
using namespace concurrency;

#include "Common.h"
#include "Heatmap.h"
#include "Core.h"
#include "System.h"
#include "Optimizer.h"
#include "Indicators.h"
#include "HeatmapLooper.h"
#include "Manager.h"

namespace Forecast {

void DrawVectorPolyline(Draw& d, Size sz, const Vector<double>& pts, Vector<Point>& cache, int max_count=0, double zero_line=0.0);

class ManagerCtrl;


struct DrawLines : public Ctrl {
	Heatmap image;
	Task* t = NULL;
	double zero_line = 1.0;
	int type;
	int gen_id = 0;
	
	enum {GENVIEW, HISVIEW, FCASTVIEW, OPTSTATS, INDIVIEW};
	
	Vector<Point> polyline;
	
	virtual void Paint(Draw& d);
	void SetTask(Task& t) {this->t = &t;}
};

struct OptimizationCtrl : public DataCtrl {
	
	TabCtrl tabs;
	ParentCtrl status;
	Splitter main_vsplit, main_hsplit;
	DrawLines opt_draw;
	DrawLines his_draw;
	ArrayCtrl his_list;
	Array<DrawLines> gens;
	Task* task = NULL;
	int prev_id = -1;
	
	typedef OptimizationCtrl CLASSNAME;
	OptimizationCtrl();
	
	virtual void Data();
	void SelectHistoryItem();
	void SetTask(Task& t);
};

struct ForecastCtrl : public DataCtrl {
	
	DrawLines draw;
	
	typedef ForecastCtrl CLASSNAME;
	ForecastCtrl();
	
	virtual void Data();
	void SetTask(Task& t) {draw.SetTask(t);}
};

struct IndicatorCtrl : public DataCtrl {
	
	DrawLines draw;
	
	typedef IndicatorCtrl CLASSNAME;
	IndicatorCtrl();
	
	virtual void Data();
	void SetTask(Task& t) {draw.SetTask(t);}
};

struct NNSampleDraw : public Ctrl {
	NNSample* sample = NULL;
	Vector<Point> polyline;
	virtual void Paint(Draw& d);
};

struct DqnTrainingCtrl : public DataCtrl {
	Splitter split;
	ArrayCtrl list;
	NNSampleDraw draw;
	Task* t = NULL;
	
	typedef DqnTrainingCtrl CLASSNAME;
	DqnTrainingCtrl();
	
	virtual void Data();
	void SelectSample();
	void SetTask(Task& t) {this->t = &t;}
};

class ManagerCtrl : public TopWindow {
	
protected:
	friend class DrawLines;
	
	
	ParentCtrl main_task;
	Splitter vsplit, hsplit;
	ArrayCtrl seslist, tasklist;
	DataCtrl* prev_ctrl = NULL;
	
	TimeCallback tc;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl();
	~ManagerCtrl();
	
	void Data();
	void PeriodicalRefresh();
	void SelectTask();
	void SwitchCtrl(DataCtrl* c);
	
};

}

#endif
