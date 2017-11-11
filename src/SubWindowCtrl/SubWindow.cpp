#include "SubWindowCtrl.h"


SubWindowCtrl::SubWindowCtrl() : wm(0), id(-1) {
	
}

void SubWindowCtrl::Init(SubWindows* wm, int id) {
	this->wm = wm;
	this->id = id;
}

void SubWindowCtrl::Title(const String& title) {
	if (wm)
		wm->SetTitle(id, title);
}

SubWindow::SubWindow() : stored_rect(0,0,0,0) {
	ctrl = 0;
	maximized = false;
	
	Add(decor.SizePos());
	
	close.SetImage(SubWindowsImg::close());
	maximize.SetImage(SubWindowsImg::maximize());
	minimize.SetImage(SubWindowsImg::minimize());
	
	Add(close.TopPos(3, 19).RightPos(3, 19));
	Add(maximize.TopPos(3, 19).RightPos(3+24, 19));
	Add(minimize.TopPos(3, 19).RightPos(3+24*2, 19));
	
	decor.WhenWindowMove = Proxy(WhenWindowMove);
	decor.WhenFocus = THISBACK(FocusEvent);
	decor.WhenMaximize = THISBACK(Maximize);
	decor.WhenMinimize = Proxy(WhenMinimize);
	decor.WhenClose = Proxy(WhenClose);
	close.WhenAction = Proxy(WhenClose);
	maximize.WhenAction = THISBACK(Maximize);
	minimize.WhenAction = Proxy(WhenMinimize);
	
}

void SubWindow::SetContent(SubWindowCtrl& ctrl) {
	this->ctrl = &ctrl;
	
	Add(ctrl.VSizePos(24, 1).HSizePos(1, 1));
	
	
}

void SubWindow::LeftDown(Point p, dword keyflags) {
	FocusEvent();
	
}

void SubWindow::ChildGotFocus() {
	FocusEvent();
	
}

void SubWindow::ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags) {
	
	if (event == LEFTDOWN) {
		FocusEvent();
	}
	
}




SubWindowDecoration::SubWindowDecoration() {
	left_down = false;
	
}
	
void SubWindowDecoration::Paint(Draw& draw) {
	Color border_tl = GrayColor(128+64);
	Color border_br = GrayColor(128);
	
	Size sz(GetSize());
	draw.DrawRect(sz, White());
	draw.DrawLine(0,0, 0, sz.cy-1, 1, border_tl);
	draw.DrawLine(0,0, sz.cx-1, 0, 1, border_tl);
	draw.DrawLine(sz.cx-1,0, sz.cx-1, sz.cy-1, 1, border_br);
	draw.DrawLine(0,sz.cy-1, sz.cx-1, sz.cy-1, 1, border_br);
	
	Color left, right;
	if (dynamic_cast<SubWindow*>(GetParent())->IsActive()) {
		left = Color(0, 64, 128);
		right = Color(57, 141, 195);
	} else {
		left = GrayColor();
		right = GrayColor(195);
	}
	for(int i = 0; i < sz.cx; i++) {
		int x = i;
		int alpha = 255 * i / (sz.cx -2);
		Color clr = Blend(left, right, alpha);
		draw.DrawLine(i, 0, i, 23, 1, clr);
	}
	
	draw.DrawText(7, 4, label, StdFont(15), Black());
	draw.DrawText(6, 3, label, StdFont(15), White());
	
	Color tl = GrayColor(128+64+32);
	Color br = GrayColor(128-64-32);
	
	draw.DrawLine(0,0, sz.cx-1, 0, 1, tl);
	draw.DrawLine(0,0, 0, sz.cy-1, 1, tl);
	draw.DrawLine(sz.cx-1, sz.cy-1, sz.cx-1, 0, 1, br);
	draw.DrawLine(sz.cx-1, sz.cy-1, 0, sz.cy-1, 1, br);
	
}

void SubWindowDecoration::LeftDown(Point p, dword keyflags) {
	left_down = true;
	left_down_pt = p;
	
	WhenFocus();
}

void SubWindowDecoration::LeftDouble(Point p, dword keyflags) {
	WhenMaximize();
}

void SubWindowDecoration::LeftDrag(Point p, dword keyflags) {
	if (left_down) {
		WhenWindowMove(p - left_down_pt);
	}
}

void SubWindowDecoration::LeftUp(Point p, dword keyflags) {
	left_down = false;
	
}

void SubWindowDecoration::MouseMove(Point p, dword keyflags) {
	if (left_down) {
		WhenWindowMove(p - left_down_pt);
	}
}

void SubWindowDecoration::RightDown(Point p, dword keyflags) {
	MenuBar::Execute(THISBACK(LocalMenu));
}

void SubWindowDecoration::LocalMenu(Bar& bar) {
	bar.Add("Maximize / Restore", Proxy(WhenMaximize));
	bar.Add("Minimize", Proxy(WhenMinimize));
	bar.Separator();
	bar.Add("Close", Proxy(WhenClose));
}
