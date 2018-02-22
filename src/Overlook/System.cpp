#include "Overlook.h"

namespace Overlook {

int SourceImage::HighestHigh(int period, int shift) {
	ASSERT(period > 0);
	double highest = -DBL_MAX;
	int highest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double high = High(shift);
		if (high > highest) {
			highest = high;
			highest_pos = shift;
		}
	}
	return highest_pos;
}

int SourceImage::LowestLow(int period, int shift) {
	ASSERT(period > 0);
	double lowest = DBL_MAX;
	int lowest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double low = Low(shift);
		if (low < lowest) {
			lowest = low;
			lowest_pos = shift;
		}
	}
	return lowest_pos;
}

int SourceImage::HighestOpen(int period, int shift) {
	ASSERT(period > 0);
	double highest = -DBL_MAX;
	int highest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double open = Open(shift);
		if (open > highest) {
			highest = open;
			highest_pos = shift;
		}
	}
	return highest_pos;
}

int SourceImage::LowestOpen(int period, int shift) {
	ASSERT(period > 0);
	double lowest = DBL_MAX;
	int lowest_pos = -1;
	for (int i = 0; i < period && shift >= 0; i++, shift--) {
		double open = Open(shift);
		if (open < lowest) {
			lowest = open;
			lowest_pos = shift;
		}
	}
	return lowest_pos;
}

double SourceImage::GetAppliedValue ( int applied_value, int i ) {
	double dValue;
	
	switch ( applied_value ) {
		case 0:
			dValue = Open(i);
			break;
		case 1:
			dValue = High(i);
			break;
		case 2:
			dValue = Low(i);
			break;
		case 3:
			dValue =
				( High(i) + Low(i) )
				/ 2.0;
			break;
		case 4:
			dValue =
				( High(i) + Low(i) + Open(i) )
				/ 3.0;
			break;
		case 5:
			dValue =
				( High(i) + Low(i) + 2 * Open(i) )
				/ 4.0;
			break;
		default:
			dValue = 0.0;
	}
	return dValue;
}
















System::System() {
	allowed_symbols.Add("AUDCAD");
	allowed_symbols.Add("AUDJPY");
	allowed_symbols.Add("AUDNZD");
	allowed_symbols.Add("AUDUSD");
	allowed_symbols.Add("CADJPY");
	allowed_symbols.Add("CHFJPY");
	allowed_symbols.Add("EURCAD");
	allowed_symbols.Add("EURCHF");
	allowed_symbols.Add("EURGBP");
	allowed_symbols.Add("EURJPY");
	allowed_symbols.Add("EURUSD");
	allowed_symbols.Add("EURAUD");
	allowed_symbols.Add("GBPCHF");
	allowed_symbols.Add("GBPUSD");
	allowed_symbols.Add("GBPJPY");
	allowed_symbols.Add("NZDUSD");
	allowed_symbols.Add("USDCAD");
	allowed_symbols.Add("USDCHF");
	allowed_symbols.Add("USDJPY");
	allowed_symbols.Add("USDMXN");
	allowed_symbols.Add("USDTRY");
}

System::~System() {
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
		
		data.SetCount(sym_count);
		for(int i = 0; i < data.GetCount(); i++) {
			data[i].SetCount(tf_count);
			for(int j = 0; j < data[i].GetCount(); j++) {
				DataBridge& db = data[i][j].db;
				
				db.sym_id = i;
				db.tf_id = j;
				db.period = mt.GetTimeframe(j);
				db.point = mt.GetSymbol(i).point;
			}
		}
	
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
	signals.Add(0);
}















ImageCompiler::ImageCompiler() {
	
}

void ImageCompiler::SetMain(const FactoryDeclaration& decl) {
	pipeline_size = 0;
	pipeline[pipeline_size++] = decl;
	
	int pipeline_cursor = 0;
	while (pipeline_cursor < pipeline_size) {
		FactoryDeclaration& decl = pipeline[pipeline_cursor++];
		
		ValueRegister reg;
		
		ConfFactory(decl, reg);
		
		decl.input_count = reg.input_count;
		for(int i = 0; i < reg.input_count; i++) {
			int id = pipeline_size++;
			decl.input_id[i] = id;
			pipeline[id] = reg.inputs[i];
		}
		decl.arg_count = reg.arg_count;
		for(int i = 0; i < 8; i++) decl.args[i] = reg.args[i].def;
			
		ASSERT(pipeline_size < MAX_PIPELINE);
	}
	
	
	// Remove duplicates and prefer later
	for(int i = 0; i < pipeline_size; i++) {
		FactoryDeclaration& a = pipeline[i];
		bool remove = false;
		int replace_id = -1;
		for(int j = i+1; j < pipeline_size; j++) {
			FactoryDeclaration& b = pipeline[j];
			if (a == b) {
				remove = true;
				replace_id = j;
				break;
			}
		}
		if (remove) {
			Remove(i, replace_id);
			i--;
		}
	}
}

