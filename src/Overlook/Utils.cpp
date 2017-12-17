#include "Overlook.h"

namespace Overlook {

ValueChange::ValueChange() :
	AdvisorBase(3, 2) // buffers total, visible
{
	SetCoreSeparateWindow();
}

void ValueChange::Init() {
	SetBufferColor(0, Color(128,0,0));
	SetBufferLineWidth(0, 2);
	SetBufferColor(1, Color(0,128,0));
	SetBufferLineWidth(1, 2);
	SetBufferColor(2, Color(0,0,128));
	SetBufferLineWidth(2, 2);
	
	BaseInit();
	
	if (IsAdvisorBaseSymbol(GetSymbol())) {
		EnableJobs();
	}
}

void ValueChange::Start() {
	int tf = GetTimeframe();
	int bars = GetBars();
	int counted = GetCounted();
	
	double point = 0.00001;
	if (GetSymbol() < GetMetaTrader().GetSymbolCount()) {
		const Symbol& sym = GetMetaTrader().GetSymbol(GetSymbol());
		point = sym.point;
	}
	
	const Buffer& open				= GetInputBuffer(0, 0);
	const Buffer& low				= GetInputBuffer(0, 1);
	const Buffer& high				= GetInputBuffer(0, 2);
	Buffer& low_change				= GetBuffer(0);
	Buffer& high_change				= GetBuffer(1);
	Buffer& value_change			= GetBuffer(2);
	
	if (!counted) counted = 1;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double open_value = open.Get(i-1);
		double change_value = open.Get(i) / open_value - 1.0;
		double low_change_ = low.Get(i-1) / open_value - 1.0;
		double high_change_ = high.Get(i-1) / open_value - 1.0;
		change_av.Add(fabs(change_value));
		value_change.Set(i, change_value);
		low_change.Set(i, low_change_);
		high_change.Set(i, high_change_);
	}
	
	for(int i = counted; i < bars; i++) {
		value_change	.Set(i, value_change	.Get(i) / (3 * change_av.mean));
		low_change		.Set(i, low_change		.Get(i) / (3 * change_av.mean));
		high_change		.Set(i, high_change		.Get(i) / (3 * change_av.mean));
	}
	
	if (IsAdvisorBaseSymbol(GetSymbol()) &&
		IsJobsFinished() && counted < GetBars())
		RefreshAll();
}


}
