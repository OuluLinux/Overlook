#ifndef _DataCtrl_RunnerCtrl_h_
#define _DataCtrl_RunnerCtrl_h_

#include <GraphLib/GraphLib.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

class RunnerCtrl;

struct ProgressDisplay : Display {
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const {
		Font fnt = SansSerif(r.Height() - 2);
		int perc = q;
		int cx = r.GetWidth() * perc / 1000;
		String txt = IntStr(perc / 10) + "%";
		w.DrawRect(r, paper);
		w.DrawRect(r.left, r.top, cx, r.GetHeight(), Color(0, 255, 0));
		w.DrawText(r.left + 2, r.top + (r.Height() - GetTextSize(txt, fnt).cy) / 2, txt, fnt, ink);
	}
};

#define LAYOUTFILE <DataCtrl/RunnerCtrl.lay>
#include <CtrlCore/lay.h>

class RunnerCtrl : public MetaNodeCtrl {
	
protected:
	friend class RunnerDraw;
	
	bool progress_batch_updating;
	bool progress_updating;
	
	TabCtrl tabs;
	WithProgressLayout<ParentCtrl> progress;
	GraphLib::SpringGraph dependencies;
	WithDetails1Layout<ParentCtrl> details;
	ParentCtrl sysmon;
	
public:
	typedef RunnerCtrl CLASSNAME;
	RunnerCtrl();
	
	void PostRefreshData() {PostCallback(THISBACK(RefreshData));}
	void PostRefreshProgress();
	void PostProgress(int actual, int total) {PostCallback(THISBACK2(SetProgressTotal, actual, total));}
	void PostPartProgress(int actual, int total);
	
	void RefreshData();
	void RefreshProgress();
	void RefreshDependencyGraph();
	void RefreshDetails();
	
	void SetProgressTotal(int actual, int total) {progress.total.Set(actual, total);}
	void SetProgressBatch(int actual, int total) {progress.batch.Set(actual, total); progress_batch_updating = false;}
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "runnerctrl";}
	static String GetKeyStatic()  {return "runnerctrl";}
};

}

#endif
