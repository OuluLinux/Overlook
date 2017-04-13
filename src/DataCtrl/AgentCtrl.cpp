#include "DataCtrl.h"

namespace DataCtrl {

AgentDraw::AgentDraw() {
	
}

void AgentDraw::Paint(Draw& w) {
	if (!IsVisible()) return;
	
}



AgentCtrl::AgentCtrl() {
	CtrlLayout(*this);
	
}

void AgentCtrl::Refresher() {
	
}

void AgentCtrl::Reset() {
	
}

void AgentCtrl::SetArguments(const VectorMap<String, Value>& args) {
	MetaNode::SetArguments(args);
	
}

void AgentCtrl::Init() {
	
}

}
