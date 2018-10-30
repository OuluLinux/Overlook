#include "Agent.h"

namespace Agent {

void DrawLines::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	Manager& mgr = GetManager();
	const Vector<double>* data0 = NULL;
	
	if (!t) return;
	
	if (type == DQN) {
		data0 = &t->results;
	}
	else return;
	
	if (!data0) return;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = -DBL_MAX;
	
	int max_steps = 0;
	int count0 = data0->GetCount();
	for(int j = 0; j < count0; j++) {
		double d = (*data0)[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	max += (max - min) * 0.125;
	min -= (max - min) * 0.125;
	max_steps = count0;
	
	if (max_steps > 1 && max >= min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		
		if (max_steps >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < max_steps; j++) {
				if (j >= data0->GetCount()) break;
				double v = (*data0)[j];
				last = v;
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				polyline.Add(Point(x, y));
				if (v > peak) peak = v;
			}
			if (polyline.GetCount() >= 2)
				d.DrawPolyline(polyline, 1, Color(81, 145, 137));
		}
		
		{
			int y = 0;
			String str = DblStr(peak);
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(16, y, str, fnt, Black());
		}
		{
			int y = 0;
			String str = DblStr(last);
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(sz.cx - 16 - str_sz.cx, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(sz.cx - 16 - str_sz.cx, y, str, fnt, Black());
		}
		{
			int y = (int)(sz.cy - (zero_line - min) / diff * sz.cy);
			d.DrawLine(0, y, sz.cx, y, 1, Black());
			if (zero_line != 0.0) {
				int y = sz.cy - 10;
				String str = DblStr(zero_line);
				Size str_sz = GetTextSize(str, fnt);
				d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
				d.DrawText(16, y, str, fnt, Black());
			}
		}
	}
	
}

}
