#include "Overlook.h"

namespace Overlook {







System::System() {
	timediff = 0;
	base_period = 60;
	SetEnd(GetSysTime());
	
	addr = Config::arg_addr;
	port = Config::arg_port;
	
	exploration = 0.2;
	task_counter = 0;
	
	source_symbol_count = 0;
}

System::~System() {
	StopJobs();
	data.Clear();
}

void System::Init() {
	ASSERT(symbols.IsEmpty());
	
	MetaTrader& mt = GetMetaTrader();
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetSymbols();
	
	try {
		
		// Init sym/tfs/time space
		bool connected = mt.Init(addr, port);
		ASSERTUSER_(!connected, "Can't connect to MT4. Is MT4Connection script activated in MT4?");
		
		// Add symbols
		source_symbol_count = mt.GetSymbolCount();
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			AddSymbol(s.name);
		}
		
		for(int i = 0; i < mt.GetCurrencyCount(); i++) {
			const Currency& c = mt.GetCurrency(i);
			AddSymbol(c.name);
		}
		
		AddSymbol("Account");
		
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 1; // mins
		//SetBasePeriod(60*base);
		Vector<int> tfs;
		bool has_h12 = false, has_h8 = false;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf == 720) has_h12 = true;
			if (tf == 480) has_h8 = true;
			if (tf >= base) {
				tfs.Add(tf / base);
				AddPeriod(mt.GetTimeframeString(i), tf * 60 / base);
			}
		}
		if (!has_h12 && (720 % base) == 0) AddPeriod("H12 gen", 720 * 60 / base);
		if (!has_h8 && (480 % base) == 0)  AddPeriod("H8 gen", 480 * 60 / base);
		
		int sym_count = symbols.GetCount();
		int tf_count = periods.GetCount();
	
		if (sym_count == 0) throw DataExc();
		if (tf_count == 0)  throw DataExc();
		
		bars.SetCount(tf_count);
		for(int i = 0; i < bars.GetCount(); i++) {
			int count = GetCountTf(i);
			if (!count) throw DataExc();
			bars[i] = count;
		}
	}
	catch (UserExc e) {
		throw e;
	}
	catch (Exc e) {
		throw e;
	}
	catch (...) {
		ASSERTUSER_(false, "Unknown error with MT4 connection.");
	}
	
	const int priosym_count = 19;
	
	spread_points.SetCount(priosym_count, 0);
	proxy_id.SetCount(priosym_count, 0);
	proxy_base_mul.SetCount(priosym_count, 0);

	int prio = 0;
	int spread_point = 0;
	sym_priority.SetCount(priosym_count, 100000);
	for(int i = 0; i < priosym_count; i++) {
		String symstr;
		switch (i) {
			case 0: symstr = "EURUSD";	spread_point = 3;  break;
			case 1: symstr = "GBPUSD";	spread_point = 3;  break;
			case 2: symstr = "USDJPY";	spread_point = 3;  break;
			case 3: symstr = "USDCAD";	spread_point = 3;  break;
			case 4: symstr = "EURJPY";	spread_point = 3;  break;
			case 5: symstr = "EURCHF";	spread_point = 3;  break;
			case 6: symstr = "USDCHF";	spread_point = 3;  break;
			case 7: symstr = "AUDUSD";	spread_point = 3;  break;
			case 8: symstr = "NZDUSD";	spread_point = 3;  break;
			case 9: symstr = "EURGBP";	spread_point = 3;  break;
			case 10: symstr = "AUDCAD";	spread_point = 10; break;
			case 11: symstr = "AUDJPY";	spread_point = 10; break;
			case 12: symstr = "CADJPY";	spread_point = 10; break;
			case 13: symstr = "CHFJPY";	spread_point = 10; break;
			case 14: symstr = "EURAUD";	spread_point = 7;  break;
			case 15: symstr = "GBPCHF";	spread_point = 7;  break;
			case 16: symstr = "GBPJPY";	spread_point = 7;  break;
			case 17: symstr = "AUDNZD";	spread_point = 12; break;
			case 18: symstr = "EURCAD";	spread_point = 12; break;
			default: Panic("Broken");
		};
		
		int sym = this->symbols.Find(symstr);
		sym_priority[prio] = sym;
		priority[sym] = prio++;
	}
	
	for(int i = 0; i < prio; i++) {
		const Symbol& symbol = mt.GetSymbol(sym_priority[i]);
		if (symbol.proxy_id != -1) {
			int k = GetSymbolPriority(symbol.proxy_id);
			ASSERT(k < prio);
			proxy_id[i] = k;
			proxy_base_mul[i] = symbol.base_mul;
		}
		else {
			proxy_id[i] = -1;
			proxy_base_mul[i] = 0;
		}
		
		spread_points[i] = spread_point * symbol.point;
	}
	
	
	InitRegistry();
	
	
	#ifdef flagGUITASK
	jobs_tc.Set(10, THISBACK(PostProcessJobs));
	#else
	jobs_running = true;
	jobs_stopped = false;
	Thread::Start(THISBACK(ProcessJobs));
	#endif
	
}

