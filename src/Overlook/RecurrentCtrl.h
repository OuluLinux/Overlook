#ifndef _Overlook_RecurrentCtrl_h_
#define _Overlook_RecurrentCtrl_h_

namespace Overlook {

using namespace ConvNet;

class RecurrentDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	Core *src, *rnn;
	Recurrent* rec;
	int sym, tf;
	
public:
	typedef RecurrentDraw CLASSNAME;
	RecurrentDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <Overlook/RecurrentCtrl.lay>
#include <CtrlCore/lay.h>

class RecurrentCtrl : public WithRecurrentLayout<CustomCtrl> {
	Recurrent* rec;
	Vector<double> ppl_list;
	String slotpath;
	
public:
	typedef RecurrentCtrl CLASSNAME;
	RecurrentCtrl();
	
	void Refresher();
	void Reset();
	double Median(Vector<double>& values);
	
	void SetWeekFromSlider();
	void SetLearningRate();
	void SetSampleTemperature(int i);
	void SetPreset(int i);
	void SetStats(double epoch, double ppl, int time);
	
	
	virtual void Init();
	
};

}

#endif
