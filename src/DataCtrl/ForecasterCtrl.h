#ifndef _DataCtrl_ForecasterCtrl_h_
#define _DataCtrl_ForecasterCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;

class ForecasterDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	SlotPtr src, rnn;
	DataCore::Recurrent* rec;
	int sym, tf;
	
public:
	typedef ForecasterDraw CLASSNAME;
	ForecasterDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <DataCtrl/ForecasterCtrl.lay>
#include <CtrlCore/lay.h>

class ForecasterCtrl : public WithForecasterLayout<MetaNodeCtrl> {
	DataCore::Recurrent* rec;
	//Vector<double> ppl_list;
	
public:
	typedef ForecasterCtrl CLASSNAME;
	ForecasterCtrl();
	
	void Refresher();
	void Reset();
	/*double Median(Vector<double>& values);
	
	void SetWeekFromSlider();
	void SetLearningRate();
	void SetSampleTemperature(int i);
	void SetPreset(int i);
	void SetStats(double epoch, double ppl, int time);*/
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "forecasterctrl";}
	static String GetKeyStatic()  {return "forecasterctrl";}
	
};

}

#endif
