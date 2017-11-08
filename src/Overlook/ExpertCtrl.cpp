#include "Overlook.h"

namespace Overlook {

void SourceProcessingGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	
	const Vector<AccuracyConf>& acc_list = esys.acc_list;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = acc_list.GetCount();
	for(int j = 0; j < count; j++) {
		double d = acc_list[j].test_valuehourfactor;
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = acc_list[j].test_valuehourfactor;
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


void SourceEquityGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	
	const Vector<double>& last_test_equity = esys.simcore.last_test_equity;
	const Vector<double>& last_test_spreadcost = esys.simcore.last_test_spreadcost;
	
	double emin = +DBL_MAX;
	double smin = +DBL_MAX;
	double emax = -DBL_MAX;
	double smax = -DBL_MAX;
	
	int count = Upp::min(last_test_equity.GetCount(), last_test_spreadcost.GetCount());
	for(int j = 0; j < count; j++) {
		double ed = last_test_equity[j];
		if (ed > emax) emax = ed;
		if (ed < emin) emin = ed;
		double sd = last_test_spreadcost[j];
		if (sd > smax) smax = sd;
		if (sd < smin) smin = sd;
	}
	
	if (count >= 2 && emax > emin) {
		double ediff = emax - emin;
		double sdiff = smax - smin;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = last_test_equity[j];
			int y = (int)(sz.cy - (v - emin) / ediff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 2, Color(28, 127, 150));
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = last_test_spreadcost[j];
			int y = (int)(sz.cy - (v - smin) / sdiff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 1, Color(198, 85, 0));
	}
	
	if (count)
		id.DrawText(3, 3, DblStr(last_test_equity.Top()), Monospace(15), Black());
	
	w.DrawImage(0, 0, id);
}



ExpertOptimizerCtrl::ExpertOptimizerCtrl() {
	Add(vsplit.SizePos());
	
	vsplit << graph << hsplit;
	vsplit.Vert();
	vsplit.SetPos(3333);
	
	hsplit << pop << unit;
	hsplit.Horz();
	hsplit.SetPos(8000);
	
	pop.AddColumn("id");
	pop.AddColumn("symbol");
	pop.AddColumn("label_id");
	pop.AddColumn("period");
	pop.AddColumn("ext");
	pop.AddColumn("label");
	pop.AddColumn("fastinput");
	pop.AddColumn("labelpattern");
	pop.AddColumn("ext_dir");
	pop.AddColumn("valuefactor");
	pop.AddColumn("valuehourfactor");
	pop.AddColumn("hourtotal");
	pop.AddColumn("is_processed");
	
	unit.AddColumn("Key");
	unit.AddColumn("Value");
}

void ExpertOptimizerCtrl::Data() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	const Vector<AccuracyConf>& acc_list = esys.acc_list;
	
	graph.Refresh();
	
	for(int i = 0; i < acc_list.GetCount(); i++) {
		const AccuracyConf& conf = acc_list[i];
		
		pop.Set(i, 0, conf.id);
		pop.Set(i, 1, conf.symbol);
		pop.Set(i, 2, conf.label_id);
		pop.Set(i, 3, conf.period);
		pop.Set(i, 4, conf.ext);
		pop.Set(i, 5, conf.label);
		pop.Set(i, 6, conf.fastinput);
		pop.Set(i, 7, conf.labelpattern);
		pop.Set(i, 8, conf.ext_dir ? "true" : "false");
		pop.Set(i, 9, Format("%2!,n", conf.test_valuefactor));
		pop.Set(i, 10, Format("%4!,n", conf.test_valuehourfactor));
		pop.Set(i, 11, Format("%2!,n", conf.test_hourtotal));
		pop.Set(i, 12, conf.is_processed ? "true" : "false");
	}
	
	int cursor = pop.GetCursor();
	if (cursor >= 0 && cursor < acc_list.GetCount()) {
		const AccuracyConf& conf = acc_list[cursor];
		ArrayCtrlPrinter printer(unit);
		conf.Print(printer);
	}
	else unit.Clear();
	
}
























void OptimizationGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	
	const Vector<double>& opt_results = esys.opt_results;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = opt_results.GetCount();
	for(int j = 0; j < count; j++) {
		double d = opt_results[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = opt_results[j];
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


void OptimizationEquityGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	
	const Vector<double>& best_test_equity = esys.best_test_equity;
	
	double emin = +DBL_MAX;
	double emax = -DBL_MAX;
	
	int count = best_test_equity.GetCount();
	for(int j = 0; j < count; j++) {
		double ed = best_test_equity[j];
		if (ed > emax) emax = ed;
		if (ed < emin) emin = ed;
	}
	
	if (count >= 2 && emax > emin) {
		double ediff = emax - emin;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = best_test_equity[j];
			int y = (int)(sz.cy - (v - emin) / ediff * sz.cy);
			int x = (int)(j * xstep);
			polyline[j] = Point(x, y);
		}
		id.DrawPolyline(polyline, 2, Color(28, 127, 150));
	}
	
	if (count)
		id.DrawText(3, 3, DblStr(best_test_equity.Top()), Monospace(15), Black());
	
	w.DrawImage(0, 0, id);
}

