#include "DataCtrl.h"

namespace DataCtrl {

ForecasterDraw::ForecasterDraw() {
	
}

void ForecasterDraw::Paint(Draw& w) {
	if (!IsVisible()) return;
	
}



ForecasterCtrl::ForecasterCtrl() {
	CtrlLayout(*this);
	
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
