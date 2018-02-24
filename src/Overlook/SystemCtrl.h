#ifndef _Overlook_SystemCtrl_h_
#define _Overlook_SystemCtrl_h_


namespace Overlook {
using namespace Upp;


class SystemCtrl : public ParentCtrl {
	ProgressIndicator prog;
	Button load_sources, load_booleans;
	bool running = false, stopped = true;
	
	void LoadSources();
	void LoadBooleans();
	void LoadStats();
public:
	typedef SystemCtrl CLASSNAME;
	SystemCtrl();
	~SystemCtrl() {Stop();}
	
	void StartLoadSources() {Stop(); running = true; stopped = false; Thread::Start(THISBACK(LoadSources));}
	void StartLoadBooleans() {Stop(); running = true; stopped = false; Thread::Start(THISBACK(LoadBooleans));}
	void Stop() {running = false; while (!stopped) Sleep(100);}
	void SetProg(int a, int t) {prog.Set(a, t);}
	
};


}


#endif
