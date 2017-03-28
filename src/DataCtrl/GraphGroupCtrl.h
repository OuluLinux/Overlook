#ifndef _DataCtrl_GraphGroupCtrl_h_
#define _DataCtrl_GraphGroupCtrl_h_

#include "GraphCtrl.h"

namespace DataCtrl {

class GraphGroupCtrl;


class GraphGroupCtrl : public MetaNodeCtrl {
	
protected:
	
	BarData* bardata;
	Array<GraphCtrl> graphs;
	Splitter split;
	
	int period;
	int div;
	int shift;
	bool right_offset, keep_at_end;
	
	void AddSeparateContainer(int id, const Vector<double>& settings);
	void AddValueContainer(int id, const Vector<double>& settings);
	void SetTimeValueTool(bool enable);
	void GraphMouseMove(Point pt, GraphCtrl* g);
	void Refresh0() {ParentCtrl::Refresh();}
	
	
public:
	typedef GraphGroupCtrl CLASSNAME;
	GraphGroupCtrl();
	
	void Init(MetaVar src);
	void PostRefresh() {PostCallback(THISBACK(Refresh0));}
	void ClearContainers();
	GraphCtrl& AddGraph(MetaVar src);
	void SetGraph(MetaVar src);
	void SetShift(int i) {shift = i;}
	void SetRightOffset(bool enable=true);
	void SetKeepAtEnd(bool enable=true);

	virtual String GetKey() const {return "graph";}
	static String GetKeyStatic()  {return "graph";}
	
	virtual void Init();
	virtual void Start();
	virtual void RefreshData();
	
	BarData& GetBarData() {return *bardata;}
	Color GetBackground() {return White();}
	Color GetGridColor() {return GrayColor();}
	int GetPeriod() {return period;}
	int GetWidthDivider() {return div;}
	int GetShift() {return shift;}
	bool GetRightOffset() {return right_offset;}
	bool GetKeepAtEnd() {return keep_at_end;}
	
	GraphGroupCtrl& SetTimeframe(int tf_id);
	
};

}


#endif
