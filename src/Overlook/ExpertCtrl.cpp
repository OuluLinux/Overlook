#include "Overlook.h"

namespace Overlook {

void EvolutionGraph::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());
	
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	Vector<double>& sector_results = esys.fusion.sector_results;
	
	double min = +DBL_MAX;
	double max = -DBL_MAX;
	
	int count = sector_results.GetCount();
	for(int j = 0; j < count; j++) {
		double d = sector_results[j];
		if (d > max) max = d;
		if (d < min) min = d;
	}
	
	if (count >= 2 && max > min) {
		double diff = max - min;
		double xstep = (double)sz.cx / (count - 1);
		
		polyline.SetCount(count);
		for(int j = 0; j < count; j++) {
			double v = sector_results[j];
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
	
	w.DrawImage(0, 0, id);
}





ExpertSectorsCtrl::ExpertSectorsCtrl() {
	Add(vsplit.SizePos());
	
	vsplit << graph << hsplit;
	vsplit.Vert();
	vsplit.SetPos(3333);
	
	hsplit << pop << unit;
	hsplit.Horz();
	
	pop.AddColumn("Accuracy");
	pop.AddColumn("Symbol");
	pop.AddColumn("Period");
	pop.AddColumn("Prediction Length");
	for(int i = 0; i < SECTOR_2EXP; i++)
		pop.AddColumn("Decision");
	
}

void ExpertSectorsCtrl::Data() {
	System& sys = GetSystem();
	ExpertSystem& esys = sys.GetExpertSystem();
	
	const Vector<SectorConf>& sec_confs = esys.fusion.sec_confs;
	
	for(int i = 0; i < sec_confs.GetCount(); i++) {
		const SectorConf& conf = sec_confs[i];
		
		pop.Set(i, 0, conf.accuracy);
		pop.Set(i, 1, conf.symbol);
		pop.Set(i, 2, conf.period);
		pop.Set(i, 3, conf.minimum_prediction_len);
		for(int j = 0; j < SECTOR_2EXP; j++) {
			pop.Set(i, 4+j, conf.dec[j].ToString());
		}
	}
	
	int cursor = pop.GetCursor();
	if (cursor >= 0 && cursor < sec_confs.GetCount()) {
		const SectorConf& conf = sec_confs[cursor];
		
		// ??? 
		
	}
	else unit.Clear();
}







ExpertOptimizerCtrl::ExpertOptimizerCtrl() {
	TabCtrl::Add(sectors, "Sectors");
	TabCtrl::Add(sectors);
	
}

void ExpertOptimizerCtrl::Data() {
	int tab = Get();
	
	if (tab == 0) {
		sectors.Data();
	}
}

}
