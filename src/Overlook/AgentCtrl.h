#ifndef _Overlook_AgentCtrl_h_
#define _Overlook_AgentCtrl_h_

namespace Overlook {

using namespace ConvNet;

class AgentDraw : public Ctrl {
	Vector<Vector<Point> > pts;
	Vector<double> tmp;
	Core *src, *rnn;
	Recurrent* rec;
	int sym, tf;
	
public:
	typedef AgentDraw CLASSNAME;
	AgentDraw();
	
	virtual void Paint(Draw& w);
	
	
	int week;
	
};

#define LAYOUTFILE <Overlook/AgentCtrl.lay>
#include <CtrlCore/lay.h>

class AgentCtrl : public WithAgentLayout<CustomCtrl> {
	Recurrent* rec;
	//Vector<double> ppl_list;
	GraphLib::SpringGraph graph;
	
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
	
};

}

#endif
