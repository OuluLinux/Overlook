#ifndef _Overlook_GraphGroupCtrl_h_
#define _Overlook_GraphGroupCtrl_h_

namespace Overlook {

class GraphGroupCtrl;

class GraphGroupCtrl : public CustomCtrl {
	
protected:
	
	Pipe* bardata;
	Array<GraphCtrl> graphs;
	Splitter split;
	
	int period;
	int div;
	int shift;
	bool right_offset, keep_at_end;
	
	void AddSeparatePipe(int id, const Vector<double>& settings);
	void AddValuePipe(int id, const Vector<double>& settings);
	void SetTimeValueTool(bool enable);
	void GraphMouseMove(Point pt, GraphCtrl* g);
	void Refresh0() {ParentCtrl::Refresh();}
	
	
public:
	typedef GraphGroupCtrl CLASSNAME;
	GraphGroupCtrl();
	
	void PostRefresh() {PostCallback(THISBACK(Refresh0));}
	void ClearPipes();
	GraphCtrl& AddGraph(Core* src);
	void SetGraph(Core* src);
	void SetShift(int i) {shift = i;}
	void SetRightOffset(bool enable=true);
	void SetKeepAtEnd(bool enable=true);
		
	virtual void Init(Core* src);
	virtual void Start();
	virtual void RefreshData();
	
	Pipe& GetPipe() {return *bardata;}
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