ExpertGroupOptimizerCtrl::ExpertGroupOptimizerCtrl() {
	Add(status.TopPos(0,30).LeftPos(0,200));
	Add(prog.HSizePos(200).TopPos(0,30));
	Add(vsplit.HSizePos().VSizePos(30));
	
	prog.Set(0, 1);
	
	vsplit.Vert();
	vsplit << opt << equity;
	
}

void ExpertGroupOptimizerCtrl::Data() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	if (esys.phase == ExpertSystem::PHASE_OPTIMIZING) {
		status.SetLabel(esys.opt_status);
		prog.Set(esys.opt_actual, esys.opt_total);
	} else {
		status.SetLabel("Idle");
		prog.Set(esys.optimizer.GetRound(), esys.optimizer.GetMaxRounds());
	}
	
	
	opt.Refresh();
	equity.Refresh();
}











ExpertRealCtrl::ExpertRealCtrl() {
	Add(refresh_now.LeftPos(2,96).TopPos(2,26));
	Add(last_update.LeftPos(100,200).TopPos(2,26));
	Add(prog.HSizePos(300,3).TopPos(2,26));
	Add(testequity.HSizePos().TopPos(30, 270));
	Add(hsplit.HSizePos().VSizePos(300));
	
	prog.Set(0, 1);
	
	refresh_now.SetLabel("Refresh now");
	refresh_now <<= THISBACK(RefreshNow);
	last_update.SetLabel("Last update:");
	
	hsplit << pop << unit;
	hsplit.Horz();
	hsplit.SetPos(8000);
	
	pop.AddColumn("id");
	pop.AddColumn("symbol");
	pop.AddColumn("label_id");
	pop.AddColumn("period");
	pop.AddColumn("ext");
	pop.AddColumn("label");
	pop.AddColumn("fastinput");
	pop.AddColumn("labelpattern");
	pop.AddColumn("ext_dir");
	pop.AddColumn("mask_valuefactor");
	pop.AddColumn("mask_valuehourfactor");
	pop.AddColumn("mask_hourtotal");
	pop.AddColumn("succ_valuefactor");
	pop.AddColumn("succ_valuehourfactor");
	pop.AddColumn("succ_hourtotal");
	pop.AddColumn("mult_valuefactor");
	pop.AddColumn("mult_valuehourfactor");
	pop.AddColumn("mult_hourtotal");
	pop.AddColumn("is_processed");
	
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
	const Vector<AccuracyConf>& acc_list = esys.acc_list;
	/*
	prog.Set(esys.processed_used_conf, esys.simcore.used_conf.GetCount());
	
	for(int i = 0; i < esys.simcore.used_conf.GetCount(); i++) {
		const AccuracyConf& conf = acc_list[esys.simcore.used_conf[i]];
		
		pop.Set(i, 0, conf.id);
		pop.Set(i, 1, conf.symbol);
		pop.Set(i, 2, conf.label_id);
		pop.Set(i, 3, conf.period);
		pop.Set(i, 4, conf.ext);
		pop.Set(i, 5, conf.label);
		pop.Set(i, 6, conf.fastinput);
		pop.Set(i, 7, conf.labelpattern);
		pop.Set(i, 8, conf.ext_dir ? "true" : "false");
		pop.Set(i, 9, Format("%2!,n", conf.test_mask_valuefactor));
		pop.Set(i, 10, Format("%4!,n", conf.test_mask_valuehourfactor));
		pop.Set(i, 11, Format("%2!,n", conf.test_mask_hourtotal));
		pop.Set(i, 12, Format("%2!,n", conf.test_succ_valuefactor));
		pop.Set(i, 13, Format("%4!,n", conf.test_succ_valuehourfactor));
		pop.Set(i, 14, Format("%2!,n", conf.test_succ_hourtotal));
		pop.Set(i, 15, Format("%2!,n", conf.test_mult_valuefactor));
		pop.Set(i, 16, Format("%4!,n", conf.test_mult_valuehourfactor));
		pop.Set(i, 17, Format("%2!,n", conf.test_mult_hourtotal));
		pop.Set(i, 18, conf.is_processed ? "true" : "false");
	}
	pop.SetCount(acc_list.GetCount());
	
	int cursor = pop.GetCursor();
	if (cursor >= 0 && cursor < esys.simcore.used_conf.GetCount()) {
		const AccuracyConf& conf = acc_list[esys.simcore.used_conf[cursor]];
		ArrayCtrlPrinter printer(unit);
		conf.Print(printer);
	}
	else unit.Clear();
	*/
}

}
