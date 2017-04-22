#include "DataCtrl.h"

namespace DataCtrl {


ForecasterCtrl::ForecasterCtrl() {
	Add(hsplit.SizePos());
	
	hsplit.Horz();
	hsplit << srcctrl << lctrl;
	
	srcctrl.Add(pctrl.HSizePos().VSizePos(0,30));
	srcctrl.Add(srclist.HSizePos().BottomPos(0,30));
	
}

void ForecasterCtrl::Refresher() {
	
}

void ForecasterCtrl::Reset() {
	
}

void ForecasterCtrl::SetArguments(const VectorMap<String, Value>& args) {
	MetaNode::SetArguments(args);
	
}

void ForecasterCtrl::Init() {
	
}

}
