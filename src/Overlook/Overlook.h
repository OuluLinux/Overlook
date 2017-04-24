#ifndef _Overlook_Overlook_h
#define _Overlook_Overlook_h

#include <CtrlLib/CtrlLib.h>
#include <CtrlUtils/CtrlUtils.h>
#include <DataCtrl/DataCtrl.h>


using namespace Upp;
using namespace DataCore;
using namespace DataCtrl;
using namespace RefCore;

#define IMAGECLASS OverlookImg
#define IMAGEFILE <Overlook/Overlook.iml>
#include <Draw/iml_header.h>

#define LAYOUTFILE <Overlook/Overlook.lay>
#include <CtrlCore/lay.h>


struct OverlookSession {
	
	Time begin, end;
	String addr;
	int port;
	Vector<int> tfs;
	VectorMap<String, String> link_core, link_ctrl;
	String datadir;
	
	void Serialize(Stream& s) {
		s % begin % end % addr % port % tfs % link_core % link_ctrl % datadir;
	}
};


class Loader : public WithLoaderLayout<TopWindow> {
	
public:
	typedef Loader CLASSNAME;
	Loader();
	
	void RefreshSessions();
	void ShowLicense();
	void Load();
	void Create();
	void LoadSession(OverlookSession& ses);
	
	void Exit() {exit = true;}
	
	bool exit;
};

class Overlook : public TopWindow {
	
protected:
	friend class HeatmapCtrl;
	
	ToolBar tool;
	
	ParentCtrl main_ctrl;
	ParentCtrl* added_ctrl;
	
	PathCtrl path;
	Splitter h_split;
	FileList files;
	ParentCtrl main;
	//GraphGroupCtrl graphs;
	//TableCtrl tablectrl;
	//EventCtrl eventctrl;
	ParentCtrl err_ctrl;
	Label err_label;
	
	String current_directory;
	Vector<String> tree_paths;
	VectorMap<int, int> tree_files;
	
	String err_msg;
	
	Vector<Pointf> s1;
	
	ButtonOption btn_fast_open;
	bool fast_open;
	
	PathResolverVar resolver;
	
public:
	typedef Overlook CLASSNAME;
	Overlook();
	~Overlook();
	
	void ToolMenu(Bar& bar);
	void SelectFile();
	void ViewFile();
	void SetDirectory(String dir);
	void RefreshToolBar();
	void RefreshPathCtrl();
	void RefreshFileList();
	void RefreshData();
	void SetView(int i);
	void ToggleFastView();
	void AddExtCtrl(ParentCtrl* ctrl);
	void RemoveExtCtrl();
	void ViewPath(const String& view_path, bool try_ctrl=true);
	
	//bool LinkPath(String dest, String src) {} //bool r = resolver->LinkPath(dest, src); RefreshFileList(); return r;}
	void LinkFoundPeriods();
	
	
	
	//
	
	
};

#endif
