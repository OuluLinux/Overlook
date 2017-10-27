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

	int depth = 5;
	Vector<Buffer> buf;
	buf.SetCount(depth);
	for(int i = 0; i < buf.GetCount(); i++)
		buf[i].SetCount(1);
	ConstBufferSource src;
	src.SetDepth(depth);
	for(int i = 0; i < buf.GetCount(); i++)
		src.SetSource(i, buf[i]);
	ConstBufferSourceIter iter(src);
	
	buf[2].Set(0, d0);
	buf[3].Set(0, d1);
	buf[4].Set(0, d2);
	
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
	
			double x = data->src[0].Get(i);
			double y = data->src[1].Get(i);
			double s0 = data->src[2].Get(i);
			double s1 = data->src[3].Get(i);
			double s2 = data->src[4].Get(i);
			if ((s0 >= d0 - 0.5 && s0 <= d0 + 0.5) &&
				(s1 >= d1 - 0.5 && s1 <= d1 + 0.5) &&
				(s2 >= d2 - 0.5 && s2 <= d2 + 0.5)) {
				id.DrawEllipse(
					x * ss + sz.cx / 2 - 3,
					y * ss + sz.cy / 2 - 3,
					6, 6, fill, 1, Color(0,0,0));
			}
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
	
	d0.MinMax(-5, +5);
	d0.SetData(0);
	d0 <<= THISBACK(DoRefresh);
	d1.MinMax(-5, +5);
	d1.SetData(0);
	d1 <<= THISBACK(DoRefresh);
	d2.MinMax(-5, +5);
	d2.SetData(0);
	d2 <<= THISBACK(DoRefresh);
	
	toggle_type <<= THISBACK(ToggleType);
	toggle_drawing <<= THISBACK(ToggleDrawing);
	
	rfctrl.WhenItem = THISBACK(AddItem);
	
	PostCallback(THISBACK(Init));
}

void RandomForestTester::Init() {
	
	data.options.type = 1;
	
	/*
	data.SetCount(10);
	data.SetXY(0, -0.4326,		1.1909,		1);
	data.SetXY(1, 1.5,			3.0,		1);
	data.SetXY(2, 0.1253,		-0.0376,	1);
	data.SetXY(3, 0.2877,		0.3273,		1);
	data.SetXY(4, -1.1465,		0.1746,		1);
	data.SetXY(5, 1.8133,		2.1139,		0);
	data.SetXY(6, 2.7258,		3.0668,		0);
	data.SetXY(7, 1.4117,		2.0593,		0);
	data.SetXY(8, 4.1832,		1.9044,		0);
	data.SetXY(9, 1.8636,		1.1677,		0);*/
	
	data.SetDepth(5);
	data.SetCount(100);
	for(int i = 0; i < 100; i++) {
		for(int j = 0; j < 5; j++)
			data.Set(i, j, Randomf() * 10.0 - 5.0);
		data.SetLabel(i, Random(2));
	}
	
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
	DoRefresh();
}

void RandomForestTester::DoRefresh() {
	rfctrl.d0 = d0.GetData();
	rfctrl.d1 = d1.GetData();
	rfctrl.d2 = d2.GetData();
	rfctrl.Refresh();
}

void RandomForestTester::AddItem(double x, double y, bool value) {
	int i = data.GetCount();
	data.SetCount(i+1);
	data.Set(i, 0, x);
	data.Set(i, 1, y);
	data.Set(i, 2, d0.GetData());
	data.Set(i, 3, d1.GetData());
	data.Set(i, 4, d2.GetData());
	data.SetLabel(i, value);
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
