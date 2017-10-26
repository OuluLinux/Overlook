#if 0

#include "Overlook.h"

namespace Overlook {

AgentGroup::AgentGroup() {
	
}

AgentGroup::~AgentGroup() {
	
}

void AgentGroup::Serialize(Stream& s) {
	s % agents;
}

}
#endif
