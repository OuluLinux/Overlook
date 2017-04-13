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
		DataCore::TimeVector& tv = DataCore::GetTimeVector();
		
		
		// Find slot from argument
		String path = args[i];
		DataCore::PathLinkPtr link = tv.FindLinkPath(path);
		ASSERTEXC(link);
		ASSERTEXC(link->link);
		slot = link->link;
		
		// Create default settings for values in the slot
		int buffers = slot->GetCount();
		SetBufferCount(buffers);
		for(int i = 0; i < buffers; i++) {
			DataBufferSettings& set = buffer_settings[i];
			set.clr = Blue();
			set.line_width = 1;
			set.line_style = STYLE_SOLID;
		}
		
		
		// Load style from JSON embedded to the slot
		String style_json = slot->GetStyle();
		Value js = ParseJSON(style_json);
		if (js.IsNull()) {
			LOG("JSON parse failed");
		} else {
			ValueMap map = js;
			for(int i = 0; i < map.GetCount(); i++) {
				String key = map.GetKey(i);
				if (key == "window_type") {
					String s = map.GetValue(i);
					if (s == "SEPARATE")	window_type = WINDOW_SEPARATE;
					else if (s == "CHART")	window_type = WINDOW_CHART;
					else {LOG("ERROR: Invalid window_type argument " << s);}
				}
				else if (key == "minimum")	{minimum = map.GetValue(i);	has_minimum = true;}
				else if (key == "maximum")	{maximum = map.GetValue(i);	has_maximum = true;}
				else if (key == "point")	point = map.GetValue(i);
			}
			
			
			// Find arguments for values
			for(int i = 0; i < buffers; i++) {
				int j = map.Find("value" + IntStr(i));
				if (j == -1) continue;
				ValueMap vmap = map.GetValue(j);
				for(int j = 0; j < vmap.GetCount(); j++) {
					DataBufferSettings& set = buffer_settings[i];
					String key = vmap.GetKey(j);
					if (key == "label")				set.label = vmap.GetValue(j);
					else if (key == "color") {
						Vector<String> v = Split((String)vmap.GetValue(j), ",");
						if (v.GetCount() == 3)
							set.clr = Color(StrInt(v[0]), StrInt(v[1]), StrInt(v[2]));
						else {
							LOG("ERROR: Invalid color argument " << (String)vmap.GetValue(j));
						}
					}
					else if (key == "style")	{
						String style = vmap.GetValue(j);
						if      (style == "HISTOGRAM")	set.style = DRAW_HISTOGRAM;
						else if (style == "LINE")		set.style = DRAW_LINE;
						else if (style == "ARROW")		set.style = DRAW_ARROW;
						else if (style == "NONE")		set.style = DRAW_NONE;
						else if (style == "SECTION")	set.style = DRAW_SECTION;
						else if (style == "ZIGZAG")		set.style = DRAW_ZIGZAG;
						else {LOG("ERROR: invalid value style value " << style);}
					}
					else if (key == "line_style")	{
						String style = vmap.GetValue(j);
						if      (style == "DASH")		set.line_style = STYLE_DASH;
						else if (style == "DASHDOT")	set.line_style = STYLE_DASHDOT;
						else if (style == "DASHDOTDOT")	set.line_style = STYLE_DASHDOTDOT;
						else if (style == "DOT")		set.line_style = STYLE_DOT;
						else if (style == "SOLID")		set.line_style = STYLE_SOLID;
						else {LOG("ERROR: invalid value style value " << style);}
					}
					else if (key == "line_width")	set.line_width	= vmap.GetValue(j);
					else if (key == "chr")			set.chr			= vmap.GetValue(j);
					else if (key == "begin")		set.begin		= vmap.GetValue(j);
					else if (key == "shift")		set.shift		= vmap.GetValue(j);
					else {LOG("ERROR: invalid value key " << key);}
				}
			}
			
			
			// Find arguments for levels
			levels.SetCount(0);
			for(int i = 0;; i++) {
				int j = map.Find("level" + IntStr(i));
				if (j == -1) break;
				DataLevel& lev = levels.Add();
				ValueMap vmap = map.GetValue(j);
				for(int j = 0; j < vmap.GetCount(); j++) {
					String key = vmap.GetKey(j);
					if (key == "style")	{
						String style = vmap.GetValue(j);
						if      (style == "DASH")		lev.style = STYLE_DASH;
						else if (style == "DASHDOT")	lev.style = STYLE_DASHDOT;
						else if (style == "DASHDOTDOT")	lev.style = STYLE_DASHDOTDOT;
						else if (style == "DOT")		lev.style = STYLE_DOT;
						else if (style == "SOLID")		lev.style = STYLE_SOLID;
						else {LOG("ERROR: invalid level style value " << style);}
					}
					else if (key == "line_width")		lev.line_width = vmap.GetValue(j);
					else if (key == "value")			lev.value = vmap.GetValue(j);
					else if (key == "color") {
						Vector<String> v = Split((String)vmap.GetValue(j), ",");
						if (v.GetCount() == 3)
							lev.clr = Color(StrInt(v[0]), StrInt(v[1]), StrInt(v[2]));
						else {LOG("ERROR: Invalid color argument " << (String)vmap.GetValue(j));}
					}
					else {LOG("ERROR: invalid level key " << key);}
				}
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
	ASSERT_(period_id != -1, "Container has no timeframe set");
	
	int id = GetId();
	DataCore::TimeVector& tv = DataCore::GetTimeVector();
	
	DataCore::Slot& slot = *this->slot;
	const DataCore::TimeVector::SlotData& slots = tv.GetSlot(id, period_id, shift);
	
	ASSERTEXC_(locked, "Cache should be locked before call because of performance reasons");
	if (slots.IsEmpty())
		tv.LoadCache(id, period_id, shift, locked);
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
