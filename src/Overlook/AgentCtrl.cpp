#include "Overlook.h"

namespace Overlook {

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

void AgentCtrl::Arguments(ArgumentBase& args) {
	//MetaNode::SetArguments(args);
	
}

void AgentCtrl::Init() {
	
}

}
