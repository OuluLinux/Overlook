#include "Overlook.h"

namespace Overlook {

void EvolutionGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	Vector<double>& fus_results = esys.fusion.fus_results;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = fus_results.GetCount();
	for(int j = 0; j < count; j++) {
		double d = fus_results[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = fus_results[j];
			int y = (int)(sz.cy - (v - min) / diff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 1, Color(193, 255, 255));
		for(int j = 0; j < polyline.GetCount(); j++) {
			const Point& p = polyline[j];
			id.DrawRect(p.x-1, p.y-1, 3, 3, Blue());
		}
	}
	
	id.DrawText(3, 3, IntStr(count), Monospace(15), Black());
	
	w.DrawImage(0, 0, id);
}





ExpertOptimizerCtrl::ExpertOptimizerCtrl() {
	Add(vsplit.SizePos());
	
	vsplit << graph << hsplit;
	vsplit.Vert();
	vsplit.SetPos(3333);
	
	hsplit << pop << unit;
	hsplit.Horz();
	
	pop.AddColumn("Accuracy");
	pop.AddColumn("Test 0 equity");
	pop.AddColumn("Test 0 dd");
	pop.AddColumn("Test 1 equity");
	pop.AddColumn("Test 1 dd");
	
	unit.AddColumn("Key");
	unit.AddColumn("Value");
}

void ExpertOptimizerCtrl::Data() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	const Vector<FusionConf>& fus_confs = esys.fusion.fus_confs;
	
	graph.Refresh();
	
	for(int i = 0; i < fus_confs.GetCount(); i++) {
		const FusionConf& conf = fus_confs[i];
		
		pop.Set(i, 0, Format("%2!,n", conf.accuracy));
		pop.Set(i, 1, Format("%2!,n", conf.test0_equity));
		pop.Set(i, 2, Format("%2!,n", conf.test0_dd));
		pop.Set(i, 3, Format("%2!,n", conf.test1_equity));
		pop.Set(i, 4, Format("%2!,n", conf.test1_dd));
	}
	
	int cursor = pop.GetCursor();
	if (cursor >= 0 && cursor < fus_confs.GetCount()) {
		const FusionConf& conf = fus_confs[cursor];
		ArrayCtrlPrinter printer(unit);
		conf.Print(printer);
	}
	else unit.Clear();
}






ExpertRealCtrl::ExpertRealCtrl() {
	Add(refresh_now.LeftPos(2,96).TopPos(2,26));
	Add(last_update.LeftPos(100,200).TopPos(2,26));
	Add(hsplit.HSizePos().VSizePos(30));
	
	refresh_now.SetLabel("Refresh now");
	refresh_now <<= THISBACK(RefreshNow);
	last_update.SetLabel("Last update:");
	
	hsplit << pop << unit;
	hsplit.Horz();
	
	pop.AddColumn("Accuracy");
	pop.AddColumn("Test 0 equity");
	pop.AddColumn("Test 0 dd");
	pop.AddColumn("Test 1 equity");
	pop.AddColumn("Test 1 dd");
	
	unit.AddColumn("Key");
	unit.AddColumn("Value");
}

void ExpertRealCtrl::RefreshNow() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	esys.forced_update = true;
}

void ExpertRealCtrl::Data() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	last_update.SetLabel("Last update: " + Format("%", esys.last_update));
	const Vector<FusionConf>& fus_confs = esys.rt_confs;
	
	for(int i = 0; i < fus_confs.GetCount(); i++) {
		const FusionConf& conf = fus_confs[i];
		
		pop.Set(i, 0, Format("%2!,n", conf.accuracy));
		pop.Set(i, 1, Format("%2!,n", conf.test0_equity));
		pop.Set(i, 2, Format("%2!,n", conf.test0_dd));
		pop.Set(i, 3, Format("%2!,n", conf.test1_equity));
		pop.Set(i, 4, Format("%2!,n", conf.test1_dd));
	}
	pop.SetCount(fus_confs.GetCount());
	
	int cursor = pop.GetCursor();
	if (cursor >= 0 && cursor < fus_confs.GetCount()) {
		const FusionConf& conf = fus_confs[cursor];
		ArrayCtrlPrinter printer(unit);
		conf.Print(printer);
	}
	else unit.Clear();
}

}
