#include "SubWindowCtrl.h"


#define IMAGECLASS SubWindowsImg
#define IMAGEFILE <SubWindowCtrl/SubWindows.iml>
#include <Draw/iml_source.h>


SubWindows::SubWindows() : menu(this) {
	win_counter = 0;
	maximize_all = false;
	active_pos = -1;
	active_id = -1;
	
	Add(menu.HSizePos().BottomPos(0,20));
	Add(sub_area.HSizePos().VSizePos(0,20));
	
	menu.WhenFocus = THISBACK(FocusWindowPos);
	menu.WhenClose = THISBACK(CloseWindow);
	menu.WhenMaximize = THISBACK(MaximizeWindow);
	menu.WhenMinimize = THISBACK(MinimizeWindow);
	menu.WhenTileWindows = THISBACK(OrderTileWindows);
	menu.WhenCloseAll = THISBACK(CloseAll);
	menu.WhenCloseOthers = THISBACK(CloseOthers);
}

SubWindows::~SubWindows() {
	for(; owned_wins.GetCount();) {
		SubWindowCtrl* ptr = owned_wins[0];
		owned_wins.Remove(0);
		delete ptr;
	}
}

SubWindowCtrl& SubWindows::NewWindow() {
	SubWindowCtrl& swc = created_wins.Add();
	SubWindow& subwin = AddWindow(swc);
	subwin.Title(swc.GetTitle());
	RefreshFrame();
	return swc;
}

SubWindow& SubWindows::AddWindow(SubWindowCtrl& ctrl) {
	int id = win_counter++;
	int pos = wins.GetCount();
	SubWindow& sw = wins.Add(id);
	sub_area.Add(sw);
	sw.SetContent(ctrl);
	sw.WhenWindowMove = THISBACK1(MoveWindow, id);
	sw.WhenFocus = THISBACK1(FocusWindow, id);
	sw.WhenClose = THISBACK1(CloseWindow, id);
	sw.WhenMaximize = THISBACK1(MaximizeWindow, id);
	sw.WhenMinimize = THISBACK1(MinimizeWindow, id);
	sw.WhenIsActive = THISBACK1(IsActiveWindow, id);
	int i = wins.GetCount();
	sw.SetRect(i * 30, i * 30, 320, 240);
	if (maximize_all) MaximizeWindow(id);
	int prev_active_id = active_id;
	active_pos = pos;
	active_id = id;
	if (wins.Find(prev_active_id) != -1) wins.Get(prev_active_id).Refresh();
	menu.Refresh();
	ctrl.SubWindowCtrl::Init(this, id);
	WhenActiveWindowChanges();
	return sw;
}

SubWindow& SubWindows::GetWindow(SubWindowCtrl& ctrl) {
	for(int i = 0; i < wins.GetCount(); i++) {
		SubWindowCtrl* ptr = wins[i].GetSubWindowCtrl();
		if (ptr == &ctrl) {
			return wins[i];
		}
	}
	NEVER();
}

void SubWindows::FocusSubWindow(SubWindowCtrl* ctrl) {
	for(int i = 0; i < wins.GetCount(); i++) {
		SubWindowCtrl* ptr = wins[i].GetSubWindowCtrl();
		if (ptr == ctrl) {
			FocusWindow(i);
			return;
		}
	}
}

void SubWindows::MoveWindow(Point pt, int win_id) {
	if (maximize_all) return;
	SubWindow& sw = wins.Get(win_id);
	Size sz(sw.GetSize());
	Point tl = sw.GetRect().TopLeft();
	sw.SetRect(tl.x + pt.x, tl.y + pt.y, sz.cx, sz.cy);
}

void SubWindows::FocusWindow(int win_id) {
	int i = wins.Find(win_id);
	if (i >= 0) FocusWindowPos(i);
}

