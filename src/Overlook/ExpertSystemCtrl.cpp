#include "Overlook.h"

namespace Overlook {
using namespace Upp;


ExchangeSlotsCtrl::ExchangeSlotsCtrl() {
	Add(find_current.TopPos(0,30).LeftPos(0,100));
	Add(splitter.HSizePos(100).VSizePos());
	splitter << slotlist << openlist;
	splitter.Horz();
	
	find_current.SetLabel("Find current");
	find_current << THISBACK(FindCurrent);
	
	slotlist.AddColumn("Id");
	slotlist << THISBACK(Data);
	
	openlist.AddColumn("Open time");
	openlist.AddColumn("Close time");
	
}

void ExchangeSlotsCtrl::Data() {
	ExchangeSlots& es = GetExchangeSlots();
	
	es.Refresh();
	
	for(int i = 0; i < es.slots.GetCount(); i++) {
		Slot& slot = es.slots[i];
		
		slotlist.Set(i, 0, slot.id);
	}
	
	int cursor = slotlist.GetCursor();
	if (cursor >= 0 && cursor < es.slots.GetCount()) {
		Slot& slot = es.slots[cursor];
		
		for(int j = 0; j < slot.open_time.GetCount(); j++) {
			openlist.Set(j, 0, slot.open_time[j]);
			openlist.Set(j, 1, slot.close_time[j]);
		}
		openlist.SetCount(slot.open_time.GetCount());
	}
}

void ExchangeSlotsCtrl::FindCurrent() {
	ExchangeSlots& es = GetExchangeSlots();
	Time now = GetUtcTime();

	Time max_time(1970,1,1);
	int max_i = 0;

	for(int i = 0; i < es.slots.GetCount(); i++) {
		Slot& slot = es.slots[i];

		for(int j = slot.open_time.GetCount() - 1; j >= 0; j--) {
			if (slot.open_time[j] > now)
				continue;

			if (slot.open_time[j] > max_time) {
				max_time = slot.open_time[j];
				max_i = i;
			}
			break;
		}
	}

	slotlist.SetCursor(max_i);
	openlist.SetCursor(openlist.GetCount() - 1);
}

}
