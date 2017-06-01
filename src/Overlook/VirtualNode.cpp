#include "Overlook.h"

namespace Overlook {

VirtualNode::VirtualNode()  {
	SetSkipAllocate();
	max_value = 0;
	min_value = 0;
	median_max = 0;
	median_min = 0;
	point = 0.00001;
}

VirtualNode::~VirtualNode()  {
	
}

void VirtualNode::Init() {
	//SetBufferCount(4, 4); // open, low, high, volume
	
}

void VirtualNode::Start() {
	Buffer& open   = GetBuffer(0);
	Buffer& low    = GetBuffer(1);
	Buffer& high   = GetBuffer(2);
	Buffer& volume = GetBuffer(3);
	
	BaseSystem& bs = GetBaseSystem();
	MetaTrader& mt = GetMetaTrader();
	int sym_count = mt.GetSymbolCount();
	int cur = GetSymbol() - sym_count;
	if (cur < 0)
		return;
	ASSERT(cur >= 0 && cur < mt.GetCurrencyCount());
	
	int bars = GetBars();
	ASSERT(bars > 0);
	for(int i = 0; i < outputs.GetCount(); i++)
		for(int j = 0; j < outputs[i].buffers.GetCount(); j++)
			outputs[i].buffers[j].value.SetCount(bars, 0);
		
	const Currency& c = mt.GetCurrency(cur);
	int tf = GetTimeframe();
	
	typedef Tuple3<ConstBuffer*,ConstBuffer*,bool> Source;
	Vector<Source> sources;
	for(int i = 0; i < c.pairs0.GetCount(); i++) {
		int id = c.pairs0[i];
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, false));
	}
	for(int i = 0; i < c.pairs1.GetCount(); i++) {
		int id = c.pairs1[i];
		ConstBuffer& open_buf = GetInputBuffer(0,id,tf,0);
		ConstBuffer& vol_buf  = GetInputBuffer(0,id,tf,3);
		sources.Add(Source(&open_buf, &vol_buf, true));
	}
	
	int counted = GetCounted();
	if (!counted) {
		open.Set(0, 1.0);
		low.Set(0, 1.0);
		high.Set(0, 1.0);
		volume.Set(0, 0);
		counted = 1;
	}
	else counted--;
	
	for(int i = counted; i < bars; i++) {
		SetSafetyLimit(i);
		double change_sum = 0;
		double volume_sum = 0;
		
		for(int j = 0; j < sources.GetCount(); j++) {
			Source& s = sources[j];
			ConstBuffer& open_buf = *s.a;
			ConstBuffer& vol_buf  = *s.b;
			bool inverse = s.c;
			
			double open   = open_buf.Get(i-1);
			double close  = open_buf.Get(i);
			double vol    = vol_buf.Get(i);
			double change = open != 0.0 ? (close / open) - 1.0 : 0.0;
			if (inverse) change *= -1.0;
			
			change_sum += change;
			volume_sum += vol;
		}
		
		double prev = open.Get(i-1);
		double change = change_sum / sources.GetCount();
		double value = prev * (1.0 + change);
		double volume_av = volume_sum / sources.GetCount();
		
		open.Set(i, value);
		low.Set(i, value);
		high.Set(i, value);
		volume.Set(i, volume_av);
		
		//LOG(Format("pos=%d open=%f volume=%f", i, value, volume_av));
		
		if (i) {
			low		.Set(i-1, Upp::min(low		.Get(i-1), value));
			high	.Set(i-1, Upp::max(high		.Get(i-1), value));
		}
		
		
		// Find min/max
		double diff = i ? open.Get(i) - open.Get(i-1) : 0.0;
		int step = diff / point;
		if (step >= 0) median_max_map.GetAdd(step, 0)++;
		else median_min_map.GetAdd(step, 0)++;
		if (diff > max_value) max_value = diff;
		if (diff < min_value) min_value = diff;
	}
	
	
	// Get median values
	{
		SortByKey(median_max_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_max_map.GetCount(); i++)
			total += median_max_map[i];
		int64 target = total / 2;
		total = 0;
		for(int i = 0; i < median_max_map.GetCount(); i++) {
			total += median_max_map[i];
			if (total > target) {
				median_max = median_max_map.GetKey(i);
				break;
			}
		}
	}
	{
		SortByKey(median_min_map, StdLess<int>());
		int64 total = 0;
		for(int i = 0; i < median_min_map.GetCount(); i++)
			total += median_min_map[i];
		int64 target = total / 2;
		total = 0;
		for(int i = 0; i < median_min_map.GetCount(); i++) {
			total += median_min_map[i];
			if (total > target) {
				median_min = median_min_map.GetKey(i);
				break;
			}
		}
	}
}

}