void SubWindows::FocusWindowPos(int win_pos) {
	if (win_pos < 0 || win_pos >= wins.GetCount()) return;
	SubWindow& sw = wins[win_pos];
	sw.Show();
	Ctrl* c = &sw;
	if (sub_area.GetLastChild() == c) {
		int prev_active_id = active_id;
		active_pos = win_pos;
		active_id = wins.GetKey(win_pos);
		if (wins.Find(prev_active_id) != -1)wins.Get(prev_active_id).Refresh();
		menu.Refresh();
		return;
	}
	if (!sw.IsMaximized()) {
		if (maximize_all) MaximizeWindow(wins.GetKey(win_pos));
	} else {
		Size sz(sub_area.GetSize());
		SubWindow& sw = wins[win_pos];
		sw.SetRect(0, 0, sz.cx, sz.cy);
	}
	sw.GetSubWindowCtrl()->FocusEvent();
	sub_area.RemoveChild(&sw);
	sub_area.AddChild(&sw);
	int prev_active_id = active_id;
	active_pos = win_pos;
	active_id = wins.GetKey(win_pos);
	if (wins.Find(prev_active_id) != -1)wins.Get(prev_active_id).Refresh();
	menu.Refresh();
	WhenActiveWindowChanges();
	
}

void SubWindows::CloseWindow(int win_id) {
	int win_pos = wins.Find(win_id);
	if (win_pos == -1) return;
	SubWindow& sw = wins.Get(win_id);
	sub_area.RemoveChild(&sw);
	SubWindowCtrl* ctrl = sw.GetSubWindowCtrl();
	wins.Remove(win_pos);
	RefreshFrame();
	if (ctrl) {
		bool found = false;
		for(int i = 0; i < owned_wins.GetCount(); i++) {
			if (owned_wins[i] == ctrl) {
				owned_wins.Remove(i);
				ctrl->CloseWindow();
				delete ctrl;
				found = true;
				break;
			}
		}
		if (!found)
			ctrl->CloseWindow();
	}
	FocusPrevious();
}

void SubWindows::MaximizeWindow(int win_id) {
	int win_pos = wins.Find(win_id);
	if (win_pos == -1) return;
	Size sz(sub_area.GetSize());
	SubWindow& sw = wins[win_pos];
	if (!sw.IsMaximized()) {
		sw.StoreRect();
		sw.SetRect(0, 0, sz.cx, sz.cy);
		sw.maximized = true;
		maximize_all = true;
	} else {
		sw.LoadRect();
		sw.maximized = false;
		maximize_all = false;
		LoadRectAll();
	}
}

void SubWindows::LoadRectAll() {
	for(int i = 0; i < wins.GetCount(); i++) {
		SubWindow& sw = wins[i];
		if (sw.IsMaximized()) {
			sw.LoadRect();
			sw.maximized = false;
		}
	}
}

void SubWindows::MinimizeWindow(int win_id) {
	int win_pos = wins.Find(win_id);
	if (win_pos == -1) return;
	SubWindow& sw = wins[win_pos];
	sw.Hide();
	FocusPrevious();
}

void SubWindows::FocusPrevious() {
	int win_pos = active_pos;
	// Focus previous
	int count = wins.GetCount();
	while (win_pos >= count) win_pos--;
	for(int i = 0; i < count; i++) {
		if (win_pos <= -1) win_pos = count - 1;
		SubWindow& sw2 = wins[win_pos];
		//if (!maximize_all && !sw2.IsShown()) {
		if (!sw2.IsShown()) {
			win_pos--;
			continue;
		}
		FocusWindowPos(win_pos);
		return;
	}
	active_id = -1;
	active_pos = -1;
}

void SubWindows::SetTitle(int win_id, const String& title) {
	int win_pos = wins.Find(win_id);
	if (win_pos == -1) return;
	SubWindow& sw = wins[win_pos];
	sw.Title(title);
	PostCallback(THISBACK(Refresh0));
}

void SubWindows::CloseAll() {
	while (wins.GetCount())
		CloseWindow(wins.GetKey(0));
}

void SubWindows::CloseOthers(int win_id) {
	int i = 0;
	for (; i < wins.GetCount();) {
		if (wins.GetKey(i) == win_id)
			i++;
		else
			CloseWindow(wins.GetKey(i));
	}
}

bool SubWindows::Key(dword key, int count) {
	if (key == (K_CTRL|K_F4)) {
		CloseWindow(active_id);
		return true;
	}
	if (key == (K_CTRL|K_TAB)) {
		active_pos++;
		if (active_pos >= wins.GetCount()) active_pos = 0;
		FocusWindowPos(active_pos);
		return true;
	}
	if (key == (K_CTRL|K_SHIFT|K_TAB)) {
		active_pos--;
		if (active_pos < 0) active_pos = wins.GetCount()-1;
		FocusWindowPos(active_pos);
		return true;
	}
	return false;
}

