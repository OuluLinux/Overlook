#include "Overlook.h"


namespace Overlook {


SpreadStats::SpreadStats() {
	SetCoreSeparateWindow();
}

void SpreadStats::Arguments(ArgumentBase& args) {
	
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
	BaseSystem& bs = *Get<BaseSystem>();
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

void SpreadMeanProfit::Arguments(ArgumentBase& args) {
	
}

void SpreadMeanProfit::Init() {
	/*SetCoreSeparateWindow();
	//SetBufferCount(1, 1);
	//SetIndexBuffer ( 0, mean_profit );
	SetBufferLineWidth(0, 2);
	
	SetBufferColor(0, Color(0,0,127));
	
	if (RequireIndicator("askbid")) throw DataExc();
	
	askbid = dynamic_cast<BridgeAskBid*>(&At(0));
	if (!askbid)
		throw DataExc();
	
	if (!GetSource().Get<Core>())
		throw DataExc(); // require bardata source
	*/
}

void SpreadMeanProfit::Start() {
	/*BridgeAskBid& askbid = *this->askbid;
	
	int bars = GetBars();
	int counted = GetCounted();
	const int size = 300;
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& pb = *GetSource().Get<Core>();
	Vector<double>& open = pb.GetBuffer(0);
	
	bars--;
	for(int i = counted; i < bars; i++) {
		Time t = GetTime().GetTime(GetPeriod(), i);
		
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
		
		OnlineVariance& var = askbid.stats[dh];
		
		double change = open.Get(i+1) - open.Get(i);
		mean_profit.Set(i, fabs(change) - var.GetMean());
	}*/
}

















SpreadProfitDistribution::SpreadProfitDistribution() {
	
}

void SpreadProfitDistribution::Arguments(ArgumentBase& args) {
	
}

void SpreadProfitDistribution::Init() {
	/*SetCoreSeparateWindow();
	//SetBufferCount(2, 2);
	//SetIndexBuffer ( 0, mean );
	//SetIndexBuffer ( 1, stddev );
	SetBufferLineWidth(1, 2);
	
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	
	if (RequireIndicator("askbid")) throw DataExc();
	if (RequireIndicator("stfc")) throw DataExc();
	*/
}

void SpreadProfitDistribution::Start() {
	/*int bars = GetBars();
	int counted = GetCounted();
	const int size = 300;
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	BridgeAskBid& askbid = dynamic_cast<BridgeAskBid&>(At(0));
	Core& subtfchanges = At(1);
	
	Vector<double>& abschange_mean   = subtfchanges.GetBuffer(2);
	Vector<double>& abschange_stddev = subtfchanges.GetBuffer(3);
	
	for(int i = counted; i < bars; i++) {
		Time t = GetTime().GetTime(GetPeriod(), i);
		
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
		
		OnlineVariance& var = askbid.stats[dh];
		
		double mean = abschange_mean.Get(i) - var.GetMean();
		double stddev = sqrt(pow(abschange_stddev.Get(i), 2) - var.GetVariance());
		
		this->mean.Set(i, mean);
		this->stddev.Set(i, stddev);
	}*/
}






















SpreadProbability::SpreadProbability() {
	
}

void SpreadProbability::Arguments(ArgumentBase& args) {
	
}

void SpreadProbability::Init() {
	/*SetCoreSeparateWindow();
	//SetBufferCount(2, 2);
	//SetIndexBuffer ( 0, profit );
	//SetIndexBuffer ( 1, real );
	SetBufferLineWidth(0, 2);
	
	SetBufferColor(0, Color(127,0,0));
	SetBufferColor(1, Color(127,0,0));
	
	SetCoreLevelCount(3);
	SetCoreLevel(0, 0.5);
	SetCoreLevel(1, 0.7);
	SetCoreLevel(2, 0.3);
	SetCoreLevelsColor(Color(192, 192, 192));
	SetCoreLevelsStyle(STYLE_DOT);
	SetCoreMinimum(0);
	SetCoreMaximum(1);
	
	if (RequireIndicator("costpdist")) throw DataExc();
	*/
}

void SpreadProbability::Start() {
	/*int bars = GetBars();
	int counted = GetCounted();
	const int size = 300;
	int period = GetMinutePeriod();
	int h_count = 24 * 60 / period; // originally hour only
	bool force_d0 = period >= 7*24*60;
	
	Core& cost_profit_dist = At(0);
	
	Vector<double>& cost_profit_mean   = cost_profit_dist.GetBuffer(0);
	Vector<double>& cost_profit_stddev = cost_profit_dist.GetBuffer(1);
	
	Core& pb = *GetSource().Get<Core>();
	Vector<double>& open = pb.GetBuffer(0);
	
	bars--;
	
	for(int i = counted; i < bars; i++) {
		Time t = GetTime().GetTime(GetPeriod(), i);
		
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
		
		double mean = cost_profit_mean.Get(i);
		double stddev = cost_profit_stddev.Get(i);
		double change = open.Get(i+1) - open.Get(i);
		
		profit.Set(i, NormalCDF(0, mean, stddev));
		real.Set(i, NormalCDF(change, mean, stddev));
		
	}*/
}













ValueChange::ValueChange() {
	/*
	AddValue<double>("Change");
	AddValue<double>("Change With Proxy");
	AddValue<double>("Low Change With Proxy");
	AddValue<double>("High Change With Proxy");
	AddValue<double>("Spread Change");
	
	SetStyle(
		"{"
			"\"window_type\":\"SEPARATE\","
			"\"value0\":{"
				"\"color\":\"128,128,128\","
				"\"line_width\":4,"
			"},"
			"\"value1\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":3,"
			"},"
			"\"value2\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":3,"
			"},"
			"\"value3\":{"
				"\"color\":\"0,0,128\","
				"\"line_width\":1,"
			"},"
			"\"value4\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":2,"
			"},"
			"\"value5\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":2,"
			"},"
			"\"value6\":{"
				"\"color\":\"0,128,0\","
				"\"line_width\":1,"
			"},"
			"\"value7\":{"
				"\"color\":\"128,0,0\","
				"\"line_width\":1,"
			"},"
			"\"value8\":{"
				"\"color\":\"255,255,255\","
				"\"line_width\":1,"
			"},"
		"}"
	);
	*/
}

void ValueChange::Arguments(ArgumentBase& args) {
	
}

void ValueChange::Init() {
	/*AddDependency("/open", 1, 0);
	AddDependency("/spread", 1, 0);
	
	TimeVector& tv = GetTimeVector();
	SlotPtr src = tv.FindLinkSlot("/open");
	db = dynamic_cast<DataBridge*>(&*src);
	ASSERTEXC(db);
	
	int sym_count = tv.GetSymbolCount();
	data.SetCount(sym_count);
	
	for(int i = 0; i < sym_count; i++) {
		Sym& s = data[i];
		if (i < db->GetSymbolCount()) {
			const Symbol& sym = db->GetSymbol(i);
			s.has_proxy = sym.proxy_id != -1;
			s.proxy_id = sym.proxy_id;
			s.proxy_factor = sym.proxy_factor;
			ASSERTEXC(!s.has_proxy || s.proxy_factor != 0);
		} else {
			s.has_proxy = false;
			s.proxy_id = -1;
			s.proxy_factor = 0;
		}
	}*/
}

void ValueChange::Start() {
	/*const Slot& src = GetDependency(0);
	const Slot& spreadsrc = GetDependency(1);
	double* change_			= GetValue<double>(0, attr);
	double* value_change	= GetValue<double>(1, attr);
	double* low_change		= GetValue<double>(2, attr);
	double* high_change		= GetValue<double>(3, attr);
	double* spread_change	= GetValue<double>(4, attr);
	double* open			= src.GetValue<double>(0, 1, attr);
	double* close			= src.GetValue<double>(0, 0, attr);
	double* low				= src.GetValue<double>(1, 1, attr);
	double* high			= src.GetValue<double>(2, 1, attr);
	double* spread			= spreadsrc.GetValue<double>(0, 0, attr);
	
	if (!open || *open == 0.0) return false;
	
	int id = GetSymbol();
	
	Sym& s = data[id];
	
	if (!s.has_proxy) {
		double open_value = *open;
		double change = *close / open_value - 1.0;
		*change_ = change; // same without proxy
		*low_change  = *low / open_value - 1.0;
		*high_change = *high / open_value - 1.0;
		*spread_change = *spread;
		*value_change = change;
		ASSERT(*high_change >= *low_change);
	} else {
		double* proxy_spread = spreadsrc.GetValue<double>(0, s.proxy_id, GetTimeframe(), 0, attr);
		
		// Get buffers
		double* proxy_open	= src.GetValue<double>(0, s.proxy_id, GetTimeframe(), 1, attr);
		double* proxy_close	= src.GetValue<double>(0, s.proxy_id, GetTimeframe(), 0, attr);
		double* proxy_low	= src.GetValue<double>(1, s.proxy_id, GetTimeframe(), 1, attr);
		double* proxy_high	= src.GetValue<double>(2, s.proxy_id, GetTimeframe(), 1, attr);
		
		// Proxy and normal.
		double proxy_open_value = *proxy_open;
		double open_value = *open;
		double proxy_change, proxy_high_change, proxy_low_change;
		ASSERTEXC(s.proxy_factor != 0);
		if (s.proxy_factor == 1) {
			proxy_change		= *proxy_close / proxy_open_value - 1.0;
			proxy_high_change	= *proxy_high / proxy_open_value - 1.0;
			proxy_low_change	= *proxy_low / proxy_open_value - 1.0;
		} else {
			proxy_change		= proxy_open_value / *proxy_close - 1.0;
			proxy_low_change	= proxy_open_value / *proxy_high - 1.0;
			proxy_high_change	= proxy_open_value / *proxy_low - 1.0;
		}
		double change = *close / open_value - 1.0;
		double low_change_	= *low / open_value - 1.0;
		double high_change_	= *high / open_value - 1.0;
		*change_ = change; // different with proxy
		*low_change = proxy_low_change + low_change_;		// Note: unrealistic, probably not simultaneously
		*high_change = proxy_high_change + high_change_;	// Note: unrealistic, probably not simultaneously
		*spread_change = *spread + *proxy_spread;
		*value_change	= change + proxy_change;
		ASSERT(*high_change >= *low_change);
	}
	
	return true;*/
}



















IdealOrders::IdealOrders() {
	
}

String IdealOrders::GetStyle() const {
	return "";
}

void IdealOrders::Arguments(ArgumentBase& args) {
	
}

void IdealOrders::Init() {
	//AddDependency("/open", 1, 0);
	
}

void IdealOrders::Start() {
	
	// Search ideal order sequence with A* search in SimBroker
	Panic("TODO");
	
}



}
