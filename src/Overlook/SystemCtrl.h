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
	ArrayCtrl stats, strands;
	Splitter hsplit;
	SliderCtrl slider;
	Button  load_all;
	bool running = false, stopped = true;
	
public:
	typedef SystemCtrl CLASSNAME;
	SystemCtrl();
	~SystemCtrl() {Stop();}
	
	void Enable() {load_all.Enable();}
	void Disable() {load_all.Disable();}
	void Data();
	void StartLoadAll() {Stop(); running = true; stopped = false; Disable(); Thread::Start(THISBACK(LoadAll));}
	void Stop() {running = false; while (!stopped) Sleep(100);}
	void SetProg(int a, int t) {prog.Set(a, t);}
	void LoadAll();
	
	
};


}


#endif
