#ifndef _Overlook_SystemCtrl_h_
#define _Overlook_SystemCtrl_h_


namespace Overlook {
using namespace Upp;


struct BooleansDraw : public Ctrl {
	virtual void Paint(Draw& w);
	
	int cursor = 0;
};

class SystemCtrl : public ParentCtrl {
	ProgressIndicator prog;
	BooleansDraw bools;
	ArrayCtrl stats;
	Splitter hsplit;
	SliderCtrl slider;
	Button load_sources, load_booleans, load_stats;
	bool running = false, stopped = true;
	
	void LoadSources();
	void LoadBooleans();
	void LoadStats();
public:
	typedef SystemCtrl CLASSNAME;
	SystemCtrl();
	~SystemCtrl() {Stop();}
	
	void Enable() {load_sources.Enable(); load_booleans.Enable(); load_stats.Enable();}
	void Disable() {load_sources.Disable(); load_booleans.Disable(); load_stats.Disable();}
	void Data();
	void StartLoadSources() {Stop(); running = true; stopped = false; Disable(); Thread::Start(THISBACK(LoadSources));}
	void StartLoadBooleans() {Stop(); running = true; stopped = false; Disable(); Thread::Start(THISBACK(LoadBooleans));}
	void StartLoadStats() {Stop(); running = true; stopped = false; Disable(); Thread::Start(THISBACK(LoadStats));}
	void Stop() {running = false; while (!stopped) Sleep(100);}
	void SetProg(int a, int t) {prog.Set(a, t);}
	
	
	// NOTE: update SnapStatVector or make macros
	static const int period_count = 6;
	static const int volat_div = 6;
	static const int row_size = period_count * (9 + volat_div);
};


}


#endif
