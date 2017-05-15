#ifndef _Overlook_CustomCtrl_h_
#define _Overlook_CustomCtrl_h_

namespace Overlook {

class CustomCtrl : public ParentCtrl {
	
protected:
	friend class Overlook;
	
	int sym, tf;
	Core* core;
	
	void SetTf(int i) {tf = i;}
	void SetSymbol(int i) {sym = i;}
	
public:
	typedef CustomCtrl CLASSNAME;
	CustomCtrl() {
		core = NULL;
		sym = -1;
		tf = -1;
	}
	virtual ~CustomCtrl() {}
	virtual void Init(Core* c) {}
	virtual void RefreshData() {}
	
	
	Core& GetCore() {return *core;}
	
	
	
};

}

#endif
