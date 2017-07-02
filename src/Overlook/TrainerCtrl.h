#ifndef _Overlook_TrainerCtrl_h_
#define _Overlook_TrainerCtrl_h_


namespace Overlook {
using namespace Upp;

class TrainerCtrl;

#define LAYOUTFILE <Overlook/TrainerCtrl.lay>
#include <CtrlCore/lay.h>


class TrainerConfiguration : public WithConfiguration<ParentCtrl> {
	Trainer* trainer;
	
public:
	typedef TrainerConfiguration CLASSNAME;
	TrainerConfiguration(Trainer& trainer);
	
	void Data();
	
	
};

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
	TrainerDraw draw;
	Button step_bwd, step_fwd;
	
public:
	typedef TrainerCtrl CLASSNAME;
	TrainerCtrl(Trainer& trainer);
	
	void Data();
	void SeekCur(int step);
	
};

class TrainerStatistics : public WithStatistics<ParentCtrl> {
	Trainer* trainer;
	
public:
	typedef TrainerStatistics CLASSNAME;
	TrainerStatistics(Trainer& trainer);
	
	void Data();
	
	
};

}

#endif
