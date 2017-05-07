#ifndef _DataCtrl_AnalyzerCtrl_h_
#define _DataCtrl_AnalyzerCtrl_h_

#include <ConvNetCtrl/TrainingGraph.h>

#include "Container.h"

namespace DataCtrl {
using namespace DataCore;

#define LAYOUTFILE <DataCtrl/AnalyzerCtrl.lay>
#include <CtrlCore/lay.h>

class AnalyzerCtrl : public MetaNodeCtrl {
	TabCtrl tabs;
	WithToolBar<ParentCtrl> tool;
	WithValueAnalyzer<ParentCtrl> value;
	WithForecasterAnalyzer<ParentCtrl> forecaster;
	WithForecastUserAnalyzer<ParentCtrl> usesforecaster;
	WithOrderAnalyzer<ParentCtrl> order;
	Splitter fcuse_statsplit, fcuse_graphsplit;
	Splitter order_statsplit, order_graphsplit;
	ArrayCtrl fcast_diff;
	ArrayCtrl fcuse_ideal, fcuse_fcast, fcuse_cmp;
	ArrayCtrl order_ideal, order_fcast, order_cmp;
	ConvNet::TrainingGraph fcast_graph;
	ConvNet::TrainingGraph fcuse_idealgraph, fcuse_fcastgraph, fcuse_cmpgraph;
	ConvNet::TrainingGraph order_idealgraph, order_fcastgraph, order_cmpgraph;
	
public:
	typedef AnalyzerCtrl CLASSNAME;
	AnalyzerCtrl();
	~AnalyzerCtrl();
	
	void CheckNow();
	void Refresher();
	
	virtual void SetArguments(const VectorMap<String, Value>& args);
	virtual void Init();
	virtual String GetKey() const {return "analyzerctrl";}
	static String GetKeyStatic()  {return "analyzerctrl";}
	
};

}

#endif
