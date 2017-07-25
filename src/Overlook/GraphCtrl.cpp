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
#define DATAUP Color(126, 229, 52)
#define DATADOWN Color(229, 52, 78)
#define DATAUP_DARK Color(59, 107, 24);
#define DATADOWN_DARK Color(84, 19, 29);


GraphCtrl::GraphCtrl() {
	group = 0;
	count = 0;
	shift = 0;
	base = NULL;
	
	latest_screen_count = 0;
	mouse_left_is_down = false;
	show_timevalue_tool = false;
	right_offset = false;
	
	latest_left_down_pt.x = -1;
	latest_mouse_move_pt.x = -1;
	
	WantFocus();
	IgnoreMouse(false);
}

void GraphCtrl::GetDataRange(Core& cont, int buffer) {
	
	if (buffer >= cont.GetVisibleCount()) {
		LOG("GraphCtrl::GetDataRange error no settings for buffer");
		return;
	}
	
	int style = cont.GetBufferStyle(buffer);
	if (style == DRAW_NONE) return;
	
	bool skip_zero = false;
	if (style == DRAW_SECTION || style == DRAW_ARROW) skip_zero = true;
	else if (style == DRAW_HISTOGRAM) {
		if (hi < 0) hi = 0;
		if (lo > 0) lo = 0;
	}
	
	int c = cont.GetBars();
	bool get_hi = !cont.HasMaximum();
	bool get_lo = !cont.HasMinimum();
	bool get_any = get_hi || get_lo;
	if (!get_hi)
		hi = cont.GetMaximum();
	if (!get_lo)
		lo = cont.GetMinimum();
	if (!get_any) return;
    int data_shift = cont.GetBuffer(buffer).shift;
    int data_begin = cont.GetBuffer(buffer).begin;
    ASSERT(data_begin >= 0);
    
    if (cont.GetBuffer(buffer).IsEmpty()) return;
    
	for(int i = 0; i < count; i++) {
        int pos = c - 1 - (shift + i + data_shift);
        if (pos > cont.GetBuffer(buffer).GetCount()) continue;
        if (pos >= c || pos < data_begin) continue;
        double value = cont.GetBufferValue(buffer, pos);
        if (skip_zero && value == 0) continue;
		if (get_hi && value > hi)
			hi = value;
		if (get_lo && value  < lo)
			lo = value;
    }
}

