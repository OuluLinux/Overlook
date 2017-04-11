#ifndef _DataCtrl_AgentCtrl_h_
#define _DataCtrl_AgentCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;

class AgentDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	SlotPtr src, rnn;
	DataCore::Recurrent* rec;
	int sym, tf;
	
public:
	typedef AgentDraw CLASSNAME;
	AgentDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <DataCtrl/AgentCtrl.lay>
#include <CtrlCore/lay.h>

class AgentCtrl : public WithAgentLayout<MetaNodeCtrl> {
	DataCore::Recurrent* rec;
	//Vector<double> ppl_list;
	
public:
	typedef AgentCtrl CLASSNAME;
	AgentCtrl();
	
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
	virtual String GetKey() const {return "agentctrl";}
	static String GetKeyStatic()  {return "agentctrl";}
	
};

}

#endif
