#ifndef _Overlook_TrainerCtrl_h_
#define _Overlook_TrainerCtrl_h_

namespace Overlook {
using namespace Upp;

class TrainerDraw : public Ctrl {
	
public:
	TrainerDraw();
	
	virtual void Paint(Draw& w);
	
};


class TrainerCtrl : public ParentCtrl {
	
public:
	TrainerCtrl(Trainer& trainer);
	
	void RefreshData();
	
};

}

#endif
