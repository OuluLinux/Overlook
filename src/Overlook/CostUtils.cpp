#include "Overlook.h"


namespace Overlook {


SpreadStats::SpreadStats() {
	SetCoreSeparateWindow();
}

void SpreadStats::Init() {
	SetBufferLineWidth(0, 3);
	SetBufferColor(0, Color(0,128,0));
}

void SpreadStats::Start() {
	int id = GetSymbol();
	int tf = GetTimeframe();
	int bars = GetBars();
	int counted = GetCounted();
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	BridgeAskBid& ab = *Get<BridgeAskBid>();
	BaseSystem& bs = GetBaseSystem();
	Buffer& spread_buf = GetBuffer(0);
	
	for (int i = counted; i < bars; i++) {
		
		double spread = 0;
		
		Time t = bs.GetTime(GetPeriod(), i);
		int h = (t.minute + t.hour * 60) / period;
		int d = DayOfWeek(t) - 1;
		int dh = h + d * h_count;
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		if (d == -1 || d == 5) {
			
		}
		else {
			OnlineVariance& var = ab.stats[dh];
			if (var.GetEventCount()) {
				spread = var.GetMean();
			}
		}
		
		spread_buf.Set(i, spread);
	}
}











SpreadMeanProfit::SpreadMeanProfit() {
	askbid = NULL;
	
}

void SpreadMeanProfit::Init() {
	SetCoreSeparateWindow();
	SetBufferLineWidth(0, 2);
	
	SetBufferColor(0, Color(0,0,127));
}

void SpreadMeanProfit::Start() {
	const BridgeAskBid& askbid = *Get<BridgeAskBid>();
	const BaseSystem& bs = GetBaseSystem();
	
	int bars = GetBars();
	int counted = GetCounted();
	const int size = 300;
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	const Buffer& open = GetInputBuffer(1, 0);
	Buffer& mean_profit = GetBuffer(0);
	
	if (!counted) counted = 1;
	
	for(int i = counted; i < bars; i++) {
		Time t = bs.GetTime(GetPeriod(), i);
		
		int h = (t.minute + t.hour * 60) / period;
		int d = DayOfWeek(t) - 1;
		int dh = h + d * h_count;
		
		if (force_d0) {
			h = 0;
			d = 0;
			dh = 0;
		}
		else if (d == -1 || d == 5) {
			continue;
		}
		
		const OnlineVariance& var = askbid.GetStat(dh);
		
		double abs_change = fabs(open.Get(i) / open.Get(i-1) - 1.0) ;
		mean_profit.Set(i, abs_change - var.GetMean());
	}
}











ValueChange::ValueChange() {
	SetCoreSeparateWindow();
	
}

void ValueChange::Init() {
	SetBufferColor(0, GrayColor(128));
	SetBufferLineWidth(0, 4);
	SetBufferColor(1, Color(0,128,0));
	SetBufferLineWidth(1, 3);
	SetBufferColor(2, Color(128,0,0));
	SetBufferLineWidth(2, 3);
	SetBufferColor(3, Color(0,0,128));
	SetBufferLineWidth(3, 1);
	SetBufferColor(4, Color(0,128,0));
	SetBufferLineWidth(4, 2);
	SetBufferColor(5, Color(128,0,0));
	SetBufferLineWidth(5, 2);
	SetBufferColor(6, Color(0,128,0));
	SetBufferLineWidth(6, 1);
	/*SetBufferColor(7, Color(128,0,0));
	SetBufferLineWidth(7, 1);
	SetBufferColor(8, Color(255,255,255));
	SetBufferLineWidth(8, 1);*/
	
	MetaTrader& mt = GetMetaTrader();
	int id = GetSymbol();
	if (id < mt.GetSymbolCount()) {
		const Symbol& sym = mt.GetSymbol(id);
		has_proxy = sym.proxy_id != -1;
		proxy_id = sym.proxy_id;
		proxy_factor = sym.proxy_factor;
		ASSERTEXC(!has_proxy || proxy_factor != 0);
	} else {
		has_proxy = false;
		proxy_id = -1;
		proxy_factor = 0;
	}
}

void ValueChange::Start() {
	int bars = GetBars();
	int counted = GetCounted();
	
	const Buffer& open				= GetInputBuffer(0, 0);
	const Buffer& low				= GetInputBuffer(0, 1);
	const Buffer& high				= GetInputBuffer(0, 2);
	const Buffer& spread			= GetInputBuffer(1, 0);
	ConstBuffer* proxy_spread		= has_proxy ? &GetInputBuffer(1, proxy_id, GetTimeframe(), 0) : 0;
	ConstBuffer* proxy_open			= has_proxy ? &GetInputBuffer(0, proxy_id, GetTimeframe(), 0) : 0;
	ConstBuffer* proxy_low			= has_proxy ? &GetInputBuffer(0, proxy_id, GetTimeframe(), 1) : 0;
	ConstBuffer* proxy_high			= has_proxy ? &GetInputBuffer(0, proxy_id, GetTimeframe(), 2) : 0;
	Buffer& change					= GetBuffer(0);
	Buffer& value_change			= GetBuffer(1);
	Buffer& low_change				= GetBuffer(2);
	Buffer& high_change				= GetBuffer(3);
	Buffer& spread_change			= GetBuffer(4);
	Buffer& proxy_low_buf			= GetBuffer(5);
	Buffer& proxy_high_buf			= GetBuffer(6);
	
	if (!counted) counted = 1;
	
	for(int i = counted; i < bars; i++) {
		if (!has_proxy) {
			double open_value = open.Get(i-1);
			double change_value = open.Get(i) / open_value - 1.0;
			change.Set(i, change_value);
			double low_change_ = low.Get(i-1) / open_value - 1.0;
			double high_change_ = high.Get(i-1) / open_value - 1.0;
			low_change.Set(i, low_change_);
			high_change.Set(i, high_change_);
			spread_change.Set(i, spread.Get(i-1));
			value_change.Set(i, change_value);
			proxy_low_buf.Set(i, low_change_);
			proxy_high_buf.Set(i, high_change_);
		} else {
			double proxy_spread_value = proxy_spread->Get(i-1);
			
			// Get buffers
			double proxy_open_value		= proxy_open->Get(i-1);
			double proxy_close_value	= proxy_open->Get(i);
			double proxy_low_value		= proxy_low->Get(i-1);
			double proxy_high_value		= proxy_high->Get(i-1);
			
			// Proxy and normal.
			double open_value = open.Get(i-1);
			double proxy_change, proxy_high_change, proxy_low_change;
			ASSERTEXC(proxy_factor != 0);
			if (proxy_factor == 1) {
				proxy_change		= proxy_close_value / proxy_open_value - 1.0;
				proxy_high_change	= proxy_high_value / proxy_open_value - 1.0;
				proxy_low_change	= proxy_low_value / proxy_open_value - 1.0;
			} else {
				proxy_change		= proxy_open_value / proxy_close_value - 1.0;
				proxy_low_change	= proxy_open_value / proxy_high_value - 1.0;
				proxy_high_change	= proxy_open_value / proxy_low_value - 1.0;
			}
			double change_value = open.Get(i) / open_value - 1.0;
			double low_change_	= low.Get(i-1) / open_value - 1.0;
			double high_change_	= high.Get(i-1) / open_value - 1.0;
			change.Set(i, change_value); // different with proxy
			low_change.Set(i, low_change_);
			high_change.Set(i, high_change_);
			spread_change.Set(i, spread.Get(i-1) + proxy_spread_value);
			proxy_low_buf.Set(i, proxy_low_change + low_change_);		// Note: unrealistic, probably not simultaneously
			proxy_high_buf.Set(i, proxy_high_change + high_change_);	// Note: unrealistic, probably not simultaneously
			value_change.Set(i, change_value + proxy_change);
		}
	}
}



















IdealOrders::IdealOrders() {
	
}

void IdealOrders::Init() {
	//AddDependency("/open", 1, 0);
	
}

void IdealOrders::Start() {
	
	// Search ideal order sequence with A* search in SimBroker
	Panic("TODO");
	
}



}
