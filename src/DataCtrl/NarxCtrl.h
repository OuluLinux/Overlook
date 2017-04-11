#ifndef _DataCtrl_NarxCtrl_h_
#define _DataCtrl_NarxCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;

class NarxDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	SlotPtr src, narx_slot;
	Narx::NARX* narx;
	int sym, tf;
	
public:
	typedef NarxDraw CLASSNAME;
	NarxDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <DataCtrl/NarxCtrl.lay>
#include <CtrlCore/lay.h>

class NarxCtrl : public WithNarxLayout<MetaNodeCtrl> {
	Narx::NARX* narx;
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
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "rnnctrl";}
	static String GetKeyStatic()  {return "rnnctrl";}
	
};

}

#endif
