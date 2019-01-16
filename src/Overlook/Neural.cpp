#include "Overlook.h"

namespace Overlook {

void SingleChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * (train_percent * 0.01) - begin;
	
	LoadSymbol(cl_sym, symbol, tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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






void MultinetChangeNeural::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNets(cl_net, tf);
	
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadSymbol(cl_sym, symbol, tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNetSymbols(cl_sym, tf);
	
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	int begin = 100;
	int count = idx.GetCount() * 0.5 - begin;
	
	LoadNets(cl_net, tf);
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
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
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
















void DqnAgent::Init() {
	DataBridgeCommon& dbc = GetDataBridgeCommon();
	const Index<Time>& idx = dbc.GetTimeIndex(tf);
	begin = 100;
	count = idx.GetCount() * (train_percent * 0.01) - begin;
	
	LoadSymbol(cl_sym, symbol, tf);
	LoadSymbolIndicators(cl_indi, symbol, tf);
	
	int input_count = windowsize * cl_indi.GetIndiCount() * cl_indi.GetSymbolCount() + SENSOR_COUNT;
	InitDqnDefault(dqn, input_count, ACTION_COUNT);
	
	cl_wait.AddSymbol(GetSystem().GetSymbol(symbol));
	cl_wait.AddTf(tf);
	cl_wait.AddIndi(System::Find<SpeculationOscillator>());
	cl_wait.AddIndi(System::Find<VolumeOscillator>());
	cl_wait.AddIndi(System::Find<PipChange>());
	cl_wait.AddIndi(System::Find<UnsustainableMovement>());
	cl_wait.Init();
	cl_wait.Refresh();
}

void DqnAgent::Run() {
	if (dqn.GetExperienceCount() != 0)
		return;
	
	qtf_test_result= DeQtf("Training...");
	
	#ifdef flagDEBUG
	total = 200000;
	#else
	total = 20000000;
	#endif
	
	//TrainDqn(dqn, total, actual);
	
	double point = cl_sym.GetDataBridge(0)->GetPoint();
	ConstBuffer& open_buf = cl_sym.GetBuffer(0, 0, 0);
	ConstBuffer& low_buf = cl_sym.GetBuffer(0, 0, 1);
	ConstBuffer& high_buf = cl_sym.GetBuffer(0, 0, 2);
	
	ConstBuffer& specosc = cl_wait.GetBuffer(0, 0, 0);
	ConstBuffer& volosc = cl_wait.GetBuffer(0, 1, 0);
	ConstLabelSignal& pipch = cl_wait.GetLabelSignal(0, 2, 0);
	ConstLabelSignal& unsust = cl_wait.GetLabelSignal(0, 3, 0);
	
	OnlineAverage1 reward_av;
	int reward_period = 1000;
	
	
	Vector<double> input;
	int pos = begin;
	for(; actual < total; actual++) {
		double max_epsilon = 0.2;
		double epsilon = (1.0 - actual / total) * max_epsilon;
		dqn.SetEpsilon(epsilon);
		
		LoadInput(pos, input);
		int action = dqn.Act(input);
		prev_action = action;
		double reward = 0;
		
		if (action < BET_ACTIONS) {
			bool sig = action % 2;
			int level = action / 2;
			int factor = 1 << level;
			
			double open = open_buf.Get(pos);
			double lo = open - postpips_count * point;
			double hi = open + postpips_count * point;
			
			bool result = false;
			
			for(int k = pos+1; k < open_buf.GetCount(); k++) {
				double low  = low_buf.Get(k-1);
				double high = high_buf.Get(k-1);
				if (low <= lo) {
					result = true;
					pos = k;
					break;
				}
				else if (high >= hi) {
					result = false;
					pos = k;
					break;
				}
			}
			
			bool succ = result == sig;
			
			reward = (succ ? +1 : -1) * factor * postpips_count;
			
			reward_av.Add(reward);
			if (reward_av.count >= reward_period) {
				qtf_test_result = DeQtf("average_reward_without_spread = " + DblStr(reward_av.GetMean()));
				reward_av.Clear();
			}
		}
		else {
			action -= BET_ACTIONS;
			
			// Wait until speculation oscillator > 0
			if (action == 0) {
				while (true) {
					if (pos >= begin + count)
						pos = begin;
					if (specosc.Get(pos) > 0)
						break;
					pos++;
				}
			}
			// Wait until volume oscillator >= 0.5
			else if (action == 1) {
				while (true) {
					if (pos >= begin + count)
						pos = begin;
					if (volosc.Get(pos) >= 0.5)
						break;
					pos++;
				}
			}
			// Wait until volume oscillator >= 0.9
			else if (action == 2) {
				while (true) {
					if (pos >= begin + count)
						pos = begin;
					if (volosc.Get(pos) >= 0.9)
						break;
					pos++;
				}
			}
			// Wait until pip change signal
			else if (action == 3) {
				while (true) {
					if (pos >= begin + count)
						pos = begin;
					if (pipch.enabled.Get(pos))
						break;
					pos++;
				}
			}
			// Wait until unsustainable movement signal changes
			else if (action == 4) {
				while (true) {
					if (pos >= begin + count)
						pos = begin;
					if (unsust.signal.Get(pos) != unsust.signal.Get(pos-1))
						break;
					pos++;
				}
			}
			else Panic("Invalid action");
		}
	}
}

void DqnAgent::LoadInput(int pos, Vector<double>& input) {
	input.SetCount(cl_indi.GetSymbolCount() * cl_indi.GetIndiCount() * windowsize + SENSOR_COUNT);
	
	int col = 0;
	for(int j = 0; j < cl_indi.GetSymbolCount(); j++) {
		for(int k = 0; k < cl_indi.GetIndiCount(); k++) {
			ConstBuffer& buf = cl_indi.GetBuffer(j, k, 0);
			for(int l = 0; l < windowsize; l++) {
				int pos2 = max(0, pos - l - 1);
				double cur = buf.Get(pos2);
				input[col++] = cur;
			}
		}
	}
	
	for(int j = 0; j < ACTION_COUNT; j++) {
		input[col++] = (prev_action == j ? +1 : 0);
	}
}

void DqnAgent::GetSignal(int symbol, LabelSignal& signal) {
	
}

}

