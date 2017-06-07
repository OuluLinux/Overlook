#include "Overlook.h"

namespace Overlook {




System::System() {
	timediff = 0;
	base_period = 60;
	end = GetSysTime();
	
	addr = "127.0.0.1";
	port = 42000;
	running = false;
	stopped = true;
	exploration = 0.2;
	
	structural_columns = 0;
	template_arg_count = 0;
	slot_args = 0;
	traditional_indicators = 0;
	traditional_arg_count = 0;
	ma_id = -1;
	
}

System::~System() {
	Stop();
	data.Clear();
}

void System::Init() {
	
	MetaTrader& mt = GetMetaTrader();
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetCacheSymbols();
	
	try {
		
		// Init sym/tfs/time space
		ASSERTEXC_(!mt.Init(addr, port), "Can't connect to MT4. Is MT4Connection script activated in MT4?");
		
		// Add symbols
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			AddSymbol(s.name);
		}
		for(int i = 0; i < mt.GetCurrencyCount(); i++) {
			const Currency& c = mt.GetCurrency(i);
			AddSymbol(c.name);
		}
		
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 1; // mins
		//SetBasePeriod(60*base);
		Vector<int> tfs;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base) {
				tfs.Add(tf / base);
				AddPeriod(mt.GetTimeframeString(i), tf * 60);
			}
		}
		
		
		// Init time range
		/*Time begin, end;
		begin = Time(2016, 12, 1);
		Date now = GetSysTime();
		do {++now;}
		while (now.day != 15);
		end = Time(now.year, now.month, now.day);
		SetBegin(begin);
		SetEnd(end);*/
		
		
		int64 sym_count = symbols.GetCount();
		int64 tf_count = periods.GetCount();
	
		if (sym_count == 0) throw DataExc();
		if (tf_count == 0)  throw DataExc();
		
		bars.SetCount(tf_count);
		for(int i = 0; i < bars.GetCount(); i++) {
			int count = GetCountTf(i);
			if (!count) throw DataExc();
			bars[i] = count;
		}
		
		//StoreThis();
		
	}
	catch (...) {
		LOG("Load failed");
	}
	
	InitRegistry();
	InitGeneticOptimizer();
	
}

void System::AddPeriod(String nice_str, int period) {
	int count = periods.GetCount();
	
	// Currently first period must match base period
	if (count == 0 && base_period != period)
		throw DataExc();
	
	period /= base_period;
	
	if (count) {
		int i = period / periods[count-1];
		if (i <= 1) throw DataExc();
		if (count-1 != tfbars_in_slowtf.GetCount()) throw DataExc();
		tfbars_in_slowtf.Add(i);
	}
	
	period_strings.Add(nice_str);
	periods.Add(period);
	
	// TODO: some algorithm to calculate begins and ends, and persistently using it again
	Time begin(2017,1,1);
	if (period == 1)			begin = Time(2016,11,15);
	else if (period == 5)		begin = Time(2016,5,1);
	else if (period == 15)		begin = Time(2016,1,20);
	else if (period == 30)		begin = Time(2015,11,9);
	else if (period == 60)		begin = Time(2015,5,13);
	else if (period == 240)		begin = Time(2009,12,21);
	else if (period == 1440)	begin = Time(2000,5,3);
	else if (period == 10080)	begin = Time(1996,6,23);
	else if (period == 43200)	begin = Time(1995,1,1);
	else Panic("Invalid period: " + IntStr(period));
	this->begin.Add(begin);
	this->begin_ts.Add(begin.Get() - Time(1970,1,1).Get());
}

void System::AddSymbol(String sym) {
	ASSERT(symbols.Find(sym) == -1); // no duplicates
	symbols.Add(sym);
}

Time System::GetTimeTf(int tf, int pos) const {
	return begin[tf] + periods[tf] * pos * base_period;
}

int System::GetCountTf(int tf_id) const {
	int64 timediff = end.Get() - begin[tf_id].Get();
	int div = base_period * periods[tf_id];
	int count = timediff / div;
	if (count % div != 0) count++;
	return count;
}

int64 System::GetShiftTf(int src_tf, int dst_tf, int shift) {
	int64 src_period = periods[src_tf];
	int64 dst_period = periods[dst_tf];
	int64 timediff = shift * src_period * base_period;
	timediff -= begin_ts[dst_tf] - begin_ts[src_tf];
	int64 dst_shift = timediff / base_period / dst_period;
	
	// Sanity check
	timediff = GetTimeTf(src_tf, shift).Get() - GetTimeTf(dst_tf, dst_shift).Get();
	if (src_tf > dst_tf) {
		ASSERT(timediff == 0);
		Panic("TODO");
	} else {
		int64 maxdiff = src_period * base_period;
		ASSERT(timediff > -maxdiff && timediff < maxdiff);
		Panic("TODO");
	}
	
	return dst_shift;
}

int64 System::GetShiftFromTimeTf(int timestamp, int tf) {
	int64 timediff = timestamp - begin_ts[tf];
	return (int64)(timediff / periods[tf] / base_period);
}

int64 System::GetShiftFromTimeTf(const Time& t, int tf) {
	int64 timediff = t.Get() - begin[tf].Get();
	return (int64)(timediff / periods[tf] / base_period);
}

void System::AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c) {
	CtrlFactories().Add(CoreCtrlSystem(name, f, c));
}

}
