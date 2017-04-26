#ifndef _DataCtrl_RunnerCtrl_h_
#define _DataCtrl_RunnerCtrl_h_

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class RunnerCtrl;

class RunnerDraw : public Ctrl {
	RunnerCtrl* runner;
	Vector<Vector<double> > tmp;
	Vector<Point> pts;
	Vector<double> xsteps;
	One<ImageDraw> tmp_draw;
	SlotPtr src;
	int sym, tf;
	int week;
	int mode;
	
public:
	typedef RunnerDraw CLASSNAME;
	RunnerDraw();
	void Init(int mode, RunnerCtrl* r);
	void RefreshDraw();
	
	virtual void Paint(Draw& w);
	
};

#define LAYOUTFILE <DataCtrl/RunnerCtrl.lay>
#include <CtrlCore/lay.h>

class RunnerCtrl : public MetaNodeCtrl {
	
protected:
	friend class RunnerDraw;
	Vector<SlotProcessAttributes> attrs;
	int last_total_duration, last_total, last_total_ready;
	bool running, stopped;
	
	TabCtrl tabs;
	WithProgressLayout<ParentCtrl> progress;
	GraphLib::SpringGraph dependencies;
	ParentCtrl details;
	ParentCtrl sysmon;
	
	void Run();
	
public:
	typedef RunnerCtrl CLASSNAME;
	RunnerCtrl();
	~RunnerCtrl();
	
	void RefreshData();
	void RefreshProgress();
	void RefreshDependencyGraph();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "runnerctrl";}
	static String GetKeyStatic()  {return "runnerctrl";}
	
};

}

#endif