void System::AddPeriod(String nice_str, int period) {
	int count = periods.GetCount();
	
	// Currently first period must match base period
	if (count == 0 && base_period != period)
		throw DataExc();
	
	period /= base_period;
	
	period_strings.Add(nice_str);
	periods.Add(period);
	
	// TODO: some algorithm to calculate begins and ends, and persistently using it again
	#if ONLY_M1_SOURCE
	Time begin(2016,1,4);
	#else
	Time begin(2017,1,1);
	if (period == 1)			begin = Time(2016,9,5);
	else if (period == 5)		begin = Time(2016,9,5);
	else if (period == 15)		begin = Time(2016,9,5);
	else if (period == 30)		begin = Time(2015,11,2);
	else if (period == 60)		begin = Time(2015,5,4);
	else if (period == 240)		begin = Time(2009,12,21);
	else if (period == 480)		begin = Time(2009,12,21);
	else if (period == 720)		begin = Time(2009,12,21);
	else if (period == 1440)	begin = Time(2000,5,1);
	else if (period == 10080)	begin = Time(1996,6,24);
	else if (period == 43200)	begin = Time(1995,1,2);
	else Panic("Invalid period: " + IntStr(period));
	#endif
	
	this->begin.Add(begin);
	this->begin_ts.Add((int)(begin.Get() - Time(1970,1,1).Get()));
}

void System::AddSymbol(String sym) {
	ASSERT(symbols.Find(sym) == -1); // no duplicates
	symbols.Add(sym);
	priority.Add(100000);
}

Time System::GetTimeTf(int tf, int pos) const {
	int64 seconds = periods[tf] * pos * base_period;
	int64 weeks = seconds / (5*24*60*60);
	seconds += weeks * 2*24*60*60; // add weekends
	return begin[tf] + seconds;
}

int System::GetCountTf(int tf_id) const {
	int64 timediff = end.Get() - begin[tf_id].Get();
	int64 weeks = timediff / (7*24*60*60);
	timediff -= weeks * 2*24*60*60; // subtract weekends
	int div = base_period * periods[tf_id];
	int count = (int)(timediff / div);
	if (timediff % div != 0) count++;
	return count;
}

int System::GetShiftTf(int src_tf, int dst_tf, int shift) {
	if (src_tf == dst_tf) return shift;
	int64 src_period = periods[src_tf];
	int64 dst_period = periods[dst_tf];
	int64 timediff = shift * src_period * base_period;
	int64 weeks = timediff / (5*24*60*60); // shift has no weekends
	timediff += weeks * 2*24*60*60; // add weekends
	timediff -= begin_ts[dst_tf] - begin_ts[src_tf];
	weeks = timediff / (7*24*60*60);
	timediff -= weeks * 2*24*60*60; // subtract weekends
	int64 dst_shift = timediff / base_period / dst_period;
	
	#if 0
	// Sanity check
	timediff = GetTimeTf(src_tf, shift).Get() - GetTimeTf(dst_tf, dst_shift).Get();
	if (src_tf > dst_tf) {
		ASSERT(timediff == 0);
		Panic("TODO");
	} else {
		int64 maxdiff = dst_period * base_period;
		ASSERT(timediff > -maxdiff && timediff < maxdiff);
		Panic("TODO");
	}
	#endif
	
	return (int)dst_shift;
}

int System::GetShiftFromTimeTf(int timestamp, int tf) {
	int64 timediff = timestamp - begin_ts[tf];
	int64 weeks = timediff / (7*24*60*60);
	timediff -= weeks * 2*24*60*60; // subtract weekends
	return (int)(timediff / periods[tf] / base_period);
}

int System::GetShiftFromTimeTf(const Time& t, int tf) {
	int64 timediff = t.Get() - begin[tf].Get();
	int64 weeks = timediff / (7*24*60*60);
	timediff -= weeks * 2*24*60*60; // subtract weekends
	return (int)(timediff / periods[tf] / base_period);
}

void System::AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef) {
	CoreFactories().Add(CoreSystem(name, f, singlef));
}

void System::SetEnd(const Time& t) {
	end = t;
	int dow = DayOfWeek(end);
	if (dow >= 1 && dow <= 5) return;
	if (dow == 0) end -= 24 * 60 * 60;
	end.hour = 0;
	end.minute = 0;
	end.second = 0;
	end -= 60;
}

}
