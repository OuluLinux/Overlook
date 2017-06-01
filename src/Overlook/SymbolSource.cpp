#include "Overlook.h"

namespace Overlook {

SymbolSource::SymbolSource() {
	max_value = 0;
	min_value = 0;
	median_max = 0;
	median_min = 0;
}

void SymbolSource::Init() {
	
}

void SymbolSource::Start() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	DataBridgeCommon& common = Single<DataBridgeCommon>();
	int sym_count = common.GetSymbolCount();
	int sym = GetSymbol();
	int cur = sym - sym_count;
	int tf = GetTimeframe();
	int counted = GetCounted();
	int bars = GetBars();
	int input = cur < 0 ? 0 : 1; // DataBridgeValue or VirtualNode
	
	ConstBuffer& src_open   = GetInputBuffer(input, sym, tf, 0);
	ConstBuffer& src_low    = GetInputBuffer(input, sym, tf, 1);
	ConstBuffer& src_high   = GetInputBuffer(input, sym, tf, 2);
	ConstBuffer& src_volume = GetInputBuffer(input, sym, tf, 3);
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		open	.Set(i, src_open.Get(i));
		low		.Set(i, src_low.Get(i));
		high	.Set(i, src_high.Get(i));
		volume	.Set(i, src_volume.Get(i));
	}
	
	if (!input) {
		DataBridge& db = *dynamic_cast<DataBridge*>(GetInputCore(input, sym, tf));
		max_value = db.GetMax();
		min_value = db.GetMin();
		median_max = db.GetMedianMax();
		median_min = db.GetMedianMin();
	} else {
		VirtualNode& vn = *dynamic_cast<VirtualNode*>(GetInputCore(input, sym, tf));
		max_value = vn.GetMax();
		min_value = vn.GetMin();
		median_max = vn.GetMedianMax();
		median_min = vn.GetMedianMin();
	}
}

}
