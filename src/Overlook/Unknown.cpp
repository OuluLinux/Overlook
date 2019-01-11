#include "Overlook.h"

namespace Overlook {

void MultinetSimple::Init() {
	sl_change.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change.Init();
	
	
	sl_volat.AddFactory(System::FindScript<MultiVolatNeural>())
		.AddArg(src_trainpercent)
		.AddArg(volat_enum)
		.AddArg(volat_ticks)
		.AddArg(volat_extrapolationmode)
		.AddArg(src_windowsize)
		.AddArg(change_postpips);
	sl_volat.Init();
	
}

void MultinetSimple::Run() {
	LabelSignal change;
	sl_change.GetScript(0).GetSignal(symbol, change);
	
	LabelSignal volat;
	sl_volat.GetScript(0).GetSignal(symbol, volat);
	
	int change_popcount = change.enabled.PopCount();
	int volat_popcount = volat.enabled.PopCount();
	
	change.enabled.And(volat.enabled);
	//change.enabled.LimitRight(change.enabled.GetCount() * 0.5);
	
	int popcount = change.enabled.PopCount();
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("change_popcount=" + IntStr(change_popcount) + "\n");
	qtf_test_result << DeQtf("volat_popcount=" + IntStr(volat_popcount) + "\n");
	qtf_test_result << DeQtf("popcount=" + IntStr(popcount) + "\n");
	qtf_test_result << DeQtf("Unknown multinet change & multi volat\n");
	qtf_test_result << TestTrade(symbol, change_postpips, change);
}


















void AllSame::Init() {
	sl_change_single.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips)
		.AddArg(src_windowsize);
	sl_change_single.Init();
	
	sl_change_multi.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_multi.Init();
	
	sl_change_net.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_net.Init();
	
}

void AllSame::Run() {
	System& sys = GetSystem();
	
	LabelSignal change_single, change_multi, change_net;
	sl_change_single.GetScript(0).GetSignal(symbol, change_single);
	sl_change_multi.GetScript(0).GetSignal(symbol, change_multi);
	sl_change_net.GetScript(0).GetSignal(symbol, change_net);
	
	int change_single_popcount = change_single.enabled.PopCount();
	int change_multi_popcount = change_multi.enabled.PopCount();
	int change_net_popcount = change_net.enabled.PopCount();
	
	if (change_multi_popcount)		change_single.And(change_multi);
	if (change_net_popcount)		change_single.And(change_net);
	
	/*CoreList cl;
	cl.AddSymbol(sys.GetSymbol(symbol));
	cl.AddIndi(sys.Find<VolumeOscillator>());
	cl.AddTf(ScriptCore::fast_tf);
	cl.Init();
	cl.Refresh();
	const LabelSignal& lb = cl.GetLabelSignal(0, 0, 0);
	change_single.And(lb);
	int pipchange_popcount = lb.enabled.PopCount();*/
	
	int popcount = change_single.enabled.PopCount();
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("change_single_popcount=" + IntStr(change_single_popcount) + "\n");
	qtf_test_result << DeQtf("change_multi_popcount=" + IntStr(change_multi_popcount) + "\n");
	qtf_test_result << DeQtf("change_net_popcount=" + IntStr(change_net_popcount) + "\n");
	//qtf_test_result << DeQtf("pipchange_popcount=" + IntStr(pipchange_popcount) + "\n");
	qtf_test_result << DeQtf("popcount=" + IntStr(popcount) + "\n");
	qtf_test_result << DeQtf("Unknown all same single, multi & net\n");
	qtf_test_result << TestTrade(symbol, change_postpips, change_single);
}

















void AllSameMultiPips::Init() {
	sl_change_single0.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips0)
		.AddArg(src_windowsize);
	sl_change_single0.Init();
	
	sl_change_multi0.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips0)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_multi0.Init();
	
	sl_change_net0.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips0)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_net0.Init();
	
	
	sl_change_single1.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips1)
		.AddArg(src_windowsize);
	sl_change_single1.Init();
	
	sl_change_multi1.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips1)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_multi1.Init();
	
	sl_change_net1.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips1)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_net1.Init();
	
	
	sl_change_single2.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips2)
		.AddArg(src_windowsize);
	sl_change_single2.Init();
	
	sl_change_multi2.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips2)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_multi2.Init();
	
	sl_change_net2.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum)
		.AddArg(change_postpips2)
		.AddArg(change_extrapolationmode)
		.AddArg(src_windowsize);
	sl_change_net2.Init();
	
}

