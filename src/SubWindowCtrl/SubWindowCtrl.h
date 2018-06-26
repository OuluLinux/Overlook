#ifndef _SubWindowCtrl_SubWindowCtrl_h
#define _SubWindowCtrl_SubWindowCtrl_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#define IMAGECLASS SubWindowsImg
#define IMAGEFILE <SubWindowCtrl/SubWindows.iml>
#include <Draw/iml_header.h>

class SubWindows;

class SubWindowDecoration : public Ctrl {
	String label;
	bool left_down;
	Point left_down_pt;
	
public:
	typedef SubWindowDecoration CLASSNAME;
	SubWindowDecoration();
	
	virtual void Paint(Draw& draw);
	
	void SetLabel(String str) {label = str;}
	
	String GetLabel() const {return label;}
	
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftDouble(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);
	virtual void MouseMove(Point p, dword keyflags);
	virtual void RightDown(Point p, dword keyflags);
	
	void LocalMenu(Bar& bar);
	
	
	Callback1<Point> WhenWindowMove;
	Callback WhenFocus, WhenClose, WhenMaximize, WhenMinimize;
	
};


class SubWindowCtrl : public ParentCtrl {
	SubWindows* wm;
	int id;
	
public:
	SubWindowCtrl();
	virtual ~SubWindowCtrl() {}
	void Init(SubWindows* wm, int id);
	
	virtual void Start() {}
	virtual void CloseWindow() {}
	virtual String GetTitle() {return "";}
	virtual void RefreshData() {};
	virtual void FocusEvent() {}
	
	void Title(const String& title);
	
};

class SubWindow : public ParentCtrl {
	SubWindowDecoration decor;
	Button minimize, maximize, close;
	SubWindowCtrl* ctrl = NULL;
	Rect stored_rect;
	
protected:
	friend class SubWindows;
	void SetContent(SubWindowCtrl& ctrl);
	
	bool maximized;
	
public:
	typedef SubWindow CLASSNAME;
	SubWindow();
	
	void Title(String label) {decor.SetLabel(label);}
	void StoreRect() {stored_rect = GetRect();}
	void LoadRect() {ASSERT(stored_rect.bottom && stored_rect.right); SetRect(stored_rect);}
	void SetStoredRect(Rect r) {stored_rect = r;}
	
	Rect GetStoredRect() const {return stored_rect;}
	String GetTitle() const {return decor.GetLabel();}
	SubWindowCtrl* GetSubWindowCtrl() {return ctrl;}
	bool IsMaximized() const {return maximized;}
	bool IsActive() const {bool b = false; WhenIsActive(&b); return b;}
	void Maximize() {WhenMaximize(); FocusEvent();}
	void FocusEvent() {WhenFocus(); ctrl->FocusEvent();}
	
	virtual void LeftDown(Point p, dword keyflags);
	virtual void ChildGotFocus();
	virtual void ChildMouseEvent(Ctrl *child, int event, Point p, int zdelta, dword keyflags);
	
	Callback1<Point> WhenWindowMove;
	Callback1<bool*> WhenIsActive;
	Callback WhenFocus, WhenClose, WhenMaximize, WhenMinimize;
	
};


class SubMenuFrameList : public Ctrl {
	
public:
	
};

class SubMenuFrame : public Ctrl {
	int height, shift, arrow_border;
	bool left_arrow, right_arrow;
	SubWindows* wins;
	Color clr_bg, clr_tr;
	Vector<int> id_pos;
	int clicked_id;
	
protected:
	void Close() {WhenClose(clicked_id);}
	void Maximize() {WhenFocus(clicked_id); WhenMaximize(clicked_id);}
	void Minimize() {WhenMinimize(clicked_id);}
	void CloseOthers() {WhenCloseOthers(clicked_id);}
	
public:
	typedef SubMenuFrame CLASSNAME;
	SubMenuFrame(SubWindows* wins);
	
	
	virtual void Paint(Draw& w);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void RightDown(Point p, dword keyflags);
	virtual void MouseWheel(Point p, int zdelta, dword keyflags);
	
	void LocalMenu(Bar& bar);
	
	Callback WhenTileWindows, WhenCloseAll;
	Callback1<int> WhenFocus, WhenClose, WhenMaximize, WhenMinimize, WhenCloseOthers;
};

class SubWindows : public ParentCtrl {
	ArrayMap<int, SubWindow> wins;
	SubMenuFrame menu;
	ParentCtrl sub_area;
	Vector<SubWindowCtrl*> owned_wins;
	int win_counter;
	bool maximize_all;
	int active_pos, active_id;
	Array<SubWindowCtrl> created_wins;
	
protected:
	typedef SubWindows CLASSNAME;
	
	friend class SubWindowCtrl;
	friend class SubWindow;
	
	void MoveWindow(Point pt, int win_id);
	void FocusWindow(int win_id);
	void FocusWindowPos(int win_pos);
	void MaximizeWindow(int win_id);
	void MinimizeWindow(int win_id);
	void IsActiveWindow(bool* result, int win_id) {*result = win_id == active_id;}
	
	void FocusPrevious();
	void SetTitle(int win_id, const String& title);
	void CloseOthers(int win_id);
	
	void LoadRectAll();
	void SetMaximizeAll(bool b=true) {maximize_all = b;}
	
public:
	SubWindows();
	~SubWindows();
	
	template <class T> T& AddSubWindow(T* obj_ptr) {
		SubWindowCtrl* swc = obj_ptr;
		owned_wins.Add(swc); // Class must inherit SubWindowCtrl
		SubWindow& subwin = AddWindow(*swc);
		subwin.Title(swc->GetTitle());
		RefreshFrame();
		return *obj_ptr;
	}
	
	template <class T> T& AddSubWindow() {
		T* obj_ptr = new T();
		return AddSubWindow(obj_ptr);
	}
	
	uint32 GetHashValue() const {return (uint64)this;}
	
	SubWindowCtrl& NewWindow();
	SubWindow& AddWindow(SubWindowCtrl& ctrl);
	SubWindow& GetWindow(SubWindowCtrl& ctrl);
	int GetActiveWindowPos() {return active_pos;}
	int GetActiveWindowId()  {return active_id;}
	void CloseWindow(int win_id);
	int GetPosId(int win_pos) {if (win_pos >= 0 && win_pos < wins.GetCount()) return wins.GetKey(win_pos); return -1;}
	
	void FocusSubWindow(SubWindowCtrl* ctrl);
	void OrderTileWindows();
	void OrderTileWindowsVert();
	void CloseAll();
	
	virtual bool Key(dword key, int count);
	virtual void LeftDown(Point p, dword keyflags);
	virtual void Layout();
	
	int GetCount() const {return wins.GetCount();}
	SubWindow& operator[] (int i) {return wins[i];}
	SubWindow& Get(int i) {return wins[i];}
	
	SubWindowCtrl* GetVisibleSubWindowCtrl();
	SubWindowCtrl& GetVisibleSubWindowCtrlRef() {return *GetVisibleSubWindowCtrl();}
	
	
	Callback WhenActiveWindowChanges;
};

#endif
