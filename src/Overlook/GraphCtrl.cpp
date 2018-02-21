#include "Overlook.h"

namespace Overlook {

// Static vars
static int border = 2;
static int right_off = 50;
static int bottom_off = 15;
static int grid = 40;
static int fonth = 8;
static Font gridfont = Arial(fonth);


// Temporary
#define DATAUP Color(56, 212, 150)
#define DATADOWN Color(28, 85, 150)
#define DATAUP_DARK Color(0, 138, 78)
#define DATADOWN_DARK Color(23, 58, 99)


GraphCtrl::GraphCtrl() {
	chart = 0;
	count = 0;
	shift = 0;
	
	latest_screen_count = 0;
	mouse_left_is_down = false;
	show_timevalue_tool = false;
	right_offset = false;
	
	latest_left_down_pt.x = -1;
	latest_mouse_move_pt.x = -1;
	
	WantFocus();
	IgnoreMouse(false);
}

void GraphCtrl::GetDataRange(int buffer) {
	GraphImage& img = *this->gi;
	ChartImage& chart = *this->ci;
	
	if (buffer >= img.GetVisibleCount()) {
		LOG("GraphCtrl::GetDataRange error no settings for buffer");
		return;
	}
	
	int style = img.GetBuffer(buffer).style;
	if (style == DRAW_NONE) return;
	
	bool skip_zero = false;
	if (style == DRAW_SECTION || style == DRAW_ARROW) skip_zero = true;
	else if (style == DRAW_HISTOGRAM) {
		if (hi < 0) hi = 0;
		if (lo > 0) lo = 0;
	}
	
	if (img.HasMaximum())
		hi = img.GetMaximum();
	else
		hi = img.GetBuffer(buffer).GetMaximum();
	if (img.HasMaximum())
		hi = img.GetMinimum();
	else
		hi = img.GetBuffer(buffer).GetMinimum();
}

void GraphCtrl::Paint(Draw& draw) {
	if (ci == NULL) {draw.DrawRect(GetSize(), White()); return;}
	
	
	System& sys = GetSystem();
	
	Size sz(GetSize());
	ImageDraw w(sz);
	
	w.DrawRect(sz, chart->GetBackground());
	
	/*
	div = chart->GetWidthDivider();
	shift = chart->GetShift();
	
	// Enable offset
	int max_future_bars = 0;
	for(int i = 0; i < src.GetCount(); i++)
		max_future_bars = Upp::max(max_future_bars, src[i]->GetFutureBars());
	
	// Set shift with right offset
	int max_right_offset = Upp::max(max_future_bars, 10);
	int right_offset = (this->right_offset || max_future_bars ? Upp::max(max_right_offset - shift, 0) : 0);
	shift = Upp::max(shift - max_right_offset, 0);
	
	// Get real count of bars in the screen
	real_screen_count = (GetRect().GetWidth() - right_off - 2 * border) / div;
	count = real_screen_count - right_offset;
    latest_screen_count = count;
    
    if (!src[0]) {
		draw.DrawImage(0,0,w);
		return;
	}
    
    int sym = src[0]->GetSymbol();
    int tf = src[0]->GetTf();
    
    if (sym == -1 || tf == -1) {
        LOG("GraphCtrl::Paint invalid tf");
        draw.DrawImage(0,0,w);
        return;
    }
    int data_count = src[0]->GetBufferCount() > 0 ? src[0]->GetBuffer(0).GetCount() : src[0]->GetOutput(0).label.GetCount();
    int max_shift = data_count - count;
    
    if (max_shift < 0) max_shift = 0;
    if (shift > max_shift) {
		shift = max_shift;
		chart->SetShift(shift);
    }
    
	hi = -DBL_MAX;
    lo = DBL_MAX;
    for(int i = 0; i < src.GetCount(); i++) {
		
		Core*& cont_ = src[i];
		if (!cont_) {draw.DrawImage(0,0,w); return;} // just bail out
		
		Core& cont = *cont_;
		// cont.Refresh();
		
		if (dynamic_cast<DataBridge*>(&cont)) {
		    GetDataRange(cont, 1); // low
		    GetDataRange(cont, 2); // high
		}
		else {
			int bufs = cont.GetVisibleCount();
			for(int j = 0; j < bufs; j++) {
				GetDataRange(cont, j);
			}
		}
	}
	
	DrawGrid(w, false);
	
	String graph_label;
	DataBridge* bd_src = NULL;
	for(int i = 0; i < src.GetCount(); i++) {
		Core& cont = *src[i];
		DataBridge* bardata = dynamic_cast<DataBridge*>(&cont);
		if (bardata) {
			bd_src = bardata;
			PaintCandlesticks(w, cont);
		} else {
			if (graph_label.GetCount()) graph_label += ", ";
			
			graph_label += System::GetCoreFactories()[cont.GetFactory()].a;
			
			int bufs = cont.GetVisibleCount();
			for(int j = bufs-1; j >= 0; j--) {
				PaintCoreLine(w, cont, shift,j);
			}
		}
	}
	
	DrawBorders(w, *src.Top());
	DrawBorder(w);
	DrawLines(w, *src.Top());
	
	w.DrawText(5,5, graph_label, StdFont(10), GrayColor());
	
    if (show_timevalue_tool && bd_src) {
        w.DrawLine(latest_mouse_move_pt.x, 0, latest_mouse_move_pt.x, sz.cy, 1, Black());
        w.DrawLine(0, latest_mouse_move_pt.y, sz.cx,  latest_mouse_move_pt.y, 1, Black());
        
        if (latest_mouse_move_pt.y >= 0) {
	        int x = latest_mouse_move_pt.x;
	        int pos = data_count - count + (x - border) / div - shift;
	        last_time_value_tool_pos = pos;
	        if (pos >= 0 && pos < data_count) {
	            Time t = Time(1970,1,1) + bd_src->GetBuffer(4).Get(pos);
	            String timestr = Format("%", t);
	            Font fnt = StdFont(12);
	            Size str_sz = GetTextSize(timestr, fnt);
	            x = x - str_sz.cx / 2;
	            int y = sz.cy-15;
	            w.DrawRect(x, y, str_sz.cx, str_sz.cy, Black());
	            w.DrawText(x, y, timestr, fnt, White());
	        }
        }
    }*/
    
    draw.DrawImage(0,0,w);
}

void GraphCtrl::DrawGrid(Draw& W, bool draw_vert_grid) {
	/*System& sys = GetSystem();
	int gridw, gridh, w, h, y, pos, c;
	double diff, step;
	Rect r(GetGraphCtrlRect());
	Color gridcolor = chart->GetGridColor();
	DataBridge& pb = chart->GetDataBridge();
    int sym = chart->GetSymbol();
    int tf = chart->GetTf();
    if (sym == -1 || tf == -1) return;
    
	y = r.top;
    w = r.GetWidth();
	h = r.GetHeight();
	c = pb.GetBuffer(0).GetCount();
	
	gridw = w / grid + 1;
    gridh = h / grid + 1;
    
	diff = hi - lo;
    step = diff / (1.*h / grid);

	for(int i = 1; i < gridw; i++)
        W.DrawLine(border + i*grid, y, border + i*grid, y+h, PEN_DOT, gridcolor);

	DataBridge* bardata = dynamic_cast<DataBridge*>(src[0]);
	if (!bardata) return;
	ConstBuffer& time_buf = bardata->GetBuffer(4);
	
	int gridstep = 4;
    for(int i = gridw - 1; i >= gridstep; i-=gridstep ) {
        pos = c - count - shift + i * grid/div;
        if (pos >= c || pos < 0) continue;
        
        Time time = Time(1970,1,1) + time_buf.Get(pos);
        String text = Format("%", time);
        Size text_sz = GetTextSize(text, gridfont);
        W.DrawText(border+i*grid-text_sz.cx-5, y+h+2, text, gridfont, gridcolor);
        W.DrawLine(border+i*grid, y+h, border + i * grid, y+h+fonth, PEN_SOLID, gridcolor);
    }
    if (draw_vert_grid) {
	    for(int i = 0; i < gridh; i++) {
	        String text = FormatDoubleFix(hi - i*step, 5, FD_ZEROS); // TODO: digits according to symbol
	        
	        W.DrawLine(border, y+i*grid, w + border, y+i*grid, PEN_DOT, gridcolor);
	        W.DrawText(3+w+2*border, y+i*grid-fonth/2, text, gridfont, gridcolor);
	    }
    } else {
        double v;
        int vy;
        String text;
        
        v = 0;
        vy = (int)(y + (diff - (v - lo)) / diff * h);
        W.DrawLine(r.left, vy, r.left + w, vy, PEN_DOT, gridcolor);
        text = FormatDoubleFix(v, 5, FD_ZEROS);
        W.DrawText(3+w+2*border, vy-fonth/2, text, gridfont, gridcolor);
        
        v = hi;
        vy = (int)(y + (diff - (v - lo)) / diff * h);
        W.DrawLine(r.left, vy, r.left + w, vy, PEN_DOT, gridcolor);
        text = FormatDoubleFix(v, 5, FD_ZEROS);
        W.DrawText(3+w+2*border, vy-fonth/2, text, gridfont, gridcolor);
        
        v = lo;
        vy =(int)(y + (diff - (v - lo)) / diff * h);
        W.DrawLine(r.left, vy, r.left + w, vy, PEN_DOT, gridcolor);
        text = FormatDoubleFix(v, 5, FD_ZEROS);
        W.DrawText(3+w+2*border, vy-fonth/2, text, gridfont, gridcolor);
    }*/
}

void GraphCtrl::DrawBorder(Draw& W) {
	Color gridcolor = chart->GetGridColor();
	Rect r(GetGraphCtrlRect());
    int x, y, w, h;
    
    x = border;
    y = r.top;
    w = r.GetWidth();
    h = r.GetHeight();
    
	Vector<Point> P;
    P << Point(x, y) << Point(x+w, y) << Point(x+w, y+h) << Point(x, y+h) << Point(x,y);
    W.DrawPolyline(P, 1, gridcolor);
}

Rect GraphCtrl::GetGraphCtrlRect() {
	Rect r(GetSize());
	
	r.top += border;
	r.left += border;
	r.right -= border + right_off;
	r.bottom -= border + bottom_off;
    
    return r;
}

void GraphCtrl::PaintCandlesticks(Draw& W) {
	/*DrawGrid(W, true);
    
	int f, pos, x, y, h, c, w;
	double diff;
    Rect r(GetGraphCtrlRect());
	System& sys = GetSystem();
    int sym = chart->GetSymbol();
    int tf = chart->GetTf();
    
    f = 2;
    x = border;
	y = r.top;
    h = r.GetHeight();
    w = r.GetWidth();
	c = values.GetBuffer(0).GetCount();
	diff = hi - lo;
	
	for(int i = 0; i < count; i++) {
        Vector<Point> P;
        double O, H, L, C;
        pos = c - (count + shift - i);
        if (pos >= c || pos < 0) continue;
        
        double open  = values.GetBufferValue(0, pos);
        double low   = values.GetBufferValue(1, pos);
        double high  = values.GetBufferValue(2, pos);
		double close =
			pos+1 < c ?
				values.GetBufferValue(0, pos+1) :
				open;
        
        O = (1 - (open  - lo) / diff) * h;
        H = (1 - (high  - lo) / diff) * h;
        L = (1 - (low   - lo) / diff) * h;
        C = (1 - (close - lo) / diff) * h;
		
		P <<
			Point((int)(x+i*div+f),		(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+O)) <<
			Point((int)(x+(i+1)*div-f),	(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+C)) <<
			Point((int)(x+i*div+f),		(int)(y+O));
        
        {
	        Color c, c2;
	        if (C < O) {c = DATAUP; c2 = DATAUP_DARK;}
	        else {c = DATADOWN; c2 = DATADOWN_DARK;}
	        
	        W.DrawLine(
				(int)(x+(i+0.5)*div), (int)(y+H),
				(int)(x+(i+0.5)*div), (int)(y+L),
				2, c2);
	        W.DrawPolygon(P, c, 1, c2);
        }
    }
	
    DrawBorder(W);
    
    // Draw indicator about window range in total data range
	int shift_x  = c ? (c - shift - count) * w / c : 0;
	int width_x = c ? count * w / c + 1 : 0;
    W.DrawRect(shift_x, 0, width_x, 3, Blue());*/
}

void GraphCtrl::PaintCoreLine(Draw& W, int shift, int buffer) {
	/*r = GetGraphCtrlRect();
    
    bool skip_zero;
    Color value_color = cont.GetBufferColor(buffer);
	System& sys = GetSystem();
    int sym = chart->GetSymbol();
    int tf = chart->GetTf();
    
    x = border;
	y = r.top;
    h = r.GetHeight();
	c = cont.GetBuffer(0).GetCount();
	diff = hi - lo;
	data_shift = cont.GetBuffer(buffer).shift;
	data_begin = cont.GetBuffer(buffer).begin;
	data_count = c;
	skip_zero = false;
	draw_type = 0;
	style = cont.GetBufferStyle(buffer);
	line_width = cont.GetBufferLineWidth(buffer);
	line_style = cont.GetBufferType(buffer);
	
	switch (style) {
		case DRAW_LINE:
			break;
		case DRAW_ARROW:
			draw_type = 2;
			skip_zero = true;
			break;
		case DRAW_HISTOGRAM:
			draw_type = 1;
			break;
		case DRAW_SECTION:
			skip_zero = true;
			break;
		case DRAW_ZIGZAG:
			break;
		case DRAW_NONE:
			draw_type = -1;
			break;
		default:
			break;
	}
	
	if (line_width == 0) draw_type = -1;
	
	buf_count = cont.GetBars();
	
	if (draw_type == 0) {
		Vector<Point> P;
		int begin = 0;
		int end = real_screen_count;
		if (style==DRAW_SECTION) {
			begin -= 200;
			end += 200;
		}
		for(int i = begin; i < end; i++) {
	        double V;
			pos = c - (count + shift - i + data_shift);
	        if (pos >= data_count || pos < data_begin || pos >= buf_count)
				continue;
	        
	        double value = cont.GetBufferValue(buffer, pos);
	        if (value < lo) value = lo;
	        if (value > hi) value = hi;
	        
			int xi = (x+(i+0.5)*div);
			
	        if (skip_zero && value == 0) continue;
	        
	        V = (1 - (value  - lo) / diff) * h;
			
			P << Point((int)xi, (int)(y+V));
		}
		
		if (line_style == STYLE_DASH)				line_width = PEN_DASH;
		else if (line_style == STYLE_DOT)			line_width = PEN_DOT;
		else if (line_style == STYLE_DASHDOT)		line_width = PEN_DASHDOT;
		else if (line_style == STYLE_DASHDOTDOT)	line_width = PEN_DASHDOTDOT;
		
		if (P.GetCount() >= 2)
			W.DrawPolyline(P, line_width, value_color);
	}
	else if (draw_type == 1) {
		for(int i = 0; i < real_screen_count; i++) {
	        double V, Z;
	        pos = c - (count + shift - i + data_shift);
	        if (pos >= data_count || pos < data_begin)
				continue;
	        
	        double value = cont.GetBufferValue(buffer, pos);
	        if (skip_zero && value == 0) continue;
	        V = (1 - (value  - lo) / diff) * h;
			Z = (1 - (0      - lo) / diff) * h;
			int xV = (int)(x+(i+0.25)*div);
			int yV = (int)(y+V);
			int yZ = (int)(y+Z);
			int height = abs(yV-yZ);
			if (value < 0) {
				W.DrawRect(xV, yZ, div/2, height, value_color);
			} else {
				W.DrawRect(xV, yV, div/2, height, value_color);
			}
		}
	}
	else if (draw_type == 2) {
		int chr = cont.GetBufferArrow(buffer);
		String str;
		str.Cat(chr);
		for(int i = 0; i < real_screen_count; i++) {
	        double V;
			pos = c - (count + shift - i + data_shift);
	        if (pos >= data_count || pos < data_begin)
				continue;
	        
	        double value = cont.GetBufferValue(buffer, pos);
	        if (skip_zero && value == 0) continue;
	        V = (1 - (value  - lo) / diff) * h;
	        
	        W.DrawText((int)(x+(i+0.5)*div), (int)(y+V), str, StdFont(), value_color);
		}
	}*/
}

void GraphCtrl::DrawBorders(Draw& d) {
    /*x = border;
    r = GetGraphCtrlRect();
    y = r.top;
	bool draw_label = cont.GetOutputCount() > 0;
	bool draw_label_enabled = cont.GetOutputCount() > 1;
	if (draw_label) {
		int label_data_count = cont.GetOutput(0).label.GetCount();
		int begin = 0;
		int end = real_screen_count;
		VectorBool* labelvec = &cont.GetSignal();
		VectorBool* enabledvec = NULL;
		if (draw_label_enabled) {
			enabledvec = &cont.GetEnabled();
			label_data_count = Upp::min(label_data_count, cont.GetOutput(0).label.GetCount());
		}
		for(int i = begin; i < end; i++) {
	        int pos = label_data_count - (count + shift - i);
	        if (pos < 0 || pos >= label_data_count)
				continue;
	        
	        int xi = (x+(i+0.5)*div);
			
			if (draw_label) {
				bool label = labelvec->Get(pos);
				bool enabled = true;
				if (draw_label_enabled)
					enabled = enabledvec->Get(pos);
				if (enabled) {
					if (label)	d.DrawRect(xi - 2, y+border - 2, 4, 4, Color(255, 96, 96));
					else		d.DrawRect(xi - 2, y+border - 2, 4, 4, Color(104, 99, 255));
				}
			}
		}
	}*/
}

void GraphCtrl::DrawLines(Draw& d) {
	/*if (cont.IsCoreSeparateWindow()) {
		Rect r = GetGraphCtrlRect();
		double diff = hi - lo;
		int h = r.GetHeight();
		int w = r.GetWidth();
		Color grid_color = chart->GetGridColor();
		String text;
		for(int i = 0; i < cont.GetCoreLevelCount(); i++) {
			double value = cont.GetCoreLevelValue(i);
			int type = cont.GetCoreLevelType(i);
			int line_width = cont.GetCoreLevelLineWidth(i);
			
			if (type == STYLE_DASH)				line_width = PEN_DASH;
			else if (type == STYLE_DOT)			line_width = PEN_DOT;
			else if (type == STYLE_DASHDOT)		line_width = PEN_DASHDOT;
			else if (type == STYLE_DASHDOTDOT)	line_width = PEN_DASHDOTDOT;
			else if (line_width == 1)			line_width = PEN_DOT;
			
			int y = (int)(r.top +  (1 - (value  - lo) / diff) * h);
			
			d.DrawLine(r.left, y, r.left+w, y, line_width, grid_color);
			text = FormatDoubleFix(value, 5, FD_ZEROS);
	        d.DrawText(3+w+2*border, y-fonth/2, text, gridfont, grid_color);
		}
		
	} else {
		// TODO: make level-line with level-line-value difference to datacator-line
	}*/
}


bool GraphCtrl::Key(dword key, int count) {
	if (key == K_LEFT) {
		Seek(GetPos() + 10);
		return true;
	}
	if (key == K_RIGHT) {
		Seek(GetPos() - 10);
		return true;
	}
	if (key == K_HOME) {
		Seek(GetCount() - 1);
		return true;
	}
	if (key == K_END) {
		Seek(0);
		return true;
	}
	if (key == K_PAGEDOWN) {
		Seek(GetPos() - latest_screen_count);
		return true;
	}
	if (key == K_PAGEUP) {
		Seek(GetPos() + latest_screen_count);
		return true;
	}
	return false;
}

void GraphCtrl::MouseMove(Point p, dword keyflags) {
	if (mouse_left_is_down && latest_mouse_move_pt.x != -1) {
		int diff = latest_mouse_move_pt.x - p.x;
		Seek(GetPos() - diff);
	}
	latest_mouse_move_pt = p;
	
	if (show_timevalue_tool) {
		WhenMouseMove(p, this);
	}
}

void GraphCtrl::MouseWheel(Point p, int zdelta, dword keyflags) {
	// TODO: if in center area
	Seek(GetPos() + zdelta / 10);
}

void GraphCtrl::LeftDown(Point p, dword keyflags) {
	latest_left_down_pt = p;
	latest_mouse_move_pt = p;
	SetFocus();
	mouse_left_is_down = true;
	if (show_timevalue_tool)
		WhenTimeValueTool(false);
}

void GraphCtrl::LeftUp(Point p, dword keyflags) {
	mouse_left_is_down = false;
}

void GraphCtrl::RightDown(Point, dword) {
	SetFocus();
	chart->OpenContextMenu();
}

void GraphCtrl::MiddleDown(Point p, dword keyflags) {
	WhenTimeValueTool(!show_timevalue_tool);
	WhenMouseMove(p, this);
}

int GraphCtrl::GetCount() {
	if (!chart) return 0;
	/*int c = chart->GetCore().GetBuffer(0).GetCount();
    return c;*/
    Panic("TODO");
    return 0;
}

int GraphCtrl::GetPos() {
	return chart->GetShift();
}

void GraphCtrl::Seek(int pos) {
	pos = Upp::max(0, Upp::min(GetCount()-1, pos));
	chart->SetShift(pos);
	chart->Refresh();
}

void GraphCtrl::GotMouseMove(Point p, GraphCtrl* g) {
	latest_mouse_move_pt.x = p.x;
	if (g == this)
		latest_mouse_move_pt.y = p.y;
	else
		latest_mouse_move_pt.y = -1;
	Refresh();
}

}