void SubWindows::LeftDown(Point p, dword keyflags) {
	//LOG(p);
	
}

void SubWindows::Layout() {
	// Resize active SubWindow if it is maximised and SubWindows area size changes (TopWindow area changes usually)
	if (maximize_all) {
		if (active_pos < 0 || active_pos >= wins.GetCount())
			return;
		Size sz(sub_area.GetSize());
		SubWindow& sw = wins[active_pos];
		sw.SetRect(0, 0, sz.cx, sz.cy);
	}
}



SubWindowCtrl* SubWindows::GetVisibleSubWindowCtrl() {
	Ctrl* last = sub_area.GetLastChild();
	for(int i = 0; i < wins.GetCount(); i++) {
		SubWindow* swc = &wins[i];
		Ctrl* ptr = swc;
		if (ptr == last) {
			return swc->GetSubWindowCtrl();
		}
	}
	return 0;
}

void SubWindows::OrderTileWindows() {
	int count = wins.GetCount();
	if (maximize_all && count == 1) return;
	
	Size sz(sub_area.GetSize());
	
	double target_ratio = 16.0 / 9.0; //4.0 / 3.0;
	
	int rows, cols, mod, diff;
	int smallest_diff = INT_MAX;
	int smallest_diff_cols = -1;
	double smallest_diff_ratio_diff = DBL_MAX, ratio, ratio_diff;
	for(int i = 1; i < 100; i++) {
		rows = count / i;
		if (rows == 0) break;
		cols = i;
		mod = count % (rows * cols);
		diff = abs(rows-cols);
		//if (mod == 0)
			ratio = ((double)sz.cx/cols) / ((double)sz.cy/rows);
		//else
			//ratio = (((double)sz.cx/cols) / ((double)sz.cy/rows) * (rows * (cols-1)) + ((double)sz.cx/cols) / ((double)sz.cy/(rows + mod)) * ((rows + mod) * 1)) / count;
		ratio_diff = fabs(ratio - target_ratio);
		LOG(Format("rows=%d cols=%d mod=%d diff=%d ratio_diff=%f", rows, cols, mod, diff, ratio_diff));
		//if (i <= smallest_diff) {
		if (ratio_diff < smallest_diff_ratio_diff && mod <= 2) {
			smallest_diff = diff;
			smallest_diff_cols = i;
			smallest_diff_ratio_diff = ratio_diff;
		}
	}
	rows = count / smallest_diff_cols;
	if (rows == 0) return;
	cols = smallest_diff_cols;
	mod = count % (rows * cols);
	diff = abs(rows-cols);
	ratio_diff = smallest_diff_ratio_diff;
	LOG(Format("rows=%d cols=%d mod=%d diff=%d ratio_diff=%f", rows, cols, mod, diff, ratio_diff));
	
	
	double x_step, y_step, ymod_step;
	x_step = (double)sz.cx / cols;
	y_step = (double)sz.cy / rows;
	ymod_step = (double)sz.cy / (rows + mod);
	
	maximize_all = false;
	
	int pos = 0;
	for(int i = 0; i < cols; i++) {
		int x = i * x_step;
		int rows_i;
		double yi_step;
		if (i < cols-1) {
			rows_i = rows;
			yi_step = y_step;
		} else {
			rows_i = rows + mod;
			yi_step = ymod_step;
		}
		for(int j = 0; j < rows_i; j++) {
			int y = j * yi_step;
			SubWindow& sw = wins[pos];
			sw.SetRect(x, y, x_step, yi_step);
			sw.maximized = false;
			pos++;
		}
	}
}

void SubWindows::OrderTileWindowsVert() {
	int count = wins.GetCount();
	if (maximize_all && count == 1) return;
	
	Size sz(sub_area.GetSize());
	
	double y_step = (double)sz.cy / wins.GetCount();
	
	maximize_all = false;
	
	for(int i = 0; i < wins.GetCount(); i++) {
		SubWindow& sw = wins[i];
		sw.SetRect(0, i * y_step, sz.cx, y_step);
		sw.maximized = false;
	}
}
