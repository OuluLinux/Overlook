#include "Overlook.h"

namespace Overlook {

void SingleChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * (train_percent * 0.01) - begin;
	
	LoadSymbol(cl_sym, symbol, tf);
	LoadSymbol(cl_sym0, symbol, 0);
	InitSessionDefault(ses, windowsize, 1);
	
	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(1, count, windowsize);
		LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadSymbolIndicators(cl_indi, symbol, tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");
	
	LoadDataPipOutput(ses, tf, cl_sym0, begin, count, postpips_count);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_sym, cl_sym0, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_sym, cl_sym0, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_sym, cl_sym0, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_sym, cl_sym0, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
	
	LabelSignal change_single;
	GetSignal(symbol, change_single);
	qtf_test_result << DeQtf("Unknown test trade\n");
	qtf_test_result << TestTrade(symbol, tf, postpips_count, change_single);
}

void SingleChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	if (symbol != this->symbol)
		return;
	
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNetSymbols(cl_sym, tf);
	LoadNetSymbols(cl_sym0, 0);
	InitSessionDefault(ses, cl_sym.GetSymbolCount() * windowsize, cl_sym.GetSymbolCount());
	
	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(cl_sym.GetSymbolCount(), count, cl_sym.GetSymbolCount() * windowsize);
		LoadDataPriceInput(ses, cl_sym, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadNetIndicators(cl_indi, tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");
	
	LoadDataPipOutput(ses, tf, cl_sym0, begin, count, postpips_count);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;
	
	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_sym, cl_sym0, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_sym, cl_sym0, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_sym, cl_sym0, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_sym, cl_sym0, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
	
	
	int symbol = GetSystem().FindSymbol(CommonSpreads().GetKey(0));
	LabelSignal change_single;
	GetSignal(symbol, change_single);
	qtf_test_result << DeQtf("Unknown test trade\n");
	qtf_test_result << TestTrade(symbol, tf, postpips_count, change_single);
}

void MultiChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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













void Change2Tf::Init() {
	System& sys = GetSystem();
	
	sl0.AddFactory(sys.FindScript<SingleChangeNeural>())
		.AddArg(tf0)
		.AddArg(symbol)
		.AddArg(train_percent)
		.AddArg(input_enum)
		.AddArg(postpips_count)
		.AddArg(windowsize);
	sl0.Init();
	
	sl1.AddFactory(sys.FindScript<SingleChangeNeural>())
		.AddArg(tf1)
		.AddArg(symbol)
		.AddArg(train_percent)
		.AddArg(input_enum)
		.AddArg(postpips_count)
		.AddArg(windowsize);
	sl1.Init();
	
}

void Change2Tf::Run() {
	
	LabelSignal change_single;
	GetSignal(symbol, change_single);
	qtf_test_result << DeQtf("Unknown test trade\n");
	qtf_test_result << TestTrade(symbol, tf1, postpips_count, change_single);
}

void Change2Tf::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx0 = dbc.GetTimeIndex(tf0);
	const Index<Time>& idx1 = dbc.GetTimeIndex(tf1);
	
	LabelSignal ls0;
	sl0.GetScript(0).GetSignal(symbol, ls0);
	
	sl1.GetScript(0).GetSignal(symbol, signal);
	
	
	for(int i = 0; i < idx1.GetCount(); i++) {
		Time t = idx1[i];
		t = SyncTime(tf0, t);
		int j = idx0.Find(t);
		if (j == -1) {
			signal.enabled.Set(i, 0);
		}
		else {
			bool sig0 = ls0.signal.Get(j);
			bool sig1 = signal.signal.Get(i);
			if (sig0 != sig1)
				signal.enabled.Set(i, 0);
		}
	}
}








void MultinetChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;

	LoadNets(cl_net, tf);
	LoadNets(cl_net0, 0);

	if (ses.GetStepCount() != 0)
		return;

	InitSessionDefault(ses, cl_net.GetSymbolCount() * windowsize, cl_net.GetSymbolCount());

	if (input_enum == PRICE) {
		ses.GetData().BeginDataResult(cl_net.GetSymbolCount(), count, cl_net.GetSymbolCount() * windowsize);
		LoadDataPriceInput(ses, cl_net, begin, count, windowsize);
	}
	else if (input_enum == INDI) {
		LoadNetsIndicators(cl_indi, tf);
		ses.GetData().BeginDataResult(1, count, windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount());
		LoadDataIndiInput(ses, cl_indi, begin, count, windowsize);
	}
	else Panic("TODO");

	LoadDataPipOutput(ses, tf, cl_net0, begin, count, postpips_count);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int inter_begin = 100;
	int inter_count = idx.GetCount() * (train_percent * 0.01) - inter_begin;
	int extra_begin = idx.GetCount() * (train_percent * 0.01);
	int extra_count = idx.GetCount() - 10 - extra_begin;

	qtf_test_result = "";
	if (input_enum == PRICE) {
		qtf_test_result << DeQtf("Known price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_net, cl_net0, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown price in / pip out\n");
		qtf_test_result << TestPriceInPipOut(ses, tf, cl_net, cl_net0, extra_begin, extra_count, windowsize, postpips_count);
	}
	else if (input_enum == INDI) {
		qtf_test_result << DeQtf("Known indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_net, cl_net0, cl_indi, inter_begin, inter_count, windowsize, postpips_count);
		qtf_test_result << DeQtf("Unknown indicators in / pip out\n");
		qtf_test_result << TestIndicatorsInPipOut(ses, tf, cl_net, cl_net0, cl_indi, extra_begin, extra_count, windowsize, postpips_count);
	}
	else Panic("TODO");
	
	int symbol = GetSystem().FindSymbol(CommonSpreads().GetKey(0));
	LabelSignal change_single;
	GetSignal(symbol, change_single);
	qtf_test_result << DeQtf("Unknown test trade\n");
	qtf_test_result << TestTrade(symbol, tf, postpips_count, change_single);
}

void MultinetChangeNeural::GetSignal(int symbol, LabelSignal& signal) {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	
	VectorMap<int, double> map;
	
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
		
		#if 0
		double sum = 0.0;
		for(int j = 0; j < sys.GetNetCount(); j++) {
			System::NetSetting& net = sys.GetNet(j);
			int sym_sig = net.symbol_ids[sym_pos];
			double pred = out.Get(j);
			sum += sym_sig * pred;
		}

		signal.enabled.Set(pos, fabs(sum) > 0.0);
		signal.signal.Set(pos, sum < 0.0);
		#else
		for(int j = 0; j < map.GetCount(); j++)
			map[j] = 0;
		for(int j = 0; j < sys.GetNetCount(); j++) {
			System::NetSetting& net = sys.GetNet(j);
			double pred = out.Get(j);
			for(int k = 0; k < net.symbols.GetCount(); k++) {
				int sym_sig = net.symbol_ids[k];
				map.GetAdd(k, 0) += sym_sig * pred;
			}
		}
		struct Sorter {bool operator()(double a, double b) const {return fabs(a) > fabs(b);}};
		SortByValue(map, Sorter());
		int j = map.Find(sym_pos);
		double pred = map[j];
		signal.enabled.Set(pos, j < 1);
		signal.signal.Set(pos, pred < 0);
		#endif
	}
}



}

