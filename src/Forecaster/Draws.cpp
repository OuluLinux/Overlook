#include "Forecaster.h"

namespace Forecast {

void DrawLines::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	Manager& mgr = GetManager();
	const Vector<double>* data0 = NULL;
	Session& ses = mgr.GetSession(ses_id);
	Heatmap* heatmap = NULL;
	int max_count = 0;
	
	/*if (type == GENVIEW) {
		gen = &ses->regen.GetHeatmapLooper(gen_id);
		if (!gen) return;
		data0 = gen->real_data;
		heatmap = &gen->image;
		max_count = HeatmapLooper::errtest_size;
	}
	else if (type == HISVIEW) {
		if (!ses->regen.HasHeatmapLoopers()) return;
		gen = &ses->regen.GetHeatmapLooper(0);
		if (!gen) return;
		data0 = &ses->regen.real_data;
		heatmap = &this->image;
		max_count = HeatmapLooper::errtest_size;
	}
	else if (type == OPTSTATS) {
		data0 = &ses->regen.result_errors;
	}
	else if (type == FCASTVIEW) {
		data0 = &data;
		heatmap = &this->image;
	}
	else return;
	
	if (gen) gen->view_lock.Enter();*/
	if (!data0) return;
	
	double zero_line = 1.0;
	
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
	if (max_count && max_steps > max_count) max_steps = max_count;
	
	if (max_steps > 1 && max >= min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		if (heatmap) {
			int mult = 5;
			double ystep = (max - min) / ((double)sz.cy / mult);
			typedef Tuple<int, int, double> XYP;
			Vector<XYP> data;
			double max_pres = 0;
			for(int i = 0; i < sz.cx; i += mult) {
				int k = i * max_steps / sz.cx;
				int y = 0;
				for(double j = min; j < max; j += ystep) {
					double pres = heatmap->Get(k, j);
					data.Add(XYP(i, sz.cy - y, pres));
					if (pres > max_pres)
						max_pres = pres;
					y += mult;
				}
			}
			
			for(int i = 0; i < data.GetCount(); i++) {
				const XYP& xyp = data[i];
				double pres = xyp.c;
				pres = 255 - pres / max_pres * 255;
				if (pres < 0) pres = 0;
				if (pres > 255) pres = 255;
				d.DrawRect(xyp.a, xyp.b, mult, mult, GrayColor(pres));
			}
		}
		
		if (max_steps >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < max_steps; j++) {
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
		/*if (regen) {
			int y = 20;
			String str = DblStr(regen->GetBestEnergy());
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(16, y, str, fnt, Black());
		}
		if (regen) {
			int y = 40;
			String str = DblStr(regen->GetLastEnergy());
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(16, y, str, fnt, Black());
		}*/
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
	
	//if (gen) gen->view_lock.Leave();
}

}
