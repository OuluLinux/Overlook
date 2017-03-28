#include "DataCtrl.h"

namespace DataCtrl {

EventCtrl::EventCtrl() {
	Add(ctrl.SizePos());
	ctrl.AddColumn("Id");
	ctrl.AddColumn("Time");
	ctrl.AddColumn("Node");
	ctrl.AddColumn("Impact");
	ctrl.AddColumn("Description");
	ctrl.AddColumn("Actual");
	ctrl.AddColumn("Forecast");
	ctrl.AddColumn("Previous");
}

void EventCtrl::SetEventManager(MetaVar src) {
	data << src;
	RefreshData();
}

void EventCtrl::RefreshData() {
	
	EventManager* em = data.Get<EventManager>();
	if (!em) return;
	
	
	// List all added events
	int scroll = ctrl.GetScroll();
	int cursor = ctrl.GetCursor();
	
	
	// Set data
	for(int i = 0; i < em->GetCount(); i++) {
		DataCore::Event& e = em->GetEvent(i);
		ctrl.Set(i, 0, e.id);
		ctrl.Set(i, 1, TimeFromTimestamp(e.timestamp));
		ctrl.Set(i, 2, e.currency);
		ctrl.Set(i, 3, (int)e.impact);
		ctrl.Set(i, 4, e.title);
		ctrl.Set(i, 5, e.actual);
		ctrl.Set(i, 6, e.forecast);
		ctrl.Set(i, 7, e.previous);
	}
	
	ctrl.SetSortColumn(1, true);
	
	// Restore view
	ctrl.ScrollTo(scroll);
	if (cursor >= 0 && cursor < ctrl.GetCount())
		ctrl.SetCursor(cursor);
	
}

}
