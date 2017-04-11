#ifndef _DataCtrl_DQNCtrl_h_
#define _DataCtrl_DQNCtrl_h_


#include <ConvNetCtrl/ConvNetCtrl.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;

class DQNDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	SlotPtr src, rnn;
	DataCore::Recurrent* rec;
	int sym, tf;
	
public:
	typedef DQNDraw CLASSNAME;
	DQNDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <DataCtrl/DQNCtrl.lay>
#include <CtrlCore/lay.h>

class DQNCtrl : public WithDQNLayout<MetaNodeCtrl> {
	DataCore::Recurrent* rec;
	//Vector<double> ppl_list;
	
public:
	typedef DQNCtrl CLASSNAME;
	DQNCtrl();
	
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
	virtual String GetKey() const {return "dqnctrl";}
	static String GetKeyStatic()  {return "dqnctrl";}
	
};

}

#endif
