#ifndef _DataCtrl_RecurrentCtrl_h_
#define _DataCtrl_RecurrentCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;

class RecurrentDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	SlotPtr src, rnn;
	DataCore::Recurrent* rec;
	int sym, tf;
	
public:
	typedef RecurrentDraw CLASSNAME;
	RecurrentDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <DataCtrl/RecurrentCtrl.lay>
#include <CtrlCore/lay.h>

class RecurrentCtrl : public WithRecurrentLayout<MetaNodeCtrl> {
	DataCore::Recurrent* rec;
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
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "rnnctrl";}
	static String GetKeyStatic()  {return "rnnctrl";}
	
};

}

#endif
