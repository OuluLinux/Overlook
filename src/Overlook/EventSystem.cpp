#include "Overlook.h"


namespace Overlook {

EventSystem::EventSystem() {
	
}
	
void EventSystem::Data() {
	System& sys = GetSystem();
	for(int i = 0; i < sys.CommonFactories().GetCount(); i++) {
		sys.CommonFactories()[i].b()->Start();
	}
}

}
