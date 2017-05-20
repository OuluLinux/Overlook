#ifndef _Overlook_NarxCtrl_h_
#define _Overlook_NarxCtrl_h_

namespace Overlook {

using namespace ConvNet;

class NarxDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	Core *src, *narx_slot;
	int sym, tf;
	
public:
	typedef NarxDraw CLASSNAME;
	NarxDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <Overlook/NarxCtrl.lay>
#include <CtrlCore/lay.h>

class NarxCtrl : public WithNarxLayout<CustomCtrl> {
	Vector<double> ppl_list;
	
public:
	typedef NarxCtrl CLASSNAME;
	NarxCtrl();
	
	void Refresher();
	void Reset();
	double Median(Vector<double>& values);
	
	void SetWeekFromSlider();
	void SetLearningRate();
	void SetSampleTemperature(int i);
	void SetStats(double epoch, double ppl, int time);
	
	virtual void Arguments(ArgumentBase& args);
	virtual void Init();
	
};

}

#endif
