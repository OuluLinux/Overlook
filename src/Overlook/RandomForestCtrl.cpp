#include "Overlook.h"

namespace Overlook {

RandomForestCtrl::RandomForestCtrl() {

}

void RandomForestCtrl::LeftDown(Point p, dword keyflags) {
	Size sz(GetSize());
	double x = (p.x - sz.cx / 2.0) / ss;
	double y = (p.y - sz.cy / 2.0) / ss;
	bool value = keyflags & (K_ALT | K_SHIFT);

	WhenItem(x, y, value);
}

void RandomForestCtrl::Paint(Draw& w) {
	Size sz(GetSize());
	ImageDraw id(sz);
	id.DrawRect(sz, White());

	if (tree) {
		// Draw decisions in the grid
		for (double x = 0.0; x <= sz.cx; x += density) {
			for (double y = 0.0; y <= sz.cy; y += density) {
	
				double dec = tree->PredictOne(Pair((x-sz.cx/2)/ss, (y-sz.cy/2)/ss));
				
				Color fill;
				if (!drawSoft) {
					if (dec > 0.5)
						fill = Color(150,150,250);
					else
						fill = Color(150,250,150);
				}
	
				else {
					int ri = Upp::min(255.0, Upp::max(0.0, 250 * (1 - dec) + 150 * dec));
					int gi = Upp::min(255.0, Upp::max(0.0, 250 * dec + 150 * (1 - dec)));
					fill = Color(150, ri, gi);
				}
				
				id.DrawRect(x - density / 2 - 1, y - density - 1, density + 2, density + 2, fill);
			}
		}
	}

	// Draw axes
	id.DrawLine(sz.cx / 2, 0, sz.cx / 2, sz.cy, 1, GrayColor(50));
	id.DrawLine(0, sz.cy / 2, sz.cx, sz.cy / 2, 1, GrayColor(50));
	
	
	if (data) {
		// Draw datapoints
		for (int i = 0; i < data->data.GetCount(); i++) {
			Color fill;
			if (data->labels[i])
				fill = Color(100, 100, 200);
			else
				fill = Color(100, 200, 100);
	
			id.DrawEllipse(
				data->data[i][0]*ss + sz.cx / 2 - 3,
				data->data[i][1]*ss + sz.cy / 2 - 3,
				6, 6, fill, 1, Color(0,0,0));
		}
	}

	w.DrawImage(0, 0, id);
}


















RandomForestTester::RandomForestTester() {
	CtrlLayout(*this, "Random Forest Tester");
	Icon(RandomForextImg::icon());
	
	tree_count.MinMax(1, 200);
	tree_count.SetData(100);
	tree_count <<= THISBACK(Retrain);
	
	max_depth.MinMax(2, 12);
	max_depth.SetData(4);
	max_depth <<= THISBACK(Retrain);
	
	hypothesis.MinMax(1, 50);
	hypothesis.SetData(10);
	hypothesis <<= THISBACK(Retrain);
	
	toggle_type <<= THISBACK(ToggleType);
	toggle_drawing <<= THISBACK(ToggleDrawing);
	
	rfctrl.WhenItem = THISBACK(AddItem);
	
	PostCallback(THISBACK(Init));
}

void RandomForestTester::Init() {
	N = 10;
	data.data.SetCount(N);
	data.labels.SetCount(N);
	
	data.options.type = 1;
	
	data.data[0] = Pair(-0.4326,	1.1909);
	data.data[1] = Pair(1.5,		3.0);
	data.data[2] = Pair(0.1253,		-0.0376);
	data.data[3] = Pair(0.2877,		0.3273);
	data.data[4] = Pair(-1.1465,	0.1746);
	data.data[5] = Pair(1.8133,		2.1139);
	data.data[6] = Pair(2.7258,		3.0668);
	data.data[7] = Pair(1.4117,		2.0593);
	data.data[8] = Pair(4.1832,		1.9044);
	data.data[9] = Pair(1.8636,		1.1677);
	
	data.labels[0]= 1;
	data.labels[1]= 1;
	data.labels[2]= 1;
	data.labels[3]= 1;
	data.labels[4]= 1;
	data.labels[5]= 0;
	data.labels[6]= 0;
	data.labels[7]= 0;
	data.labels[8]= 0;
	data.labels[9]= 0;
	
	rfctrl.SetData(data);
	rfctrl.SetTree(tree);
	
	Retrain();
}

void RandomForestTester::Retrain() {
	data.options.tree_count = tree_count.GetData();
	data.options.max_depth = max_depth.GetData();
	data.options.tries_count = hypothesis.GetData();
	tree.Train(data.data, data.labels, data.options);
	dirty = true;
	rfctrl.Refresh();
}

void RandomForestTester::AddItem(double x, double y, bool value) {
	data.data.Add(Pair(x, y));
	data.labels.Add(value);
	Retrain();
}

void RandomForestTester::ToggleType() {
	data.options.type = !data.options.type;
	Retrain();
}

void RandomForestTester::ToggleDrawing() {
	rfctrl.ToggleDrawing();
}

}
