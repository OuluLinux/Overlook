#ifndef _DataCtrl_GraphCtrl_h_
#define _DataCtrl_GraphCtrl_h_

#include "Container.h"


namespace DataCtrl {
using namespace DataCore;


enum {GRAPH_PRICE_NONE, GRAPH_PRICE_BAR, GRAPH_PRICE_CANDLESTICKS, GRAPH_PRICE_LINE};


class GraphGroupCtrl;


class GraphCtrl : public Ctrl {
	
	Vector<MetaVar> src;
	GraphGroupCtrl* group;
	Point latest_left_down_pt, latest_mouse_move_pt;
	double hi, lo;
	int shift;
	int div, count;
	int datapos;
	int real_screen_count, latest_screen_count;
	bool mouse_left_is_down, show_timevalue_tool;
	bool right_offset;
	
protected:
	friend class GraphGroupCtrl;
	
	
	void ShowTimeValueTool(bool b) {show_timevalue_tool = b;}
	void GotMouseMove(Point p, GraphCtrl* g);
	void DrawBorder(Draw& W);
	void DrawGrid(Draw& W, bool draw_vert_grid);
	void DrawLines(Draw& d, Container& cont);
	void PaintContainerLine(Draw& d, Container& cont, int shift, bool draw_border, int buffer);
	Rect GetGraphCtrlRect();
	
	
public:
	
    typedef GraphCtrl CLASSNAME;
    GraphCtrl();
    
    void PaintCandlesticks(Draw& W, BarData& values);
	void PostRefresh() {PostCallback(THISBACK(GraphCtrlRefresh));}
	void GraphCtrlRefresh() {Refresh();}
	void AddSource(MetaVar src) {this->src.Add(src);}
	void SetRightOffset(bool b=true) {right_offset = b;}
	virtual void Paint(Draw& w);
	virtual bool Key(dword key, int count);
	virtual void MouseMove(Point p, dword keyflags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void RightDown(Point, dword);
	virtual void MiddleDown(Point p, dword keyflags);
	
	void GetDataRange(Container& cont, int buffer);
	int GetCount();
	int GetPos();
	
	void Seek(int pos);
	GraphCtrl& SetGraphGroupCtrl(GraphGroupCtrl& group) {this->group = &group; return *this;}
	
	Callback2<Point, GraphCtrl*> WhenMouseMove;
	Callback1<bool> WhenTimeValueTool;
	
	
};

}

#endif
