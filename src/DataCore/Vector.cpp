#include "DataCore.h"

namespace DataCore {

TimeVector::TimeVector() {
	timediff = 0;
	base_period = 60;
	enable_cache = false;
	memory_limit = 0;
	current_slotattr = NULL;
}

TimeVector::~TimeVector() {
	for(int i = 0; i < slot.GetCount(); i++) {
		SlotPtr sp = slot[i];
		if (sp) {
			delete sp;
		}
	}
}

void TimeVector::Serialize(Stream& s) {
	s % begin % end % memory_limit % timediff % base_period % begin_ts % end_ts % enable_cache;
}

void TimeVector::CheckMemoryLimit() {
	TimeVector& tv = GetTimeVector();
	int64 reserved_memory = 0;
	
	for(int i = 0; i < tv.GetCustomSlotCount(); i++) {
		Slot& slot = tv.GetCustomSlot(i);
		reserved_memory += slot.GetReserved();
	}
	
	if (reserved_memory >= memory_limit) {
		int64 target = memory_limit * 2 / 3;
		String time_str = Format("%", GetSysTime());
		LOG("TimeVector::CheckMemoryLimit: " << time_str << " relasing memory from " << reserved_memory << " to " << target);
		TimeStop ts;
		for(int i = 0; i < slot.GetCount(); i++) {
			Slot& slot = *this->slot[i];
			slot.Free(target, reserved_memory);
			if (reserved_memory <= target) break;
		}
		LOG("TimeVector::CheckMemoryLimit: relased memory to " << reserved_memory << " in " << ts.ToString());
	}
}

void TimeVector::RefreshData() {
	int64 sym_count = symbols.GetCount();
	int64 tf_count = periods.GetCount();

	if (sym_count == 0) throw DataExc();
	if (tf_count == 0)  throw DataExc();
	if (slot.IsEmpty()) throw DataExc();
	
	bars.SetCount(tf_count);
	for(int i = 0; i < bars.GetCount(); i++) {
		int period = periods[i];
		int count = GetCount(period);
		if (!count) throw DataExc();
		bars[i] = count;
	}
}

void TimeVector::AddPeriod(int period) {
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
	
	periods.Add(period);
}

void TimeVector::AddSymbol(String sym) {
	ASSERT(symbols.Find(sym) == -1); // no duplicates
	symbols.Add(sym);
}

int TimeVector::GetCount(int period) const {
	int div = base_period * period;
	int count = timediff / div;
	if (count % div != 0) count++;
	return count;
}

int64 TimeVector::GetShift(int src_period, int dst_period, int shift) {
	int64 timediff = shift * src_period; //		* base_period
	return (int)(timediff / dst_period); //			/ base_period
}

int64 TimeVector::GetShiftFromTime(int timestamp, int period) {
	int64 timediff = timestamp - begin_ts;
	return (int)(timediff / period / base_period);
}

int TimeVector::GetTfFromSeconds(int period_seconds) {
	return period_seconds / base_period;
}

void TimeVector::AddCustomSlot(String key, SlotFactory f) {
	GetFactories().Add(key, f);
}

}
