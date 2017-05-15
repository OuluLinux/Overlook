#include "Overlook.h"

namespace Overlook {



void Factory::Init() {
	for (int i = 0; i < CtrlFactories().GetCount(); i++) {
		
		// Init Core component
		CtrlFactories()[i].b()->Init();
		
	}
}

void Factory::Deinit() {
	for (int i = 0; i < CtrlFactories().GetCount(); i++) {
		
		// Deinit Core component
		CtrlFactories()[i].b()->Deinit();
		
	}
}

void Factory::AddCustomCtrl(const String& name, CoreFactoryPtr f, CtrlFactoryPtr c) {
	CtrlFactories().Add(CoreCtrlFactory(name, f, c));
}

void Factory::AddCustomPipe(const String& name, PipeFactoryPtr f, CtrlFactoryPtr c) {
	PipeFactories().Add(PipeCtrlFactory(name, f, c));
}

}
