#include "Overlook.h"

namespace Overlook {







System::System() {
	SetEnd(GetUtcTime());
	
	addr = Config::arg_addr;
	port = Config::arg_port;
}

System::~System() {
	StopJobs();
	StopMain();
	data.Clear();
}

void System::Init() {
	LoadThis();
	
	
	MetaTrader& mt = GetMetaTrader();
	try {
		bool connected = mt.Init(addr, port);
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
		if (mt.GetSymbolCount() + GetCommonCount() != symbols.GetCount())
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
	
	
	StartMain();
}

void System::FirstStart() {
	MetaTrader& mt = GetMetaTrader();
	
	try {
		time_offset = mt.GetTimeOffset();
		
		
		// Add symbols
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			AddSymbol(s.name);
		}
		for(int i = 0; i < COMMON_COUNT; i++)
			AddSymbol("Common " + IntStr(i+1));
		
		
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
	for(int i = 0; i < mt.GetSymbolCount(); i++) spread_points[i] = mt.GetSymbol(i).point;
		
	
	const int priosym_count = GetCommonCount() * GetCommonSymbolCount();
	proxy_id.SetCount(priosym_count, 0);
	proxy_base_mul.SetCount(priosym_count, 0);
	
	
	int prio = 0;
	sym_priority.SetCount(priosym_count, 100000);
	for(int i = 0; i < priosym_count; i++) {
		String symstr;
		double spread_point = 0;
		switch (i) {
			
			// Generic Low/Medium spread broker
			case 0: symstr = "EURUSD";	spread_point = 0.0002; break;
			case 1: symstr = "EURJPY";	spread_point = 0.03;   break;
			case 2: symstr = "USDCHF";	spread_point = 0.0003; break;
			case 3: symstr = "USDJPY";	spread_point = 0.02;   break;
			
			case 4: symstr = "EURCHF";	spread_point = 0.0003; break;
			case 5: symstr = "EURGBP";	spread_point = 0.0003; break;
			case 6: symstr = "GBPUSD";	spread_point = 0.0003; break;
			case 7: symstr = "USDCAD";	spread_point = 0.0003; break;
			
			case 8: symstr = "$US100";	spread_point = 3.0; break;
			case 9: symstr = "$US30";	spread_point = 5.8; break;
			case 10: symstr = "$US500";	spread_point = 0.9; break;
			case 11: symstr = "$USDX";	spread_point = 0.7; break;
			
			case 12: symstr = "$UK100";	spread_point = 4.0; break;
			case 13: symstr = "$DE30";	spread_point = 5.0; break;
			case 14: symstr = "$EU50";	spread_point = 3.0; break;
			case 15: symstr = "$F40";	spread_point = 4.0; break;
			
			case 16: symstr = "#TSLA";	spread_point = 0.09; break;
			case 17: symstr = "#MTSC";	spread_point = 0.10; break;
			case 18: symstr = "#INTC";	spread_point = 0.03; break;
			case 19: symstr = "#GE";	spread_point = 0.03; break;
			
			case 20: symstr = "#AA";	spread_point = 0.03; break;
			case 21: symstr = "#VZ";	spread_point = 0.01; break;
			case 22: symstr = "#T";		spread_point = 0.03; break;
			case 23: symstr = "#PTR";	spread_point = 0.03; break;
			
			// MO, IP, HPQ, FB, CVX, CAT, C, BAC, BABA, BA..
			
			default: Panic("Broken");
		};
		
		// "Add commission" TODO: real commission... usually not specified, so it's difficult.
		if (i >= 8)
			spread_point *= 2;
		
		int sym = this->symbols.Find(symstr);
		sym_priority[prio] = sym;
		spread_points[sym] = spread_point;
		prio++;
	}
	
	common_symbol_id		.SetCount(symbols.GetCount(), -1);
	common_symbol_pos		.SetCount(symbols.GetCount(), -1);
	common_symbol_group_pos	.SetCount(symbols.GetCount());
	for(int i = 0; i < common_symbol_group_pos.GetCount(); i++)
		common_symbol_group_pos[i].SetCount(symbols.GetCount(), -1);
	int r = 0;
	for(int common_pos = 0; common_pos < GetCommonCount(); common_pos++) {
		int common_id = GetCommonSymbolId(common_pos);
		for(int j = 0; j < GetCommonSymbolCount(); j++) {
			int sym = sym_priority[r++];
			common_symbol_id[sym] = common_id;
			common_symbol_pos[sym] = common_pos;
			common_symbol_group_pos[common_id][sym] = common_pos;
		}
		common_symbol_id[common_id] = common_id;
		common_symbol_pos[common_id] = common_pos;
	}
	
	for(int i = 0; i < priosym_count; i++) {
		const Symbol& symbol = mt.GetSymbol(sym_priority[i]);
		if (symbol.proxy_id != -1) {
			proxy_id[i] = symbol.proxy_id;
			proxy_base_mul[i] = symbol.base_mul;
		}
		else {
			proxy_id[i] = -1;
			proxy_base_mul[i] = 0;
		}
	}
	
	
	if (pos_time.IsEmpty()) {
		int sym_count = symbols.GetCount();
		int tf_count = periods.GetCount();
		ASSERT(sym_count && tf_count);
		
		pos_time		.SetCount(sym_count);
		posconv_from	.SetCount(sym_count);
		posconv_to		.SetCount(sym_count);
		for(int i = 0; i < sym_count; i++) {
			pos_time[i]		.SetCount(tf_count);
			posconv_from[i]	.SetCount(tf_count);
			posconv_to[i]	.SetCount(tf_count);
		}
		
		main_time		.SetCount(tf_count);
		main_conv		.SetCount(tf_count);
		for(int i = 0; i < tf_count; i++) {
			main_conv[i].SetCount(tf_count);
		}
	}
}

void System::Deinit() {
	StopJobs();
	StopMain();
	StoreAll();
}

void System::StoreAll() {
	StoreThis();
	StoreCores();
}

void System::DataTimeBegin(int sym, int tf) {
	Vector<Time>& symtf_pos_time = pos_time[sym][tf];
	symtf_pos_time.Clear();
}

void System::DataTimeEnd(int sym, int tf) {
	auto& main_time = this->main_time[tf];
	
	
	// If main_time changed, refresh all sym time-vectors
	bool was_main_time_changed = main_time_changed;
	if (main_time_changed) {
		SortByKey(main_time, StdLess<Time>());
		
		RefreshTimeTfVectors(tf);
		
		for(int i = 0; i < symbols.GetCount(); i++)
			RefreshTimeSymVectors(i, tf);
		
		store_this = true;
		main_time_changed = false;
	}
	// Else refresh just this sym/tf
	else {
		
		RefreshTimeSymVectors(sym, tf);
		
	}
}

int System::DataTimeAdd(int sym, int tf, Time utc_time) {
	auto& main_time = this->main_time[tf];
	
	if (utc_time >= end) return -1;
	
	int i = main_time.Find(utc_time);
	if (i == -1) {
		//if (main_time.IsEmpty() || utc_time <= main_time.Top())
		main_time_changed = true;
		main_time.Add(utc_time);
	}
	
	Vector<Time>& symtf_pos_time = pos_time[sym][tf];
	if (symtf_pos_time.IsEmpty()) {
		symtf_pos_time.Add(utc_time);
		return 0;
	}
	else {
		const Time& latest = symtf_pos_time.Top();
		if (latest < utc_time) {
			symtf_pos_time.Add(utc_time);
			return symtf_pos_time.GetCount() - 1;
		}
		for(int i = symtf_pos_time.GetCount()-1; i >= 0; i--) {
			const Time& t = symtf_pos_time[i];
			if (t == utc_time)
				return i;
			if (t < utc_time)
				break;
		}
		
		return -1;
	}
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

void System::RefreshTimeTfVectors(int tf) {
	
	for(int i = 0; i < periods.GetCount(); i++) {
		if (tf == i) continue;
		RefreshTimeTfVector(i, tf);
	}
}

void System::RefreshTimeTfVector(int tf_from, int tf_to) {
	Vector<int>& vec_from = main_conv[tf_from][tf_to];
	Vector<int>& vec_to = main_conv[tf_to][tf_from];
	
	VectorMap<Time, byte>& time_from = main_time[tf_from];
	VectorMap<Time, byte>& time_to = main_time[tf_to];
	if (time_from.IsEmpty() || time_to.IsEmpty())
		return;
	
	vec_from.SetCount(time_from.GetCount(), -1);
	vec_to.SetCount(time_to.GetCount(), -1);
	
	int c0 = 0, c1 = 0;
	bool c0_written = false, c1_written = false;
	while (c0 < time_from.GetCount() && c1 < time_to.GetCount()) {
		if (!c0_written) {vec_from[c0] = c1; c0_written = true;}
		if (!c1_written) {vec_to[c1] = c0; c1_written = true;}
		
		if (c0 < time_from.GetCount()-1 && c1 < time_to.GetCount()-1) {
			const Time& next0 = time_from.GetKey(c0+1);
			const Time& next1 = time_to.GetKey(c1+1);
			if      (next0 < next1)	{c0++; c0_written = false;}
			else if (next0 > next1)	{c1++; c1_written = false;}
			else {c0++; c1++; c0_written = false; c1_written = false;}
		}
		else if (c0 < time_from.GetCount()-1) {
			c0++; c0_written = false;
		}
		else if (c1 < time_to.GetCount()-1) {
			c1++; c1_written = false;
		}
		else {
			break;
		}
	}
}

void System::RefreshTimeSymVectors(int sym, int tf) {
	const VectorMap<Time, byte>&	main_time		= this->main_time[tf];
	const Vector<Time>&				pos_time		= this->pos_time[sym][tf];
	Vector<int>&					posconv_from	= this->posconv_from[sym][tf];
	Vector<int>&					posconv_to		= this->posconv_to[sym][tf];
	
	posconv_from.SetCount(pos_time.GetCount(), -1);
	posconv_to.SetCount(main_time.GetCount(), -1);
	
	int c0 = 0, c1 = 0;
	bool c0_written = false, c1_written = false;
	while (c0 < main_time.GetCount() && c1 < pos_time.GetCount()) {
		int& to = posconv_to[c0];
		int& from = posconv_from[c1];
		
		if (!c0_written) {to = c1; c0_written = true;}
		if (!c1_written) {from = c0; c1_written = true;}
		
		if (c0 < main_time.GetCount()-1 && c1 < pos_time.GetCount()-1) {
			const Time& next0 = main_time.GetKey(c0+1);
			const Time& next1 = pos_time[c1+1];
			if      (next0 < next1)	{c0++; c0_written = false;}
			else if (next0 > next1)	{c1++; c1_written = false;}
			else {c0++; c1++; c0_written = false; c1_written = false;}
		}
		else if (c0 < main_time.GetCount()-1) {
			c0++; c0_written = false;
		}
		else if (c1 < pos_time.GetCount()-1) {
			c1++; c1_written = false;
		}
		else {
			break;
		}
	}
}

Time System::GetTimeTf(int sym, int tf, int pos) const {
	return pos_time[sym][tf][pos];
}

int System::GetCountTf(int sym, int tf) const {
	return pos_time[sym][tf].GetCount();
}

int System::GetShiftTf(int src_sym, int src_tf, int dst_sym, int dst_tf, int src_shift) {
	int src_mainpos = posconv_from[src_sym][src_tf][src_shift];
	if (src_tf != dst_tf) {
		src_mainpos = main_conv[src_tf][dst_tf][src_mainpos];
	}
	int dst_shift = posconv_to[dst_sym][dst_tf][src_mainpos];
	ASSERT(dst_shift != -1); // warn
	return dst_shift;
}

int System::GetShiftMainTf(int src_tf, int dst_sym, int dst_tf, int src_shift) {
	int src_mainpos = src_shift;
	if (src_tf != dst_tf) {
		src_mainpos = main_conv[src_tf][dst_tf][src_mainpos];
	}
	int dst_shift = posconv_to[dst_sym][dst_tf][src_mainpos];
	ASSERT(dst_shift != -1); // warn
	return dst_shift;
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
