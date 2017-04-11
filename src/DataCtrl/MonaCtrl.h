#ifndef _DataCtrl_MonaCtrl_h_
#define _DataCtrl_MonaCtrl_h_

#include <ConvNetCtrl/ConvNetCtrl.h>
#include <GraphLib/GraphLib.h>
#include "Container.h"

namespace DataCtrl {
using namespace DataCore;
using namespace ConvNet;


#define LAYOUTFILE <DataCtrl/MonaCtrl.lay>
#include <CtrlCore/lay.h>

class MonaCtrl : public WithMonaLayout<MetaNodeCtrl> {
	DataCore::Recurrent* rec;
	//Vector<double> ppl_list;
	
public:
	typedef MonaCtrl CLASSNAME;
	MonaCtrl();
	
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
	virtual String GetKey() const {return "monactrl";}
	static String GetKeyStatic()  {return "monactrl";}
	
};

}

#endif
