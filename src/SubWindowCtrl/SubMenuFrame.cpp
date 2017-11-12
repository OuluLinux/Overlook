#include "SubWindowCtrl.h"


SubMenuFrame::SubMenuFrame(SubWindows* wins) : wins(wins) {
	height = 20;
	shift = 0;
	arrow_border = 12;
	
	left_arrow = false;
	right_arrow = false;
	
	clr_bg = GrayColor(255);
	clr_tr = GrayColor(128+64);
}

void SubMenuFrame::Paint(Draw& w) {
	ImageDraw id(GetSize());
	
	Rect r = GetRect();
	
	int width = r.Width();
	id.DrawRect(0, 0, width, height, clr_bg);
	
	Size menu_sz(r.GetSize());
	
	id.DrawLine(0, 0, menu_sz.cx-1, 0, 1, clr_tr);
	
	Font fnt = StdFont(15);
	
	int count = wins->GetCount();
	id_pos.SetCount(count);
	int x = -shift;
	int active_pos = wins->GetActiveWindowPos();
	
	
	
	bool draw_right_arrow = false;
	for(int i = 0; i < count; i++) {
		
		x += 5;
		if (i) {
			id.DrawLine(x, 3, x, height-2, 1, GrayColor());
			x += 5;
		}
		SubWindow& sub = (*wins)[i];
		
		String title = sub.GetTitle();
		bool active = active_pos == i;
		Size sz = GetTextSize(title, fnt);
		
		if (x + sz.cx >= menu_sz.cx)
			draw_right_arrow = true;
		
		if (active)
			id.DrawRect(x-5+1, 0, sz.cx+10-1, sz.cy, GrayColor(128+64+32));
		
		id.DrawText(x, 0, title, fnt, Black());
		
		x += sz.cx;
		
		id_pos[i] = x;
	}
	
	// The last line at the right end
	if (count)
		id.DrawLine(x+5, 3, x+5, height-2, 1, GrayColor());
	
	// Draw shift arrows
	left_arrow = false;
	right_arrow = false;
	if (shift) {
		id.DrawRect(0, 0, arrow_border, menu_sz.cy, clr_bg);
		id.DrawText(0, 0, "<", fnt, Black());
		left_arrow = true;
	}
	if (draw_right_arrow) {
		id.DrawRect(menu_sz.cx - arrow_border, 0, arrow_border, menu_sz.cy, clr_bg);
		id.DrawText(menu_sz.cx - arrow_border, 0, ">", fnt, Black());
		right_arrow = true;
	}
	
	w.DrawImage(0, 0, id);
}




void SubMenuFrame::LeftDown(Point p, dword keyflags) {
	int limit = GetSize().cx - arrow_border;
	
	if (left_arrow && p.x < arrow_border) {
		shift -= 20;
		if (shift <= 0) shift = 0;
		Refresh();
		return;
	}
	else if (right_arrow && p.x >= limit) {
		shift += 20;
		Refresh();
		return;
	}
	
	int id = -1;
	for(int i = 0; i < id_pos.GetCount(); i++) {
		if (p.x < id_pos[i]) {id = i; break;}
	}
	if (id < 0) return;
	
	WhenFocus(id);
}

void SubMenuFrame::RightDown(Point p, dword keyflags) {
	clicked_id = -1;
	for(int i = 0; i < id_pos.GetCount(); i++) {
		if (p.x < id_pos[i]) {clicked_id = i; break;}
	}
	if (clicked_id >= 0)
		clicked_id = wins->GetPosId(clicked_id);
	MenuBar::Execute(THISBACK(LocalMenu));
}

void SubMenuFrame::MouseWheel(Point p, int zdelta, dword keyflags) {
	if (right_arrow && zdelta > 0) {
		shift += zdelta / 40 * 10;
		Refresh();
	}
	else if (left_arrow && zdelta < 0) {
		shift += zdelta / 40 * 10;
		if (shift <= 0) shift = 0;
		Refresh();
	}
}

void SubMenuFrame::LocalMenu(Bar& bar) {
	if (clicked_id >= 0) {
		bar.Add("Close", THISBACK(Close));
		bar.Separator();
		bar.Add("Maximize / Restore", THISBACK(Maximize));
		bar.Add("Minimize", THISBACK(Minimize));
		bar.Separator();
		bar.Add("Close All", Proxy(WhenCloseAll));
		bar.Add("Close Others", THISBACK(CloseOthers));
	}
	bar.Add("Tile Windows", Proxy(WhenTileWindows));
	
}
