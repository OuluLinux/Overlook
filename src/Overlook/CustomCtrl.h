#ifndef _Overlook_CustomCtrl_h_
#define _Overlook_CustomCtrl_h_

namespace Overlook {

class CustomCtrl : public ParentCtrl {
	
protected:
	friend class Overlook;
	
	int sym, tf;
	bool inited;
	
	void SetTf(int i) {tf = i;}
	void SetSymbol(int i) {sym = i;}
	void SetInited(bool b=true) {inited = b;}
	
public:
	typedef CustomCtrl CLASSNAME;
	CustomCtrl() {
		inited = false;
		sym = -1;
		tf = -1;
	}
	
	virtual void Init(Core* c) {}
	virtual void RefreshData() {}
	
	
	
	bool IsInited() const {return inited;}
	
	
	
};

}

#endif
