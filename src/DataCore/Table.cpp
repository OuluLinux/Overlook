#include "DataCore.h"

namespace DataCore {

Table::Table() {
	
}

void Table::Refresh() {
	lock.EnterWrite();
	
	/*if (src.Is()) {
		Container* cont = src.Get<Container>();
		if (cont)
			cont->Refresh();
	}*/
	
	Start();
	
	lock.LeaveWrite();
}

void Table::ClearContent() {
	
}

void Table::Set(int i, int j, const Value& val) {
	if (i < 0 || j < 0) return;
	ASSERT(i < 1000 && j < 50); // Sane limits
	if (values.GetCount() <= i) values.SetCount(i+1);
	Vector<Value>& row = values[i];
	if (row.GetCount() <= j) row.SetCount(j+1);
	row[j] = val;
}






}
