#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

class SnapshotDraw : public Ctrl {
	int snap_id;
	
public:
	SnapshotDraw();
	
	virtual void Paint(Draw& w);
	void SetSnap(int i) {snap_id = i;}
};

class EquityGraph : public Ctrl {
	
protected:
	Agent* agent;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	int type;
	
public:
	typedef EquityGraph CLASSNAME;
	EquityGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent, int type) {this->agent = &agent; clr = Color(49, 28, 150); this->type = type;}
	
};

class ResultGraph : public Ctrl {
	Agent* agent;
	Vector<Point> polyline;
	int type = -1;
	
public:
	typedef ResultGraph CLASSNAME;
	ResultGraph();
	
	virtual void Paint(Draw& w);
	
	void SetAgent(Agent& agent, int type) {this->agent = &agent; this->type = type;}
	
};

class HeatmapTimeView : public Ctrl {
	Callback1<Draw*> fn;
	
	Array<Image> lines;
	Vector<double> tmp;
	int mode;
	
public:
	typedef HeatmapTimeView CLASSNAME;
	HeatmapTimeView();
	
	virtual void Paint(Draw& d);
	
	template <class T>
	void PaintTemplate(Draw* dptr, void* agentptr) {
		Draw& d = *dptr;
		T& agent = *(T*)agentptr;
		
		Size sz = GetSize();
		
		int total_output = 0;
		total_output += agent.data.mul1.output.GetLength();
		total_output += agent.data.add1.output.GetLength();
		total_output += agent.data.tanh.output.GetLength();
		total_output += agent.data.mul2.output.GetLength();
		total_output += agent.data.add2.output.GetLength();
		tmp.SetCount(total_output);
		
		int cur = 0;
		for(int i = 0; i < agent.data.mul1.output.GetLength(); i++) tmp[cur++] = agent.data.mul1.output.Get(i);
		for(int i = 0; i < agent.data.add1.output.GetLength(); i++) tmp[cur++] = agent.data.add1.output.Get(i);
		for(int i = 0; i < agent.data.tanh.output.GetLength(); i++) tmp[cur++] = agent.data.tanh.output.Get(i);
		for(int i = 0; i < agent.data.mul2.output.GetLength(); i++) tmp[cur++] = agent.data.mul2.output.Get(i);
		for(int i = 0; i < agent.data.add2.output.GetLength(); i++) tmp[cur++] = agent.data.add2.output.Get(i);
		
		
		ImageBuffer ib(total_output, 1);
		RGBA* it = ib.Begin();
		double max = 0.0;
		
		for(int j = 0; j < tmp.GetCount(); j++) {
			double d = tmp[j];
			double fd = fabs(d);
			if (fd > max) max = fd;
		}
		
		for(int j = 0; j < tmp.GetCount(); j++) {
			double d = tmp[j];
			byte b = (byte)(fabs(d) / max * 255);
			if (d >= 0)	{
				it->r = 0;
				it->g = 0;
				it->b = b;
				it->a = 255;
			}
			else {
				it->r = b;
				it->g = 0;
				it->b = 0;
				it->a = 255;
			}
			it++;
		}
		
		lines.Add(ib);
		while (lines.GetCount() > sz.cy) lines.Remove(0);
		
		
		ImageDraw id(sz);
		id.DrawRect(sz, Black());
		for(int i = 0; i < lines.GetCount(); i++) {
			Image& ib = lines[i];
			id.DrawImage(0, i, sz.cx, 1, ib);
		}
		
		
		d.DrawImage(0, 0, id);
	}
	
	template <class T>
	void SetAgent(T& t) {this->fn = THISBACK1(PaintTemplate<T>, &t); }
	
};

class RewardGraph : public Ctrl {
	Agent* agent = NULL;
	int type = -1;
	Color clr;
	Vector<Point> polyline;
	
public:
	typedef RewardGraph CLASSNAME;
	RewardGraph();
	
	virtual void Paint(Draw& d);
	
	void SetAgent(Agent& agent, int type);
	
};

class TrainingCtrl : public ParentCtrl {
	
protected:
	friend class SnapshotDraw;
	Agent* agent;
	Splitter hsplit;
	int type;
	
	Label epoch;
	SnapshotDraw draw;
	ResultGraph result;
	EquityGraph stats;
	RewardGraph reward;
	HeatmapTimeView timescroll;
	ArrayCtrl trade;
	
	
	Splitter bsplit, vsplit;
	
public:
	typedef TrainingCtrl CLASSNAME;
	TrainingCtrl();
	
	void Data();
	void ApplySettings();
	void SetAgent(Agent& agent, int type);
	
};

class SnapshotCtrl : public ParentCtrl {
	Splitter hsplit;
	ArrayCtrl list;
	SnapshotDraw draw;
	
public:
	typedef SnapshotCtrl CLASSNAME;
	SnapshotCtrl();
	
	void Data();
};

class ExportCtrl;

class DataGraph : public Ctrl {
	
protected:
	ExportCtrl* dc;
	Color clr;
	Vector<Point> polyline;
	Vector<double> last;
	
public:
	typedef DataGraph CLASSNAME;
	DataGraph(ExportCtrl* dc);
	
	virtual void Paint(Draw& w);
	
};

class SignalGraph : public Ctrl {
	ExportCtrl* dc;
	bool mode = 0;
	
	enum {MODE_FILTER, MODE_SIGNAL};
	
public:
	typedef SignalGraph CLASSNAME;
	SignalGraph(ExportCtrl* dc);
	
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags) {mode = !mode; Refresh();}
	
};

class ExportCtrl : public ParentCtrl {
	DataGraph graph;
	SignalGraph siggraph;
	Label data;
	SliderCtrl timeslider;
	Splitter hsplit;
	ArrayCtrl siglist, trade;
	Vector<int> poslist;
	int last_pos;
	
public:
	typedef ExportCtrl CLASSNAME;
	ExportCtrl();
	
	void Data();
	void GuiData();
	
	
	Vector<double> equity;
};

}

#endif
