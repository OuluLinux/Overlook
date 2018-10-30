#include "Agent.h"

namespace Agent {

void System::AddCustomCore(const String& name, CoreFactoryPtr f, CoreFactoryPtr singlef) {
	CoreFactories().Add(CoreSystem(name, f, singlef));
}

void System::GetCoreQueue(const Vector<double>& real_data, Vector<CoreItem>& queue, const Vector<FactoryDeclaration>& decl) {
	ASSERT(real_data);
	
	queue.SetCount(0);
	queue.SetCount(decl.GetCount());
	
	for(int i = 0; i < decl.GetCount(); i++) {
		const FactoryDeclaration& d = decl[i];
		CoreItem& ci = queue[i];
		
		ci.core = System::GetCoreFactories()[d.factory].b();
		
		ci.core->input = &real_data;
		
		if (d.arg_count > 0) {
			ArgChanger ch;
			ch.SetLoading();
			ci.core->IO(ch);
			for(int j = 0; j < d.arg_count; j++) {
				ch.args[j] = d.args[j];
			}
			ch.SetStoring();
			ci.core->IO(ch);
		}
		
		ci.core->SetupBuffers();
		ci.core->Init();
	}
}

}
