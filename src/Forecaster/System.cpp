#include "Forecaster.h"

namespace Forecast {

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

void System::GetBitStream(Vector<CoreItem>& work_queue, BitStream& stream) {
	
	Vector<ConstLabelSignal*> lbls;
	GetLabels(work_queue, lbls);
	
	stream.Clear();
	stream.SetColumnCount(lbls.GetCount());
	stream.SetCount(Generator::data_count);
	for(int i = 0; i < Generator::data_count; i++) {
		
		for(int j = 0; j < lbls.GetCount(); j++) {
			bool value = lbls[j]->signal.Get(i);
			stream.Write(value);
		}
		
	}
}

void System::GetLabels(Vector<CoreItem>& work_queue, Vector<ConstLabelSignal*>& lbls) {
	for(int i = 0; i < work_queue.GetCount(); i++) {
		CoreItem& ci = work_queue[i];
		
		OutputCounter lc;
		ci.core->IO(lc);
		
		for(int j = 0; j < lc.lbl_counts.GetCount(); j++) {
			int count = lc.lbl_counts[j];
			for(int k = 0; k < count; k++) {
				ConstLabelSignal& buf = ci.core->GetLabelBuffer(j, k);
				lbls.Add(&buf);
			}
		}
	}
}

}
