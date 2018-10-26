#include "Forecaster.h"
#include <plugin/bz2/bz2.h>

namespace Forecast {

ManagerCtrl::ManagerCtrl() {
	
	Title("ManagerCtrl");
	Sizeable().MaximizeBox().MinimizeBox();
	
	Add(seslist.LeftPos(0, 250).VSizePos());
	Add(tabs.HSizePos(250).VSizePos());
	
	seslist.AddColumn("Name");
	seslist.AddColumn("Status");
	seslist.AddColumn("Progress");
	seslist <<= THISBACK(SelectSession);
	tabs.Add(fcast.SizePos(), "Forecast");
	tabs.Add(regenctrl.SizePos(), "Regenerator");
	
	
	Manager& m = GetManager();
	
	
	PeriodicalRefresh();
}

void ManagerCtrl::PeriodicalRefresh() {
	tc.Set(60, THISBACK(PeriodicalRefresh));
	Data();
	switch (tabs.Get()) {
		case 0: fcast.Data(); fcast.Refresh(); break;
		case 1: regenctrl.Data(); regenctrl.Refresh(); break;
	}
}

void ManagerCtrl::SelectSession() {
	int cursor = seslist.GetCursor();
	GetManager().Select(cursor);
	PeriodicalRefresh();
}

void ManagerCtrl::Data() {
	Manager& m = GetManager();
	for(int i = 0; i < m.sessions.GetCount(); i++) {
		Session& ses = m.sessions[i];
		seslist.Set(i, 0, ses.name);
		
		seslist.Set(i, 1, m.GetActiveSession() == &ses ? "Active" : "");
		
		int round = ses.regen.opt.GetRound();
		int max_rounds = ses.regen.opt.GetMaxRounds();
		seslist.Set(i, 2, round * 1000 / max_rounds);
		seslist.SetDisplay(i, 2, ProgressDisplay());
	}
}





void DrawLines::Paint(Draw& d) {
	Size sz(GetSize());
	d.DrawRect(sz, White());
	
	Manager& mgr = GetManager();
	Vector<double>* data0;
	Generator* gen = NULL;
	Regenerator* regen = NULL;
	Session* ses = mgr.GetSelectedSession();
	Heatmap* heatmap = NULL;
	if (!ses) return;
	
	if (type == GENVIEW) {
		gen = &ses->regen.GetGenerator(gen_id);
		if (!gen) return;
		data0 = &gen->real_data;
		heatmap = &gen->image;
	}
	else if (type == HISVIEW) {
		if (!ses->regen.HasGenerators()) return;
		gen = &ses->regen.GetGenerator(0);
		if (!gen) return;
		data0 = &ses->regen.real_data;
		heatmap = &this->image;
	}
	else if (type == OPTSTATS) {
		data0 = &ses->regen.result_errors;
	}
	else if (type == FCASTVIEW) {
		data0 = &data;
		heatmap = &this->image;
	}
	else return;
	
	if (gen) gen->view_lock.Enter();
	
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
		if (regen) {
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
	
	if (gen) gen->view_lock.Leave();
}






RegeneratorCtrl::RegeneratorCtrl() {
	Add(status.SizePos(), "Status");
	
	for(int i = 0; i < GetUsedCpuCores(); i++) {
		GeneratorCtrl& gc = gens.Add();
		gc.SetId(i);
		Add(gc.SizePos(), "Gen " + IntStr(i));
	}
	
	
	status.Add(main_vsplit.HSizePos().VSizePos(0, 30));
	status.Add(optprog.BottomPos(0, 30).HSizePos());
	main_vsplit << main_hsplit << opt_draw;
	main_vsplit.Vert();
	main_vsplit.SetPos(8000);
	main_hsplit << his_list << his_draw;
	main_hsplit.Horz();
	main_hsplit.SetPos(2000);
	
	opt_draw.type = opt_draw.OPTSTATS;
	his_draw.type = opt_draw.HISVIEW;
	
	his_list.AddColumn("#");
	his_list.AddColumn("Gen#");
	his_list.AddColumn("Score");
	his_list <<= THISBACK(SelectHistoryItem);
}

void RegeneratorCtrl::SelectHistoryItem() {
	Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	int cursor = his_list.GetCursor();
	if (cursor < 0 ) return;
	
	int id = his_list.Get(cursor, 0);
	if (id == prev_id)
		return;
	prev_id = id;
	
	ses->regen.result_lock.Enter();
	if (id < 0 || id >= ses->regen.results.GetCount()) {
		ses->regen.result_lock.Leave();
		return;
	}
	const RegenResult& rr = ses->regen.results[id];
	StringStream ss;
	ss << BZ2Decompress(rr.heatmap);
	ses->regen.result_lock.Leave();
	
	ss.Seek(0);
	ss.SetLoading();
	ss % his_draw.image;
	
	his_draw.Refresh();
}

void RegeneratorCtrl::Data() {
	Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	ses->regen.result_lock.Enter();
	for(int i = 0; i < ses->regen.results.GetCount(); i++) {
		const RegenResult& rr = ses->regen.results[i];
		his_list.Set(i, 0, rr.id);
		his_list.Set(i, 1, rr.gen_id);
		his_list.Set(i, 2, rr.err);
	}
	his_list.SetCount(ses->regen.results.GetCount());
	ses->regen.result_lock.Leave();
	
	his_list.SetSortColumn(2, false);
	
	if (!ses->regen.IsInit() && ses->regen.HasGenerators())
		optprog.Set(ses->regen.GetGenerator(0).actual, ses->regen.GetGenerator(0).total);
	else
		optprog.Set(ses->regen.opt.GetRound(), ses->regen.opt.GetMaxRounds());
	
}










ForecastCtrl::ForecastCtrl() {
	Add(fcast_list.LeftPos(0, 200).VSizePos());
	Add(draw.HSizePos(200).VSizePos());
	
	draw.type = draw.FCASTVIEW;
	
	fcast_list.AddColumn("#");
	fcast_list << THISBACK(SelectForecastItem);
}

void ForecastCtrl::Data() {
	
	Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	ses->regen.result_lock.Enter();
	for(int i = 0; i < ses->regen.forecasts.GetCount(); i++) {
		const ForecastResult& fr = ses->regen.forecasts[i];
		fcast_list.Set(i, 0, fr.id);
	}
	fcast_list.SetCount(ses->regen.forecasts.GetCount());
	ses->regen.result_lock.Leave();
	
}

void ForecastCtrl::SelectForecastItem() {
	Manager& mgr = GetManager();
	Session* ses = mgr.GetSelectedSession();
	if (!ses) return;
	
	int cursor = fcast_list.GetCursor();
	if (cursor < 0 ) return;
	
	int id = fcast_list.Get(cursor, 0);
	if (id == prev_id)
		return;
	prev_id = id;
	
	ses->regen.result_lock.Enter();
	if (id < 0 ||id >= ses->regen.forecasts.GetCount()) {
		ses->regen.result_lock.Leave();
		return;
	}
	const ForecastResult& fr = ses->regen.forecasts[id];
	StringStream ss;
	ss << BZ2Decompress(fr.heatmap);
	draw.data <<= fr.data;
	ses->regen.result_lock.Leave();
	
	ss.Seek(0);
	ss.SetLoading();
	ss % draw.image;
	
	draw.Refresh();
}











GeneratorCtrl::GeneratorCtrl() {
	Add(draw.SizePos());
}

}

GUI_APP_MAIN
{
	LOG(GetAmpDevices());
	Forecast::GetManager();
	Forecast::ManagerCtrl().Run();
	
	Thread::ShutdownThreads();
}
