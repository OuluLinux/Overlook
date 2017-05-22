#include "Overlook.h"


namespace Overlook {
using namespace Upp;

BaseSystem::BaseSystem() {
	SetSkipAllocate();
	timediff = 0;
	base_period = 60;
	
	
	
	addr = "192.168.0.106";
	port = 42000;
}

void BaseSystem::Init() {
	// Don't reserve memory for vector contents in this class
	SetSkipSetCount();
	
	MetaTrader& mt = GetMetaTrader();
	
	const Vector<Symbol>& symbols = GetMetaTrader().GetCacheSymbols();
	
	try {
		
		// Init sym/tfs/time space
		ASSERTEXC(!mt.Init(addr, port));
		
		// Add symbols
		for(int i = 0; i < mt.GetSymbolCount(); i++) {
			const Symbol& s = mt.GetSymbol(i);
			AddSymbol(s.name);
		}
		for(int i = 0; i < mt.GetCurrencyCount(); i++) {
			const Currency& c = mt.GetCurrency(i);
			AddSymbol(c.name);
		}
		
		
		// TODO: store symbols to session file and check that mt supports them
		
		
		// Add periods
		ASSERT(mt.GetTimeframe(0) == 1);
		int base = 1; // mins
		SetBasePeriod(60*base);
		Vector<int> tfs;
		for(int i = 0; i < mt.GetTimeframeCount(); i++) {
			int tf = mt.GetTimeframe(i);
			if (tf >= base) {
				tfs.Add(tf / base);
				AddPeriod(mt.GetTimeframeString(i), tf * 60);
			}
		}
		
		
		// Init time range
		Time begin, end;
		begin = Time(2016, 12, 1);
		Date now = GetSysTime();
		do {++now;}
		while (now.day != 15);
		end = Time(now.year, now.month, now.day);
		SetBegin(begin);
		SetEnd(end);
		
		
		int64 sym_count = symbols.GetCount();
		int64 tf_count = periods.GetCount();
	
		if (sym_count == 0) throw DataExc();
		if (tf_count == 0)  throw DataExc();
		
		bars.SetCount(tf_count);
		for(int i = 0; i < bars.GetCount(); i++) {
			int period = periods[i];
			int count = GetCount(period);
			if (!count) throw DataExc();
			bars[i] = count;
		}
		
		//StoreThis();
		
	}
	
	catch (...) {
		LOG("Load failed");
	}
	
}

void BaseSystem::AddPeriod(String nice_str, int period) {
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
}

void BaseSystem::AddSymbol(String sym) {
	ASSERT(symbols.Find(sym) == -1); // no duplicates
	symbols.Add(sym);
}

int BaseSystem::GetCount(int period) const {
	int div = base_period * period;
	int count = timediff / div;
	if (count % div != 0) count++;
	return count;
}

int BaseSystem::GetCountTf(int tf_id) const {
	return GetCount(periods[tf_id]);
}

int64 BaseSystem::GetShift(int src_period, int dst_period, int shift) {
	int64 timediff = shift * src_period; //		* base_period
	return (int)(timediff / dst_period); //			/ base_period
}

int64 BaseSystem::GetShiftFromTime(int timestamp, int period) {
	int64 timediff = timestamp - begin_ts;
	return (int)(timediff / period / base_period);
}

int BaseSystem::GetTfFromSeconds(int period_seconds) {
	return period_seconds / base_period;
}





BaseSystemCtrl::BaseSystemCtrl() {
	
}

}

