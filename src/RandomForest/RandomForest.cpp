#include "RandomForest.h"

namespace RandomForest {

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

}
