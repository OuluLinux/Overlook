#include "DataCtrl.h"

namespace DataCtrl {
using namespace DataCore;

Container::Container() {
	window_type = WINDOW_CHART;
	bars = 0;
	next_count = 0;
	counted = 0;
	has_maximum = 0;
	has_minimum = 0;
	maximum = 0;
	minimum = 0;
	skip_setcount = false;
	point = 0.01;
	period_id = -1;
}

void Container::SetArguments(const VectorMap<String, Value>& args) {
	BarData* bd = GetSource().Get<BarData>();
	if (bd) {
		SetId(bd->GetId());
		SetPeriod(bd->GetPeriod());
	}
	else {
		int i;
		
		i = args.Find("period");
		ASSERTREF_(i != -1, "Container requires period as argument.");
		int period = args[i];
		SetPeriod(period);
		
		i = args.Find("id");
		ASSERTREF_(i != -1, "Container requires id as argument.");
		int id = args[i];
		SetId(id);
	}
	
	period_id = DataCore::GetTimeVector().FindPeriod(GetPeriod());
	
	
	int i;
	i = args.Find("slot");
	if (i != -1) {
	
		String path = args[i];
		
		DataCore::TimeVector& tv = DataCore::GetTimeVector();
		
		DataCore::PathLinkPtr link = tv.FindLinkPath(path);
		ASSERTEXC(link);
		ASSERTEXC(link->link);
		slot = link->link;
		
		int buffers = slot->GetCount();
		 
		SetBufferCount(buffers);
		for(int i = 0; i < buffers; i++) {
			DataBufferSettings& set = buffer_settings[i];
			set.clr = Blue();
			set.line_width = 1;
			set.line_style = STYLE_SOLID;
			
			if (!i) {
				if ((*slot)[i].type == DataCore::Slot::TYPE_SEPARATEDOUBLE)
					SetContainerSeparateWindow();
			}
		}
		
	}
	else ASSERTEXC(slot);
	
}

void Container::Init() {
	LOG("Container::Init");
	
}

void Container::Start() {
	LOG("Container::Start");
	
}

double Container::GetBufferValue(int i, int shift) {
	int id = GetId();
	DataCore::TimeVector& tv = DataCore::GetTimeVector();
	
	DataCore::Slot& slot = *this->slot;
	const DataCore::TimeVector::SlotData& slots = tv.GetSlot(id, period_id, shift);
	
	if (slots.IsEmpty()) tv.LoadCache(id, period_id, shift);
	ASSERTEXC(!slots.IsEmpty());
	
	const byte* cur = slots.Begin() + slot.GetOffset() + slot[i].offset;
	
	double value = *((double*)cur);
	//DLOG(Format("%d %d %d: %f", id, period_id, shift, value));
	return value;
}

double Container::GetBufferValue(int shift) {
	return GetBufferValue(0, shift);
}

int Container::GetBufferDataCount() {
	DataCore::TimeVector& tv = DataCore::GetTimeVector();
	int count = tv.GetCount(GetPeriod());
	return count;
}

void Container::Refresh() {
	LOG("Container::Refresh");
	
	
}































BarData::BarData() {
	
}

void BarData::SetArguments(const VectorMap<String, Value>& args) {
	
	int i;
	
	i = args.Find("bar");
	ASSERTREF_(i != -1, "BarData::SetArguments no bar-data sources defined");
	String path = args[i];
	
	
	DataCore::TimeVector& tv = DataCore::GetTimeVector();
	
	DataCore::PathLinkPtr link = tv.FindLinkPath(path);
	ASSERTEXC(link);
	ASSERTEXC(link->link);
	slot = link->link;
	
	int buffers = slot->GetCount();
	
	SetBufferCount(buffers);
	for(int i = 0; i < buffers; i++) {
		DataBufferSettings& set = buffer_settings[i];
		set.clr = Blue();
		set.line_width = 1;
		set.line_style = STYLE_SOLID;
	}
	
	
	Container::SetArguments(args);
}

void BarData::Init() {
	
}

void BarData::Start() {
	
}

}