void GraphCtrl::Paint(Draw& draw) {
	if (src.IsEmpty()) {draw.DrawRect(GetSize(), White()); return;}
	
	if (!base) {
		base = &src[0]->GetSystem();
		if (!base) {draw.DrawRect(GetSize(), White()); return;}
	}
	
	System& bs = *base;
	
	Size sz(GetSize());
	ImageDraw w(sz);
	
	w.DrawRect(sz, group->GetBackground());
	
	
	div = group->GetWidthDivider();
	shift = group->GetShift();
	
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
    
    int tf = src[0]->GetTf();
    if (tf == -1) {
        LOG("GraphCtrl::Paint invalid tf");
        draw.DrawImage(0,0,w);
        return;
    }
    int data_count = bs.GetCountTf(tf);
    int max_shift = data_count - count;
    
    if (max_shift < 0) max_shift = 0;
    if (shift > max_shift) {
		shift = max_shift;
		group->SetShift(shift);
    }
    
	hi = -DBL_MAX;
    lo = DBL_MAX;
    for(int i = 0; i < src.GetCount(); i++) {
		
		Core*& cont_ = src[i];
		if (!cont_) {draw.DrawImage(0,0,w); return;} // just bail out
		
		Core& cont = *cont_;
		cont.Refresh();
		
		if (dynamic_cast<BarData*>(&cont)) {
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
	
	String graph_label;
	for(int i = 0; i < src.GetCount(); i++) {
		Core& cont = *src[i];
		BarData* bardata = dynamic_cast<BarData*>(&cont);
		if (bardata) {
			PaintCandlesticks(w, cont);
		} else {
			if (graph_label.GetCount()) graph_label += ", ";
			
			graph_label += System::GetCtrlFactories()[cont.GetFactory()].a;
			
			int bufs = cont.GetVisibleCount();
			for(int j = 0; j < bufs; j++) {
				PaintCoreLine(w, cont, shift, i==0, j);
			}
		}
	}
	
	w.DrawText(5,5, graph_label, StdFont(10), GrayColor());
	
    if (show_timevalue_tool) {
        w.DrawLine(latest_mouse_move_pt.x, 0, latest_mouse_move_pt.x, sz.cy, 1, Black());
        w.DrawLine(0, latest_mouse_move_pt.y, sz.cx,  latest_mouse_move_pt.y, 1, Black());
        
        if (latest_mouse_move_pt.y >= 0) {
	        int x = latest_mouse_move_pt.x;
	        int pos = bs.GetCountTf(tf) - count + (x - border) / div - shift;
	        if (pos >= 0 && pos < data_count) {
	            Time t = bs.GetTimeTf(tf, pos);
	            String timestr = Format("%", t);
	            Font fnt = StdFont(12);
	            Size str_sz = GetTextSize(timestr, fnt);
	            x = x - str_sz.cx / 2;
	            int y = sz.cy-15;
	            w.DrawRect(x, y, str_sz.cx, str_sz.cy, Black());
	            w.DrawText(x, y, timestr, fnt, White());
	        }
        }
    }
    
    draw.DrawImage(0,0,w);
}

void GraphCtrl::DrawGrid(Draw& W, bool draw_vert_grid) {
	
	System& bs = *base;
	
	int gridw, gridh, w, h, y, pos, c;
	double diff, step;
	Rect r(GetGraphCtrlRect());
	Color gridcolor = group->GetGridColor();
	Core& pb = group->GetCore();
    int tf = group->GetTf();
    if (tf == -1) return;
    
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

	int gridstep = 4;
    for(int i = gridw - 1; i >= gridstep; i-=gridstep ) {
        pos = c - count - shift + i * grid/div;
        if (pos >= c || pos < 0) continue;
        
        Time time = bs.GetTimeTf(tf, pos);
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
    }
}

void GraphCtrl::DrawBorder(Draw& W) {
	Color gridcolor = group->GetGridColor();
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

void GraphCtrl::PaintCandlesticks(Draw& W, Core& values) {
	System& bs = group->GetCore().GetSystem();
	
	DrawGrid(W, true);
    
	int f, pos, x, y, h, c, w;
	double diff;
    Rect r(GetGraphCtrlRect());
    int tf = group->GetTf();
    
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
    W.DrawRect(shift_x, 0, width_x, 3, Blue());
}

void GraphCtrl::PaintCoreLine(Draw& W, Core& cont, int shift, bool draw_border, int buffer) {
	ASSERT(base);
	System& bs = *base;
	
	if (draw_border) {
		DrawGrid(W, false);
	}
	
	int pos, x, y, h, c, data_shift, data_begin, data_count, draw_type, style, line_width, line_style;
	double diff;
    Rect r(GetGraphCtrlRect());
    bool skip_zero;
    Color value_color = cont.GetBufferColor(buffer);
    int tf = group->GetTf();
    
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
	
	int buf_count = cont.GetBars();
	
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
	        if (skip_zero && value == 0) continue;
	        V = (1 - (value  - lo) / diff) * h;
			
			P << Point((int)(x+(i+0.5)*div), (int)(y+V));
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
	}
	
	if (draw_border) {
		DrawBorder(W);
		DrawLines(W, cont);
	}
}

void GraphCtrl::DrawLines(Draw& d, Core& cont) {
	if (cont.IsCoreSeparateWindow()) {
		Rect r = GetGraphCtrlRect();
		double diff = hi - lo;
		int h = r.GetHeight();
		int w = r.GetWidth();
		Color grid_color = group->GetGridColor();
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
	}
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
}

void GraphCtrl::MiddleDown(Point p, dword keyflags) {
	WhenTimeValueTool(!show_timevalue_tool);
	WhenMouseMove(p, this);
}

int GraphCtrl::GetCount() {
	if (!group) return 0;
	int c = group->GetCore().GetBars();
    return c;
}

int GraphCtrl::GetPos() {
	return group->GetShift();
}

void GraphCtrl::Seek(int pos) {
	pos = Upp::max(0, Upp::min(GetCount()-1, pos));
	group->SetShift(pos);
	group->Refresh();
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
