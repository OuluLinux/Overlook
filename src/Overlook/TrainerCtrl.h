#ifndef _Overlook_TrainerCtrl_h_
#define _Overlook_TrainerCtrl_h_


namespace Overlook {
using namespace Upp;

class TrainerThreadCtrl;

#define LAYOUTFILE <Overlook/TrainerCtrl.lay>
#include <CtrlCore/lay.h>


class TrainerConfiguration : public WithConfiguration<ParentCtrl> {
	Trainer* trainer;
	
public:
	typedef TrainerConfiguration CLASSNAME;
	TrainerConfiguration(Trainer& trainer);
	
	void Data();
	void Apply();
	void Reset();
	
	
};

class TrainerDraw : public Ctrl {
	TrainerThreadCtrl* ctrl;
	
public:
	TrainerDraw(TrainerThreadCtrl& ctrl);
	
	virtual void Paint(Draw& w);
	
};

class TrainerThreadCtrl : public ParentCtrl {
	
protected:
	friend class TrainerDraw;
	
	Trainer* trainer;
	SliderCtrl time_slider;
	Label time_lbl;
	TrainerDraw draw;
	Splitter hsplit;
	ConvNet::SessionConvLayers conv;
	ConvNet::HeatmapTimeView timescroll;
	int thrd_id;
	bool init;
	
public:
	typedef TrainerThreadCtrl CLASSNAME;
	TrainerThreadCtrl(Trainer& trainer, int thrd_id);
	
	void Data();
	
};

class TrainerCtrl : public ParentCtrl {
protected:
	Trainer* trainer;
	Array<TrainerThreadCtrl> thrds;
	DropList thrdlist;
	bool init;
	
public:
	typedef TrainerCtrl CLASSNAME;
	TrainerCtrl(Trainer& trainer);
	
	void Data();
	void SetView();
	
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
