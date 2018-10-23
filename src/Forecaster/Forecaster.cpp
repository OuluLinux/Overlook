#include "Forecaster.h"

namespace Forecast {

Test1::Test1() {
	data0 = &real_forecast;
	data1 = &forecast;
	/*data0 = &real_data;
	data1 = &real_data;*/
	draw0.t = this;
	
	Title("Test 1");
	Sizeable().MaximizeBox().MinimizeBox();
	
	Add(draw0.SizePos());
	
	//GenerateData();
	LoadData();
	
	regen.real_data = &real_data;
	
	PeriodicalRefresh();
}

void Test1::LoadData() {
	FileIn fin(ConfigFile("test.bin"));
	
	while (!fin.IsEof()) {
		double d;
		fin.Get(&d, sizeof(double));
		real_data.Add(d);
	}
	
	for(int i = Generator::data_count; i < real_data.GetCount(); i++) {
		real_forecast.Add(real_data[i]);
	}
	real_data.SetCount(Generator::data_count);
	forecast.SetCount(real_forecast.GetCount());
}

void Test1::GenerateData() {
	
	real_data.SetCount(Generator::data_count);
	
	
	// Get and refresh indicators
	Vector<CoreItem> work_queue;
	Vector<FactoryDeclaration> decl;
	AddDefaultDeclarations(decl);
	System::GetCoreQueue(real_data, work_queue, decl);
	
	
	
	gen.ResetGenerate();
	for(int i = 0; i < 3; i++)
		gen.AddRandomPressure();
	
	
	
	Vector<ConstLabelSignal*> lbls;
	System::GetLabels(work_queue, lbls);
	
	BitStream stream;
	stream.SetColumnCount(lbls.GetCount());
	stream.SetCount(10);
	
	Vector<double> params;
	for(int i = 0; i < lbls.GetCount(); i++)
		for(int j = 0; j < 3; j++)
			params.Add(Randomf() * 2.0 - 1.0);
	
	for(int i = 0; i < Generator::data_count; i++) {
		
		gen.Iteration(stream, i, params);
		real_data[i] = gen.price;
		
		for(int j = 0; j < work_queue.GetCount(); j++) {
			CoreItem& ci = work_queue[j];
			ci.core->SetBars(i+1);
			ci.core->Refresh(true);
		}
		
		stream.SetBit(0);
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
		stream.SetBit(0);
	}
	
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

void Test1::DrawLines::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	double zero_line = 1.0;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	double last = 0.0;
	double peak = -DBL_MAX;
	
	int max_steps = 0;
	int count0 = t->data0->GetCount();
	int count1 = t->data1->GetCount();
	for(int j = 0; j < count0; j++) {
		double d = (*t->data0)[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	/*for(int j = 0; j < count1; j++) {
		double d = (*t->data1)[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}*/
	max_steps = Upp::min(count0, count1);
	
	if (max_steps > 1 && max >= min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (max_steps - 1);
		Font fnt = Monospace(10);
		
		/*int mult = 5;
		double ystep = (max - min) / ((double)sz.cy / mult);
		for(int i = 0; i < sz.cx; i += mult) {
			int k = i * max_steps / sz.cx;
			int y = 0;
			for(double j = min; j < max; j += ystep) {
				double pres = t->gen.a.Get(j).pres;
				pres = 255 - pres / 2550 * 255;
				if (pres < 0) pres = 0;
				if (pres > 255) pres = 255;
				d.DrawRect(i, sz.cy - y, mult, mult, GrayColor(pres));
				y += mult;
			}
		}*/
		
		if (max_steps >= 2) {
			polyline.SetCount(0);
			for(int j = 0; j < max_steps; j++) {
				double v = (*t->data0)[j];
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
				double v = (*t->data1)[j];
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
