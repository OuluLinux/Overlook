#include "DataCore.h"

namespace DataCore {

CostStats::CostStats() {
	
}

void CostStats::SetArguments(const VectorMap<String, Value>& args) {
	
}

void CostStats::Init() {
	
}

bool CostStats::Process(const SlotProcessAttributes& attr) {
	
	
	
	return true;
}










ValueChange::ValueChange()
{
	//src_a = NULL;
	//src_b = NULL;
}

void ValueChange::SetArguments(const VectorMap<String, Value>& args) {
	
	//return 0;
}

void ValueChange::Init()
{
	/*
	int id = GetId();
	if (id == -1) return 1;
	
	SetContainerSeparateWindow();
	
#ifdef PRICE_CHANGE_DEBUG
	if (has_proxy) {
		SetBufferCount(12);
		SetIndexCount(12);
		SetBufferColor(8, Black);
		SetIndexBuffer(8, proxy_status);
		SetBufferColor(9, Color(139, 69, 19));
		SetIndexBuffer(9, proxy_cost);
		SetBufferColor(10, Color(0, 255, 0));
		SetIndexBuffer(10, proxy_inc);
		SetBufferColor(11, Magenta);
		SetIndexBuffer(11, proxy_dec);
		
	} else 
#endif
	{
		SetBufferCount(8);
		SetIndexCount(8);
	}
	
	SetBufferColor(0, Green);
	SetIndexBuffer(0, inc);
	SetBufferLineWidth(0, 3);
	
	SetBufferColor(1, Red);
	SetIndexBuffer(1, dec);
	SetBufferLineWidth(1, 3);
	
	SetBufferColor(2, Blue);
	SetIndexBuffer(2, cost);
	
	SetBufferColor(3, Green);
	SetIndexBuffer(3, best_inc);
	SetBufferLineWidth(3, 2);
	
	SetBufferColor(4, Red);
	SetIndexBuffer(4, best_dec);
	SetBufferLineWidth(4, 2);
	
	SetBufferColor(5, Green);
	SetIndexBuffer(5, worst_inc);
	
	SetBufferColor(6, Red);
	SetIndexBuffer(6, worst_dec);
	
	SetBufferColor(7, White);
	SetIndexBuffer(7, value_change);
	
	
	DataVar bridge = GetResolver().ResolvePath("/bridge");
	if (!bridge.Is()) return 1;
	DataBridge* db = bridge.Get<DataBridge>();
	if (!db) return 1;
	MetaTrader& mt = db->GetMetaTrader();
	
	const Symbol& sym = db->GetSymbol(id);
	
	if (RequireIndicator("askbid")) return 1;
	src_a = GetSource().Get<BarData>();
	if (!src_a) return 1;
	
	has_proxy = sym.proxy_id != -1;
	proxy_id = sym.proxy_id;
	proxy_factor = sym.proxy_factor;
	if (has_proxy) {
		if (RequireId(proxy_id, GetPeriod())) return 1;
		if (RequireIdIndicator(proxy_id, GetPeriod(), "askbid")) return 1;
		src_b = dynamic_cast<BarData*>(&At(1));
		if (!src_b) return 1;
	}
	
	return 0;*/
}

bool ValueChange::Process(const SlotProcessAttributes& attr)
{
	/*
	BridgeAskBid& askbid = dynamic_cast<BridgeAskBid&>(At(0));
	
	double current_value, previous_value;
	
	int id = GetId();
	int bars = GetBars();
	int counted = GetCounted();
	counted = GetCounted();

	if (counted > 0)
		counted--;

	const Data32f& open  = src_a->GetOpen();
	const Data32f& low   = src_a->GetLow();
	const Data32f& high  = src_a->GetHigh();
	const Data32f& ask = askbid.GetBuffer(0);
	const Data32f& bid = askbid.GetBuffer(1);
	
	bars--;
	
	if (!has_proxy) {
		for (int i = counted; i < bars; i++)
		{
			Time now = GetTime().GetTime(GetPeriod(), i);
			int h = now.hour;
			int d = DayOfWeek(now);
			
			double cost = ask.Get(i) - bid.Get(i);
			
			this->cost.Set(i, cost);
			
			double open_value = open.Get(i);
			double value_change = open.Get(i+1) - open_value;
			inc.Set(i, value_change - cost);
			dec.Set(i, -value_change - cost);
			double high_value_change = high.Get(i) - open_value;
			double low_value_change  = low.Get(i) - open_value;
			ASSERT(high_value_change >= low_value_change);
			best_inc.Set(i, high_value_change - cost);
			best_dec.Set(i, -low_value_change - cost);
			worst_inc.Set(i, low_value_change - cost);
			worst_dec.Set(i, -high_value_change - cost);
			this->value_change.Set(i, value_change);
		}
	} else {
		BridgeAskBid& proxy_askbid = dynamic_cast<BridgeAskBid&>(At(2));
		const Data32f& proxy_ask = proxy_askbid.GetIndex(0);
		const Data32f& proxy_bid = proxy_askbid.GetIndex(1);
		
		// Refresh indicator
		src_b->Refresh();
		proxy_askbid.Refresh();
		
		// Get buffers
		const Data32f& proxy_open  = src_b->GetOpen();
		const Data32f& proxy_low   = src_b->GetLow();
		const Data32f& proxy_high  = src_b->GetHigh();
		
		int proxy_bars = proxy_open.GetCount();
		if (proxy_bars == 0) // at the very begin probably
			return 1;
		
		
		for (int i = counted; i < bars; i++)
		{
			Time now = GetTime().GetTime(GetPeriod(), i);
			int h = now.hour;
			int d = DayOfWeek(now);
			
			
			double cost = ask.Get(i) - bid.Get(i);
			
			
			// Proxy use same hour and wday, of course.
			double proxy_cost = proxy_ask.Get(i) - proxy_bid.Get(i);
			
			// You pay cost twice with proxy symbol
			this->cost.Set(i, cost + proxy_cost);
			#ifdef PRICE_CHANGE_DEBUG
			this->proxy_cost.Set(i, proxy_cost);
			#endif
	
			// Proxy and normal.
			double proxy_open_value = proxy_open.Get(i);
			double open_value = open.Get(i);
			double proxy_value_change, proxy_high_value_change, proxy_low_value_change;
			if (proxy_factor == 1) {
				proxy_value_change = proxy_open.Get(i+1) - proxy_open_value;
				proxy_high_value_change = proxy_high.Get(i) - proxy_open_value;
				proxy_low_value_change  = proxy_low.Get(i) - proxy_open_value;
			} else {
				proxy_value_change = proxy_open_value - proxy_open.Get(i+1);
				proxy_low_value_change   = proxy_open_value - proxy_high.Get(i);
				proxy_high_value_change  = proxy_open_value - proxy_low.Get(i);
			}
			double value_change = open.Get(i+1) - open_value;
			inc.Set(i, value_change + proxy_value_change - cost - proxy_cost);
			dec.Set(i, -value_change - proxy_value_change - cost - proxy_cost);
			double high_value_change = high.Get(i) - open_value;
			double low_value_change  = low.Get(i) - open_value;
			ASSERT(high_value_change >= low_value_change);
			ASSERT(proxy_high_value_change >= proxy_low_value_change);
			best_inc.Set(i,  proxy_high_value_change + high_value_change - cost - proxy_cost);
			best_dec.Set(i, -proxy_low_value_change  - low_value_change  - cost - proxy_cost);
			worst_inc.Set(i, proxy_low_value_change + low_value_change - cost - proxy_cost);
			worst_dec.Set(i, -proxy_high_value_change - high_value_change - cost - proxy_cost);
			this->value_change.Set(i, value_change + proxy_value_change);
		}
	}
	
	return 0;*/
	
	return true;
}

}
