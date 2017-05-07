#include "DataCtrl.h"

namespace DataCtrl {
using namespace DataCore;


AnalyzerCtrl::AnalyzerCtrl() {
	CtrlLayout(tool);
	CtrlLayout(value);
	CtrlLayout(forecaster);
	CtrlLayout(usesforecaster);
	CtrlLayout(order);
	
	Add(tool.TopPos(0,30).HSizePos());
	Add(tabs.VSizePos(30,0).HSizePos());
	tabs.Add(value, "Static values");
	tabs.Add(value);
	tabs.Add(forecaster, "Forecasters");
	tabs.Add(forecaster);
	tabs.Add(usesforecaster, "User of forecasters");
	tabs.Add(usesforecaster);
	tabs.Add(order, "Order maker");
	tabs.Add(order);
	
	tool.data.SetData(0);
	
	value.checknow <<= THISBACK(CheckNow);
	
	forecaster.hsplit.Horz();
	forecaster.hsplit << fcast_diff << fcast_graph;
	// fcast_diff shows difference between forecast and real values
	// fcast_graph shows changes in differences during training
	
	usesforecaster.hsplit.Horz();
	usesforecaster.hsplit << fcuse_statsplit << fcuse_graphsplit;
	fcuse_statsplit.Vert();
	fcuse_statsplit << fcuse_ideal << fcuse_fcast << fcuse_cmp;
	fcuse_graphsplit.Vert();
	fcuse_graphsplit << fcuse_idealgraph << fcuse_fcastgraph << fcuse_cmpgraph;
	// fcuse_ideal shows a slot based on real values instead of forecast (ideal, unrealistic)
	// fcuse_fcast shows a slot based on forecast (realistic)
	// fcuse_cmp shows comparison between previous ideal and realistic use cases
	// fcuse_idealgraph, fcuse_fcastgraph and fcuse_cmpgraph shows changes in stats during training
	
	order.hsplit.Horz();
	order.hsplit << order_statsplit << order_graphsplit;
	order_statsplit.Vert();
	order_statsplit << order_ideal << order_fcast << order_cmp;
	order_graphsplit.Vert();
	order_graphsplit << order_idealgraph << order_fcastgraph << order_cmpgraph;
	// order_ideal shows an ideal sequence of orders (ideal, unrealistic)
	// order_fcast shows an sequence of orders what the slot caused (realistic)
	// order_idealgraph, order_fcastgraph and order_cmpgraph shows changes in stats during training
	
}

AnalyzerCtrl::~AnalyzerCtrl() {
	
}

void AnalyzerCtrl::CheckNow() {
	
	
	
}

void AnalyzerCtrl::Refresher() {
	
}

void AnalyzerCtrl::SetArguments(const VectorMap<String, Value>& args) {
	
}

void AnalyzerCtrl::Init() {
	
}


}
