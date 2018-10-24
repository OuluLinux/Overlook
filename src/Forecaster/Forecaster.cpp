#include "Forecaster.h"

namespace Forecast {

Test1::Test1() {
	draw0.t = this;
	draw0.type = 0;
	hisctrl.draw.t = this;
	hisctrl.draw.type = 1;
	
	Title("Test 1");
	Sizeable().MaximizeBox().MinimizeBox();
	
	Add(tabs.SizePos());
	tabs.Add(draw0.SizePos(), "Forecast");
	tabs.Add(hisctrl.SizePos(), "History");
	
	LoadData();
	
	regen.real_data = &real_data;
	
	PeriodicalRefresh();
}

void Test1::PeriodicalRefresh() {
	tc.Set(60, THISBACK(PeriodicalRefresh));
	switch (tabs.Get()) {
		case 0: draw0.Refresh(); break;
		case 1: hisctrl.Refresh(); break;
	}
}

void Test1::LoadData() {
	FileIn fin(ConfigFile("EURUSD0.bin"));
	
	while (!fin.IsEof()) {
		double d;
		fin.Get(&d, sizeof(double));
		real_data.Add(d);
	}
	//#ifdef flagDEBUG
	int size = 1440*7*4;
	//int begin = real_data.GetCount() * 0.3;
	int begin = real_data.GetCount() - size;
	real_data.Remove(0, begin);
	real_data.SetCount(size);
	//#endif
}

void Test1::Train() {
	//TimeStop ts;
	
	int step = 5;
	
	while (!Thread::IsShutdownThreads() && running) {
		
		regen.Iterate();
		
		forecast <<= regen.data;
		break;
		//Sleep(3000);
		
		/*if (regen.trains_total % 3000 == 0) {
			gen.GenerateData(NULL);
		}*/
		
				
		/*if (ts.Elapsed() > 60) {
			PostCallback(THISBACK(Refresh0));
			ts.Reset();
		}*/
	}
	
	PostCallback(THISBACK(Refresh0));
	running = false;
	stopped = true;
}

void DrawLines::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	Vector<double>* data0;
	Vector<double>* data1;
	Generator* gen = NULL;
	if (type == 0) {
		data0 = &t->regen.gen[0].real_data;
		data1 = &t->regen.gen[0].real_data;
		gen = &t->regen.gen[0];
	} else {
		data0 = &t->regen.gen[0].real_data;
		data1 = &t->regen.gen[0].real_data;
		gen = &t->regen.gen[0];
	}
	
	gen->view_lock.Enter();
	
	double zero_line = 1.0;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = -DBL_MAX;
	
	int max_steps = 0;
	int count0 = data0->GetCount();
	int count1 = data1->GetCount();
	for(int j = 0; j < count0; j++) {
		double d = (*data0)[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	for(int j = 0; j < count1; j++) {
		double d = (*data1)[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	max += (max - min) * 0.125;
	min -= (max - min) * 0.125;
	max_steps = Upp::min(count0, count1);
	
	if (max_steps > 1 && max >= min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		if (gen) {
			int mult = 5;
			double ystep = (max - min) / ((double)sz.cy / mult);
			typedef Tuple<int, int, double> XYP;
			Vector<XYP> data;
			double max_pres = 0;
			for(int i = 0; i < sz.cx; i += mult) {
				int k = i * max_steps / sz.cx;
				int y = 0;
				for(double j = min; j < max; j += ystep) {
					double pres = gen->image.Get(k, j);
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
			
			polyline.SetCount(0);
			for(int j = 0; j < max_steps; j++) {
				double v = (*data1)[j];
				last = v;
				int x = (int)(j * xstep);
				int y = (int)(sz.cy - (v - min) / diff * sz.cy);
				polyline.Add(Point(x, y));
				if (v > peak) peak = v;
			}
			if (polyline.GetCount() >= 2)
				d.DrawPolyline(polyline, 1, Color(56, 42, 150));
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
			int y = 20;
			String str = DblStr(t->regen.opt.GetBestEnergy());
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(16, y, str, fnt, Black());
		}
		{
			int y = 40;
			String str = DblStr(t->regen.last_energy);
			Size str_sz = GetTextSize(str, fnt);
			d.DrawRect(16, y, str_sz.cx, str_sz.cy, White());
			d.DrawText(16, y, str, fnt, Black());
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
	
	gen->view_lock.Leave();
}



HistoryCtrl::HistoryCtrl() {
	Add(indi.TopPos(0, 30).LeftPos(0, 400));
	Add(draw.HSizePos(0).VSizePos(30));
}

}

GUI_APP_MAIN
{
	LOG(GetAmpDevices());
	
	One<Forecast::Test1> t;
	t.Create();
	t->Start();
	t->Run();
}
