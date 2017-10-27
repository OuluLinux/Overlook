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

	int depth = 2;
	Vector<Buffer> buf;
	buf.SetCount(2);
	for(int i = 0; i < buf.GetCount(); i++)
		buf[i].SetCount(1);
	ConstBufferSource src;
	src.SetDepth(depth);
	for(int i = 0; i < buf.GetCount(); i++)
		src.SetSource(i, buf[i]);
	ConstBufferSourceIter iter(src);
	
	if (tree) {
		// Draw decisions in the grid
		for (double x = 0.0; x <= sz.cx; x += density) {
			for (double y = 0.0; y <= sz.cy; y += density) {
				buf[0].Set(0, (x-sz.cx/2)/ss);
				buf[1].Set(0, (y-sz.cy/2)/ss);
				double dec = tree->PredictOne(iter);
				
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
		for (int i = 0; i < data->count; i++) {
			Color fill;
			if (data->labels.Get(i))
				fill = Color(100, 100, 200);
			else
				fill = Color(100, 200, 100);
	
			id.DrawEllipse(
				data->src[0].Get(i) * ss + sz.cx / 2 - 3,
				data->src[1].Get(i) * ss + sz.cy / 2 - 3,
				6, 6, fill, 1, Color(0,0,0));
		}
	}

	w.DrawImage(0, 0, id);
}


















RandomForestTester::RandomForestTester() {
	CtrlLayout(*this, "Random Forest Tester");
	//Icon(Std ::icon());
	
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
	data.SetCount(N);
	data.SetCount(N);
	
	data.options.type = 1;
	
	data.SetXY(0, -0.4326,		1.1909,		1);
	data.SetXY(1, 1.5,			3.0,		1);
	data.SetXY(2, 0.1253,		-0.0376,	1);
	data.SetXY(3, 0.2877,		0.3273,		1);
	data.SetXY(4, -1.1465,		0.1746,		1);
	data.SetXY(5, 1.8133,		2.1139,		0);
	data.SetXY(6, 2.7258,		3.0668,		0);
	data.SetXY(7, 1.4117,		2.0593,		0);
	data.SetXY(8, 4.1832,		1.9044,		0);
	data.SetXY(9, 1.8636,		1.1677,		0);
	
	rfctrl.SetData(data);
	rfctrl.SetTree(tree);
	
	Retrain();
}

void RandomForestTester::Retrain() {
	data.options.tree_count = tree_count.GetData();
	data.options.max_depth = max_depth.GetData();
	data.options.tries_count = hypothesis.GetData();
	
	full_mask.SetCount(data.labels.GetCount()).One();
	tree.Train(data.data, data.labels, full_mask, data.options);
	dirty = true;
	rfctrl.Refresh();
}

void RandomForestTester::AddItem(double x, double y, bool value) {
	data.AddXY(x, y, value);
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