void AllSameMultiPips::Run() {
	LabelSignal change_single0, change_multi0, change_net0;
	LabelSignal change_single1, change_multi1, change_net1;
	LabelSignal change_single2, change_multi2, change_net2;
	sl_change_single0.GetScript(0).GetSignal(symbol, change_single0);
	sl_change_multi0.GetScript(0).GetSignal(symbol, change_multi0);
	sl_change_net0.GetScript(0).GetSignal(symbol, change_net0);
	sl_change_single1.GetScript(0).GetSignal(symbol, change_single1);
	sl_change_multi1.GetScript(0).GetSignal(symbol, change_multi1);
	sl_change_net1.GetScript(0).GetSignal(symbol, change_net1);
	sl_change_single2.GetScript(0).GetSignal(symbol, change_single2);
	sl_change_multi2.GetScript(0).GetSignal(symbol, change_multi2);
	sl_change_net2.GetScript(0).GetSignal(symbol, change_net2);
	
	int change_single_popcount = change_single0.enabled.PopCount();
	int change_multi_popcount = change_multi0.enabled.PopCount();
	int change_net_popcount = change_net0.enabled.PopCount();
	
	change_single0.And(change_multi0);
	change_single0.And(change_net0);
	change_single0.And(change_single1);
	change_single0.And(change_multi1);
	change_single0.And(change_net1);
	change_single0.And(change_single2);
	change_single0.And(change_multi2);
	change_single0.And(change_net2);
	
	int popcount = change_single0.enabled.PopCount();
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("change_single_popcount=" + IntStr(change_single_popcount) + "\n");
	qtf_test_result << DeQtf("change_multi_popcount=" + IntStr(change_multi_popcount) + "\n");
	qtf_test_result << DeQtf("change_net_popcount=" + IntStr(change_net_popcount) + "\n");
	qtf_test_result << DeQtf("popcount=" + IntStr(popcount) + "\n");
	qtf_test_result << DeQtf("Unknown all same single, multi & net\n");
	qtf_test_result << TestTrade(symbol, change_postpips1, change_single0);
}
















void AllSameMultiType::Init() {
	sl_change_single0.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum0)
		.AddArg(change_postpips)
		.AddArg(change_windowsize0);
	sl_change_single0.Init();
	
	sl_change_multi0.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum0)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize0);
	sl_change_multi0.Init();
	
	sl_change_net0.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum0)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize0);
	sl_change_net0.Init();
	
	
	sl_change_single1.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum1)
		.AddArg(change_postpips)
		.AddArg(change_windowsize1);
	sl_change_single1.Init();
	
	sl_change_multi1.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum1)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize1);
	sl_change_multi1.Init();
	
	sl_change_net1.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum1)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize1);
	sl_change_net1.Init();
	
	
	sl_change_single2.AddFactory(System::FindScript<SingleChangeNeural>())
		.AddArg(symbol)
		.AddArg(src_trainpercent)
		.AddArg(change_enum2)
		.AddArg(change_postpips)
		.AddArg(change_windowsize2);
	sl_change_single2.Init();
	
	sl_change_multi2.AddFactory(System::FindScript<MultiChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum2)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize2);
	sl_change_multi2.Init();
	
	sl_change_net2.AddFactory(System::FindScript<MultinetChangeNeural>())
		.AddArg(src_trainpercent)
		.AddArg(change_enum2)
		.AddArg(change_postpips)
		.AddArg(change_extrapolationmode)
		.AddArg(change_windowsize2);
	sl_change_net2.Init();
	
}

void AllSameMultiType::Run() {
	LabelSignal change_single0, change_multi0, change_net0;
	LabelSignal change_single1, change_multi1, change_net1;
	LabelSignal change_single2, change_multi2, change_net2;
	sl_change_single0.GetScript(0).GetSignal(symbol, change_single0);
	sl_change_multi0.GetScript(0).GetSignal(symbol, change_multi0);
	sl_change_net0.GetScript(0).GetSignal(symbol, change_net0);
	sl_change_single1.GetScript(0).GetSignal(symbol, change_single1);
	sl_change_multi1.GetScript(0).GetSignal(symbol, change_multi1);
	sl_change_net1.GetScript(0).GetSignal(symbol, change_net1);
	sl_change_single2.GetScript(0).GetSignal(symbol, change_single2);
	sl_change_multi2.GetScript(0).GetSignal(symbol, change_multi2);
	sl_change_net2.GetScript(0).GetSignal(symbol, change_net2);
	
	int change_single_popcount = change_single0.enabled.PopCount();
	int change_multi_popcount = change_multi0.enabled.PopCount();
	int change_net_popcount = change_net0.enabled.PopCount();
	
	change_single0.And(change_multi0);
	change_single0.And(change_net0);
	change_single0.And(change_single1);
	change_single0.And(change_multi1);
	change_single0.And(change_net1);
	change_single0.And(change_single2);
	change_single0.And(change_multi2);
	change_single0.And(change_net2);
	
	int popcount = change_single0.enabled.PopCount();
	
	qtf_test_result = "";
	qtf_test_result << DeQtf("change_single_popcount=" + IntStr(change_single_popcount) + "\n");
	qtf_test_result << DeQtf("change_multi_popcount=" + IntStr(change_multi_popcount) + "\n");
	qtf_test_result << DeQtf("change_net_popcount=" + IntStr(change_net_popcount) + "\n");
	qtf_test_result << DeQtf("popcount=" + IntStr(popcount) + "\n");
	qtf_test_result << DeQtf("Unknown all same single, multi & net\n");
	qtf_test_result << TestTrade(symbol, change_postpips, change_single0);
}

}
