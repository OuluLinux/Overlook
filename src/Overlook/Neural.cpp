#include "Overlook.h"

namespace Overlook {

void SingleChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * (train_percent * 0.01) - begin;
	
	LoadSymbol(cl_sym, symbol, ScriptCore::fast_tf);
	InitSessionDefault(ses, windowsize, 1);
	
	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(1, count, windowsize);
		LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadSymbolIndicators(cl_indi, symbol, ScriptCore::fast_tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");
	
	LoadDataPipOutput(ses, cl_sym, begin, count, postpips_count);
	ses.GetData().EndData();
}

void SingleChangeNeural::Run() {
	if (ses.GetStepCount() != 0)
		return;
	
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();
	
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_sym, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_sym, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_sym, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_sym, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
}

void SingleChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	signal.signal.SetCount(idx.GetCount());
	signal.enabled.SetCount(idx.GetCount());
	
	System& sys = GetSystem();
	System::NetSetting& net0 = sys.GetNet(0);
	int sym_pos = net0.symbol_ids.Find(symbol);
	if (sym_pos == -1)
		return;
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	if (input_enum == PRICE) {
		in.Init(1, 1, cl_sym.GetSymbolCount() * windowsize);
	}
	else if (input_enum == INDI) {
		in.Init(1, 1, cl_indi.GetSymbolCount() * cl_indi.GetIndiCount() * windowsize);
	}
	else Panic("TODO");
	
	for(int i = 0; i < extra_count; i++) {
		int pos = extra_begin + i;
		
		if (input_enum == PRICE) {
			LoadVolumePriceInput(cl_sym, pos, in, windowsize);
		}
		else if (input_enum == INDI) {
			LoadVolumeIndicatorsInput(cl_indi, pos, in, windowsize);
		}
		else Panic("TODO");
		
		ConvNet::Volume& out = net.Forward(in);
		
		double pred = out.Get(0);
		
		signal.enabled.Set(pos, fabs(pred) > 0.0);
		signal.signal.Set(pos, pred < 0.0);
	}
}





void MultiChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNetSymbols(cl_sym, ScriptCore::fast_tf);
	InitSessionDefault(ses, cl_sym.GetSymbolCount() * windowsize, cl_sym.GetSymbolCount());
	
	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(cl_sym.GetSymbolCount(), count, cl_sym.GetSymbolCount() * windowsize);
		LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadNetIndicators(cl_indi, ScriptCore::fast_tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");
	
	LoadDataPipOutput(ses, cl_sym, begin, count, postpips_count);
	ses.GetData().EndData();
}

void MultiChangeNeural::Run() {
	if (ses.GetStepCount() != 0)
		return;
	
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();
	
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_sym, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_sym, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_sym, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_sym, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
}

void MultiChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	signal.signal.SetCount(idx.GetCount());
	signal.enabled.SetCount(idx.GetCount());
	
	System& sys = GetSystem();
	System::NetSetting& net0 = sys.GetNet(0);
	int sym_pos = net0.symbol_ids.Find(symbol);
	if (sym_pos == -1)
		return;
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	if (input_enum == PRICE) {
		in.Init(1, 1, cl_sym.GetSymbolCount() * windowsize);
	}
	else if (input_enum == INDI) {
		in.Init(1, 1, cl_indi.GetSymbolCount() * cl_indi.GetIndiCount() * windowsize);
	}
	else Panic("TODO");
	
	for(int i = 0; i < extra_count; i++) {
		int pos = extra_begin + i;
		
		if (input_enum == PRICE) {
			LoadVolumePriceInput(cl_sym, pos, in, windowsize);
		}
		else if (input_enum == INDI) {
			LoadVolumeIndicatorsInput(cl_indi, pos, in, windowsize);
		}
		else Panic("TODO");
		
		ConvNet::Volume& out = net.Forward(in);
		
		double pred = out.Get(sym_pos);
		
		signal.enabled.Set(pos, fabs(pred) > 0.0);
		signal.signal.Set(pos, pred < 0.0);
	}
}






void MultinetChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNets(cl_net, ScriptCore::fast_tf);
	
	if (ses.GetStepCount() != 0)
		return;
	
	InitSessionDefault(ses, cl_net.GetSymbolCount() * windowsize, cl_net.GetSymbolCount());
	
	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(cl_net.GetSymbolCount(), count, cl_net.GetSymbolCount() * windowsize);
		LoadDataPriceInput(ses, cl_net, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadNetsIndicators(cl_indi, ScriptCore::fast_tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");
	
	LoadDataPipOutput(ses, cl_net, begin, count, postpips_count);
	ses.GetData().EndData();
}

void MultinetChangeNeural::Run() {
	if (ses.GetStepCount() != 0)
		return;
	
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_net, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, cl_net, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_net, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, cl_net, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
}

void MultinetChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	signal.signal.SetCount(idx.GetCount());
	signal.enabled.SetCount(idx.GetCount());
	
	System& sys = GetSystem();
	System::NetSetting& net0 = sys.GetNet(0);
	int sym_pos = net0.symbol_ids.Find(symbol);
	if (sym_pos == -1)
		return;
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	if (input_enum == PRICE) {
		in.Init(1, 1, cl_net.GetSymbolCount() * windowsize);
	}
	else if (input_enum == INDI) {
		in.Init(1, 1, cl_indi.GetSymbolCount() * cl_indi.GetIndiCount() * windowsize);
	}
	else Panic("TODO");
	
	for(int i = 0; i < extra_count; i++) {
		int pos = extra_begin + i;
		
		if (input_enum == PRICE) {
			LoadVolumePriceInput(cl_net, pos, in, windowsize);
		}
		else if (input_enum == INDI) {
			LoadVolumeIndicatorsInput(cl_indi, pos, in, windowsize);
		}
		else Panic("TODO");
		
		ConvNet::Volume& out = net.Forward(in);
		
		double sum = 0.0;
		for(int j = 0; j < sys.GetNetCount(); j++) {
			System::NetSetting& net = sys.GetNet(j);
			int sym_sig = net.symbol_ids[sym_pos];
			double pred = out.Get(j);
			sum += sym_sig * pred;
		}
		
		signal.enabled.Set(pos, fabs(sum) > 0.0);
		signal.signal.Set(pos, sum < 0.0);
	}
}




void SingleVolatNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadSymbol(cl_sym, symbol, ScriptCore::fast_tf);
	InitSessionDefault(ses, windowsize, 1);
	
	ses.GetData().BeginDataResult(1, count, windowsize);
	LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	LoadDataVolatOutput(ses, cl_sym, begin, count, ticks);
	ses.GetData().EndData();
}

void SingleVolatNeural::Run() {
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();
	
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("Known price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_sym, inter_begin, inter_count, windowsize, ticks);
	qtf_test_result << DeQtf("Unknown price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_sym, extra_begin, extra_count, windowsize, ticks);
}



void MultiVolatNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNetSymbols(cl_sym, ScriptCore::fast_tf);
	
	if (ses.GetStepCount() == 0) {
		InitSessionDefault(ses, cl_sym.GetSymbolCount() * windowsize, cl_sym.GetSymbolCount());
		
		ses.GetData().BeginDataResult(cl_sym.GetSymbolCount(), count, cl_sym.GetSymbolCount() * windowsize);
		LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
		LoadDataVolatOutput(ses, cl_sym, begin, count, ticks);
		ses.GetData().EndData();
	}
}

void MultiVolatNeural::Run() {
	if (ses.GetStepCount() != 0)
		return;
	
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();

	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	#ifndef flagDEBUG
	qtf_test_result << DeQtf("Known price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_sym, inter_begin, inter_count, windowsize, ticks);
	qtf_test_result << DeQtf("Unknown price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_sym, extra_begin, extra_count, windowsize, ticks);
	#endif
}

void MultiVolatNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	signal.signal.SetCount(idx.GetCount());
	signal.enabled.SetCount(idx.GetCount());
	
	System& sys = GetSystem();
	System::NetSetting& net0 = sys.GetNet(0);
	int sym_pos = net0.symbol_ids.Find(symbol);
	if (sym_pos == -1)
		return;
	
	ConvNet::Net& net = ses.GetNetwork();
	ConvNet::Volume in;
	in.Init(1, 1, cl_sym.GetSymbolCount() * windowsize);
	
	double point = cl_sym.GetDataBridge(sym_pos)->GetPoint();
	
	for(int i = 0; i < extra_count; i++) {
		int pos = extra_begin + i;
		
		LoadVolumePriceInput(cl_sym, pos, in, windowsize);
		
		ConvNet::Volume& out = net.Forward(in);
		
		double volat = out.Get(sym_pos) / 1000.0;
		int volat_pips = volat / point;
		
		signal.enabled.Set(pos, volat_pips >= postpips);
		signal.signal.Set(pos, false);
	}
}



void MultinetVolatNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNets(cl_net, ScriptCore::fast_tf);
	InitSessionDefault(ses, cl_net.GetSymbolCount() * windowsize, cl_net.GetSymbolCount());
	
	ses.GetData().BeginDataResult(cl_net.GetSymbolCount(), count, cl_net.GetSymbolCount() * windowsize);
	LoadDataPriceInput(ses, cl_net, begin, count, windowsize);
	LoadDataVolatOutput(ses, cl_net, begin, count, ticks);
	ses.GetData().EndData();
}

void MultinetVolatNeural::Run() {
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 20000;
	#else
	total = 200000;
	#endif
	TrainSession(ses, total, actual);
	ses.ClearData();
	
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(ScriptCore::fast_tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("Known price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_net, inter_begin, inter_count, windowsize, ticks);
	qtf_test_result << DeQtf("Unknown price in / volat out\n");
	qtf_test_result << TestPriceInVolatOut(ses, cl_net, extra_begin, extra_count, windowsize, ticks);
}

void MultinetVolatNeural::GetSignal(int symbol, LabelSignal& signal) {
	Panic("TODO");
}


}

