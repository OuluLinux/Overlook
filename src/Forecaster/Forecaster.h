#ifndef _Forecaster_Forecaster_h
#define _Forecaster_Forecaster_h

#include <CtrlLib/CtrlLib.h>
#include <ConvNet/ConvNet.h>
using namespace Upp;


#include "CompatAMP.h"
using namespace concurrency;

#include "Common.h"
#include "Heatmap.h"
#include "Core.h"
#include "System.h"
#include "Optimizer.h"
#include "Generator.h"
#include "Regenerator.h"
#include "Indicators.h"
#include "Manager.h"

namespace Forecast {

void DrawVectorPolyline(Draw& d, Size sz, const Vector<double>& pts, Vector<Point>& cache, int max_count=0, double zero_line=0.0);

class ManagerCtrl;


struct DrawLines : public Ctrl {
	ManagerCtrl* t;
	Heatmap image;
	int type;
	int gen_id = 0;
	
	enum {GENVIEW, HISVIEW, OPTSTATS};
	
	Vector<Point> polyline;
	virtual void Paint(Draw& d);
};

struct GeneratorCtrl : public ParentCtrl {
	DrawLines draw;
	
	
	GeneratorCtrl();
	void SetId(int i) {draw.type = DrawLines::GENVIEW; draw.gen_id = i;}
	
};

struct RegeneratorCtrl : public TabCtrl {
	
	ParentCtrl status;
	ProgressIndicator optprog;
	Splitter main_vsplit, main_hsplit;
	DrawLines opt_draw;
	DrawLines his_draw;
	ArrayCtrl his_list;
	Array<GeneratorCtrl> gens;
	int prev_id = -1;
	
	typedef RegeneratorCtrl CLASSNAME;
	RegeneratorCtrl();
	
	void Data();
	void SelectHistoryItem();
};

struct ForecastCtrl : public ParentCtrl {
	
};

class ManagerCtrl : public TopWindow {
	
protected:
	friend class DrawLines;
	
	
	
	TabCtrl tabs;
	ParentCtrl mgr;
	ForecastCtrl fcast;
	RegeneratorCtrl regenctrl;
	
	TimeCallback tc;
	
public:
	typedef ManagerCtrl CLASSNAME;
	ManagerCtrl();
	
	void PeriodicalRefresh();
};

}

#endif
