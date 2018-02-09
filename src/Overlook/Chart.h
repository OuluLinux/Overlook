#ifndef _Overlook_Chart_h_
#define _Overlook_Chart_h_

#include <SubWindowCtrl/SubWindowCtrl.h>

namespace Overlook {

class ChartManager;

class Chart : public SubWindowCtrl {
	
protected:
	friend class Overlook;
	
	Vector<Ptr<CoreItem> > work_queue;
	Array<GraphCtrl> graphs;
	Splitter split;
	FactoryDeclaration decl;
	String title;
	int div = 0;
	int shift = 0;
	int symbol = 0, tf = 0;
	bool right_offset = 0, keep_at_end = 0;
	Core* core = NULL;
	DataBridge* bardata = NULL;
	
	
	void AddSeparateCore(int id, const Vector<double>& settings);
	void AddValueCore(int id, const Vector<double>& settings);
	void SetTimeValueTool(bool enable);
	void GraphMouseMove(Point pt, GraphCtrl* g);
	void Refresh0() {ParentCtrl::Refresh();}
	
public:
	typedef Chart CLASSNAME;
	Chart();
	
	void PostRefresh() {PostCallback(THISBACK(Refresh0));}
	void ClearCores();
	GraphCtrl& AddGraph(Core* src);
	void SetGraph(Core* src);
	void SetShift(int i) {shift = i;}
	void SetRightOffset(bool enable=true);
	void SetKeepAtEnd(bool enable=true);
	void Settings();
	void RefreshCore();
	void RefreshCoreData(bool store_cache);
	void OpenContextMenu() {MenuBar::Execute(THISBACK(ContextMenu));}
	void ContextMenu(Bar& bar);
	
	void Init(int symbol, const FactoryDeclaration& decl, int tf);
	virtual void Start();
	virtual void Data();
	virtual String GetTitle();
	
	Core& GetCore() {return *bardata;}
	Color GetBackground() const {return White();}
	Color GetGridColor() const {return GrayColor();}
	int GetSymbol() const {return symbol;}
	int GetTf() const {return tf;}
	int GetWidthDivider() const {return div;}
	int GetShift() const {return shift;}
	bool GetRightOffset() const {return right_offset;}
	bool GetKeepAtEnd() const {return keep_at_end;}
	int GetGraphCount() const {return graphs.GetCount();}
	const GraphCtrl& GetGraph(int i) const {return graphs[i];}
	GraphCtrl& GetGraph(int i) {return graphs[i];}
	
	Chart& SetTimeframe(int tf_id);
	Chart& SetFactory(int f);
	
};



}


#endif