void ImageCompiler::Remove(int i, int replace_id) {
	ASSERT(i >= 0 && i < pipeline_size);
	replace_id--;
	pipeline_size--;
	for(; i < pipeline_size - 1; i++) {
		FactoryDeclaration& decl = pipeline[i];
		decl = pipeline[i+1];
		
		for(int j = 0; j < decl.input_count; j++) {
			int& b = decl.input_id[j];
			int a = b;
			if (a == i)
				b = replace_id;
			else if (a > i)
				b--;
		}
	}
}

void ImageCompiler::Compile(SourceImage& si, ChartImage& ci) {
	int bars = ci.end - ci.begin;
	
	ci.graphs.SetCount(pipeline_size);
	
	for(int i = pipeline_size-1; i >= 0; i--) {
		ConstFactoryDeclaration& decl = pipeline[i];
		
		
		
		GraphImage& gi = ci.graphs[i];
		
		gi.reg.arg_count = 0;
		ConfFactory(decl, gi.reg);
		
		
		gi.factory = decl.factory;
		gi.buffers.SetCount(gi.reg.output_count);
		for(int j = 0; j < gi.buffers.GetCount(); j++) {
			BufferImage& ib = gi.buffers[j];
			ib.data_begin = ci.begin;
			ib.value.SetCount(bars);
		}
		gi.GetSignal().SetCount(bars);
		gi.GetEnabled().SetCount(bars);
		gi.input_count = decl.input_count;
		for(int j = 0; j < 8; j++)
			gi.input_id[j] = decl.input_id[j];
		
		InitFactory(decl, si, ci, gi);
		StartFactory(decl, si, ci, gi);
		gi.RefreshLimits();
	}
	
}

bool System::RefreshReal() {
	Time now				= GetUtcTime();
	int wday				= DayOfWeek(now);
	Time after_3hours		= now + 3 * 60 * 60;
	int wday_after_3hours	= DayOfWeek(after_3hours);
	now.second				= 0;
	MetaTrader& mt			= GetMetaTrader();
	
	
	// Skip weekends and first hours of monday
	if (wday == 0 || wday == 6 || (wday == 1 && now.hour < 1)) {
		LOG("Skipping weekend...");
		return true;
	}
	
	
	// Inspect for market closing (weekend and holidays)
	else if (wday == 5 && wday_after_3hours == 6) {
		WhenInfo("Closing all orders before market break");
		
		for (int i = 0; i < mt.GetSymbolCount(); i++) {
			mt.SetSignal(i, 0);
			mt.SetSignalFreeze(i, false);
		}
		
		mt.SignalOrders(true);
		return true;
	}
	
	
	WhenInfo("Updating MetaTrader");
	WhenPushTask("Putting latest signals");
	
	// Reset signals
	if (realtime_count == 0) {
		for (int i = 0; i < mt.GetSymbolCount(); i++)
			mt.SetSignal(i, 0);
	}
	realtime_count++;
	
	
	try {
		mt.Data();
		mt.RefreshLimits();
		int open_count = 0;
		const int MAX_SYMOPEN = 8;
		const double FMLEVEL = 0.6;
		
		for (int sym_id = 0; sym_id < GetSymbolCount(); sym_id++) {
			int sig = signals[sym_id];
			int prev_sig = mt.GetSignal(sym_id);
			if (sig != 0 && sig == prev_sig)
				open_count++;
		}
		
		for (int sym_id = 0; sym_id < GetSymbolCount(); sym_id++) {
			int sig = signals[sym_id];
			int prev_sig = mt.GetSignal(sym_id);
			if (sig == prev_sig && sig != 0)
				mt.SetSignalFreeze(sym_id, true);
			else {
				if ((!prev_sig && sig) || (prev_sig && sig != prev_sig)) {
					if (open_count >= MAX_SYMOPEN)
						sig = 0;
					else
						open_count++;
				}
				
				mt.SetSignal(sym_id, sig);
				mt.SetSignalFreeze(sym_id, false);
			}
			LOG("Real symbol " << sym_id << " signal " << sig);
		}
		
		mt.SetFreeMarginLevel(FMLEVEL);
		mt.SetFreeMarginScale(MAX_SYMOPEN);
		mt.SignalOrders(true);
	}
	catch (UserExc e) {
		LOG(e);
		return false;
	}
	catch (...) {
		return false;
	}
	
	
	WhenRealtimeUpdate();
	WhenPopTask();
	
	return true;
}

}
