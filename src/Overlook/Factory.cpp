#include "Overlook.h"

namespace Overlook {





void Factory::AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c) {
	CtrlFactories().Add(CoreCtrlFactory(name, f, c));
}

}
