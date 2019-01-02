#include "Overlook.h"

namespace Overlook {

void MultiChangeInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadSymbol(cl_sym, symbol, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(1, count, windowsize);
	LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	LoadDataPipOutput(ses, cl_sym, begin, count, postpips_count);
	ses.GetData().EndData();
}

void SingleChangeInterpolation::Run() {
	
}



void SingleChangeInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadNetSymbols(cl_sym, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(cl_sym.GetSymbolCount(), count, cl_sym.GetSymbolCount() * windowsize);
	LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	LoadDataPipOutput(ses, cl_sym, begin, count, postpips_count);
	ses.GetData().EndData();
}

void MultiChangeInterpolation::Run() {
	
}



void MultinetChangeInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadNets(cl_net, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(cl_net.GetSymbolCount(), count, cl_net.GetSymbolCount() * windowsize);
	LoadDataPriceInput(ses, cl_net, begin, count, windowsize);
	LoadDataPipOutput(ses, cl_net, begin, count, postpips_count);
	ses.GetData().EndData();
}

void MultinetChangeInterpolation::Run() {
	
	TrainSession(ses, 200000);
	
	//qtf_test_result = TestSessionMultiSym
}



void SingleVolatInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadSymbol(cl_sym, symbol, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(1, count, windowsize);
	LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	LoadDataVolatOutput(ses, cl_sym, begin, count, ticks);
	ses.GetData().EndData();
}

void SingleVolatInterpolation::Run() {
	
}



void MultiVolatInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadNetSymbols(cl_sym, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(cl_sym.GetSymbolCount(), count, cl_sym.GetSymbolCount() * windowsize);
	LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	LoadDataVolatOutput(ses, cl_sym, begin, count, ticks);
	ses.GetData().EndData();
}

void MultiVolatInterpolation::Run() {
	
}



void MultinetVolatInterpolation::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5;
	
	LoadNets(cl_net, ScriptCore::fast_tf);
	
	ses.GetData().BeginDataResult(cl_net.GetSymbolCount(), count, cl_net.GetSymbolCount() * windowsize);
	LoadDataPriceInput(ses, cl_net, begin, count, windowsize);
	LoadDataVolatOutput(ses, cl_net, begin, count, ticks);
	ses.GetData().EndData();
}

void MultinetVolatInterpolation::Run() {
	
}



}

