#ifndef _Overlook_GraphGroupCtrl_h_
#define _Overlook_GraphGroupCtrl_h_

#include <SubWindowCtrl/SubWindowCtrl.h>

namespace Overlook {

class ChartManager;

class GraphGroupCtrl : public SubWindowCtrl {
	
protected:
	friend class Overlook;
	
	Core* core = NULL;
	BarData* bardata = NULL;
	Array<GraphCtrl> graphs;
	Splitter split;
	
	String title;
	int div;
	int shift;
	int indi = 0, symbol = 0, tf = 0;
	bool right_offset, keep_at_end;
	
	void AddSeparateCore(int id, const Vector<double>& settings);
	void AddValueCore(int id, const Vector<double>& settings);
	void SetTimeValueTool(bool enable);
	void GraphMouseMove(Point pt, GraphCtrl* g);
	void Refresh0() {ParentCtrl::Refresh();}
	
public:
	typedef GraphGroupCtrl CLASSNAME;
	GraphGroupCtrl();
	
	void PostRefresh() {PostCallback(THISBACK(Refresh0));}
	void ClearCores();
	GraphCtrl& AddGraph(Core* src);
	void SetGraph(Core* src);
	void SetShift(int i) {shift = i;}
	void SetRightOffset(bool enable=true);
	void SetKeepAtEnd(bool enable=true);
	void Settings();
	void OpenContextMenu() {MenuBar::Execute(THISBACK(ContextMenu));}
	void ContextMenu(Bar& bar);
	
	virtual void Init(Core* src);
	virtual void Start();
	virtual void Data();
	virtual String GetTitle();
	
	Core& GetCore() {return *bardata;}
	Color GetBackground() {return White();}
	Color GetGridColor() {return GrayColor();}
	int GetTf() {return tf;}
	int GetWidthDivider() {return div;}
	int GetShift() {return shift;}
	bool GetRightOffset() {return right_offset;}
	bool GetKeepAtEnd() {return keep_at_end;}
	
	GraphGroupCtrl& SetTimeframe(int tf_id);
	GraphGroupCtrl& SetIndicator(int indi);
	
};



}


#endif
