#ifndef _DataCtrl_RunnerCtrl_h_
#define _DataCtrl_RunnerCtrl_h_

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

struct BatchPartStatus : Moveable<BatchPartStatus> {
	BatchPartStatus() {slot = NULL; begin = Time(1970,1,1); end = begin; sym_id = -1; tf_id = -1; actual = 0; total = 1; complete = false;}
	Slot* slot;
	Time begin, end;
	int sym_id, tf_id, actual, total;
	bool complete;
};

struct Batch : Moveable<Batch> {
	Batch() {begin = Time(1970,1,1); end = begin; stored = begin; loaded = begin;}
	VectorMap<String, SlotPtr> slots;
	Vector<BatchPartStatus> status;
	Time begin, end, stored, loaded;
};

#define LAYOUTFILE <DataCtrl/RunnerCtrl.lay>
#include <CtrlCore/lay.h>

class RunnerCtrl : public MetaNodeCtrl {
	
protected:
	friend class RunnerDraw;
	
	Vector<Batch> batches;
	Vector<SlotProcessAttributes> attrs;
	Atomic cursor;
	bool running, stopped;
	
	TabCtrl tabs;
	WithProgressLayout<ParentCtrl> progress;
	GraphLib::SpringGraph dependencies;
	WithDetails1Layout<ParentCtrl> details;
	ParentCtrl sysmon;
	
	void Run();
	void BatchProcessor(int thread_id, Batch* batch);
	void Processor(BatchPartStatus& stat);
	
public:
	typedef RunnerCtrl CLASSNAME;
	RunnerCtrl();
	~RunnerCtrl();
	
	void RefreshData();
	void RefreshProgress();
	void RefreshDependencyGraph();
	void RefreshDetails();
	
	void RefreshBatches();
	
	void SetProgressTotal(int actual, int total) {progress.total.Set(actual, total);}
	void SetProgressBatch(int actual, int total) {progress.batch.Set(actual, total);}
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "runnerctrl";}
	static String GetKeyStatic()  {return "runnerctrl";}
	
};

}

#endif
