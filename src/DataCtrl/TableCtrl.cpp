#include "DataCtrl.h"

namespace DataCtrl {

TableCtrl::TableCtrl() {
	Add(ctrl.SizePos());
	
}

void TableCtrl::SetTable(MetaVar src) {
	data << src;
	RefreshData();
}

void TableCtrl::RefreshData() {
	Table* table = data.Get<Table>();
	if (!table) return;
	
	
	// Refresh Table
	table->Refresh();
	
	// Store view and reset TableCtrl
	int scroll = ctrl.GetScroll();
	int cursor = ctrl.GetCursor();
	ctrl.Reset();
	
	
	// Add columns and data
	int cols = table->GetColumnCount();
	for(int i = 0; i < cols; i++) {
		ctrl.AddColumn(table->GetColumn(i));
	}
	int count = table->GetCount();
	for(int i = 0; i < count; i++) {
		for(int j = 0; j < cols; j++) {
			ctrl.Set(i, j, table->Get(i, j));
		}
	}
	
	
	// Restore view
	ctrl.ScrollTo(scroll);
	if (cursor >= 0 && cursor < ctrl.GetCount())
		ctrl.SetCursor(cursor);
	
}

}
