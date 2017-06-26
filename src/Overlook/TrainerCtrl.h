#ifndef _Overlook_TrainerCtrl_h_
#define _Overlook_TrainerCtrl_h_

namespace Overlook {
using namespace Upp;

class TrainerCtrl;

class TrainerDraw : public Ctrl {
	TrainerCtrl* ctrl;
	
public:
	TrainerDraw(TrainerCtrl& ctrl);
	
	virtual void Paint(Draw& w);
	
};


class TrainerCtrl : public ParentCtrl {
	
protected:
	friend class TrainerDraw;
	
	Trainer* trainer;
	SliderCtrl time_slider;
	Label time_lbl;
	DropList tf_list;
	TrainerDraw draw;
	Button step_bwd, step_fwd;
	
public:
	typedef TrainerCtrl CLASSNAME;
	TrainerCtrl(Trainer& trainer);
	
	void Data();
	void SetTimeframe();
	void SeekCur(int step);
	
};

}

#endif
