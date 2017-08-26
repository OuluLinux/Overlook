#if 0

#include "Overlook.h"

namespace Overlook {

SymGroup::SymGroup() {
	group = NULL;
	sys = NULL;
	
}

void SymGroup::SetEpsilon(double d) {
	for(int i = 0; i < agents.GetCount(); i++)
		agents[i].SetEpsilon(d);
}


}
#endif
