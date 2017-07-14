#include "Overlook.h"

namespace Overlook {

ValueChange::ValueChange() {
	SetCoreSeparateWindow();
}

void ValueChange::Init() {
	SetBufferColor(0, Color(128,0,0));
	SetBufferLineWidth(0, 1);
	SetBufferColor(1, Color(0,128,0));
	SetBufferLineWidth(0, 1);
	SetBufferColor(2, Color(0,0,128));
	SetBufferLineWidth(0, 1);
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
	Buffer& value_change			= GetBuffer(0);
	Buffer& low_change				= GetBuffer(1);
	Buffer& high_change				= GetBuffer(2);
	
	if (!counted) counted = 1;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double open_value = open.Get(i-1);
		double change_value = open.Get(i) / open_value - 1.0;
		double low_change_ = low.Get(i-1) / open_value - 1.0;
		double high_change_ = high.Get(i-1) / open_value - 1.0;
		value_change.Set(i, change_value);
		low_change.Set(i, low_change_);
		high_change.Set(i, high_change_);
	}
}

}
