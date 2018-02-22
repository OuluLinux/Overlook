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
	
protected:
	friend class Overlook;
	
	Chart* chart = NULL;
	ChartImage* ci = NULL;
	Vector<GraphImage*> src;
	Point latest_left_down_pt, latest_mouse_move_pt;
	double hi, lo;
	int shift;
	int div, count;
	int datapos;
	int real_screen_count, latest_screen_count;
	int last_time_value_tool_pos = 0;
	bool mouse_left_is_down, show_timevalue_tool;
	bool right_offset;
	
	
	Rect r;
    double diff;
    int pos, x, y, h, c, data_shift, data_begin, data_count, draw_type, style, line_width, line_style;
	int buf_count;
	
protected:
	friend class Chart;
	
	
	
	void ShowTimeValueTool(bool b) {show_timevalue_tool = b;}
	void GotMouseMove(Point p, GraphCtrl* g);
	void DrawBorder(Draw& W);
	void DrawGrid(Draw& W, bool draw_vert_grid);
	void DrawLines(Draw& d, GraphImage& gi);
	void PaintCoreLine(Draw& d, GraphImage& gi, int shift, int buffer);
	void DrawBorders(Draw& d, GraphImage& gi);
	Rect GetGraphCtrlRect();
	GraphImage* FindDataSource();
	
public:
	
    typedef GraphCtrl CLASSNAME;
    GraphCtrl();
    
    void PaintCandlesticks(Draw& W, GraphImage& gi);
	void PostRefresh() {PostCallback(THISBACK(GraphCtrlRefresh));}
	void GraphCtrlRefresh() {Refresh();}
	void SetSource(ChartImage& ci, GraphImage& gi) {this->ci = &ci; src.Clear(); src.Add(&gi);}
	void AddSource(GraphImage& gi) {src.Add(&gi);}
	void SetRightOffset(bool b=true) {right_offset = b;}
	virtual void Paint(Draw& w);
	virtual bool Key(dword key, int count);
	virtual void MouseMove(Point p, dword keyflags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void RightDown(Point, dword);
	virtual void MiddleDown(Point p, dword keyflags);
	
	void GetDataRange(GraphImage& gi, int buffer);
	int GetCount();
	int GetPos();
	bool IsTimeValueToolShown() const {return show_timevalue_tool;}
	
	void Seek(int pos);
	
	Callback2<Point, GraphCtrl*> WhenMouseMove;
	Callback1<bool> WhenTimeValueTool;
	
	
};

}

#endif
