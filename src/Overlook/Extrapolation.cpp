#include "Overlook.h"

namespace Overlook {

void NeverTheSameExtrapolation::Run() {
	/*ScriptList sl_change;
	sl_change.AddFactory(System::FindScript<MultinetChangeInterpolation>())
		.AddArg(change_enum)
		.AddArg(change_postpips)
		.AddArg(change_extrapolation);
	
	ScriptList sl_volat;
	sl_volat.AddFactory(System::FindScript<MultinetVolatInterpolation>())
		.AddArg(volat_enum)
		.AddArg(volat_ticks)
		.AddArg(volat_extrapolation);
	
	LabelSignal change;
	sl_change.GetScript(0).GetExtrapolationSignal(symbol, change);
	
	LabelSignal volat;
	sl_volat.GetScript(0).GetExtrapolationSignal(symbol, volat);
	
	change.enabled.And(volat.enabled);
	
	//change.enabled.LimitRight(change.enabled.GetCount() * 0.5);
	
	TestTrade(symbol, change_postpips, change);*/
}

}
