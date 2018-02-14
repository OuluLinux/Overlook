#include "Overlook.h"

namespace Overlook {







System::System() {
	allowed_symbols.Add("AUDCAD");
	allowed_symbols.Add("EURCAD");
	allowed_symbols.Add("GBPUSD");
	allowed_symbols.Add("AUDJPY");
	allowed_symbols.Add("EURCHF");
	allowed_symbols.Add("NZDUSD");
	allowed_symbols.Add("AUDNZD");
	allowed_symbols.Add("EURGBP");
	allowed_symbols.Add("USDCAD");
	allowed_symbols.Add("AUDUSD");
	allowed_symbols.Add("EURJPY");
	allowed_symbols.Add("USDCHF");
	allowed_symbols.Add("CADJPY");
	allowed_symbols.Add("EURUSD");
	allowed_symbols.Add("USDJPY");
	allowed_symbols.Add("CHFJPY");
	allowed_symbols.Add("GBPCHF");
	allowed_symbols.Add("USDMXN");
	allowed_symbols.Add("EURAUD");
	allowed_symbols.Add("GBPJPY");
	allowed_symbols.Add("USDTRY");
}

System::~System() {
	StopJobs();
	data.Clear();
}

void System::Init() {
	
	MetaTrader& mt = GetMetaTrader();
	try {
		bool connected = mt.Init(Config::arg_addr, Config::arg_port);
		ASSERTUSER_(!connected, "Can't connect to MT4. Is MT4Connection script activated in MT4?");
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
	
	
	if (symbols.IsEmpty())
		FirstStart();
	else {
		if (mt.GetSymbolCount() != symbols.GetCount())
			throw UserExc("MT4 symbols changed. Remove cached data.");
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			if (s.name != symbols[i])
				throw UserExc("MT4 symbols changed. Remove cached data.");
		}
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

void System::FirstStart() {
	MetaTrader& mt = GetMetaTrader();
	
	try {
		time_offset = mt.GetTimeOffset();
		
		
		// Add symbols
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			AddSymbol(s.name);
			ASSERTUSER_(allowed_symbols.Find(s.name) != -1, "Symbol " + s.name + " does not have long M1 data. Please hide all short data symbols in MT4. Read Readme.txt for usable symbols.");
		}
		
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		for(int i = 0; i < mt.GetTimeframeCount(); i++)
			AddPeriod(mt.GetTimeframeString(i), mt.GetTimeframe(i));
		
		
		int sym_count = symbols.GetCount();
		int tf_count = periods.GetCount();
	
		if (sym_count == 0) throw DataExc();
		if (tf_count == 0)  throw DataExc();
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
	
	spread_points.SetCount(symbols.GetCount(), 0);
		
	for(int i = 0; i < symbols.GetCount(); i++) {
		spread_points[i] = mt.GetSymbol(i).point * 4;
	}
	
}

void System::Deinit() {
	StopJobs();
}

void System::AddPeriod(String nice_str, int period) {
	int count = periods.GetCount();
	
	if (count == 0 && period != 1)
		throw DataExc();
	
	period_strings.Add(nice_str);
	periods.Add(period);
}

void System::AddSymbol(String sym) {
	ASSERT(symbols.Find(sym) == -1); // no duplicates
	symbols.Add(sym);
}

void System::AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef) {
	CoreFactories().Add(CoreSystem(name, f, singlef));
}

}
