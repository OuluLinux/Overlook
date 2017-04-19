#include "Overlook.h"
#include <CoreUtils/CoreUtils.h>

Overlook::Overlook() {
	resolver = GetPathResolver();
	
	Title("Overlook");
	Icon(OverlookImg::icon());
	MinimizeBox().MaximizeBox().Sizeable();
	
	//resolver = new PathResolver();
	added_ctrl = 0;
	fast_open = false;
	
	AddFrame(tool);
	RefreshToolBar();
	
	Add(main_ctrl.SizePos());
	
	main_ctrl.Add(path.TopPos(0,30).HSizePos());
	main_ctrl.Add(h_split.VSizePos(30,0).HSizePos());
	h_split.Horz();
	h_split.Set(files, main);
	h_split.SetPos(1500);
	/*main.Add(graphs.SizePos());
	main.Add(tablectrl.SizePos());
	main.Add(eventctrl.SizePos());*/
	main.Add(err_ctrl.SizePos());
	
	err_ctrl.Add(err_label.HCenterPos(320).VCenterPos(240));
	
	//files.ColumnList::WhenLeftClick = THISBACK(SelectFile);
	files.ColumnList::WhenLeftClick = Callback();
	files.ColumnList::WhenLeftDouble = THISBACK(SelectFile);
	files.ColumnList::WhenAction = THISBACK(ViewFile);
	
	btn_fast_open.SetImage(OverlookImg::view_fast());
	btn_fast_open <<= THISBACK(ToggleFastView);
	
	SetView(0);
	
	current_directory = "/";
	path.SetDirectorySeparator("/");
	path.WhenPath = THISBACK(SetDirectory);
	SetDirectory(current_directory);
	PostCallback(THISBACK(RefreshData));
}

Overlook::~Overlook() {
	RemoveExtCtrl();
	
	
}

void Overlook::ToolMenu(Bar& bar) {
	
	bar.Add(btn_fast_open, 24, 24);
	//bar.Add(fast_open, OverlookImages::view_fast(), THISBACK(ToggleFastView));
	//bar.Add(OverlookImages::view_line(), THISBACK1(SetView, 0));
	//bar.Add(OverlookImages::view_hmap(), THISBACK1(SetView, 1));
	
}

void Overlook::ToggleFastView() {
	fast_open = btn_fast_open.Get();
	
	if (!fast_open) {
		files.ColumnList::WhenLeftClick = Callback();
		files.ColumnList::WhenLeftDouble = THISBACK(SelectFile);
		files.ColumnList::WhenAction = THISBACK(ViewFile);
	}
	else {
		files.ColumnList::WhenLeftClick = THISBACK(SelectFile);
		files.ColumnList::WhenLeftDouble = Callback();
		files.ColumnList::WhenAction = Callback();
	}
	
	RefreshToolBar();
}

void Overlook::SelectFile() {
	int cursor = files.GetCursor();
	if (cursor == -1) return;
	
	String path = AppendPosixFilename(current_directory, (String)files.Get(cursor));
	if (resolver->FindLinkPath(path) != 0) {
		SetDirectory(path);
	}
}

void Overlook::ViewFile() {
	String current = files.GetCurrentName();
	if (current.IsEmpty()) return;
	String path = AppendPosixFilename(current_directory, current);
	ViewPath(path);
	
}

void Overlook::SetDirectory(String dir) {
	if (dir.Left(5) == "link ") {
		int i = dir.Find(" -> ");
		if (i == -1) return;
		String a = dir.Mid(5, i - 5);
		String b = dir.Mid(i+4);
		
		resolver->LinkPath(a, b);
		dir = a;
	}
	current_directory = dir;
	RefreshPathCtrl();
	RefreshData();
}

void Overlook::RefreshToolBar() {
	tool.Clear();
	tool.Set(THISBACK(ToolMenu));
}

void Overlook::RefreshPathCtrl() {
	path.SetPath(current_directory);
	RefreshFileList();
}

void Overlook::RefreshFileList() {
	files.Clear();
	tree_paths.Clear();
	tree_files.Clear();
	
	RefCore::PathLink* link = resolver->FindLinkPath(current_directory);
	if (!link) {
		return;
	}
	
	int count = link->keys.GetCount();
	for(int i = 0; i < count; i++) {
		this->files.FileList::Add(link->keys.GetKey(i), Image());
	}
}

void Overlook::AddExtCtrl(ParentCtrl* ctrl) {
	RemoveExtCtrl();
	main.Add(ctrl->SizePos());
	added_ctrl = ctrl;
}

void Overlook::RemoveExtCtrl() {
	if (!added_ctrl) return;
	main.RemoveChild(added_ctrl);
	added_ctrl = 0;
}

void Overlook::RefreshData() {
	RemoveExtCtrl();
	
	if (current_directory == "/")
		return;
	
	ViewPath(current_directory);
}

void Overlook::ViewPath(const String& view_path, bool try_ctrl) {
	
	String resolve_path;
	
	RefCore::PathLink* link = resolver->FindLinkPath(view_path);
	if (link) {
		resolve_path = link->path;
	}
	
	if (resolve_path.IsEmpty())
		resolve_path = view_path;
	
	MetaVar var = resolver->ResolvePath(resolve_path);
	
	if (!var.Is()) {
		err_msg = "Invalid path: " + resolve_path;
		SetView(-1);
		return;
	}
	
	// Add ParentCtrls directly to the GUI
	ParentCtrl* ctrl = var.Get<ParentCtrl>();
	if (ctrl) {
		AddExtCtrl(ctrl);
		
		// Try to start function
		var->StartThreaded();
	}
	else if (try_ctrl) {
		String ctrl_key = var->GetCtrlKey();
		if (ctrl_key.IsEmpty()) {
			err_msg = "No known ctrl for: " + var->GetKey();
			SetView(-1);
			return;
		}
		
		resolve_path += "/" + ctrl_key;
		ViewPath(resolve_path, false);
	}
}

void Overlook::SetView(int i) {
	err_ctrl.Hide();
	if (added_ctrl) added_ctrl->Hide();
	
	if (i == -1) {
		err_label.SetLabel(err_msg);
		err_ctrl.Ctrl::Show();
	}
	if (added_ctrl) added_ctrl->Ctrl::Show();
}

