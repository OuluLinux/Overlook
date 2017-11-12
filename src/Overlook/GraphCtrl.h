#ifndef _Overlook_GraphCtrl_h_
#define _Overlook_GraphCtrl_h_

namespace Overlook {

enum {GRAPH_PRICE_NONE, GRAPH_PRICE_BAR, GRAPH_PRICE_CANDLESTICKS, GRAPH_PRICE_LINE};

class Chart;


// Class for different visual settings of a graph
class GraphSettings : public Moveable<GraphSettings> {
	
	
	// Vars
	//Vector<double> settings;
	int type;
	
	
public:
	
	//IndiSettingNode node;
	
	
	
	// Ctors
	GraphSettings() {type = -1;}
	GraphSettings(const GraphSettings& gs) {*this = gs;}
	
	
	// Main funcs
	GraphSettings& operator= (const GraphSettings& gs) {
		//node = gs.node;
		type = gs.type;
		return *this;
	}
	/*GraphSettings& operator= (const IndiSettingNode& in) {
		node = in;
		return *this;
	}*/
	
	
	// Get funcs
	int GetType() const {return type;}
	
	// Set funcs
	void SetType(int i) {type = i;}
	
};

class GraphCtrl : public Ctrl {
	Vector<Core*> src;
	System* base = NULL;
	Point latest_left_down_pt, latest_mouse_move_pt;
	double hi, lo;
	int shift;
	int div, count;
	int datapos;
	int real_screen_count, latest_screen_count;
	bool mouse_left_is_down, show_timevalue_tool;
	bool right_offset;
	
protected:
	friend class Chart;
	
	Chart* chart = NULL;
	
	
	void ShowTimeValueTool(bool b) {show_timevalue_tool = b;}
	void GotMouseMove(Point p, GraphCtrl* g);
	void DrawBorder(Draw& W);
	void DrawGrid(Draw& W, bool draw_vert_grid);
	void DrawLines(Draw& d, Core& cont);
	void PaintCoreLine(Draw& d, Core& cont, int shift, bool draw_border, int buffer);
	Rect GetGraphCtrlRect();
	
	
public:
	
    typedef GraphCtrl CLASSNAME;
    GraphCtrl();
    
    void PaintCandlesticks(Draw& W, Core& values);
	void PostRefresh() {PostCallback(THISBACK(GraphCtrlRefresh));}
	void GraphCtrlRefresh() {Refresh();}
	void AddSource(Core* src) {this->src.Add(src);}
	void SetRightOffset(bool b=true) {right_offset = b;}
	virtual void Paint(Draw& w);
	virtual bool Key(dword key, int count);
	virtual void MouseMove(Point p, dword keyflags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void RightDown(Point, dword);
	virtual void MiddleDown(Point p, dword keyflags);
	
	void GetDataRange(Core& cont, int buffer);
	int GetCount();
	int GetPos();
	
	void Seek(int pos);
	
	Callback2<Point, GraphCtrl*> WhenMouseMove;
	Callback1<bool> WhenTimeValueTool;
	
	
};

}

#endif
